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
import _bsscr
import _energy_solver

class Solver(_stgermain.StgCompoundComponent):
    _objectsDict = {  "_solver" : None  }
    _selfObjectName = "_solver"    

    def __init__(self, **kwargs):
        super(Solver, self).__init__(**kwargs)

    @staticmethod
    def factory(eqs,type="BSSCR", *args, **kwargs):
        """
        This method simply returns a necessary solver for the provided system. 
        For information on the specific solvers:
            Stokes: See within the _bsscr module.
            Thermal: See within the _energy_solver module.
        """
        if isinstance(eqs, uw.systems.Stokes):
            return _bsscr.StokesSolver(eqs, *args, **kwargs)
        elif isinstance(eqs, (uw.systems.SteadyStateHeat, uw.utils.MeshVariable_Projection) ):
            return _energy_solver.HeatSolver(eqs, *args, **kwargs)
        else:
            raise ValueError("Unable to create solver. Provided system not recognised.")

