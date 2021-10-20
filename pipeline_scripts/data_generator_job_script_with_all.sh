#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 4
#SBATCH -t 1-00:00:00
#SBATCH -J cnv_visualizer_data_generator

[[ "$1" =~ (.*)\/.*? ]]

mkdir "${BASH_REMATCH[1]}/vis"

./$7/data_generator/bin/data_generator $1 $2 $3 -b $4 $5 -s $6
