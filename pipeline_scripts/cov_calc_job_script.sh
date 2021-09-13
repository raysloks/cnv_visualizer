#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p node
#SBATCH -t 20:00:00
#SBATCH -J gatk_calc_coverage

module load bioinfo-tools
module load GATK

[[ "$1" =~ .*\/(.+)\.bam ]]
#echo ${BASH_REMATCH[1]}

mkdir "data/subjects/${BASH_REMATCH[1]}"

gatk --java-options "-Xmx112000m" CollectReadCounts -I "$1" -L data/targets_preprocessed_100bp.interval_list --interval-merging-rule OVERLAPPING_ONLY -O "data/subjects/${BASH_REMATCH[1]}/${BASH_REMATCH[1]}.hdf5"

gatk --java-options "-Xmx112000m" DenoiseReadCounts -I "data/subjects/${BASH_REMATCH[1]}/${BASH_REMATCH[1]}.hdf5" --count-panel-of-normals $2 --standardized-copy-ratios "data/subjects/${BASH_REMATCH[1]}/${BASH_REMATCH[1]}.standardizedCR.tsv" --denoised-copy-ratios "data/subjects/${BASH_REMATCH[1]}/${BASH_REMATCH[1]}.denoisedCR.tsv"
