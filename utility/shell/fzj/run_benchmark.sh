#!/bin/bash

mpiexec -np $1 -ppn 1 ../../../build/pars_benchmark/pars_benchmark $2 $3
