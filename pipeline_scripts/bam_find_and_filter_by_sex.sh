#!/bin/bash

find "$1" -regex ".*\.bam" -exec ${BASH_SOURCE%/*}/bam_filter_by_sex.sh "$2" '{}' +
