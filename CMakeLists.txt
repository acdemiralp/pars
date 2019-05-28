##################################################    Project     ##################################################
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project               (pars)
add_subdirectory      (./pa                      )
add_subdirectory      (./pars                    )
add_subdirectory      (./pars_service            )
add_subdirectory      (./pars_benchmark          )
add_subdirectory      (./pars_benchmark_generator)
