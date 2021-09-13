#!/bin/bash

find "$1" -regex ".*\.hdf5" -exec sbatch "${BASH_SOURCE%/*}/pon_build_job_script.sh" "$2" '{}' +
