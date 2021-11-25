cmake_minimum_required(VERSION 3.16)

set(sources
    src/Energy_SLE_Solver.c
    src/Finalise.c
    src/Init.c
    )

target_sources(StgFEM PRIVATE ${sources})