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

# Slab Subduction
# ======
#
# This two dimensional subduction model has a dense, high viscosity 3 layered plate overlying a lower viscosity mantle. The upper and lower plate layers have a visco-plastic rheology, yielding under large stresses. The middle, core layer has a viscous only rheology, maintaining strength during bending. The top 1000 km of the mantle is included, the upper & lower mantle is partitioned with a viscosity contrast of 100x at 600 km depth. The velocity boundary conditions on the domain are period side, free-slip top and no-slip bottom wall. 
#
# **References**
#
# 1. OzBench, M.; Regenauer-Lieb, K.; Stegman, D. R.; Morra, G.; Farrington, R.; Hale, A.; May, D. A.; Freeman, J.; Bourgouin, L.; Mühlhaus, H. & Moresi, L. A model comparison study of large-scale mantle-lithosphere dynamics driven by subduction. Physics of the Earth and Planetary Interiors, 2008, 171, 224-234. [OzBench, 2008](http://www.sciencedirect.com/science/article/pii/S0031920108002318)

# +
import underworld as uw
import math
from underworld import function as fn
import underworld.visualisation as vis
import numpy as np
import os

rank = uw.mpi.rank

# +
outputPath = os.path.join(os.path.abspath("."),"output/")

if rank==0:
    if not os.path.exists(outputPath):
        os.makedirs(outputPath)
uw.mpi.barrier()
# -

# **Setup parameters**
#

xRes = 192  
yRes =  48
boxLength = 4.0
boxHeight = 1.0

# **Create mesh and finite element variables**

# +
mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"),
                                 elementRes  = (xRes, yRes), 
                                 minCoord    = (0., 0.), 
                                 maxCoord    = (boxLength, boxHeight),
                                 periodic    = [True, False] ) 

velocityField   = mesh.add_variable(         nodeDofCount=2 )
pressureField   = mesh.subMesh.add_variable( nodeDofCount=1 )
# -

# **Create a particle swarm**

swarm = uw.swarm.Swarm( mesh=mesh )
materialVariable   = swarm.add_variable( dataType="int", count=1 )
swarmLayout = uw.swarm.layouts.PerCellSpaceFillerLayout( swarm=swarm, particlesPerCell=20 )
swarm.populate_using_layout( layout=swarmLayout )

# **Allocate materials to particles**

# +
# initialise the 'materialVariable' data to represent two different materials. 
upperMantleIndex = 0
lowerMantleIndex = 1
upperSlabIndex   = 2
lowerSlabIndex   = 3
coreSlabIndex    = 4

# Initial material layout has a flat lying slab with at 15\degree perturbation
lowerMantleY   = 0.4
slabLowerShape = np.array([ (1.2,0.925 ), (3.25,0.925 ), (3.20,0.900), (1.2,0.900), (1.02,0.825), (1.02,0.850) ])
slabCoreShape  = np.array([ (1.2,0.975 ), (3.35,0.975 ), (3.25,0.925), (1.2,0.925), (1.02,0.850), (1.02,0.900) ])
slabUpperShape = np.array([ (1.2,1.000 ), (3.40,1.000 ), (3.35,0.975), (1.2,0.975), (1.02,0.900), (1.02,0.925) ])

slabLower = fn.shape.Polygon( slabLowerShape )
slabUpper = fn.shape.Polygon( slabUpperShape )
slabCore  = fn.shape.Polygon( slabCoreShape )

# initialise everying to be upper mantle material
materialVariable.data[:] = upperMantleIndex

# change matieral index if the particle is not upper mantle
for index in range( len(swarm.particleCoordinates.data) ):
    coord = swarm.particleCoordinates.data[index][:]
    if coord[1] < lowerMantleY:
        materialVariable.data[index]     = lowerMantleIndex
    if slabCore.evaluate(tuple(coord)):
            materialVariable.data[index] = coreSlabIndex
    if slabUpper.evaluate(tuple(coord)):
            materialVariable.data[index] = upperSlabIndex
    elif slabLower.evaluate(tuple(coord)):
            materialVariable.data[index] = lowerSlabIndex
# -

# **Plot the initial positions for the particle swarm and colour by material type**
#
# We are using a vis store object to keep all our figures together and allow them to be revisualised later so create this here and pass it to all the figures created later. We'll also name this figure to make it easy to find later when revisualising.

store = vis.Store('output/subduction')
figParticle = vis.Figure( store, figsize=(960,300), name="Particles" )
figParticle.append( vis.objects.Points(swarm, materialVariable, pointSize=2, colours='white green red purple blue') )
figParticle.show()

# Set up material parameters and functions
# ----
#
# Here the functions for density, viscosity, gravity and buoyancy are set. 

# **Viscosity function**
#
# The lower mantle is 100x more viscous than the upper mantle. 
# The upper and lower slab layer weakens under high strain, it has a visco plastic rheology.  
# The lower slab layer does not yield, it has a viscous rheology.  

# +
upperMantleViscosity =    1.0
lowerMantleViscosity =  100.0
slabViscosity        =  500.0
coreViscosity        =  500.0

# The yeilding of the upper slab is dependent on the strain rate.
strainRate_2ndInvariant = fn.tensor.second_invariant( 
                            fn.tensor.symmetric( 
                            velocityField.fn_gradient ))
cohesion = 0.06
vonMises = 0.5 * cohesion / (strainRate_2ndInvariant+1.0e-18)

# The upper slab viscosity is the minimum of the 'slabViscosity' or the 'vonMises' 
slabYieldvisc = fn.exception.SafeMaths( fn.misc.min(vonMises, slabViscosity) )

# Viscosity function for the materials 
viscosityMap = { upperMantleIndex : upperMantleViscosity, 
                 lowerMantleIndex : lowerMantleViscosity, 
                 upperSlabIndex   : slabYieldvisc, 
                 lowerSlabIndex   : slabYieldvisc,
                 coreSlabIndex    : coreViscosity}
viscosityMapFn = fn.branching.map( fn_key = materialVariable, mapping = viscosityMap )
# -

# Note: ``SafeMaths`` in the above cell refers to a function which checks the value passed inside for any infinite or other not-a-number type errors.

# **Set the density function, vertical unit vector and Buoyancy Force function**
#

# +
mantleDensity = 0.0
slabDensity   = 1.0 

densityMap = { upperMantleIndex : mantleDensity, 
               lowerMantleIndex : mantleDensity, 
               upperSlabIndex   : slabDensity, 
               lowerSlabIndex   : slabDensity, 
               coreSlabIndex    : slabDensity}
densityFn = fn.branching.map( fn_key = materialVariable, mapping = densityMap )

# Define our vertical unit vector using a python tuple
z_hat = ( 0.0, 1.0 )

# now create a buoyancy force vector
buoyancyFn = -1.0 * densityFn * z_hat
# -

# **Set initial and boundary conditions**

# +
# set initial conditions (and boundary values)
velocityField.data[:] = [0.,0.]
pressureField.data[:] = 0.

# send boundary condition information to underworld
iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]
bottomWall = mesh.specialSets["MinJ_VertexSet"]

periodicBC = uw.conditions.DirichletCondition( variable        = velocityField, 
                                               indexSetsPerDof = ( bottomWall, jWalls) ) 

# -

# **System Setup**

# Initial linear slab viscosity setup
stokes = uw.systems.Stokes(    velocityField = velocityField, 
                               pressureField = pressureField,
                               conditions    = periodicBC,
                               fn_viscosity  = viscosityMapFn, 
                               fn_bodyforce  = buoyancyFn )

# Create solver & solve
try:
    # Try using "mumps" direct solve, best for 2D models.
    solver = uw.systems.Solver(stokes)
    solver.set_inner_method("mumps")
    solver.solve()
except RuntimeError:
    # If the above failed, most likely "mumps" isn't 
    # installed. Fallback to default solver. 
    solver = uw.systems.Solver( stokes )

advector = uw.systems.SwarmAdvector( swarm=swarm, velocityField=velocityField, order=2 )

# **Analysis tools**

# +
#Plot of Velocity Magnitude
figVelocityMag = vis.Figure(store, figsize=(960,300))
figVelocityMag.append( vis.objects.Surface(mesh, fn.math.sqrt(fn.math.dot(velocityField,velocityField))) )

#Plot of Strain Rate, 2nd Invariant
figStrainRate = vis.Figure(store, figsize=(960,300))
figStrainRate.append( vis.objects.Surface(mesh, strainRate_2ndInvariant, logScale=True) )

#Plot of particles viscosity
figViscosity = vis.Figure(store, figsize=(960,300))
figViscosity.append( vis.objects.Points(swarm, viscosityMapFn, pointSize=2) )

#Plot of particles stress invariant
figStress = vis.Figure( store, figsize=(960,300) )
figStress.append( vis.objects.Points(swarm, 2.0*viscosityMapFn*strainRate_2ndInvariant, pointSize=2, logScale=True) )
# -

# Main simulation loop
# =======
#
# The main time stepping loop begins here. Inside the time loop the velocity field is solved for via the Stokes system solver and then the swarm is advected using the advector integrator. Basic statistics are output to screen each timestep.

time = 0.  # Initial time
step = 0   # Initial timestep
maxSteps = 3      # Maximum timesteps (201 is recommended)
steps_output = 10   # output every 10 timesteps


# define an update function
def update():
    # Retrieve the maximum possible timestep for the advection system.
    dt = advector.get_max_dt()
    # Advect using this timestep size.
    advector.integrate(dt)
    return time+dt, step+1


while step < maxSteps:
    # Solve non linear Stokes system
    solver.solve(nonLinearIterate=True)
    # output figure to file at intervals = steps_output
    if step % steps_output == 0 or step == maxSteps-1:
        #Important to set the timestep for the store object here or will overwrite previous step
        store.step = step
        figParticle.save(    outputPath + "particle"    + str(step).zfill(4))
        figVelocityMag.save( outputPath + "velocityMag" + str(step).zfill(4))
        figStrainRate.save(  outputPath + "strainRate"  + str(step).zfill(4))
        figViscosity.save(   outputPath + "viscosity"   + str(step).zfill(4))
        figStress.save(      outputPath + "stress"      + str(step).zfill(4))
        
        vrms = stokes.velocity_rms()
        if rank==0: print('step = {0:6d}; time = {1:.3e}; Vrms = {2:.3e}'.format(step,time,vrms))

    # update
    time,step = update()

# Add test to ensure model is producing expected results.
if (step==3) and not np.isclose(vrms,4.068e-5, rtol=5e-2):
    raise RuntimeError("VRMS differs from expected value.")

# Post simulation analysis
# -----
#
# Plot all figures for the resulting system using LavaVu to load the saved visualisation.
# This allows us to open a previously saved visualisation database, just pass the same name used when creating the Store object

import underworld.visualisation as vis
viewer = vis.Viewer('output/subduction')

# Here we print some information about the loaded visualisation which shows what data is available
# (as we only gave our first figure a name "Particles" the others have been automatically named)

figs = viewer.figures
steps = viewer.steps
if rank==0:
    print("Saved database '%s'" % (viewer.filename))
    print(" - %d figures : %s" % (len(figs), str(figs.keys())))
    print(" - %d timesteps (final = %d) : %s" % (len(steps), steps[-1], steps))

# We just want to look at the final state of the simulation, so set the timestep to the last one in the list and loop through all the figures, plotting each one

#Re-visualise the final timestep
viewer.step = steps[-1]
for name in viewer.figures:
    viewer.figure(name)
    viewer["quality"] = 2
    viewer["title"] = "Timestep ##"
    viewer.show()
