#!/bin/bash

cd ../../../build/pars_benchmark_generator/

# Weak scaling 26GB vary nodes^3 and stride
sbatch benchmark_sc2_n1_p48_st32_i1024_lb0.sh
sbatch benchmark_sc2_n1_p48_st32_i1024_lb1.sh
sbatch benchmark_sc2_n8_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n8_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n64_p48_st8_i1024_lb0.sh
sbatch benchmark_sc2_n64_p48_st8_i1024_lb1.sh
sbatch benchmark_sc2_n512_p48_st4_i1024_lb0.sh
sbatch benchmark_sc2_n512_p48_st4_i1024_lb1.sh

# Weak scaling 208GB vary nodes^3 and stride
sbatch benchmark_sc4_n8_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n8_p48_st32_i1024_lb1.sh
sbatch benchmark_sc4_n64_p48_st16_i1024_lb0.sh
sbatch benchmark_sc4_n64_p48_st16_i1024_lb1.sh
sbatch benchmark_sc4_n512_p48_st8_i1024_lb0.sh
sbatch benchmark_sc4_n512_p48_st8_i1024_lb1.sh