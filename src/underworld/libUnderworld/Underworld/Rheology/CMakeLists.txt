cmake_minimum_required(VERSION 3.16)

set(sources
    src/ConstitutiveMatrix.c
    src/Finalise.c
    src/Init.c
    src/ConstitutiveMatrixCartesian.cpp
    )

target_sources(Underworld PRIVATE ${sources})