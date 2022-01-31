cmake_minimum_required(VERSION 3.16)

set(sources
    src/Binary.cpp
    src/Conditional.cpp
    src/Constant.cpp
    src/CustomException.cpp
    src/DiscreteCoordinate.cpp
    src/FEMCoordinate.cpp
    src/FeVariableFn.cpp
    src/FunctionIO.cpp
    src/GradFeVariableFn.cpp
    src/IOIterators.cpp
    src/Map.cpp
    src/MeshCoordinate.cpp
    src/MinMax.cpp
    src/ParticleCoordinate.cpp
    src/ParticleFound.cpp
    src/ParticleInCellCoordinate.cpp
    src/Query.cpp
    src/SafeMaths.cpp
    src/Shape.cpp
    src/SwarmVariableFn.cpp
    src/Tensor.cpp
    src/Unary.cpp
    )

target_sources(Underworld PRIVATE ${sources})