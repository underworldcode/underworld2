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
from _options import Options

class HeatSolver(_stgermain.StgCompoundComponent):
    """
    Steady State Heat Equation Solver.
    """

    _objectsDict = {"_heatsolver" : "Energy_SLE_Solver" }
    _selfObjectName = "_heatsolver"

    def __init__(self, heatSLE, **kwargs):
        if not isinstance(heatSLE, (uw.systems.SteadyStateHeat, uw.utils.MeshVariable_Projection)):
            raise TypeError("Provided system must be of 'SteadyStateHeat' class")
        self._heatSLE=heatSLE

        self.options=OptionsGroup()

        super(HeatSolver, self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(HeatSolver,self)._add_to_stg_dict(componentDictionary)

    def solve(self, nonLinearIterate=None, **kwargs):
        """ Solve the sle using provided solver.
        """
        # Set up options string from dictionaries.
        # We set up here so that we can set/change terms on the dictionaries before we run solve
        self._setup_options(**kwargs)
        petsc.OptionsClear() # reset the petsc options
        petsc.OptionsInsertString(self._optionsStr)

        #self._heatSLE._cself.solver=self._cself
        libUnderworld.StgFEM.Energy_SLE_Solver_SetSolver(self._cself, self._heatSLE._cself) # this sets solver on SLE struct.
        # check for non-linearity
        nonLinear = False

        if nonLinear and nonLinearIterate:
            libUnderworld.StgFEM.SystemLinearEquations_SetToNonLinear(self._heatSLE._cself, True )
        else:
            libUnderworld.StgFEM.SystemLinearEquations_SetToNonLinear(self._heatSLE._cself, False )

        if self._heatSLE._swarm:
            self._heatSLE._swarm._voronoi_swarm.repopulate()

        libUnderworld.StgFEM.SystemLinearEquations_BC_Setup(self._heatSLE._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_LM_Setup(self._heatSLE._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_ZeroAllVectors(self._heatSLE._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_MatrixSetup(self._heatSLE._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_VectorSetup(self._heatSLE._cself, None)
        if nonLinear and nonLinearIterate:
            libUnderworld.StgFEM.SystemLinearEquations_NonLinearExecute(self._heatSLE._cself, None)
        else:
            libUnderworld.StgFEM.SystemLinearEquations_ExecuteSolver(self._heatSLE._cself, None)

        libUnderworld.StgFEM.SystemLinearEquations_UpdateSolutionOntoNodes(self._heatSLE._cself, None)

    ########################################################################
    ### setup options for solve
    ########################################################################
    def _setup_options(self, **kwargs):
        self._optionsStr=''
        for key, value in self.options.EnergySolver.__dict__.iteritems():
            self._optionsStr += " "+"-EnergySolver_"+key+" "+str(value)

        for key, value in kwargs.iteritems():      # kwargs is a regular dictionary
            self._optionsStr += " "+"-"+key+" "+str(value)

    def configure(self,solve_type=""):
        """
        Configure velocity/inner solver (A11 PETSc prefix).

        solve_type can be one of:

        mumps       : MUMPS parallel direct solver.
        superludist : SuperLU parallel direct solver.
        superlu     : SuperLU direct solver (serial only).
        lu          : LU direct solver (serial only).
        """
        if not isinstance(solve_type,str):
            raise TypeError("Solver type must be provided as a string. \
                             \nCheck help for list of available solvers.")
        _solve_type = solve_type.lower()
        if _solve_type=="mumps":
            self.options.EnergySolver.set_mumps()
        elif _solve_type=="lu":
            self.options.EnergySolver.set_lu()
        elif _solve_type=="superlu":
            self.options.EnergySolver.set_superlu()
        elif _solve_type=="superludist":
            self.options.EnergySolver.set_superludist()
        else:
        # shouldn't get here
        
            raise RuntimeError("Provided solver type not supported. \
                                \nCheck help for list of available solvers.")


class OptionsGroup(object):
    """
    EnergySolver   : KSP solver options
    """
    def __init__(self):
        self.EnergySolver=Options()
