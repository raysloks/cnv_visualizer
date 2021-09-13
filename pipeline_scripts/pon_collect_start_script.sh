#!/bin/bash

#xargs -n 1 echo < "$2"
xargs -n 1 sbatch "${BASH_SOURCE%/*}/pon_collect_job_script.sh" "$1" < "$2"
