#!/bin/bash

cd ../../../build/pars_benchmark_generator/

# Weak scaling 26GB vary nodes and iterations
sbatch benchmark_sc2_n8_p48_st16_i128_lb0.sh
sbatch benchmark_sc2_n8_p48_st16_i128_lb1.sh
sbatch benchmark_sc2_n16_p48_st16_i256_lb0.sh
sbatch benchmark_sc2_n16_p48_st16_i256_lb1.sh
sbatch benchmark_sc2_n32_p48_st16_i512_lb0.sh
sbatch benchmark_sc2_n32_p48_st16_i512_lb1.sh
sbatch benchmark_sc2_n64_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n64_p48_st16_i1024_lb1.sh

# Weak scaling 208GB vary nodes and iterations
sbatch benchmark_sc4_n32_p48_st32_i128_lb0.sh
sbatch benchmark_sc4_n32_p48_st32_i128_lb1.sh
sbatch benchmark_sc4_n64_p48_st32_i256_lb0.sh
sbatch benchmark_sc4_n64_p48_st32_i256_lb1.sh
sbatch benchmark_sc4_n128_p48_st32_i512_lb0.sh
sbatch benchmark_sc4_n128_p48_st32_i512_lb1.sh
sbatch benchmark_sc4_n256_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n256_p48_st32_i1024_lb1.sh

# Weak scaling 1.6TB vary nodes and iterations
sbatch benchmark_sc8_n128_p48_st64_i128_lb0.sh
sbatch benchmark_sc8_n128_p48_st64_i128_lb1.sh
sbatch benchmark_sc8_n256_p48_st64_i256_lb0.sh
sbatch benchmark_sc8_n256_p48_st64_i256_lb1.sh
sbatch benchmark_sc8_n512_p48_st64_i512_lb0.sh
sbatch benchmark_sc8_n512_p48_st64_i512_lb1.sh
sbatch benchmark_sc8_n1024_p48_st64_i1024_lb0.sh
sbatch benchmark_sc8_n1024_p48_st64_i1024_lb1.sh