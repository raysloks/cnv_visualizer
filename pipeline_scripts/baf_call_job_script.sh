#!/bin/bash

#SBATCH -A sens2017106
#SBATCH -p core
#SBATCH -n 1
#SBATCH -t 8-00:00:00
#SBATCH -J gatk_haplotype_caller

module load bioinfo-tools
module load GATK

[[ "$2" =~ .*\/(.+)\.bam ]]
#echo ${BASH_REMATCH[1]}

mkdir "data/subjects/${BASH_REMATCH[1]}"

gatk --java-options "-Xmx4g" HaplotypeCaller -R "$1" -I "$2" -O "data/subjects/${BASH_REMATCH[1]}/${BASH_REMATCH[1]}.g.vcf" -ERC GVCF
