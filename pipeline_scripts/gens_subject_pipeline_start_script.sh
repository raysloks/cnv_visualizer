#!/bin/bash

xargs -n 1 "${BASH_SOURCE%/*}/gens_subject_pipeline.sh" "$1" "$2" < "$3"
