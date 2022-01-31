cmake_minimum_required(VERSION 3.16)

set(sources
    src/CoincidentMapper.c
    src/Finalise.c
    src/Init.c
    src/IntegrationPointsSwarm.c
    src/PeriodicBoundariesManager.c
    src/PICelleratorContext.c
    src/SwarmAdvector.c
    src/SwarmMap.c
    src/GeneralSwarm.cpp
    )

target_sources(PICellerator PRIVATE ${sources})