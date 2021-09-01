#!/bin/bash

[[ "$1" =~ .*\/(.*?)\.bam ]]

RES=$(sbatch --parsable calc_coverage_job_script.sh "$1") && sbatch --dependency=afterok:${RES} data_generator_job_script.sh "data/subjects/${BASH_REMATCH[1]}/${BASH_REMATCH[1]}.standardizedCR.tsv" cnv_visualizer/data_generator/template.html

