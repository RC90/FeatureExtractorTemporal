#!/bin/bash
#PBS -l walltime=05:00:00
#PBS -l select=1:ncpus=1:mem=10GB
#PBS -P HPCA-01347-PGR
#PBS -j oe
#PBS -o ClusterOutput.txt

cd /home/psxta4/FeatureExtractorTemporal/FeatureExtractorTemporal
./run.sh
