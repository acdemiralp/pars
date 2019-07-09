#!/bin/bash

export I_MPI_OFA_TRANSLATION_CACHE=0
export I_MPI_DAPL_TRANSLATION_CACHE=0
export I_MPI_DAPL_UD_TRANSLATION_CACHE=0
export I_MPI_FABRICS=shm:ofa
export I_MPI_JOB_RESPECT_PROCESS_PLACEMENT=off

mpiexec -np $1 -ppn 1 -c clx18 ../../../build/pars_service/pars_service
