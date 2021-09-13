#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 1
#SBATCH -t 1:00:00
#SBATCH -J find_and_filter_bams_by_sex
#SBATCH -e slurm-%j.err

find "$1" -regex ".*\.bam" -exec "$3" "$2" '{}' +
