#!/bin/bash

#SBATCH -A sens2020586
#SBATCH -p node
#SBATCH -t 20:00:00
#SBATCH -J gatk_build_pon

module load bioinfo-tools
module load GATK
module load openblas

export LD_PRELOAD=/sw/libs/openblas/0.3.15/bianca/lib/libopenblas.so

new_args=()
for arg
do
    new_args+=( '-I' )
    new_args+=( "$arg" )
done
gatk --java-options "-Xmx112000m" CreateReadCountPanelOfNormals --minimum-interval-median-percentile 10.0 --maximum-chunk-size 29349635 -O data/male_pon_100bp.hdf5 "${new_args[@]}"
