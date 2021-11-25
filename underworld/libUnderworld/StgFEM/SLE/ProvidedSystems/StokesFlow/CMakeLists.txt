cmake_minimum_required(VERSION 3.16)

set(sources
    src/Finalise.c
    src/Init.c
    src/Stokes_SLE.c
    src/Stokes_SLE_PenaltySolver.c
    src/Stokes_SLE_UzawaSolver.c
    src/UzawaPreconditionerTerm.c
    )

target_sources(StgFEM PRIVATE ${sources})