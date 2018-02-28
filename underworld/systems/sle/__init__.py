##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
"""
from _assembledvector import AssembledVector
from _assembledmatrix import AssembledMatrix
from _eqnum import EqNumber
from _svector import SolutionVector
from _assemblyterm import AssemblyTerm, VectorAssemblyTerm, MatrixAssemblyTerm, \
                          VectorAssemblyTerm_NA__Fn, GradientStiffnessMatrixTerm, PreconditionerMatrixTerm, \
                          ConstitutiveMatrixTerm, AdvDiffResidualVectorTerm, LumpedMassMatrixVectorTerm, \
                          MatrixAssemblyTerm_NA_i__NB_i__Fn, VectorSurfaceAssemblyTerm_NA__Fn__ni, \
                          AdvDiffResidualVectorTerm, VectorAssemblyTerm_NA_j__Fn_ij, MatrixAssemblyTerm_NA__NB__Fn, \
                          MatrixAssemblyTerm_RotationDof, VectorAssemblyTerm_NA_i__Fn_i
