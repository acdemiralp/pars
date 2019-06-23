#!/bin/bash

cd ../../../build/pars_benchmark_generator/

sbatch benchmark_sc4_n32_p48_st16_i1024_lb0.sh
sbatch benchmark_sc4_n32_p48_st16_i1024_lb1.sh