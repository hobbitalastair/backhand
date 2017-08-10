#!/usr/bin/sh
#
# bh-stop: stop a service.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

[ -z "${SERVICE_DIR}" ] && SERVICE_DIR="/usr/lib/backhand"
[ -z "${SERVICE_RUNDIR}" ] && SERVICE_RUNDIR="/run/backhand"
SERVICE_POST="post"
SERVICE_RUN="run"
SERVICE_TIMEOUT=10

if [ $# != 1 ]; then
    printf "usage: bh-stop <service>\n" 1>&2
    exit 1
fi
service="$1"

service_dir="${SERVICE_DIR}/${service}"
if [ ! -d "${service_dir}" ]; then
    printf "%s: no such service\n" "$0" 1>&2
    exit 1
fi

service_rundir="${SERVICE_RUNDIR}/${service}"
if [ ! -d "${service_rundir}" ]; then
    printf '%s: service runtime dir does not exist\n' "$0" 1>&2
    exit 1
fi

service_state="${service_rundir}/state"
state "${service_state}" "stopped"
ret="$?"
if [ "${ret}" == 2 ]; then
    printf "%s: updating the state failed\n" "$0" 1>&2
    exit 1
elif [ "${ret}" == 0 ]; then

    if [ -x "${service_dir}/${SERVICE_RUN}" ]; then
        socket="${service_rundir}/socket"
        connect "${socket}"
        if [ "$?" != 0 ]; then
            printf "%s: stop failed\n" "$0" 1>&2
            state "${service_state}" "failed"
            exit 1
        fi
    fi

    if [ -e "${service_dir}/${SERVICE_POST}" ]; then
        timeout "${SERVICE_TIMEOUT}" "${service_dir}/${SERVICE_POST}"
        if [ "$?" != 0 ]; then
            printf "%s: post failed\n" "$0" 1>&2
            state "${service_state}" "failed"
            exit 1
        fi
    fi

fi
