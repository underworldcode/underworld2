cmake_minimum_required(VERSION 3.16)

set(sources
    src/ConvexHull.c
    src/Finalise.c
    src/Init.c
    src/PolygonShape.c
    src/ShapeClass.c)

target_sources(StgDomain PRIVATE ${sources})