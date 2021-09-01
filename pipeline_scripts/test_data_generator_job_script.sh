#!/bin/bash

#SBATCH -A sens2020586
#SBATCH -p core
#SBATCH -n 2
#SBATCH -t 4:00:00
#SBATCH -J test_data_generator

./test_data_generator/test_data_generator $1
