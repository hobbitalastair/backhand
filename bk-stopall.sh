#!/usr/bin/sh
#
# bk-stopall: stop all running services.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

set -e

SERVICE_RUNDIR="/run/backhand/"

# FIXME: Services should be stopped in parallel.

cd "${SERVICE_RUNDIR}"
for service in *; do
    if [ -d "${service}" ]; then
        bk-stop "${service}"
    fi
done

