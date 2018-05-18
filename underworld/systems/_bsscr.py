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
from _options import Options
from mpi4py import MPI
import numpy as np


class Stats(object):
    pressure_its=0
    velocity_backsolve_its=0
    pressure_time=0.
    velocity_backsolve_time=0.
    total_time=0.
    total_flops=0.
    pressure_flops=0.
    velocity_backsolve_flops=0.
    vmin=0.
    vmax=0.
    pmin=0.
    pmax=0.
    p_sum=0.


class OptionsMG(Options):
    """
    Set Multigrid PETSc options

    active = <True,False>                             : activates Multigrid
    levels = <n>                                      : Multigrid grid levels
    pc_mg_type <additive,multiplicative,full,kaskade> : multiplicative is default
    pc_mg_cycle_type <v,w>                            : v or w
    pc_mg_multiplicative_cycles <n>                   : Sets the number of cycles to use for each preconditioner step of multigrid
    mg_levels_ksp_type <minres>                       : Krylov method
    mg_levels_ksp_max_its <n>                         : Maximum iterations for Krylov method
    mg_levels_ksp_convergence_test <default, skip>    :
    mg_levels_pc_type <sor>                           : Preconditioner type
    pc_mg_smoothup <n>                                : Number of smoothing steps after interpolation
    pc_mg_smoothdown <n>                              : Number of smoothing steps before applying restriction operator
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
        if not isinstance( field, uw.mesh.MeshVariable):
            raise TypeError( "Provided field must be of 'MeshVariable' class." )
        else:
            levels=1
            lvls=[]
            res_tuple=field.mesh.elementRes
            for res in res_tuple:
                levels=1
                while res%2 == 0:
                    res=res/2
                    levels += 1
                lvls.append(levels)
            self.levels=min(lvls)
            if self.levels < 2:
                raise TypeError("\n\n\033[1;35mMultigrid levels must be >= 2.\nNeed more multiples of 2 in mesh resolution.\033[00m\n" )

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
        self.mg_accelerating_smoothing_view=False
        self.mg_smooths_min = 1
        self.mg_smooths_max= 20
        self.mg_smooths_to_start= 1
        self.mg_smoothing_acceleration=1.1
        self.mg_smoothing_increment=1
        self.mg_target_cycles_10fold_reduction=5

class OptionsMain(Options):
    """
    penalty = 0                                       : Penalty number for Augmented Lagrangian
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

    def __init__(self, field, eqNum, levels=2, **kwargs):
        if not isinstance(levels, int) or levels < 1:
            raise TypeError( "'levels' must be positive integer.")
        if not isinstance( field, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'field' must be of 'MeshVariable' class." )
        if not isinstance( eqNum, sle.EqNumber):
            raise TypeError( "Provided 'eqNum' must be of 'EqNumber' class." )
        if not eqNum.meshVariable == field:
            raise ValueError("Supplied 'eqNum' doesn't correspond to the supplied 'meshVariable'")

        self._levels=levels
        self._field=field
        super(MGSolver, self).__init__(**kwargs)

        # attach the fine equation number via python
        self._mgGenerator.fineEqNum = eqNum._cself

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MGSolver,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][     "levels"] = self._levels
        componentDictionary[ self._cself.name ]["opGenerator"] = self._mgGenerator.name
        componentDictionary[ self._mgGenerator.name ]["fineVariable"] = self._field._cself.name

class StokesSolver(_stgermain.StgCompoundComponent):
    """
    The Block Stokes Schur Complement Solver:
    This solves the saddle-point system

    .. math::
        \\begin{bmatrix} K & G \\\\ G^T & C \\end{bmatrix} \\begin{bmatrix} u \\\\ p \\end{bmatrix} = \\begin{bmatrix}f \\\\ h \\end{bmatrix}

    via a Schur complement method.

    We first solve:

    .. math::
        S p= G^T  K^{-1} f - h,
       :label: a

    where :math:`S = G^T K^{-1} G-C`

    Then we backsolve for the velocity:

    .. math::
        K u = f - G p.
       :label: b

    The effect of :math:`K^{-1}` in :eq:`a` is obtained via a KSPSolve in PETSc.
    This has the prefix 'A11' (often called the 'inner' solve)

    The solve in :eq:`a` for the pressure has prefix 'scr'.

    Assuming the returned solver is called 'solver', it is possible to configure
    these solves individually via the `solver.options.A11` and
    `solver.options.scr` dictionaries.

    Try uw.help(solver.options.A11) for some details.

    Common configurations are provided via the
    solver.set_inner_method() function.

    solver.set_inner_method("mg") sets up a multigrid solve for the inner solve. This is the default.
    solver.set_inner_method("mumps") will set up a parallel direct solve on the inner solve.
    solver.set_inner_method("lu") will set up a serial direct solve on the inner solve.

    uw.help(solver.set_inner_method) for more.

    For more advanced configurations use the
    solver.options.A11/scr dictionaries directly.

    uw.help(solver.options) to see more.
    """
    _objectsDict = {  "_stokessolver" : "StokesBlockKSPInterface"  }
    _selfObjectName = "_stokessolver"
    _optionsStr=''

    def __init__(self, stokesSLE, **kwargs):
        if not isinstance(stokesSLE, uw.systems.Stokes):
            raise TypeError("Provided system must be of 'Stokes' class")

        self.options=OptionsGroup()
        self.options.A11.ksp_rtol=1e-6

        self._stokesSLE=stokesSLE

        velocityField=stokesSLE._velocityField
        pressureField=stokesSLE._pressureField

        if not isinstance( velocityField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'velocityField' must be of 'MeshVariable' class." )
        self._velocityField = velocityField
        if not isinstance( pressureField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'pressureField' must be of 'MeshVariable' class." )
        self._pressureField = pressureField

        self._velocityEqNums = stokesSLE._eqNums[velocityField]
        self._pressureEqNums = stokesSLE._eqNums[pressureField]

        super(StokesSolver, self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(StokesSolver,self)._add_to_stg_dict(componentDictionary)

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

    def solve(self, nonLinearIterate=None, nonLinearTolerance=1.0e-2,
              nonLinearKillNonConvergent=False,
              nonLinearMinIterations=1,
              nonLinearMaxIterations=500,
              callback_post_solve=None,
              print_stats=False, reinitialise=True, fpwarning=True, petscwarning=True, **kwargs):
        """
        Solve the stokes system

        Parameters
        ----------
        nonLinearIterate: bool
            True will perform non linear iterations iterations, False (or 0) will not

        nonLinearTolerance: float, Default=1.0e-2
            Relative tolerance criterion for the change in the velocity field

        nonLinearMaxIterations: int, Default=500
            Maximum number of non linear iteration to perform

        callback_post_sovle: func, Default=None
            Optional callback function to be performed at the end of a linear solve iteration.
            Commonly this will be used to perform operations between non linear iterations, for example,
            calibrating the pressure solution or removing the system null space.

        print_stats: bool, Default=False
            Print out solver iteration and timing counts per solver

        reinitialise: bool, Default=True,
            Rebuild the system discretisation storage (location matrix/petsc mats & vecs) and repopulate, if available,
            the stokes voronio swarm before the system is solved.
        """

        # clear the floating-point env registers. Should return 0
        rubbish = uw.libUnderworld.Underworld.Underworld_feclearexcept()

        Solvers.SBKSP_SetSolver(self._cself, self._stokesSLE._cself)
        if isinstance(self.options.main.penalty,float) and self.options.main.penalty > 0.0:
            Solvers.SBKSP_SetPenalty(self._cself, self.options.main.penalty)
            if self.options.main.ksp_k2_type == "NULL":
                self.options.main.ksp_k2_type = "GMG"

        # if user provided callback_post_solve then use it, else use stokes system
        if callback_post_solve is not None:
            if not callable(callback_post_solve):
                raise RuntimeError("The 'callback_post_solve' parameter is not 'None' and isn't callable")
        else:
            callback_post_solve = self._stokesSLE.callback_post_solve

        # in this c function we handle callback_post_solve=None
        uw.libUnderworld.StgFEM.SystemLinearEquations_SetCallback(self._stokesSLE._cself, callback_post_solve)

        if not isinstance(nonLinearTolerance, float) or nonLinearTolerance < 0.0:
            raise ValueError("'nonLinearTolerance' option must be of type 'float' and greater than 0.0")

        # Set up options string from dictionaries.
        # We set up here so that we can set/change terms on the dictionaries before we run solve
        # self.options.A11._mg_active is True by default but can be changed before here via
        # functions like set_lu

        self._setup_options(**kwargs)
        petsc.OptionsClear() # reset the petsc options
        petsc.OptionsInsertString(self._optionsStr)

        # set up MG
        if self.options.mg.active == True:
            self._setup_mg()

        # check for non-linearity
        nonLinear=self._check_linearity(nonLinearIterate)

        if nonLinear and nonLinearIterate:
            # self._stokesSLE._cself.nonLinearTolerance = nonLinearTolerance # set via python
            libUnderworld.StgFEM.SystemLinearEquations_SetNonLinearTolerance(self._stokesSLE._cself, nonLinearTolerance)
            libUnderworld.StgFEM.SystemLinearEquations_SetToNonLinear(self._stokesSLE._cself, True, )
            self._stokesSLE._cself.nonLinearMinIterations = nonLinearMinIterations
            self._stokesSLE._cself.nonLinearMaxIterations = nonLinearMaxIterations
            self._stokesSLE._cself.killNonConvergent = nonLinearKillNonConvergent

        else:
            libUnderworld.StgFEM.SystemLinearEquations_SetToNonLinear(self._stokesSLE._cself, False )



        if self._stokesSLE._swarm and reinitialise:
            self._stokesSLE._swarm._voronoi_swarm.repopulate()

        # set up objects on SLE
        if reinitialise:
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


        if print_stats:
            self.print_stats()
            if nonLinear and nonLinearIterate:
                if uw.rank()==0:
                    purple = "\033[0;35m"
                    endcol = "\033[00m"
                    boldpurple = "\033[1;35m"
                    print boldpurple
                    print( "Non linear iterations: %3d of 500 " % (self._stokesSLE._cself.nonLinearIteration_I) )
                    print endcol
                    print

        # check the petsc convergence reasons, see StokesBlockKSPInterface.h
        if petscwarning and (self._cself.fhat_reason < 0  or \
            self._cself.outer_reason < 0 or \
            self._cself.backsolve_reason < 0 ):
            import warnings

            estring = "A petsc error has been detected during the solve.\n" + \
            "The resultant solution fields are most likely erroneous, check them thoroughly.\n"+ \
            "This is possibly due to many things, for example solar flares or insufficient boundary conditions.\n"+ \
            "The resultant KSPConvergedReasons are (f_hat, outer, backsolve) ({},{},{}).".format(
                self._cself.fhat_reason,self._cself.outer_reason, self._cself.backsolve_reason)
            if uw.rank() == 0:
                warnings.warn(estring)

        # check if fp error was detected and 'reduce' result to proc 0

        lres, gres = np.zeros(1), np.zeros(1)

        lres[:] = uw.libUnderworld.Underworld.Underworld_fetestexcept()
        comm = MPI.COMM_WORLD
        comm.Allreduce(lres, gres, op=MPI.SUM)

        if gres[0] > 0 and fpwarning:
            import warnings
            estring = "A floating-point operation error has been detected during the solve.\n" + \
            "The resultant solution fields are most likely erroneous, check them thoroughly.\n"+ \
            "This is likely due to large number variations in the linear algrebra or fragile solver configurations.\n"+ \
            "Consider rescaling the fn_viscosity or fn_bodyforce inputs to avoid this problem.\n"+ \
            "This warning can be supressed with the argument 'fpwarning=False'."
            if uw.rank() == 0:
                warnings.warn(estring)
        return

    ########################################################################
    ### create vectors and matrices for augmented lagrangian solve
    ########################################################################
    def _create_penalty_objects(self):
        # using this function se we don't need to add anything extra to the stokeSLE struct

        velocityField=self._velocityField
        pressureField=self._pressureField
        stokesSLE = self._stokesSLE
        # create junk force vectors -- we provide no assembly terms for these so they are 0 vectors.
        self._vmfvector   = sle.AssembledVector(velocityField, stokesSLE._eqNums[velocityField])
        self._junkfvector = sle.AssembledVector(pressureField, stokesSLE._eqNums[pressureField])

        # and matrices
        self._vmmatrix = sle.AssembledMatrix( stokesSLE._velocitySol, stokesSLE._velocitySol, rhs=self._vmfvector )
        self._mmatrix  = sle.AssembledMatrix( stokesSLE._pressureSol, stokesSLE._pressureSol, rhs=self._junkfvector )

        # create assembly terms
        self._pressMassMatTerm = sle.MatrixAssemblyTerm_NA__NB__Fn( integrationSwarm=uw.swarm.GaussIntegrationSwarm(velocityField.mesh), fn=1.0, assembledObject=self._mmatrix,
                                                             mesh = velocityField._mesh)

        # attach terms to live solver struct
        self._cself.vmForceVec = self._vmfvector._cself
        self._cself.vmStiffMat = self._vmmatrix._cself
        self._cself.jForceVec  = self._junkfvector._cself
        self._cself.mStiffMat  = self._mmatrix._cself
    ########################################################################
    ### assemble vectors and matrices for augmented lagrangian solve
    ########################################################################
    def _setup_penalty_objects(self):
        # using this function so we don't need to add anything extra to the stokeSLE struct

        # zero the vectors
        petsc.SetVec(self._vmfvector._cself.vector, 0.0)  # this complains about memory leaks
        petsc.SetVec(self._junkfvector._cself.vector, 0.0)
        # vector set up
        libUnderworld.StgFEM.ForceVector_Assemble(self._vmfvector._cself)
        libUnderworld.StgFEM.ForceVector_Assemble(self._junkfvector._cself)
        # matrix set up
        libUnderworld.StgFEM.StiffnessMatrix_Assemble( self._vmmatrix._cself, self._stokesSLE._cself, None );
        libUnderworld.StgFEM.StiffnessMatrix_Assemble( self._mmatrix._cself,  self._stokesSLE._cself, None );
    ########################################################################
    ### setup options for solve
    ########################################################################
    def _setup_options(self, **kwargs):
        self._optionsStr=''
        # the A11._mg_active overrides the mg.active so we can set direct solve using A11 prefix
        # if A11._mg_active is true we can still deactivate mg using its own flag
        if self.options.A11._mg_active == False:
            self.options.mg.active = self.options.A11._mg_active
        self.options.scr._mg_active=0
        del self.options.scr._mg_active # not currently used

        for key, value in self.options.main.__dict__.iteritems():
            if key != 'penalty': # don't add penalty to petsc options
                if value == 'bfbt': # allowed alias
                    value = 'gtkg'
                if key != 'force_correction' or self.options.main.penalty > 0.0: # then add option
                    if key != 'k_scale_only' or self.options.main.rescale_equations==True:
                        self._optionsStr = self._optionsStr+" "+"-"+key+" "+str(value)

        for key, value in self.options.A11.__dict__.iteritems():
            if key != '_mg_active':
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

        if self.options.mg_accel.mg_accelerating_smoothing and self.options.mg.active:
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
        if self._velocityField in self._stokesSLE.fn_viscosity._underlyingDataItems:
            nonLinear = True
            message += "\nviscosity function depends on the velocity field provided to the Stokes system."
        if self._pressureField in self._stokesSLE.fn_viscosity._underlyingDataItems:
            nonLinear = True
            message += "\nviscosity function depends on the pressure field provided to the Stokes system."
        if self._velocityField in self._stokesSLE.fn_bodyforce._underlyingDataItems:
            nonLinear = True
            message += "\nBody force function depends on the velocity field provided to the Stokes system."
        if self._pressureField in self._stokesSLE.fn_bodyforce._underlyingDataItems:
            nonLinear = True
            message += "\nBody force function depends on the pressure field provided to the Stokes system."

        if self._stokesSLE._constitMatTerm.fn_visc2:
            if self._velocityField in self._stokesSLE._constitMatTerm.fn_visc2._underlyingDataItems:
                nonLinear = True
                message += "\nviscosity2 function depends on the velocity field provided to the Stokes system."

            if self._pressureField in self._stokesSLE._constitMatTerm.fn_visc2._underlyingDataItems:
                nonLinear = True
                message += "\nviscosity2 function depends on the pressure field provided to the Stokes system."

        message += "\nPlease set the 'nonLinearIterate' solve parameter to 'True' or 'False' to continue."

        if nonLinear and (nonLinearIterate==None):
            raise RuntimeError(message)


        return nonLinear

    ########################################################################
    ### set up MG
    ########################################################################

    def _setup_mg(self):

        field =self._velocityField
        eqNum = self._velocityEqNums

        # If the levels have been set explicitly then no point in over-riding
        if self.options.mg.levels == 0:
            self.options.mg.set_levels(field=field)

        mgObj=MGSolver(field,eqNum,self.options.mg.levels)
        # attach MG object to Solver struct
        self.mgObj=mgObj # must attach object here: else immediately goes out of scope and is destroyed
        self._cself.mg = mgObj._cself

    def set_inner_method(self, solve_type="mg"):
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
            self.options.A11.reset() # sets self.options.A11._mg_active=True
            self.options.mg.reset()
            self.options.mg.set_levels(field=velocityField)
        if solve_type=="mumps":
            self.options.A11.set_mumps()
        if solve_type=="lu":
            self.options.A11.set_lu()
        if solve_type=="superlu":
            self.options.A11.set_superlu()
        if solve_type=="superludist":
            self.options.A11.set_superludist()
        if solve_type=="nomg":
            self.options.A11.reset()
            self.options.mg.reset()
            self.options.A11._mg_active=False

    def set_inner_rtol(self, rtol):
        self.options.A11.ksp_rtol=rtol

    def set_outer_rtol(self, rtol):
        self.options.scr.ksp_rtol=rtol

    def set_mg_levels(self, levels):
        """
        Set the number of multigrid levels manually.
        It is set automatically by default.
        """
        self.options.mg.levels=levels

    def get_stats(self):
        return self._cself.stats

    def get_nonLinearStats(self):

        class blank(object):
            pass

        a = blank()

        a.picard_iterations = self._stokesSLE._cself.nonLinearIteration_I
        a.picard_residual =   self._stokesSLE._cself.curResidual

        return a

    def print_stats(self):
        purple = "\033[0;35m"
        endcol = "\033[00m"
        boldpurple = "\033[1;35m"
        if 0==uw.rank():
            print boldpurple
            print( " " )
            print( "Pressure iterations: %3d" % (self._cself.stats.pressure_its) )
            print( "Velocity iterations: %3d (presolve)      " % (self._cself.stats.velocity_presolve_its) )
            print( "Velocity iterations: %3d (pressure solve)" % (self._cself.stats.velocity_pressuresolve_its) )
            print( "Velocity iterations: %3d (backsolve)     " % (self._cself.stats.velocity_backsolve_its) )
            print( "Velocity iterations: %3d (total solve)   " % (self._cself.stats.velocity_total_its) )
            print( " " )
            print( "SCR RHS  solve time: %.4e" %(self._cself.stats.velocity_presolve_time) )
            print( "Pressure solve time: %.4e" %(self._cself.stats.pressure_time) )
            print( "Velocity solve time: %.4e (backsolve)" %(self._cself.stats.velocity_backsolve_time) )
            print( "Total solve time   : %.4e" %(self._cself.stats.total_time) )
            print( " " )
            print( "Velocity solution min/max: %.4e/%.4e" % (self._cself.stats.vmin,self._cself.stats.vmax) )
            print( "Pressure solution min/max: %.4e/%.4e" % (self._cself.stats.pmin,self._cself.stats.pmax) )
            print( " " )
            print endcol

    def set_penalty(self, penalty):
        """
        By setting the penalty, the Augmented Lagrangian Method is used as the solve.
        This method is not recommended for normal use as there is additional memory and cpu overhead.
        This method can often help improve convergence issues for problems with large viscosity
        contrasts that are having trouble converging.

        A penalty of roughly 0.1 of the maximum viscosity contrast is not a bad place to start as a guess. (check notes/paper)
        """
        if isinstance(self.options.main.penalty, float) and self.options.main.penalty >= 0.0:
            self.options.main.penalty=penalty
            self.options.main.Q22_pc_type="gkgdiag"
        elif 0==uw.rank():
            print( "Invalid penalty number chosen. Penalty must be a positive float." )

    def _debug(self):
        import pdb; pdb.set_trace()
