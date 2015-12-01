##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld as uw
import underworld._stgermain as _stgermain
import sle
import libUnderworld
from libUnderworld import petsc
from libUnderworld import Solvers

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

    def clear(self):
        """
        Clear all options.
        """
        self.__dict__.clear()

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

class OptionsMG(Options):
    """
    Set Multigrid PETSc options
    
    active = <True,False>           : activates Multigrid
    levels = <n>                    : Multigrid grid levels
    pc_mg_type  <additive,multiplicative,full,kaskade> : multiplicative is default
    pc_mg_cycle_type <v,w>          : v or w
    pc_mg_multiplicative_cycles <n> : Sets the number of cycles to use for each preconditioner 
                                      step of multigrid
    mg_levels_ksp_type <minres>     : Krylov method
    mg_levels_ksp_max_its <n>       : Maximum iterations for Krylov method
    mg_levels_ksp_convergence_test <default, skip>  :
    mg_levels_pc_type <sor>         : Preconditioner type
    pc_mg_smoothup <n>              : Number of smoothing steps after interpolation
    pc_mg_smoothdown <n>            : Number of smoothing steps before applying restriction operator
    """
    def reset(self):
        """
        Reset values to initial defaults.
        """
        self.__dict__.clear()
        self.levels=0
        self.active=True
        # add to A11 ksp when MG active
        #self.pc_mg_type="multiplicative"
        #self.pc_mg_type="additive"
        #self.pc_mg_type="additive"
        #self.pc_mg_cycle_type="v"
        #self.pc_mg_multiplicative_cycles=1
        #self.mg_levels_ksp_type="minres"
        #self.mg_levels_ksp_max_its=3
        #self.mg_levels_ksp_convergence_test="skip"
        #self.mg_levels_pc_type="sor"
        #self.pc_mg_smoothup= 5
        #self.pc_mg_smoothdown= 5
    def set_levels(self, field=''):
        """
        Automatically set Multigrid levels based off mesh resolution.
        """
        if not isinstance( field, uw.fevariable.FeVariable):
            raise TypeError( "Provided field must be of 'FeVariable' class." )
        else:
            levels=0
            lvls=[] 
            res_tuple=field.feMesh.elementRes
            for res in res_tuple:
                levels=0
                while res%2 == 0:
                    res=res/2
                    levels += 1
                lvls.append(levels)
            self.levels=min(lvls)
            if self.levels < 2:
                raise TypeError("\nMultigrid levels must be >= 2.\nNeed more multiples of 2 in mesh resolution." )

class OptionsMGA(Options):
    """
    The accelerating MG is one of the best ways to kill off nasty problems effectively. Some tuning helps
    because you can bracket low and high smoothing values if you have seen what works. But a wide range can
    be very effective, so that's what we set by default.
    
    mg_accelerating_smoothing = <True,False> : Activate accelerating multigrid
    mg_smoothing_adjust_on_convergence_rate = <True,False>
    mg_accelerating_smoothing_view = = <True,False>
    
    Range of values for the up / down smooth and 
    where to start at the beginning of each new iteration - if you have experience
    that a particular solution needs a lot of iterations then you can help the 
    algorithm out by suggesting it starts high.
    
    mg_smooths_min = <n>
    mg_smooths_max = <n>
    mg_smooths_to_start = <n>
    
    The manner in which the smoothing cycles changes as the problem gets easier or harder. 
    The specified acceleration is a factor which increases or decreases the number of cycles 
    to smooths * or / acceleration
    The specified increment increases or decreases the number of cycles to smooths + or - increment.
    Should be a big number if a lot of variation is seen in the problem.
    
    mg_smoothing_acceleration = 1.1
    mg_smoothing_increment = <n>
    
    This is a target which says we'll try to get at least one order of magnitude reduction in 
    residual over this number of V cycles with the fiddling about in smoothing, but not more than
    two orders. This is to allow us to progress to smaller, cheaper operations when the calculation 
    is easy

    mg_target_cycles_10fold_reduction = <n>
    
    """
    def reset(self):
        """
        Reset values to initial defaults.
        """
        self.__dict__.clear()
        self.mg_accelerating_smoothing=False
        self.mg_smoothing_adjust_on_convergence_rate=False
        # self.mg_accelerating_smoothing_view=True
        self.mg_smooths_min = 2 
        self.mg_smooths_max= 200
        self.mg_smooths_to_start= 3
        self.mg_smoothing_acceleration=1.1
        self.mg_smoothing_increment=1
        self.mg_target_cycles_10fold_reduction=5

class OptionsMain(Options):
    """
    penalty = 0                  : Penalty number for Augmented Lagrangian       
    Q22_pc_type = <"uw","uwscale", "gkgdiag", "bfbt"> : Schur preconditioner operators
    force_correction = <True,False>                   : Correct force term for Augmented Lagrangian
    rescale_equations = <True,False>                  : Use scaling on matrices
    k_scale_only = <True,False>                       : Only scale Velocity matrix
    remove_constant_pressure_null_space = <True,False> 
    change_backsolve = <True,False>                   : Activate backsolveA11 options
    change_A11rhspresolve = <True,False>              : Activate rhsA11 options
    restore_K = <True,False>                          : Restore K matrix before velocity back solve
    """
    def reset(self):
        """
        Reset values to initial defaults.
        """
        self.Q22_pc_type = "uw"
        self.force_correction = True
        self.ksp_type = "bsscr"
        self.pc_type = "none"
        self.ksp_k2_type = "NULL"
        self.rescale_equations = False
        self.k_scale_only = True
        self.remove_constant_pressure_null_space = False
        self.change_backsolve = False
        self.change_A11rhspresolve = False
        self.penalty = 0.0
        self.restore_K = True

class OptionsGroup(object):
    """
    A collection of options
    
    A11           : Velocity KSP solver options
    scr           : Configures the Schur complement (pressure) KSP solver
    mg            : Configures multigrid on the velocity solves
    mg_accel      : Accelerating multigrid options
    main          : Configures the top level KSP as well as miscellaneous options
    rhsA11        : Options for the Schur right-hand-side A11 ksp pre-solve
    backsolveA11  : Options for the velocity back solve
    """
    def __init__(self):
        self.A11  =Options()
        self.scr  =Options()
        self.main =OptionsMain()
        self.mg   =OptionsMG()
        self.mg_accel =OptionsMGA()
        self.rhsA11   =Options()
        self.backsolveA11=Options()


class MGSolver(_stgermain.StgCompoundComponent):
    """
    """
    _objectsDict = {  "_mgSolver" : "PETScMGSolver",
                      "_mgGenerator" : "SROpGenerator"      }
    _selfObjectName = "_mgSolver"

    def __init__(self, field, levels=2, **kwargs):
        if not isinstance(levels, int) or levels < 1:
            raise TypeError( "'levels' must be positive integer.")
        if not isinstance( field, uw.fevariable.FeVariable):
            raise TypeError( "Provided 'field' must be of 'FeVariable' class." )

        self._levels=levels
        self._field=field
        super(MGSolver, self).__init__(**kwargs)
        
    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MGSolver,self)._add_to_stg_dict(componentDictionary)
        #import pdb;
        #pdb.set_trace()
        componentDictionary[ self._cself.name ][     "levels"] = self._levels
        componentDictionary[ self._cself.name ]["opGenerator"] = self._mgGenerator.name
        componentDictionary[ self._mgGenerator.name ]["fineVariable"] = self._field._cself.name     

class Solver(_stgermain.StgCompoundComponent):
    """
    """
    _objectsDict = {  "_solver" : "StokesBlockKSPInterface"  }
    _selfObjectName = "_solver"    
    _optionsStr=''

    def __init__(self, stokesSLE, **kwargs):
        #import pdb;
        #pdb.set_trace()

        self.options=OptionsGroup()
        self._stokesSLE=stokesSLE

        velocityField=stokesSLE._velocityField
        pressureField=stokesSLE._pressureField
        
        self.options.mg.set_levels(field=velocityField)

        if not isinstance( velocityField, uw.fevariable.FeVariable):
            raise TypeError( "Provided 'velocityField' must be of 'FeVariable' class." )
        self._velocityField = velocityField
        if not isinstance( pressureField, uw.fevariable.FeVariable):
            raise TypeError( "Provided 'pressureField' must be of 'FeVariable' class." )
        self._pressureField = pressureField

        super(Solver, self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(Solver,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][       "Preconditioner"] = self._stokesSLE._preconditioner._cself.name
        componentDictionary[ self._cself.name ][            "stokesEqn"] = self._stokesSLE._cself.name
        componentDictionary[ self._cself.name ]["2ndStressTensorMatrix"] = None # used when we assemble K2 directly
        componentDictionary[ self._cself.name ][       "2ndForceVector"] = None # used when we assemble K2 directly
        componentDictionary[ self._cself.name ][        "penaltyNumber"] = None #self.options.main.penalty
        componentDictionary[ self._cself.name ][           "MassMatrix"] = None #self._mmatrix._cself.name
        componentDictionary[ self._cself.name ][      "JunkForceVector"] = None #self._junkfvector._cself.name
        componentDictionary[ self._cself.name ][   "VelocityMassMatrix"] = None #self._vmmatrix._cself.name
        componentDictionary[ self._cself.name ][     "VMassForceVector"] = None #self._vmfvector._cself.name


    ########################################################################
    ### the solve function
    ########################################################################
    def solve(self, nonLinearIterate=None, **kwargs):
        """ solve the Stokes system
        """
        Solvers.SBKSP_SetSolver(self._cself, self._stokesSLE._cself)
        if isinstance(self.options.main.penalty,float) and self.options.main.penalty > 0.0:
            Solvers.SBKSP_SetPenalty(self._cself, self.options.main.penalty)
            if self.options.main.ksp_k2_type == "NULL":
                self.options.main.ksp_k2_type = "GMG"

        # Set up options string from dictionaries.
        # We set up here so that we can set/change terms on the dictionaries before we run solve
        self._setup_options(**kwargs)
        petsc.OptionsClear() # reset the petsc options
        petsc.OptionsInsertString(self._optionsStr)
        # set up MG
        if self.options.mg.active == True:
            self._setup_mg()
        # check for non-linearity
        nonLinear=self._check_linearity(nonLinearIterate)
        if nonLinear and nonLinearIterate:
            libUnderworld.StgFEM.SystemLinearEquations_SetToNonLinear(self._stokesSLE._cself, True )
        else:
            libUnderworld.StgFEM.SystemLinearEquations_SetToNonLinear(self._stokesSLE._cself, False )
        #import pdb;
        #pdb.set_trace()
        if self._stokesSLE._PICSwarm:
            self._stokesSLE._PICSwarm.repopulate()

        # set up objects on SLE
        libUnderworld.StgFEM.SystemLinearEquations_BC_Setup(self._stokesSLE._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_LM_Setup(self._stokesSLE._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_ZeroAllVectors(self._stokesSLE._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_MatrixSetup(self._stokesSLE._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_VectorSetup(self._stokesSLE._cself, None)

        # setup penalty specific objects
        if isinstance(self.options.main.penalty, float) and self.options.main.penalty > 0.0:
            self._create_penalty_objects()
            self._setup_penalty_objects()

        # solve
        if nonLinear and nonLinearIterate:
            libUnderworld.StgFEM.SystemLinearEquations_NonLinearExecute(self._stokesSLE._cself, None)
        else:
            libUnderworld.StgFEM.SystemLinearEquations_ExecuteSolver(self._stokesSLE._cself, None)

        libUnderworld.StgFEM.SystemLinearEquations_UpdateSolutionOntoNodes(self._stokesSLE._cself, None)

    ########################################################################
    ### create vectors and matrices for augmented lagrangian solve
    ########################################################################
    def _create_penalty_objects(self):
        # using this function se we don't need to add anything extra to the stokeSLE struct

        velocityField=self._velocityField
        pressureField=self._pressureField
        stokesSLE = self._stokesSLE
        # create junk force vectors -- we provide no assembly terms for these so they are 0 vectors.
        self._vmfvector   = sle.AssembledVector(velocityField)
        self._junkfvector = sle.AssembledVector(pressureField)

        # and matrices
        self._vmmatrix = sle.AssembledMatrix( velocityField, velocityField, rhs=self._vmfvector )
        self._mmatrix  = sle.AssembledMatrix( pressureField, pressureField, rhs=self._junkfvector )
 
        # create assembly terms
        self._velocMassMatTerm = sle.VelocityMassMatrixTerm( integrationSwarm=stokesSLE._gaussSwarm, assembledObject=self._vmmatrix)
        self._pressMassMatTerm = sle.PressureMassMatrixTerm( integrationSwarm=stokesSLE._gaussSwarm, assembledObject=self._mmatrix,
                                                             mesh = velocityField._feMesh)
        
        # attach terms to live solver struct
        self._cself.vmForceVec = self._vmfvector._cself
        self._cself.vmStiffMat = self._vmmatrix._cself
        self._cself.jForceVec  = self._junkfvector._cself
        self._cself.mStiffMat  = self._mmatrix._cself
    ########################################################################
    ### assemble vectors and matrices for augmented lagrangian solve
    ########################################################################
    def _setup_penalty_objects(self):
        # using this function se we don't need to add anything extra to the stokeSLE struct

        # zero the vectors
        petsc.SetVec(self._vmfvector._cself.vector, 0.0)  # this complains about memory leaks
        petsc.SetVec(self._junkfvector._cself.vector, 0.0)
        # vector set up
        libUnderworld.StgFEM.ForceVector_Assemble(self._vmfvector._cself)
        libUnderworld.StgFEM.ForceVector_Assemble(self._junkfvector._cself)
        # matrix set up
        libUnderworld.StgFEM.StiffnessMatrix_Assemble( self._vmmatrix._cself, self._stokesSLE._cself.removeBCs, self._stokesSLE._cself, None );
        libUnderworld.StgFEM.StiffnessMatrix_Assemble( self._mmatrix._cself,  self._stokesSLE._cself.removeBCs, self._stokesSLE._cself, None );
    ########################################################################
    ### setup options for solve
    ########################################################################
    def _setup_options(self, **kwargs):
        self._optionsStr=''

        # the A11._mg_active overrides the mg.active so we can set direct solve using A11 prefix
        # if A11._mg_active is true we can still deactivate mg using its own flag
        if self.options.A11._mg_active == False:
            self.options.mg.active = self.options.A11._mg_active
        
        for key, value in self.options.main.__dict__.iteritems():
            if key != 'penalty':
                if value == 'bfbt':
                    value = 'gtkg'
                self._optionsStr = self._optionsStr+" "+"-"+key+" "+str(value)

        for key, value in self.options.A11.__dict__.iteritems():
            self._optionsStr = self._optionsStr+" "+"-A11_"+key+" "+str(value)

        for key, value in self.options.scr.__dict__.iteritems():
            self._optionsStr = self._optionsStr+" "+"-scr_"+key+" "+str(value)

        if self.options.main.change_backsolve:
            for key, value in self.options.backsolveA11.__dict__.iteritems():
                self._optionsStr = self._optionsStr+" "+"-backsolveA11_"+key+" "+str(value)

        if self.options.main.change_A11rhspresolve:
            for key, value in self.options.rhsA11.__dict__.iteritems():
                self._optionsStr = self._optionsStr+" "+"-rhsA11_"+key+" "+str(value)

        if self.options.mg.active:
            for key, value in self.options.mg.__dict__.iteritems():
                if key != 'active' and key != 'levels':
                    self._optionsStr = self._optionsStr+" "+"-A11_"+key+" "+str(str(value))
            self.options._mgLevels=self.options.mg.levels # todo dynamically set mgLevels.
        else:
            self._optionsStr = self._optionsStr+" "+"-A11_"+"mg_active"+" "+"False"

        if self.options.mg_accel.mg_accelerating_smoothing  and self.options.mg.active:
            for key, value in self.options.mg_accel.__dict__.iteritems():
                if key != 'active' and key != 'levels':
                    self._optionsStr = self._optionsStr+" "+"-"+key+" "+str(str(value))
            
        for key, value in kwargs.iteritems():      # kwargs is a regular dictionary
            self._optionsStr = self._optionsStr+" "+"-"+key+" "+str(value)
        
    ########################################################################
    ### check functional dependence of objects in solve
    ########################################################################
    def _check_linearity(self, nonLinearIterate):

        nonLinear = False

        message = "Nonlinearity detected."
        if self._velocityField in self._stokesSLE.viscosityFn._underlyingDataItems:
            nonLinear = True
            message += "\nviscosity function depends on the velocity field provided to the Stokes system."
        if self._pressureField in self._stokesSLE.viscosityFn._underlyingDataItems:
            nonLinear = True
            message += "\nviscosity function depends on the pressure field provided to the Stokes system."
        if self._velocityField in self._stokesSLE.bodyForceFn._underlyingDataItems:
            nonLinear = True
            message += "\nBody force function depends on the velocity field provided to the Stokes system."
        if self._pressureField in self._stokesSLE.bodyForceFn._underlyingDataItems:
            nonLinear = True
            message += "\nBody force function depends on the pressure field provided to the Stokes system."

        message += "\nPlease set the 'nonLinearIterate' solve parameter to 'True' or 'False' to continue."
        if nonLinear and (nonLinearIterate==None):
            raise RuntimeError(message)
     

        return nonLinear
        
    ########################################################################
    ### set up MG
    ########################################################################
    def _setup_mg(self):
        field =self._velocityField
        levels=self.options.mg.levels
        mgObj=MGSolver(field,levels)
        # attach MG object to Solver struct
        self.mgObj=mgObj # must attach object here: else immediately goes out of scope and is destroyed
        self._cself.mg = mgObj._cself

    def configure(self,solve_type="mg"):
        """
        Configure velocity/inner solver (A11 PETSc prefix).

        solve_type can be one of:

        mg          : Geometric multigrid (default).
        mumps       : MUMPS parallel direct solver.
        superludist : SuperLU parallel direct solver.
        superlu     : SuperLU direct solver (serial only).
        lu          : LU direct solver (serial only).
        """
        if solve_type=="mg":
            velocityField=self._stokesSLE._velocityField
            #self.options.A11.clear()
            #self.options.A11._mg_active=True
            self.options.A11.reset()
            self.options.mg.reset()
            self.options.mg.set_levels(field=velocityField)
            #self.options.mg.list()
            #self.options.A11.list()
        if solve_type=="mumps":
            self.options.A11.set_mumps()
        if solve_type=="lu":
            self.options.A11.set_lu()
        if solve_type=="superlu":
            self.options.A11.set_superlu()
        if solve_type=="superludist":
            self.options.A11.set_superludist()

