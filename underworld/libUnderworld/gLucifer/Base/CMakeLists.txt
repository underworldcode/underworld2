cmake_minimum_required(VERSION 3.16)

set(sources
    src/ColourMap.c
    src/Database.c
    src/DrawingObject.c
    src/Finalise.c
    src/Init.c
    src/miniz.c
    src/sqlite3.c
    )

target_sources(gLucifer PRIVATE ${sources})
