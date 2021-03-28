#!/bin/sh

if [ -z $2 ]; then
	exit 1
fi

echo -e "Subject: Tracking done!\r\n\r\nThe ground station has finished tracking of '${1}', \
the results are stored in 'http://172.16.30.52:8000/${2}'}" | \
msmtp --debug --from=default -t stanislav.barantsev@community.isunet.edu
