#!/usr/bin/sh
#
# bk-stop: stop a service.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

set -e

SERVICE_DIR="/usr/lib/backhand/"
SERVICE_RUNDIR="/run/backhand/"
SERVICE_POST="post"
SERVICE_TIMEOUT=10

if [ $# != 1 ]; then
    printf "usage: bk-stop <service>\n" 1>&2
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

if [ -e "${service_dir}/${SERVICE_START}" ]; then
    socket="${service_rundir}/socket"
    connect "${socket}" "${service_dir}/${SERVICE_START}"
    if [ "$?" != 0 ]; then
        printf "%s: stop failed\n" "$0" 1>&2
        exit 1;
    fi
fi

if [ -e "${service_dir}/${SERVICE_POST}" ]; then
    timeout "${SERVICE_TIMEOUT}" "${service_dir}/${SERVICE_POST}"
    if [ "$?" != 0 ]; then
        printf "%s: post failed\n" "$0" 1>&2
        exit 1;
    fi
fi

