#!/bin/bash

export GENS_CONFIG="$1"
for i in "${@:2}"
do
	[[ "$i" =~ .*\/(.+)\.cov\.bed\.gz ]]
	gens load sample -i "${BASH_REMATCH[1]}" -b 37 -c "$i" -a "${i%\.cov\.bed\.gz}".baf.bed.gz
done
