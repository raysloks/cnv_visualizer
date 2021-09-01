#!/bin/bash

find ./data/hdf5 -regex ".*BAB[0-9]*\.hdf5" -exec sbatch build_pon_job_script.sh '{}' +
