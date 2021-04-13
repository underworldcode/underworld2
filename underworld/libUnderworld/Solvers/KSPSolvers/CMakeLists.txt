cmake_minimum_required(VERSION 3.16)

set(sources
    src/Finalise.c
    src/Init.c
    src/ksp-register.c
    src/StokesBlockKSPInterface.c
    src/BSSCR/auglag-driver-DGTGD.c
    src/BSSCR/BSSCR.c
    src/BSSCR/bsscr-driver.c
    src/BSSCR/create_execute_script.c
    src/BSSCR/createK2.c
    src/BSSCR/create_petsc_header.c
    src/BSSCR/ksp_pressure_nullspace.c
    src/BSSCR/ksp_scale.c
    src/BSSCR/list_operations.c
    src/BSSCR/mg.c
    src/BSSCR/operator_summary.c
    src/BSSCR/pc_GtKG.c
    src/BSSCR/pc_ScaledGtKG.c
    src/BSSCR/preconditioner.c
    src/BSSCR/register_stokes_solvers.c
    src/BSSCR/solver_output.c
    src/BSSCR/stokes_block_scaling.c
    src/BSSCR/stokes_Kblock_scaling.c
    src/BSSCR/stokes_mvblock_scaling.c
    src/BSSCR/Stokes_Nullspace.c
    src/BSSCR/stokes_output.c
    src/BSSCR/stokes_residual.c
    src/BSSCR/summary.c
    src/BSSCR/timed_residual_hist.c
    src/BSSCR/writeMatVec.c
    src/Test/TestKSP.c
    )

target_sources(Solvers PRIVATE ${sources})