#!/usr/bin/env python

import os
import sys
import time
import glob
import serial
import smtplib
import subprocess
from datetime import date
from email import encoders
from os.path import basename
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from email.mime.application import MIMEApplication

RESULTS_PREFIX = 'results/'

ANT_SELECT_SERIAL_DEV       = '/dev/ttyUSB1'
ANT_SELECT_SERIAL_BAUD      = 9600
ANT_SELECT_SERIAL_PARITY    = serial.PARITY_NONE
ANT_SELECT_SERIAL_STOPBITS  = serial.STOPBITS_ONE
ANT_SELECT_SERIAL_BYTESIZE  = serial.EIGHTBITS

EMAIL_NOTIFICATION_RECEPIENT    = 'stanislav.barantsev@community.isunet.edu'
EMAIL_NOTIFICATION_CONFIG_FILE  = 'email.cfg'

def send_email_notification(auth_file, attachments=[]):

    client_login = ''
    client_password = ''

    try:
        with open(auth_file, 'r') as reader:
            config = reader.read().splitlines()
    except:
        print('[email] couldn\'t open the file')
        return False

    client_login = config[0]
    client_password = config[1]

    if client_login == '' or client_password == '': 
        print('[email] no client login or password specified')
        return False

    subject = "Test email from the GSC"
    recepient = "stanislav.barantsev@community.isunet.edu"

    msg = MIMEMultipart()
    msg["To"] = recepient
    msg["From"] = client_login
    msg["Subject"] = subject

    body = "Hi there, please find the results for another satellite!"
    msg.attach(MIMEText(body, "plain"))

    for f in attachments or []:
        with open(f, "rb") as fil:
            part = MIMEApplication(
                fil.read(),
                Name=basename(f)
            )
        # After the file is closed
        part['Content-Disposition'] = 'attachment; filename="%s"' % basename(f)
        msg.attach(part)

    text = msg.as_string()
    server = smtplib.SMTP("smtp.gmail.com", 587)

    try:
        server.starttls()
        ret = server.login(client_login, client_password)
    except:
        print('[email] wrong credentials, couldn\'t authenticate')
        return False

    try:
        server.sendmail(client_login, recepient, text)
    except:
        print('[email] couldn\'t send the email')

    server.quit()

    return True

def action_ant_select(args):

    if len(args) > 1 or len(args) < 1:
        print('Only one argument is needed (antenna number)')
        return False

    freq = float(args[0])
    print(freq)

    if freq > 30e6 and freq < 300e6:
        # VHF input
        ant_num = '3'
    elif freq > 300e6 and freq < 3000e6:
        # UHF imput
        ant_num = '1'
    else:
        # S-band imput: not supported
        ant_num = '2'

    ser = serial.Serial(
        port=ANT_SELECT_SERIAL_DEV,
        baudrate=ANT_SELECT_SERIAL_BAUD,
        parity=ANT_SELECT_SERIAL_PARITY,
        stopbits=ANT_SELECT_SERIAL_STOPBITS,
        bytesize=ANT_SELECT_SERIAL_BYTESIZE
    )

    if ser.isOpen() is not True:
        return False

    out = ''
    ser.write(ant_num.encode('utf-8'))
    time.sleep(0.1)
    while ser.inWaiting() > 0:
        out += ser.read(1).decode('utf-8')

    out = out.strip()
    if out == '':
        ser.close()
        return False

    print(out)
    ser.close()
    return True

# create directories, clean up old files
def action_pre_doit(args):
    print('== pre doit ==')
    today = date.today()
    os.system('mkdir -p ' + RESULTS_PREFIX + str(today))
    action_ant_select(args)
    pass

# put files on FTP, decode, send notifications
def action_post_doit(args):
    print('== post doit ==')
    today = date.today()
    files = glob.glob('*GMT.dat')
    send_email_notification('/etc/gsc/email.cfg', files)
    os.system('mv -vf *GMT.dat ' + RESULTS_PREFIX + str(today) + '/')
    os.system('mv -vf *GMT.raw ' + RESULTS_PREFIX + str(today) + '/')
    return

actions = {
            'pre_doit'   : action_pre_doit,
            'post_doit'  : action_post_doit,
            'ant_select' : action_ant_select,
          }

args = sys.argv
if len(args) < 2:
    print('No action specified')
    exit(1)

action = args[1]
func = actions.get(action)

if func == None:
    print('No action \'' + action + '\' found in list')
    print('Valid actions:')
    for key in actions.keys():
        print(' -> ' + key)
    exit(1)

args = args[2:]
func(args)
