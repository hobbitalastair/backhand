#!/usr/bin/sh
#
# bh-require: require a service.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

set -e

SERVICE_DIR="/usr/lib/backhand"
SERVICE_RUNDIR="/run/backhand"

if [ $# != 1 ]; then
    printf "usage: bh-require <service>\n" 1>&2
    exit 1;
fi
service="$1"
service_name="${service%%@*}"
service_target="${service#*@}"

service_dir="${SERVICE_DIR}/${service_name}"
if [ ! -d "${service_dir}" ]; then
    printf "%s: no such service\n" "$0" 1>&2
    exit 1;
fi

service_rundir="${SERVICE_RUNDIR}/${service}"
if [ ! -d "${service_rundir}" ]; then
    mkdir -p "${service_rundir}"
    if [ "$?" != 0 ]; then
        printf '%s: failed to create runtime dir\n' "$0" 1>&2
        exit 1
    fi
fi

semaphore "${service_rundir}/require" +
err="$?"
if [ "${err}" == 0 ]; then
    bh-start "${service}"
else if [ "${err}" == 2 ]; then
    printf '%s: failed to increment require count\n' "$0" 1>&2
    exit 1
fi

