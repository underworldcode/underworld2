cmake_minimum_required(VERSION 3.16)

set(sources
    src/Finalise.c
    src/Init.c
    src/LumpedMassMatrixForceTerm.c
    src/Multicorrector.c
    src/Timestep.c
    src/AdvectionDiffusionSLE.cpp
    src/Residual.cpp
    )

target_sources(Underworld PRIVATE ${sources})