#!/usr/bin/sh
#
# bh-stopall: stop all running services.
#
# Author:   Alastair Hughes
# Contact:  hobbitalastair at yandex dot com

set -e

SERVICE_RUNDIR="/run/backhand/"

# FIXME: Services should be stopped in parallel.

cd "${SERVICE_RUNDIR}"
for service in *; do
    if [ -d "${service}" ]; then
        bh-stop "${service}"
    fi
done

