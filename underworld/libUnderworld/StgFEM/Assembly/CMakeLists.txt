cmake_minimum_required(VERSION 3.16)

set(sources
    src/DivergenceMatrixTerm.c
    src/Finalise.c
    src/GradientStiffnessMatrixTerm.c
    src/Init.c
    src/IsoviscousStressTensorTerm.c
    src/LaplacianStiffnessMatrixTerm.c
    )

target_sources(StgFEM PRIVATE ${sources})