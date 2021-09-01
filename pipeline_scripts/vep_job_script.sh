#!/bin/bash

#SBATCH -A sens2020586
#SBATCH -p core
#SBATCH -n 1
#SBATCH -t 3:00:00
#SBATCH -J vep_test_run

module load bioinfo-tools
module load vep

[[ "$1" =~ (.*\/)(.*?)\.g\.vcf ]]

vep -i "$1" --cache --dir $VEP_CACHE --offline --assembly GRCh37 -o "veo.txt"
