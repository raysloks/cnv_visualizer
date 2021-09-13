#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 1
#SBATCH -t 1:00:00
#SBATCH -J gatk_preprocess_intervals

module load bioinfo-tools
module load GATK

gatk PreprocessIntervals --reference "$1" --bin-length 100 --interval-merging-rule OVERLAPPING_ONLY -O data/targets_preprocessed_100bp.interval_list
