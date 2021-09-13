#!/bin/bash

[[ "$3" =~ .*\/(.*?)\.bam ]]

SUBJECT_DIR="data/subjects/${BASH_REMATCH[1]}/"
mkdir "$SUBJECT_DIR"

sbatch ${BASH_SOURCE%/*}/gens_data_generator_job_script.sh "$SUBJECT_DIR/${BASH_REMATCH[1]}.standardizedCR.tsv" "$SUBJECT_DIR/${BASH_REMATCH[1]}.g.vcf" "${BASH_REMATCH[1]}" "$1" "$2"
