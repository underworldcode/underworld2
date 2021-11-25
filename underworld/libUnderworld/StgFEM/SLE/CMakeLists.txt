cmake_minimum_required(VERSION 3.16)

set(sources
    src/Finalise.c
    src/Init.c
    )

target_sources(StgFEM PRIVATE ${sources})