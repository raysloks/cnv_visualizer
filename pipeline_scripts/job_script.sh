#!/bin/bash -l

#SBATCH -A sens2020586
#SBATCH -p core
#SBATCH -n 2
#SBATCH -t 4:00:00
#SBATCH -J gatk_collect_read_counts

#gatk PreprocessIntervals --reference reference/human_g1k_v37.fasta --bin-length 100 --interval-merging-rule OVERLAPPING_ONLY -O data/targets_preprocessed_100bp.interval_list

[[ "$1" =~ .*(BAB[0-9]+)\.bam ]]
#echo "$1"
#echo "${BASH_REMATCH[1]}"
gatk --java-options "-Xmx14000m" CollectReadCounts -I "$1" -L data/targets_preprocessed_100bp.interval_list --interval-merging-rule OVERLAPPING_ONLY -O "data/hdf5/${BASH_REMATCH[1]}.hdf5"
