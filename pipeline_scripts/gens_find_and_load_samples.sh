#!/bin/bash

find "$1" -regex ".*cov\.bed\.gz" -exec singularity exec "$2" bash ${BASH_SOURCE%/*}/gens_load_samples.sh "$3" '{}' +
