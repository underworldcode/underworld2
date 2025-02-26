cmake_minimum_required(VERSION 3.16)

set(sources
    src/Finalise.c
    src/Init.c
    )

target_sources(gLucifer PRIVATE ${sources})

set(tools_sources
    Toolbox/Toolbox.c)

target_sources(gLucifer_Toolboxmodule PRIVATE ${tools_sources})
