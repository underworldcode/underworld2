cmake_minimum_required(VERSION 3.16)

set(sources
    src/EntryPoint.c
    src/Finalise.c
    src/FiniteElementContext.c
    src/ForceTerm.c
    src/ForceVector.c
    src/Init.c
    src/MGOpGenerator.c
    src/PETScMGSolver.c
    src/SLE_Solver.c
    src/SolutionVector.c
    src/SROpGenerator.c
    src/StiffnessMatrix.c
    src/StiffnessMatrixTerm.c
    src/SystemLinearEquations.c
    )

target_sources(StgFEM PRIVATE ${sources})