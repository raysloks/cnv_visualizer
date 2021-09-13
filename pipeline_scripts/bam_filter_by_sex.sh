#!/bin/bash

module load bioinfo-tools
module load samtools

for i in "${@:2}"
do
	X=$(samtools idxstats "$i" | grep "^X" | sed -r 's/X	[0-9]+	([0-9]+)	[0-9]+/\1/')
	Y=$(samtools idxstats "$i" | grep "^Y" | sed -r 's/Y	[0-9]+	([0-9]+)	[0-9]+/\1/')
	if [ $(echo "$X / $Y $1" | bc) = 1 ]
	then
		echo "$i"
		#echo $(echo "$X / $Y" | bc) 
	fi
done
