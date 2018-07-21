#Currently 
PETSC_DIR=/usr/local/petsc-repo
H52XDMF=${PETSC_DIR}/lib/petsc/bin/petsc_gen_xdmf.py

run_stokes_cube: stokesSinker3D.py
	mpirun -oversubscribe -np 2 python stokesSinker3D.py -mesh_file 11_6.mesh.dat -dm_partition_view       -pc_type jacobi       -snes_monitor -snes_converged_reason -ksp_error_if_not_converged 0 -ksp_converged_reason       -velocity_petscspace_order 1 -pressure_petscspace_order 0

run_cube: stokesSinker3D.py
	mpirun -oversubscribe -np 3 python stokesSinker3D.py -options_file simple_options.dat

run_poisson: poisson3D.py
	mpirun -oversubscribe -np 3 python poisson3D.py -mesh_file 11_6.mesh.dat -temperature_petscspace_order 1 -dm_view hdf5:ss.h5 -dm_partition_view -initial_vec_view hdf5:ss.h5::append -sol_vec_view hdf5:ss.h5::append -snes_monitor -ksp_converged_reason -aux_0_petscspace_order 1

	${H52XDMF} ss.h5

clean:
	rm run sol1.*
# includes should go after targets
include ${PETSC_DIR}/lib/petsc/conf/variables
