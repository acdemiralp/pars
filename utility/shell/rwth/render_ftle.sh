#!/bin/bash
#SBATCH --job-name=render_ftle
#SBATCH --output=render_ftle.log
#SBATCH --time=00:10:00
#SBATCH --mem=128000M
#SBATCH --nodes=16
#SBATCH --cpus-per-task=48
#SBATCH --ntasks-per-core=1
#SBATCH --ntasks-per-node=1

module load cmake/3.13.2
module load gcc/8
module load LIBRARIES
module load boost/1_69_0
module load hdf5/1.10.4
module load inteltbb/2019

srun --mpi=pmi2 /rwthfs/rz/cluster/home/ad784563/source/pars/build/pars_benchmark/pars_benchmark 48 ./render_ftle.json
