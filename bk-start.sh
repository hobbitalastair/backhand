#!/usr/bin/sh
#
# bk-start: start a service.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

set -e

SERVICE_DIR="/usr/lib/backhand"
SERVICE_RUNDIR="/run/backhand"
SERVICE_PRE="pre"
SERVICE_RUN="run"
SERVICE_TIMEOUT=10

if [ $# != 1 ]; then
    printf "usage: bk-start <service>\n" 1>&2
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
    mkdir -p "${service_rundir}"
    if [ "$?" != 0 ]; then
        printf '%s: failed to create runtime dir\n' "$0" 1>&2
        exit 1
    fi
fi

if [ -e "${service_dir}/${SERVICE_PRE}" ]; then
    timeout "${SERVICE_TIMEOUT}" "${service_dir}/${SERVICE_PRE}"
    if [ "$?" != 0 ]; then
        printf "%s: pre failed\n" "$0" 1>&2
        exit 1;
    fi
fi

if [ -e "${service_dir}/${SERVICE_START}" ]; then
    socket="${service_rundir}/socket"
    bk-escort "${socket}" "${service_dir}/${SERVICE_START}"
    if [ "$?" != 0 ]; then
        printf "%s: run failed\n" "$0" 1>&2
        exit 1;
    fi
fi

