#!/bin/bash

sbatch ${BASH_SOURCE%/*}/annotations_compile_job_script.sh "${BASH_SOURCE%/*/*}" "$1" "$2"
