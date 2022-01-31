cmake_minimum_required(VERSION 3.16)

set(sources
    src/Finalise.c
    src/Init.c
    )

target_sources(Underworld PRIVATE ${sources})

set(toolbox_sources ${toolbox_sources}
    Toolbox/Toolbox.c
)

target_sources(Underworld_Toolboxmodule PRIVATE ${toolbox_sources})