#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 1
#SBATCH -t 1:00:00
#SBATCH -J cnv_visualizer_compile_annotations

./$1/data_generator/bin/data_generator -a "$2" "$3"
