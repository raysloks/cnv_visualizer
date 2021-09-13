#!/bin/bash

xargs -n 1 "${BASH_SOURCE%/*}/subject_pipeline.sh" "$1" "$2" < "$3"
