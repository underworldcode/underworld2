cmake_minimum_required(VERSION 3.16)

set(sources
    src/ConstantWeights.c
    src/DVCWeights.c
    src/Finalise.c
    src/Init.c
    src/WeightsCalculator.c
    )

target_sources(PICellerator PRIVATE ${sources})