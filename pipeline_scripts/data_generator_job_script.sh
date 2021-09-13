#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 2
#SBATCH -t 4:00:00
#SBATCH -J cnv_visualizer_data_generator

[[ "$1" =~ (.*)\/.*? ]]

mkdir "${BASH_REMATCH[1]}/vis"

./${BASH_SOURCE%/*/*}/data_generator/bin/data_generator $1 $2 $3
