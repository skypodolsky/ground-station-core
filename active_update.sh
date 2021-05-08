#!/bin/bash

wget -O /etc/gsc/active.txt http://celestrak.com/NORAD/elements/active.txt

pid=$(pgrep gsc)

if [ -n "${pid}" ]; then
	kill -SIGHUP ${pid}
fi
