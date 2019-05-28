#!/bin/bash

#SBATCH --job-name=pars_benchmark
#SBATCH --output=pars_benchmark.log
#SBATCH --time=00:10:00
#SBATCH --mem=8000M
#SBATCH --nodes=16
#SBATCH --cpus-per-task=16
#SBATCH --ntasks-per-core=1
#SBATCH --ntasks-per-node=1

source ./load_common_modules.sh

srun --mpi=pmi2 ../../../build/pars_benchmark/pars_benchmark 16 ../../config/test.json
