# ---
# jupyter:
#   jupytext:
#     text_representation:
#       extension: .py
#       format_name: light
#       format_version: '1.5'
#       jupytext_version: 1.13.1
#   kernelspec:
#     display_name: Python 3
#     language: python
#     name: python3
# ---

# ### Example of a model with a compressible material
#
# An instantaneous 2D extension model. A compressible material is subject to lateral extension boundary conditions along the vertical walls.
#
# The compressible stokes flow equations are based on the compressible elasticity formulation: see Hughes, sec4.3, The Finite Element Method, 1987
#
# -----
# The momentum equtaion
#
# $
#     \sigma_{i,j} = -\mathbf{p}\delta_{i,j} + 2 \eta \dot\epsilon_{i,j} = f_{i}
# $
#
# The continuity equation
#
# $
#     \mathbf{v}_{i,i} + \frac{\mathbf{p}}{\lambda} = 0
# $ 
# where:
#  * $ \dot \epsilon_{i,j} = \frac{1}{2}\left[ \mathbf{v}_{i,j} + \mathbf{v}_{j,i} \right ]$
#  * $\mathbf{v}$ is the velocity field
#  * $\mathbf{p}$ is the pressure like variable
#  * $\eta$ is the isotropic shear viscosity
#  * $\lambda$ is the bulk viscosity
#  * $f$ is the body force

import underworld as uw
import numpy as np
import math
import os
from underworld import function as fn
import underworld.visualisation as vis

# +
minX = 0.0
maxX = 2.0
maxY = 1.0
resX = 64
resY = 32
elementType="Q1/dQ0"

mesh = uw.mesh.FeMesh_Cartesian( elementType = (elementType), 
                                 elementRes  = ( resX, resY), 
                                 minCoord    = ( minX, 0.), 
                                 maxCoord    = ( maxX, maxY),
                                 periodic    = [False, False]  ) 

vField = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=mesh.dim )
pField = uw.mesh.MeshVariable( mesh=mesh.subMesh, nodeDofCount=1 )

vField.data[:] = [0.,0.]
pField.data[:] = 0.
# -

# model parameters
viscosityFn   = 1.0        # isoviscous
vel_extend    = 0.5        # simple extension velocity
oneonlambdaFn = 1.0e3      # 1/(bulk viscosity)
buoyancyFn = ( 0.0, 0.0 )  # the body force

# first define strain rate tensor
strainRateFn = fn.tensor.symmetric( vField.fn_gradient )
strainRate_2ndInvariantFn = fn.tensor.second_invariant(strainRateFn)
velmag = fn.math.sqrt( fn.math.dot(vField, vField) )

# +
leftWall  = mesh.specialSets["MinI_VertexSet"]
rightWall = mesh.specialSets["MaxI_VertexSet"]

bottomWall = mesh.specialSets["MinJ_VertexSet"]
topWall    = mesh.specialSets["MaxJ_VertexSet"]

iWalls = leftWall + rightWall
jWalls = bottomWall + topWall


# +
def buildVelocityField( mesh, velField, extV ):
    '''
    Build the extension velocity field. Only extending the incomp beam.
    '''
    import math
    # set the all nodes on the vertical wall to extend extension
    for index in leftWall: # velocity to the left
        ycoord = mesh.data[index][1]
        velField.data[index] = [-1.0*extV, 0.]
#         velField.data[index] = [-1.0*extV*math.sin(ycoord*2*math.pi), 0.]
    for index in rightWall: # velocity to the right
        ycoord = mesh.data[index][1]
        velField.data[index] = [extV, 0.]

buildVelocityField( mesh, vField, vel_extend)
bcs_1 = uw.conditions.DirichletCondition( variable        = vField, 
                                        indexSetsPerDof = ( iWalls, jWalls) ) 

# +
# fn_bodyforce is 0. So ONLY dynamic pressure is produced in this model.
# For the incompressible material d. pressure is the mesh variable
# For the compressibly material d. pressure is -lambda*div(vField)
stokes = uw.systems.Stokes(    velocityField = vField, 
                               pressureField = pField, 
                               conditions    = bcs_1,
                               fn_viscosity  = viscosityFn,
                               fn_bodyforce  = buoyancyFn,
                               fn_one_on_lambda  = oneonlambdaFn )

solver = uw.systems.Solver( stokes )
# -

# can't use the AugmentedLagrangian with the penaly method yet
# solver.set_penalty(1.0e6) 
solver.solve( nonLinearIterate=False, nonLinearTolerance=1e-2 )
# solver.print_stats()

# +
# analytics
gradV = vField.fn_gradient
divV = gradV[0] + gradV[3] # du_dx + dv_dy
shouldBeZero = fn.math.abs(divV + pField*oneonlambdaFn)

errorInt = uw.utils.Integral( shouldBeZero, mesh)
volInt = uw.utils.Integral(1.0, mesh)
# -

# error across domain
tol = 1e-6
error = errorInt.evaluate()[0]/volInt.evaluate()[0]
if error > tol:
    raise RuntimeError("Error: The continuity equation isn't solving within a volume averaged" +
                       " tolerance of {} - it's value is {}".format(tol, error))
print(error)

stokes.eqResiduals

figVel = vis.Figure()
figVel.append( vis.objects.VectorArrows(mesh, vField, scaling=.25, arrowHead=0.2) )
figVel.show()

# +
# # The pressure using the compute field
# figP = vis.Figure( **figVel )
# figP.append( vis.objects.Surface(mesh, pField*oneonlambdaFn, onMesh=True) )
# figP.show()
