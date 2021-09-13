#!/bin/bash

[[ "$1" =~ .*\/(.*?)\.bam ]]

RES=$(sbatch --parsable ${BASH_SOURCE%/*}/calc_coverage_job_script.sh "$1") && 
sbatch --dependency=afterok:${RES} ${BASH_SOURCE%/*}data_generator_job_script.sh "data/subjects/${BASH_REMATCH[1]}/${BASH_REMATCH[1]}.standardizedCR.tsv" ${BASH_SOURCE%/*/*}/data_generator/template.html
