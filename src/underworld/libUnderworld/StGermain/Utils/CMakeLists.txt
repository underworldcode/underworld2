cmake_minimum_required(VERSION 3.16)

set(sources
    src/Finalise.c
    src/Init.c
    src/Progress.c
    )

target_sources(StGermain PRIVATE ${sources})
