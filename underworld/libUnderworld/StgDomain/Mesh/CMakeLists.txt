cmake_minimum_required(VERSION 3.16)

set(sources
    src/CartesianGenerator.c
    src/Decomp.c
    src/Finalise.c
    src/Grid.c
    src/IGraph.c
    src/Init.c
    src/Mesh_Algorithms.c
    src/Mesh_CentroidAlgorithms.c
    src/Mesh_CentroidType.c
    src/MeshClass.c
    src/Mesh_ElementType.c
    src/MeshGenerator.c
    src/Mesh_HexAlgorithms.c
    src/Mesh_HexType.c
    src/Mesh_RegularAlgorithms.c
    src/MeshTopology.c
    src/MeshVariable.c
    src/Remesher.c
    src/SpatialTree.c
    src/Sync.c)

target_sources(StgDomain PRIVATE ${sources})