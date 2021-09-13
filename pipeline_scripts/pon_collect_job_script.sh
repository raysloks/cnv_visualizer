#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 2
#SBATCH -t 8:00:00
#SBATCH -J gatk_collect_read_counts

module load bioinfo-tools
module load GATK

[[ "$2" =~ .*\/(.+)\.bam ]]
#echo "$1"
#echo "${BASH_REMATCH[1]}"
gatk --java-options "-Xmx14000m" CollectReadCounts -I "$2" -L data/targets_preprocessed_100bp.interval_list --interval-merging-rule OVERLAPPING_ONLY -O "$1/${BASH_REMATCH[1]}.hdf5"
