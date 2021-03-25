#!/bin/sh

HAMLIB_PREFIX=/home/stanislavb/Work/Hamlib

echo "Starting rotctld for azimuth control"
screen sudo ${HAMLIB_PREFIX}/tests/rotctld -s 9600 -m 1004 -r /dev/ttyUSB0 -T 127.0.0.1 -t 8080 -vvvvv
echo "Starting rotctld for elevation control"
screen sudo ${HAMLIB_PREFIX}/tests/rotctld -s 9600 -m 1005 -r /dev/ttyS0 -T 127.0.0.1 -t 8081 -vvvvv
echo "Updating Keplers"
# FIXME
rm -rfv ./build/active.txt
wget http://celestrak.com/NORAD/elements/active.txt ./build/
