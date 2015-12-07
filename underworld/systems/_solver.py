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
    """
    """
    _objectsDict = {  "_solver" : None  }
    _selfObjectName = "_solver"    

    def __init__(self, **kwargs):
        super(Solver, self).__init__(**kwargs)

    def factory(eqs,type="BSSCR"):
        if isinstance(eqs, uw.systems.Stokes):
            return _bsscr.StokesSolver(eqs)
        if isinstance(eqs, uw.systems.SteadyStateHeat):
            return _energy_solver.HeatSolver(eqs)

        assert 0, "System is of unknown type"
    factory = staticmethod(factory)

