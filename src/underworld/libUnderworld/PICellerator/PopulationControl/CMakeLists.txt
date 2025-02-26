cmake_minimum_required(VERSION 3.16)

set(sources
    src/EscapedRoutine.c
    src/Finalise.c
    src/Init.c
    )

target_sources(PICellerator PRIVATE ${sources})