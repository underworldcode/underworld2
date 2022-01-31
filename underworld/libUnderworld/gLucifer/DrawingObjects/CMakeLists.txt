cmake_minimum_required(VERSION 3.16)

set(sources
    src/ContourCrossSection.c
    src/Eigenvectors.c
    src/EigenvectorsCrossSection.c
    src/FieldSampler.c
    src/Finalise.c
    src/HistoricalSwarmTrajectory.c
    src/Init.c
    src/Isosurface.c
    src/IsosurfaceCrossSection.c
    src/MeshViewer.c
    src/ScalarField.c
    src/SwarmRGBColourViewer.c
    src/SwarmShapes.c
    src/SwarmVectors.c
    src/VectorArrows.c
    src/SwarmViewer.cpp
    src/CrossSection.cpp
    src/Sampler.cpp
    )

target_sources(gLucifer PRIVATE ${sources})
