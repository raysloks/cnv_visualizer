#!/bin/bash

sbatch -o "$3" ${BASH_SOURCE%/*}/bam_find_and_filter_by_sex_job_script.sh "$1" "$2" ${BASH_SOURCE%/*}/bam_filter_by_sex.sh
