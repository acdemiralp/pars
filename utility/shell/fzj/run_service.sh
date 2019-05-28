#!/bin/bash

mpiexec -np $1 -ppn 1 ../../../build/pars_service/pars_service
