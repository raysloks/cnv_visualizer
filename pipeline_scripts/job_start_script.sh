#!/bin/bash

module load bioinfo-tools
module load GATK

find ./bam -regex ".*BAB[0-9]*\.bam" -exec sbatch job_script.sh '{}' \;
