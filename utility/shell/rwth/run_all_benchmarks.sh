#!/bin/bash

./run_strong_scaling_benchmarks.sh
./run_weak_scaling_vary_iterations_benchmarks.sh
./run_weak_scaling_vary_stride_benchmarks.sh
./run_gantt_benchmarks.sh
./run_integrator_benchmarks.sh