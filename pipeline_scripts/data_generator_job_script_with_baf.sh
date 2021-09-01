#!/bin/bash

#SBATCH -A sens2020586
#SBATCH -p core
#SBATCH -n 2
#SBATCH -t 1-00:00:00
#SBATCH -J cnv_visualizer_data_generator

[[ "$1" =~ (.*)\/.*? ]]

mkdir "${BASH_REMATCH[1]}/vis"

./cnv_visualizer/data_generator/bin/data_generator $1 $2 $3 -b $4 $5
