#!/usr/bin/sh
#
# bh-start: start a service.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

[ -z "${SERVICE_DIR}" ] && SERVICE_DIR="/usr/lib/backhand"
[ -z "${SERVICE_RUNDIR}" ] && SERVICE_RUNDIR="/run/backhand"
[ -z "${SERVICE_LOGDIR}" ] && SERVICE_LOGDIR="/var/log/backhand"
SERVICE_PRE="pre"
SERVICE_RUN="run"
SERVICE_TIMEOUT=10

if [ $# != 1 ]; then
    printf "usage: bh-start <service>\n" 1>&2
    exit 1
fi
service="$1"
service_name="${service%%@*}"
service_target="${service#*@}"

service_log="${SERVICE_LOGDIR}/${service}"
if [ ! -w "${SERVICE_LOGDIR}" ]; then
    printf '%s: log %s not writeable; falling back to /dev/null\n' "$0" 1>&2
    service_log="/dev/null"
fi

service_dir="${SERVICE_DIR}/${service_name}"
if [ ! -d "${service_dir}" ]; then
    printf "%s: no such service\n" "$0" 1>&2
    exit 1
fi

service_rundir="${SERVICE_RUNDIR}/${service}"
if [ ! -d "${service_rundir}" ]; then
    mkdir -p "${service_rundir}"
    if [ "$?" != 0 ]; then
        printf '%s: failed to create runtime dir\n' "$0" 1>&2
        exit 1
    fi
fi

service_state="${service_rundir}/state"
state "${service_state}" "started"
ret="$?"
if [ "${ret}" == 2 ]; then
    printf "%s: updating the state failed\n" "$0" 1>&2
    exit 1
elif [ "${ret}" == 0 ]; then

    if [ -x "${service_dir}/${SERVICE_PRE}" ]; then
        @TIMEOUTCMD@ "${SERVICE_TIMEOUT}" "${service_dir}/${SERVICE_PRE}" \
            "${service_target}" >> "${service_log}" 2>&1
        if [ "$?" != 0 ]; then
            printf "%s: pre failed\n" "$0" 1>&2
            state "${service_state}" "failed"
            exit 1
        fi
    fi

    if [ -x "${service_dir}/${SERVICE_RUN}" ]; then
        socket="${service_rundir}/socket"
        escort "${socket}" "${service_dir}/${SERVICE_RUN}" \
            "${service_target}" >> "${service_log}" 2>&1
        if [ "$?" != 0 ]; then
            printf "%s: run failed\n" "$0" 1>&2
            state "${service_state}" "failed"
            exit 1
        fi
    fi

fi
