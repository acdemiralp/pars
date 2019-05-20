#!/bin/bash

#SBATCH --job-name=prs_benchmark
#SBATCH --output=prs_benchmark.log
#SBATCH --time=00:10:00
#SBATCH --mem=8000M
#SBATCH --nodes=16
#SBATCH --cpus-per-task=16
#SBATCH --ntasks-per-core=1
#SBATCH --ntasks-per-node=1

source ./load_common_modules.zsh

export I_MPI_OFA_TRANSLATION_CACHE=0
export I_MPI_DAPL_TRANSLATION_CACHE=0
export I_MPI_DAPL_UD_TRANSLATION_CACHE=0
export I_MPI_FABRICS=shm:ofa
export I_MPI_JOB_RESPECT_PROCESS_PLACEMENT=off

srun --mpi=pmi2 ../../build/benchmark/prs_benchmark ../config/test.json 16
