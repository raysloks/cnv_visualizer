#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 2
#SBATCH -t 10-00:00:00
#SBATCH -J cnv_visualizer_gnomad_filter

module load bioinfo-tools
module load samtools

bgzip -cd $1 | ./${BASH_SOURCE%/*/*}/data_generator/bin/data_generator -f $2 $3 $4
