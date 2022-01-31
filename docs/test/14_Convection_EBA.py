# -*- coding: utf-8 -*-
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

# King / Blankenbach Benchmark Case 1
# ======
#
# Isoviscous thermal convection using EBA formulation.
# ----
#
# Two-dimensional, incompressible, bottom heated, steady isoviscous thermal convection in a 1 x 1 box, see case 1 of King *et al.* 2009 / Blankenbach *et al.* 1989 for details.
#
#
# **This example introduces:**
# 1. Extended Boussinesq Approximation, EBA, formulation for Stokes Flow.
#
# **Keywords:** Stokes system, EBA, advective diffusive systems, analysis tools
#
# **References**
#
# Scott D. King, Changyeol Lee, Peter E. Van Keken, Wei Leng, Shijie Zhong, Eh Tan, Nicola Tosi, Masanori C. Kameyama, A community benchmark for 2-D Cartesian compressible convection in the Earth's mantle, Geophysical Journal International, Volume 180, Issue 1, January 2010, Pages 73–87, https://doi.org/10.1111/j.1365-246X.2009.04413.x
#
# B. Blankenbach, F. Busse, U. Christensen, L. Cserepes, D. Gunkel, U. Hansen, H. Harder, G. Jarvis, M. Koch, G. Marquart, D. Moore, P. Olson, H. Schmeling and T. Schnaubelt. A benchmark comparison for mantle convection codes. Geophysical Journal International, 98, 1, 23–38, 1989
# http://onlinelibrary.wiley.com/doi/10.1111/j.1365-246X.1989.tb05511.x/abstract
#

import underworld as uw
from underworld import function as fn
import underworld.visualisation as vis
import math
import numpy as np

from underworld.scaling import units as u

# +
# The physical S.I. units from the Blankenbach paper
# Sanity check for the Rayleigh number.
# In this implementation the equations are non-dimensionalised with Ra

# Ra = a*g*dT*h**3 / (eta0*dif)
h  = 1e6 * u.m
dT = 1e3 * u.degK
a  = 2.5e-5 * u.degK**-1
g  = 10  * u.m * u.s**-2
diff = 1e-6 * u.m**2 * u.s**-1
eta = 1e23 * u.kg * u.s**-1 * u.m**-1
rho = 4000 * u.kg * u.m**-3 # reference density, only for units

Ra = (a*g*dT*h**3)/(eta/rho*diff)
print(Ra.to_compact())
# -

# Setup parameters
# -----

boxHeight = 1.0
boxLength = 1.0
# Set grid resolution.
res = 64
# Set max & min temperautres
tempMin = 0.0
tempMax = 1.0

# Choose which Rayleigh number, see case 1 of Blankenbach *et al.* 1989 for details.

Di = 0.5
Ra = 1.e4
eta0 = 1.e23

# Set input and output file directory 

outputPath = 'EBA/'
# Make output directory if necessary.
if uw.mpi.rank==0:
    import os
    if not os.path.exists(outputPath):
        os.makedirs(outputPath)

# Create mesh and variables
# ------

# +
mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = (res, res), 
                                 minCoord    = (0., 0.), 
                                 maxCoord    = (boxLength, boxHeight))

velocityField       = mesh.add_variable(         nodeDofCount=2 )
pressureField       = mesh.subMesh.add_variable( nodeDofCount=1 )
temperatureField    = mesh.add_variable(         nodeDofCount=1 )
temperatureDotField = mesh.add_variable(         nodeDofCount=1 )

# initialise velocity, pressure and temperatureDot field
velocityField.data[:]       = [0.,0.]
pressureField.data[:]       = 0.
temperatureField.data[:]    = 0.
temperatureDotField.data[:] = 0.
# -

# Set up material parameters and functions
# -----
#
# Set values and functions for viscosity, density and buoyancy force.

# +
# Set a constant viscosity.
viscosity = 1.

# Create our density function.
densityFn = Ra * temperatureField

# Define our vertical unit vector using a python tuple (this will be automatically converted to a function).
z_hat = ( 0.0, 1.0 )

# A buoyancy function.
buoyancyFn = densityFn * z_hat
# -

# Set initial temperature field
# -----
#

# **Use a sinusodial perturbation**

temperatureField.data[:] = 0.
pertStrength = 0.1
deltaTemp = tempMax - tempMin
for index, coord in enumerate(mesh.data):
    pertCoeff = math.cos( math.pi * coord[0]/boxLength ) * math.sin( math.pi * coord[1]/boxLength )
    temperatureField.data[index] = tempMin + deltaTemp*(boxHeight - coord[1]) + pertStrength * pertCoeff
    temperatureField.data[index] = max(tempMin, min(tempMax, temperatureField.data[index]))

# **Show initial temperature field**
#

# +
# fig = vis.Figure()
# fig.append( vis.objects.Surface(mesh, temperatureField) )
# fig.show()
# -

# Create boundary conditions
# ----------
#
# Set temperature boundary conditions on the bottom ( ``MinJ`` ) and top ( ``MaxJ`` ).

for index in mesh.specialSets["MinJ_VertexSet"]:
    temperatureField.data[index] = tempMax
for index in mesh.specialSets["MaxJ_VertexSet"]:
    temperatureField.data[index] = tempMin

# Construct sets for the both horizontal and vertical walls. Combine the sets of vertices to make the ``I`` (left and right side walls) and ``J`` (top and bottom walls) sets.

# +
iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]

freeslipBC = uw.conditions.DirichletCondition( variable        = velocityField, 
                                               indexSetsPerDof = (iWalls, jWalls) )
tempBC     = uw.conditions.DirichletCondition( variable        = temperatureField, 
                                               indexSetsPerDof = (jWalls,) )
# -

# System setup
# -----
#
# **Setup a Stokes system**
#

stokes = uw.systems.Stokes(    velocityField = velocityField, 
                               pressureField = pressureField,
                               conditions    = [freeslipBC,],
                               fn_viscosity  = viscosity, 
                               fn_bodyforce  = buoyancyFn )
# get the default stokes equation solver
solver = uw.systems.Solver( stokes )

# +
# a function for the 2nd invariant strain rate tensor
fn_sr2Inv = fn.tensor.second_invariant(fn.tensor.symmetric( velocityField.fn_gradient ))

# a function for viscous dissipation, i.e.
# the contraction of dev. stress tensor with strain rate tensor.
vd =  2 * viscosity * 2 * fn_sr2Inv**2

# function for adiabatic heating
adiabatic_heating = Di * velocityField[1]*(temperatureField)

# combine viscous dissipation and adiabatic heating
# terms to the energy equation, via the argument 'fn_source'
fn_source = Di/Ra * vd - adiabatic_heating

### As discussed by King et al. (JI09) the volume integral of the viscous dissipation and 
### the adiabatic heating should balance.

int_vd = uw.utils.Integral([Di/Ra*vd,adiabatic_heating], mesh)
# -

# **Create an advection diffusion system**
#

advDiff = uw.systems.AdvectionDiffusion( phiField       = temperatureField, 
                                         phiDotField    = temperatureDotField, 
                                         velocityField  = velocityField, 
                                         fn_diffusivity = 1.0,
                                         fn_sourceTerm  = fn_source,
                                         conditions     = [tempBC,] )

# Analysis tools
# -----

# **Nusselt number**
#
# The Nusselt number is the ratio between convective and conductive heat transfer
#
# \\[
# Nu = -h \frac{ \int_0^l \partial_z T (x, z=h) dx}{ \int_0^l T (x, z=0) dx}
# \\]
#
#
#
#

# +
nuTop    = uw.utils.Integral( fn=temperatureField.fn_gradient[1], 
                              mesh=mesh, integrationType='Surface', 
                              surfaceIndexSet=mesh.specialSets["MaxJ_VertexSet"])

nuBottom = uw.utils.Integral( fn=temperatureField,               
                              mesh=mesh, integrationType='Surface', 
                              surfaceIndexSet=mesh.specialSets["MinJ_VertexSet"])
# -

nu = - nuTop.evaluate()[0]/nuBottom.evaluate()[0]
if uw.mpi.rank == 0 : print('Nusselt number = {0:.6f}'.format(nu))

# **RMS velocity**
#
# The root mean squared velocity is defined by intergrating over the entire simulation domain via
#
# \\[
# \begin{aligned}
# v_{rms}  =  \sqrt{ \frac{ \int_V (\mathbf{v}.\mathbf{v}) dV } {\int_V dV} }
# \end{aligned}
# \\]
#
# where $V$ denotes the volume of the box.

vrms = stokes.velocity_rms()
if uw.mpi.rank == 0 : print('Initial vrms = {0:.3f}'.format(vrms))

# Main simulation loop
# -----

# +
#initialise time, step, output arrays
time = 0.
step = 0
timeVal = []
vrmsVal = []
step_end = 30

# output frequency
step_output = max(1,min(100, step_end/10)) # reasonable automatic choice
epsilon     = 1.e-8

velplotmax = 0.0
nuLast = -1.0
rerr   = 1.


# -

# define an update function
def update():
    # Determining the maximum timestep for advancing the a-d system.
    dt = advDiff.get_max_dt()
    # Advect using this timestep size. 
    advDiff.integrate(dt)
    return time+dt, step+1


v_old = velocityField.copy()

# +
tol = 1e-8
# Perform steps.
while step<=step_end and rerr > tol:
    
    # copy to previous
    v_old.data[:] = velocityField.data[:]
    
    # Solving the Stokes system.
    solver.solve()
     
    aerr = uw.utils._nps_2norm(v_old.data-velocityField.data)
    magV = uw.utils._nps_2norm(v_old.data)
    rerr = ( aerr/magV if magV>1e-8 else 1)       # calculate relative variation

    # Calculate & store the RMS velocity and Nusselt number.
    vrms = stokes.velocity_rms()
    nu   = - nuTop.evaluate()[0]/nuBottom.evaluate()[0]
    vrmsVal.append(vrms)
    timeVal.append(time)
    velplotmax = max(vrms, velplotmax)

    # print output statistics 
    if step%(step_end/step_output) == 0:

#         mH = mesh.save(outputPath+"mesh-{}.h5".format(step))
#         tH = temperatureField.save(outputPath+"t-{}.h5".format(step), mH)
#         vH = velocityField.save(outputPath+"v-{}.h5".format(step), mH)
#         velocityField.xdmf(outputPath+"v-{}.xdmf".format(step), vH, "velocity", mH, "mesh")
#         temperatureField.xdmf(outputPath+"t-{}.xdmf".format(step), tH, "temperature", mH, "mesh" )

        if(uw.mpi.rank==0):
            print('steps = {0:6d}; time = {1:.3e}; v_rms = {2:.3f}; Nu = {3:.3f}; Rel change = {4:.3e} vChange = {5:.3e}'
              .format(step, time, vrms, nu, abs((nu - nuLast)/nu), rerr))
            
#     # Check loop break conditions.
#     if(abs((nu - nuLast)/nu) < epsilon):
#         if(uw.mpi.rank==0):
#             print('steps = {0:6d}; time = {1:.3e}; v_rms = {2:.3f}; Nu = {3:.3f}; Rel change = {4:.3e}'
#               .format(step, time, vrms, nu, abs((nu - nuLast)/nu)))
#         break
    nuLast = nu
    
    # update
    time, step = update()
    
print("velocity relative tolerance is: {:.3f}".format(rerr))
# -

# Post analysis
# -----
#
# **Benchmark values**
#
# We can check the volume integral of viscous dissipation and adibatic heating are equal
#

vd, ad = int_vd.evaluate()

# error if >2% difference in vd and ad
if not np.isclose(vd,ad, rtol=2e-2):
    if uw.mpi.rank == 0: print('vd = {0:.3e}, ad = {1:.3e}'.format(vd,ad))
    raise RuntimeError("The volume integral of viscous dissipation and adiabatic heating should be approximately equal")
