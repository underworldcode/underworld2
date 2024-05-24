cmake_minimum_required(VERSION 3.16)

set(sources
    src/CallGraph.c
    src/Comm.c
    src/Finalise.c
    src/HierarchyTable.c
    src/Init.c
    src/LiveComponentRegister.c
    src/Stg_Component.c
    src/Stg_ComponentFactory.c
    src/Stg_ComponentRegister.c
    )

target_sources(StGermain PRIVATE ${sources})
target_include_directories(StGermain PRIVATE "${PROJECT_SOURCE_DIR}/Base/Automation/src")