#!/bin/bash

#SBATCH -A sens2020586
#SBATCH -p core
#SBATCH -n 2
#SBATCH -t 10-00:00:00
#SBATCH -J cnv_visualizer_gnomad_filter

module load bioinfo-tools
module load samtools

bgzip -c -d $1 | ./cnv_visualizer/data_generator/bin/data_generator -f $2 $3 $4
