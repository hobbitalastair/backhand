#!/usr/bin/sh
#
# bh-release: release a request for a service.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

SERVICE_DIR="/usr/lib/backhand"
SERVICE_RUNDIR="/run/backhand"

if [ $# != 1 ]; then
    printf "usage: bh-release <service>\n" 1>&2
    exit 1
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

semaphore "${service_rundir}/require" -
err="$?"
if [ "${err}" == 0 ]; then
    bh-stop "${service}"
    if [ "$?" -ne 0 ]; then
        printf '%s: failed to stop service %s\n' "$0" "${service}" 1>&2
        exit 1
    fi
elif [ "${err}" == 2 ]; then
    printf '%s: failed to decrement require count\n' "$0" 1>&2
    exit 1
fi

