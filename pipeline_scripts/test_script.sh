#!/bin/bash

new_args=()
for arg in ${@:2}
do
    new_args+=( '-I' )
    new_args+=( "$arg" )
done

echo "$1"
echo "${new_args[@]}"
