#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 3
#SBATCH -t 1-00:00:00
#SBATCH -J gens_data_generator

module load bioinfo-tools
module load tabix

./$5/utils/generate_gens_data.pl $1 $2 $3 $4
