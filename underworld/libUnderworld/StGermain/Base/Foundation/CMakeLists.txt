cmake_minimum_required(VERSION 3.16)

set(sources
    ./src/Class.c
    ./src/Finalise.c
    ./src/Memory.c
    ./src/ObjectAdaptor.c
    ./src/TimeMonitor.c
    ./src/CommonRoutines.c
    ./src/Init.c
    ./src/NamedObject_Register.c
    ./src/Object.c
    ./src/debug.c
    ./src/Log.c
    ./src/Numerics.c
    ./src/ObjectList.c)

target_sources(StGermain PRIVATE ${sources})
