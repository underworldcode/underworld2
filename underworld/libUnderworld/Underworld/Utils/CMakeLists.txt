cmake_minimum_required(VERSION 3.16)

set(sources
    src/Exceptions.c
    src/Finalise.c
    src/Init.c
    src/Fn_Integrate.cpp
    src/MatrixAssemblyTerm_NA_i__NB_i__Fn.cpp
    src/MatrixAssemblyTerm_NA__NB__Fn.cpp
    src/VectorAssemblyTerm_NA__Fn.cpp
    src/VectorAssemblyTerm_NA_i__Fn_i.cpp
    src/VectorAssemblyTerm_NA_j__Fn_ij.cpp
    src/VectorSurfaceAssemblyTerm_NA__Fn__ni.cpp
    )

target_sources(Underworld PRIVATE ${sources})