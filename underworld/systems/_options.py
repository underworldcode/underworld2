class Options(object):
    """
    Set PETSc options on this to pass along to PETSc KSPs
    
    ksp_type = <fgmres>    : Krylov method
    ksp_rtol = <1e-05>     : Relative decrease in residual norm
    pc_type  = <sor>       : Preconditioner type
    ksp_view = 'ascii'     : Print the ksp data structure at the end of the system solution
    ksp_converged_reason = 'ascii' : Print reason for converged or diverged solve
    ksp_monitor = <stdout> : Monitor preconditioned residual norm

    for further options see PETSc manual or set help on "options.main"
    """
    def __init__(self):
        self.reset()

    def list(self):
        """
        List options.
        """
        dd=self.__dict__
        for key, value in dd.iteritems():
            print(key, value)
    
    def help(self):
        print(self.__doc__)
        
    def reset(self):
        """
        Reset values to initial defaults.
        """
        self.__dict__.clear()
        self.ksp_type="fgmres"
        self.ksp_rtol=1e-5
        self._mg_active=True

    def set_lu(self):
        """
        Set up options for LU serial solve.
        """
        self.__dict__.clear()
        self.ksp_type="preonly"
        self.pc_type="lu"
        self._mg_active=False

    def set_mumps(self,pc_type="lu"):
        """
        Set up options for MUMPS parallel solve.
        pc_type = "lu" or "cholesky"
        
        Use ./configure --download-mumps --download-scalapack --download-parmetis --download-metis --download-ptscotch
        to have PETSc installed with MUMPS
        """
        self.__dict__.clear()
        self.ksp_type="preonly"
        self.pc_type=pc_type
        self._mg_active=False
        self.pc_factor_mat_solver_package="mumps"
        # An issue with MUMPS is that it requires preallocation of arrays, and as
        # such occasionally it will report errors when the arrays are not big
        # enough. Add in this line to increase the array size, and if errors are still
        # reported increase the number.
        # -mat_mumps_icntl_14 100
        # cannot set this here....must be without A11 prefix

    def set_superludist(self):
        """
        Set up options for SuperLU parallel solve.
        Use ./configure --download-superlu_dist --download-parmetis --download-metis --download-ptscotch
        to have PETSc installed with SuperLU_DIST
        """
        self.__dict__.clear()
        self.ksp_type="preonly"
        self.pc_type="lu"
        self._mg_active=False
        self.pc_factor_mat_solver_package="superlu_dist"

    def set_superlu(self):
        """
        Set up options for SuperLU serial solve.
        Use ./configure --download-superlu to have PETSc installed with SuperLU
        """
        self.__dict__.clear()
        self.ksp_type="preonly"
        self.pc_type="lu"
        self._mg_active=False
        self.pc_factor_mat_solver_package="superlu"
