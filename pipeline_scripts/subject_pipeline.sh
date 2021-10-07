#!/bin/bash

[[ "$3" =~ .*\/(.*?)\.bam ]]

SUBJECT_DIR="data/subjects/${BASH_REMATCH[1]}/"
mkdir "$SUBJECT_DIR"

#COV_CALC_JOBID=$(sbatch --parsable ${BASH_SOURCE%/*}/cov_calc_job_script.sh "$3" "$1") &&
sbatch ${BASH_SOURCE%/*}/data_generator_job_script_with_all.sh "$SUBJECT_DIR/${BASH_REMATCH[1]}.standardizedCR.tsv" ${BASH_SOURCE%/*/*}/data_generator/template.html "" "$SUBJECT_DIR/${BASH_REMATCH[1]}.g.vcf" "$2" "$3" "${BASH_SOURCE%/*/*}"

#--dependency=afterok:${COV_CALC_JOBID}
