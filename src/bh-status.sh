#!/usr/bin/sh
#
# bh-status: print the status of a service.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

[ -z "${SERVICE_DIR}" ] && SERVICE_DIR="/usr/lib/backhand"
[ -z "${SERVICE_RUNDIR}" ] && SERVICE_RUNDIR="/run/backhand"

if [ $# != 1 ]; then
    printf "usage: bh-status <service>\n" 1>&2
    exit 1;
fi
service="$1"
service_name="${service%%@*}"
service_target="${service#*@}"

service_dir="${SERVICE_DIR}/${service_name}"
if [ ! -d "${service_dir}" ]; then
    printf "%s: no such service\n" "$0" 1>&2
    exit 1
fi

service_rundir="${SERVICE_RUNDIR}/${service}"
if [ ! -d "${service_rundir}" ]; then
    printf '%s: service runtime dir does not exist\n' "$0" 1>&2
    exit 1
fi

if [ -e "${service_rundir}/state" ]; then
    cat "${service_rundir}/state"
    err="$?"
    printf '\n'
    if [ "${err}" -ne 0 ]; then
        printf '%s: failed to read status\n' "$0" 1>&2
        exit 1
    fi
else
    printf 'stopped\n'
fi
