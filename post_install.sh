#!/bin/bash

cmake_dir="$1"
path="/etc/gsc"
mkdir -p ${path}

cmd="${path}/active_update.sh"
cp -rfv ${cmake_dir}/active_update.sh ${path}
crontab -l | { cat; echo "0 0 */5 * * ${cmd}"; } | crontab -
