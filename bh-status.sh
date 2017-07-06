#!/usr/bin/sh
#
# bh-status: print the status of a service.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

set -e

SERVICE_DIR="/usr/lib/backhand"
SERVICE_RUNDIR="/run/backhand"

if [ $# != 1 ]; then
    printf "usage: bh-stop <service>\n" 1>&2
    exit 1;
fi
service="$1"

service_dir="${SERVICE_DIR}/${service}"
if [ ! -d "${service_dir}" ]; then
    printf "%s: no such service\n" "$0" 1>&2
    exit 1;
fi

service_rundir="${SERVICE_RUNDIR}/${service}"
if [ ! -d "${service_rundir}" ]; then
    printf '%s: service runtime dir does not exist\n' "$0" 1>&2
    exit 1
fi

if [ -e "${service_rundir}/socket" ]; then
    printf 'started\n'
else
    printf 'stopped\n'
fi
