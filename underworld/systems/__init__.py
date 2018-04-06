##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module contains routines relating to differential system.
"""
import sle
from _stokes import Stokes
from _timeintegration import TimeIntegration, SwarmAdvector
from _advectiondiffusion import AdvectionDiffusion, SLCN_AdvectionDiffusion
from _solver import Solver as _Solver
from _thermal import SteadyStateHeat
from _darcyflow import SteadyStateDarcyFlow

Solver=_Solver.factory
