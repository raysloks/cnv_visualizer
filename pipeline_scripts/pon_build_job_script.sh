#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p node
#SBATCH -t 10-00:00:00
#SBATCH -J gatk_build_pon
#SBATCH -C fat

module load bioinfo-tools
module load GATK
module load openblas

export LD_PRELOAD=/sw/libs/openblas/0.3.15/bianca/lib/libopenblas.so

new_args=()
for arg in ${@:2}
do
    new_args+=( '-I' )
    new_args+=( "$arg" )
done
gatk --java-options "-Xmx255g" CreateReadCountPanelOfNormals --minimum-interval-median-percentile 10.0 --maximum-chunk-size 29349635 -O "$1" "${new_args[@]}"
