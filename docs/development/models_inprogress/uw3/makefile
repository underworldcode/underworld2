#Currently 
H52XDMF=${PETSC_DIR}/lib/petsc/bin/petsc_gen_xdmf.py

# $^ represents the right-side of the make rule

run_debug: poisson3D.py
	cgdb --args python $^ -mesh_file 11_6.mesh.dat -temperature_petscspace_order 1 -dm_view hdf5:ss.h5 -dm_partition_view -initial_vec_view hdf5:ss.h5::append -sol_vec_view hdf5:ss.h5::append -snes_monitor -ksp_converged_reason -aux_0_petscspace_order 1

run_poisson: poisson3D.py
	mpirun -np 3 python $^ -mesh_file mesh.dat -temperature_petscspace_order 1 -dm_view hdf5:ss.h5 -dm_partition_view -initial_vec_view hdf5:ss.h5::append -sol_vec_view hdf5:ss.h5::append -dmAux_view hdf5:aux.h5 -aux_vec_view hdf5:aux.h5::append -snes_monitor -ksp_converged_reason -aux_0_petscspace_order 1

	${H52XDMF} ss.h5
	${H52XDMF} aux.h5


run_stokes_sphere: stokesSinker3D.py
	mpirun -np 2 python $^ -mesh_file 11_6.mesh.dat -dm_partition_view       -pc_type jacobi       -snes_monitor -snes_converged_reason -ksp_error_if_not_converged 0 -ksp_converged_reason       -velocity_petscspace_order 1 -pressure_petscspace_order 0
	#mpirun -np 2 python stokesSinker3D.py -mesh_file 22_12.mesh.dat -dm_partition_view       -pc_type jacobi       -snes_monitor -snes_converged_reason -ksp_error_if_not_converged 0 -ksp_converged_reason       -u_petscspace_order 1 -p_petscspace_order 0

run_cube: cube_stokesSinker3D.py
	mpirun -np 3 python $^ -options_file simple_options.dat
	${H52XDMF} aux.h5 sol.h5

clean:
	rm run sol1.*
# includes should go after targets
include ${PETSC_DIR}/lib/petsc/conf/variables
