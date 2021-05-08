#!/bin/bash

if [ -z $2 ]; then
	exit 1
fi

sat_name="$1"
filename="$2"
web_addr="http://172.16.30.52:8000"
sender="gsnotification38@gmail.com"
recipient="stanislav.barantsev@community.isunet.edu"

echo -ne "To: ${recipient}\nFrom: ${sender}\nSubject: Tracking done\n\n"\
"Hello there,\n"\
"Tracking of ${1} satellite done. Please find the results via link:\n\n"\
"${web_addr}/${filename}" | msmtp ${recipient}
