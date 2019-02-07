# coding: utf-8
#Uses underworld,glucifer
import numpy as np
import underworld as uw
import math
from underworld import function as fn
import glucifer
import os

# Setup parameters for Rayleigh Taylor Benchmark
# -----
# Set simulation parameters. The box length is chosen for direct comparison to the benchmark values.

res = 64
boxLength = 0.9142
boxHeight = 1.0
eta = 1.0

# Create mesh and finite element variables
# ------
mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = (res, res), 
                                 minCoord    = (0., 0.), 
                                 maxCoord    = (boxLength, boxHeight))

velocityField = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=2 )
pressureField = uw.mesh.MeshVariable( mesh=mesh.subMesh, nodeDofCount=1 )


# Initialise a swarm.
swarm = uw.swarm.Swarm( mesh=mesh )

# Add a data variable which will store an index to determine material.
materialVariable = swarm.add_variable( dataType="int", count=1 )
vectorVariable = swarm.add_variable( dataType="double", count=2 )

# Create a layout object that will populate the swarm across the whole domain.
swarmLayout = uw.swarm.layouts.PerCellSpaceFillerLayout( swarm=swarm, particlesPerCell=20 )

# Populate.
swarm.populate_using_layout( layout=swarmLayout )

# Initialise the 'materialVariable' data to represent two different materials. 
denseIndex = 0
lightIndex = 1

# The form of this perturbation is taken from van Keken et al. 1997
wavelength = 2.0*boxLength
amplitude  = 0.02
offset     = 0.2
k = 2. * math.pi / wavelength

# The particle coordinates will be the input to the function evaluate (see final line in this cell).
# We get proxy for this now using the input() function.
coordinate = fn.input()

# Define our perturbation function. Note that it is a function of the x coordinate, 
# accessed by 'coordinate[0]'.
perturbationFn = offset + amplitude*fn.math.cos( k*coordinate[0] )

# Setup the conditions list for the following conditional function. Where the
# z coordinate (coordinate[1]) is less than the perturbation, set to lightIndex.
conditions = [ ( perturbationFn > coordinate[1] , lightIndex ),
               (                           True , denseIndex ) ]

# The actual function evaluation. Here the conditional function is evaluated at the location
# of each swarm particle. The results are then written to the materialVariable swarm variable.
materialVariable.data[:] = fn.branching.conditional( conditions ).evaluate(swarm)


# **Plot the particles by material**
fig1 = glucifer.Figure(figsize=(250,250), margin=0)
fig1.append( glucifer.objects.Points(swarm, materialVariable, pointSize=2, colourBar=False) )

# Here we set a density of '0.' for the lightMaterial, and '1.' for the heavymaterial.
densityMap   = { lightIndex:0., denseIndex:1. }
densityFn    = fn.branching.map( fn_key = materialVariable, mapping = densityMap )

# Here we set a viscosity value of '1.' for both materials.
viscosityMap = { lightIndex:eta, denseIndex:1. }
viscosityFn  = fn.branching.map( fn_key = materialVariable, mapping = viscosityMap )

# Define our vertical unit vector using a python tuple (this will be automatically converted to a function).
z_hat = ( 0.0, 1.0 )

# Now create a buoyancy force vector using the density (FEvariable) and the vertical unit vector. 
buoyancyFn = -densityFn*z_hat


# System Setup
# ----------
# **Initialise variables**
velocityField.data[:] = [0.,0.]
pressureField.data[:] = 0.

# We extract various sets of boundary nodes here.
iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]
allWalls = iWalls + jWalls

# Now, using these sets, decide which degrees of freedom (on each node) should be considered Dirichlet.
stokesBC = uw.conditions.DirichletCondition( variable      = velocityField, 
                                             indexSetsPerDof = (allWalls, jWalls) )


# **Create a Stokes system**
stokes = uw.systems.Stokes( velocityField = velocityField, 
                               pressureField = pressureField,
                               voronoi_swarm = swarm, 
                               conditions    = [stokesBC,],
                               fn_viscosity  = viscosityFn,
                               fn_bodyforce  = buoyancyFn )
solver = uw.systems.Solver( stokes )

# **Create a system to advect the particles**
advector = uw.systems.SwarmAdvector( swarm=swarm, velocityField=velocityField, order=2 )


# Analysis tools
# -----
# Set up integrals used to calculate the RMS velocity.
vdotv = fn.math.dot(velocityField,velocityField)
v2sum_integral  = uw.utils.Integral( mesh=mesh, fn=vdotv )
volume_integral = uw.utils.Integral( mesh=mesh, fn=1. )

# Main simulation loop
# -----
# Inside the loop the Stokes system is solved at each step, with the resulting velocity field used to advect the swarm.

# Stepping. Initialise time and timestep.
time         = 0.
steps        = 0
stepEnd      = 10
outputEvery  = 1
timeVal     = []
vrmsVal     = []
vrms        = 0.

def doOutput():
    # Calculate the RMS velocity.
    vrms = math.sqrt( v2sum_integral.evaluate()[0] / volume_integral.evaluate()[0] )
    # Store values in variables.
    vrmsVal.append(vrms)
    timeVal.append(time)

    # print output
    if steps%outputEvery == 0:
        print('step = {0:6d}; time = {1:.3e}; v_rms = {2:.3e}'.format(steps,time,vrms))
            
        # output snapshot of particles to figure.
        outputFilename = "image"+str(steps).zfill(4)
        fig1.save_image(outputFilename)

# Run time loop until $v_{rms}$ is expected to settle; t = 2000 taken from van Keken *et al.* 1997.
while steps<stepEnd:
    # print output
    doOutput()

    # Get solution for initial configuration.
    solver.solve()
    # Retrieve the maximum possible timestep for the advection system.
    dt = advector.get_max_dt()
    # Advect using this timestep size.
    advector.integrate(dt)
        
    time += dt
    steps += 1

#Output final step state
doOutput()

#Check the image results
if uw.rank() == 0:
    lv = glucifer.lavavu.Viewer(quality=1)
    lv.testimages(tolerance=1e-3)

