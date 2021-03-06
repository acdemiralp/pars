#!/bin/bash

cd ../../../build/pars_benchmark_generator/

# Strong scaling 26GB vary nodes
sbatch benchmark_sc2_n1_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n1_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n2_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n2_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n4_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n4_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n8_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n8_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n16_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n16_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n32_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n32_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n64_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n64_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n128_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n128_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n256_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n256_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n512_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n512_p48_st16_i1024_lb1.sh
sbatch benchmark_sc2_n1024_p48_st16_i1024_lb0.sh
sbatch benchmark_sc2_n1024_p48_st16_i1024_lb1.sh

# Strong scaling 208GB vary nodes
sbatch benchmark_sc4_n8_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n8_p48_st32_i1024_lb1.sh
sbatch benchmark_sc4_n16_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n16_p48_st32_i1024_lb1.sh
sbatch benchmark_sc4_n32_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n32_p48_st32_i1024_lb1.sh
sbatch benchmark_sc4_n64_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n64_p48_st32_i1024_lb1.sh
sbatch benchmark_sc4_n128_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n128_p48_st32_i1024_lb1.sh
sbatch benchmark_sc4_n256_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n256_p48_st32_i1024_lb1.sh
sbatch benchmark_sc4_n512_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n512_p48_st32_i1024_lb1.sh
sbatch benchmark_sc4_n1024_p48_st32_i1024_lb0.sh
sbatch benchmark_sc4_n1024_p48_st32_i1024_lb1.sh

# Strong scaling 1.6TB vary nodes
sbatch benchmark_sc8_n32_p48_st64_i1024_lb0.sh
sbatch benchmark_sc8_n32_p48_st64_i1024_lb1.sh
sbatch benchmark_sc8_n64_p48_st64_i1024_lb0.sh
sbatch benchmark_sc8_n64_p48_st64_i1024_lb1.sh
sbatch benchmark_sc8_n128_p48_st64_i1024_lb0.sh
sbatch benchmark_sc8_n128_p48_st64_i1024_lb1.sh
sbatch benchmark_sc8_n256_p48_st64_i1024_lb0.sh
sbatch benchmark_sc8_n256_p48_st64_i1024_lb1.sh
sbatch benchmark_sc8_n512_p48_st64_i1024_lb0.sh
sbatch benchmark_sc8_n512_p48_st64_i1024_lb1.sh
sbatch benchmark_sc8_n1024_p48_st64_i1024_lb0.sh
sbatch benchmark_sc8_n1024_p48_st64_i1024_lb1.sh