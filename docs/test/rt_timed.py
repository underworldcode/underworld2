"""
This test runs a basic RT type simulation, and measures the
timing for various routines against that provided by the
timing routine.
"""
import underworld as uw
from underworld import function as fn
import glucifer
import math
import numpy as np
from collections import defaultdict
from time import time
uw.timing.start()

timing_data = defaultdict(lambda: [0,0.])
def add_timing(name, time):
    data = timing_data[name]
    data[0] += 1
    data[1] += time

import os
res = 16
RESKEY = "UW_RESOLUTION"
if RESKEY in os.environ:
    res = int(os.environ[RESKEY])

ts = time()
mesh = uw.mesh.FeMesh_Cartesian(elementRes  = (res, res, res),
                                minCoord    = ( 0., 0., 0., ),
                                maxCoord    = ( 1., 1., 1., ))
add_timing("FeMesh_Cartesian.__init__()", time()-ts)

ts = time()
velocityField         = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=3 )
add_timing("MeshVariable.__init__()", time()-ts)
ts = time()
pressureField         = uw.mesh.MeshVariable( mesh=mesh.subMesh, nodeDofCount=1 )
add_timing("MeshVariable.__init__()", time()-ts)
ts = time()
temperatureField      = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=1 )
add_timing("MeshVariable.__init__()", time()-ts)
ts = time()
temperatureFieldDeriv = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=1 )
add_timing("MeshVariable.__init__()", time()-ts)


# initialise 
velocityField.data[:] = [0.,0.,0.]
pressureField.data[:] = 0.

for index, coord in enumerate(mesh.data):
    temperatureField.data[index] = coord[2]

temperatureFieldDeriv.data[:] = 0.

# Create a swarm.
ts = time()
swarm = uw.swarm.Swarm( mesh=mesh )
add_timing("Swarm.__init__()", time()-ts)

# Create a data variable. It will be used to store the material index of each particle.
materialIndex = swarm.add_variable( dataType="int", count=1 )

# Create a layout object, populate the swarm with particles.
swarmLayout = uw.swarm.layouts.PerCellSpaceFillerLayout( swarm=swarm, particlesPerCell=40 )
ts = time()
swarm.populate_using_layout( layout=swarmLayout )
add_timing("Swarm.populate_using_layout()", time()-ts)


# define these for convience. 
denseIndex = 0
lightIndex = 1

# material perturbation from van Keken et al. 1997
wavelength = 2.0
amplitude  = 0.02
offset     = 0.2
k = 2. * math.pi / wavelength

# Create function to return particle's coordinate
coord = fn.coord()

# Define the material perturbation, a function of the x coordinate (accessed by `coord[0]`).
perturbationFn = offset + amplitude*fn.math.cos( k*coord[0] )

# Setup the conditions list. 
# If z is less than the perturbation, set to lightIndex.
conditions = [ ( perturbationFn > coord[1] , lightIndex ),
               (                      True , denseIndex ) ]

# The swarm is passed as an argument to the evaluation, providing evaluation on each particle.
# Results are written to the materialIndex swarm variable.
fnc = fn.branching.conditional( conditions )
ts = time()
matdat = fnc.evaluate(swarm)
add_timing("Function.evaluate()", time()-ts)
materialIndex.data[:] = matdat

store = glucifer.Store('RT')

fig = glucifer.Figure( store, name="firstFig" )
fig.append( glucifer.objects.Points(swarm, materialIndex, pointSize=2, colourBar=False) )
fig.append( glucifer.objects.Surface(mesh, pressureField))
fig.append( glucifer.objects.VectorArrows( mesh, velocityField, scaling=1.0e2))


# Set a density of '0.' for light material, '1.' for dense material.
densityMap   = { lightIndex:0., denseIndex:1. }
densityFn    = fn.branching.map( fn_key = materialIndex, mapping = densityMap )

# Set a viscosity value of '1.' for both materials.
viscosityMap = { lightIndex:1., denseIndex:1. }
fn_viscosity  = fn.branching.map( fn_key = materialIndex, mapping = viscosityMap )

# Define a vertical unit vector using a python tuple.
z_hat = ( 0., 0., 1. )

# Create buoyancy force vector
buoyancyFn = -densityFn*z_hat


# Construct node sets using the mesh specialSets
iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]
kWalls = mesh.specialSets["MinK_VertexSet"] + mesh.specialSets["MaxK_VertexSet"]

allWalls = iWalls + jWalls + kWalls

# Prescribe degrees of freedom on each node to be considered Dirichlet conditions.
# In the x direction on allWalls flag as Dirichlet
# In the y direction on jWalls (horizontal) flag as Dirichlet
stokesBC = uw.conditions.DirichletCondition( variable        = velocityField,
                                             indexSetsPerDof = (allWalls, allWalls, kWalls))
advdiffBc = uw.conditions.DirichletCondition( variable        = temperatureField,
                                              indexSetsPerDof = kWalls )

ts = time()
stokes = uw.systems.Stokes( velocityField = velocityField,
                            pressureField = pressureField,
#                            voronoi_swarm = swarm,
                            conditions    = stokesBC,
                            fn_viscosity  = fn_viscosity, 
                            fn_bodyforce  = buoyancyFn )
add_timing("Stokes.__init__()", time()-ts)

solver = uw.systems.Solver( stokes )

# Create a system to advect the swarm
advector = uw.systems.SwarmAdvector( swarm=swarm, velocityField=velocityField, order=2 )

# Create a dummy temperature field.
ts = time()
advdiff = uw.systems.AdvectionDiffusion(velocityField=velocityField, phiField=temperatureField, phiDotField=temperatureFieldDeriv, 
                                        fn_diffusivity=1.,conditions=advdiffBc)
add_timing("AdvectionDiffusion.__init__()", time()-ts)


# functions for calculating RMS velocity
vdotv = fn.math.dot(velocityField,velocityField)
ts = time()
v2sum_integral  = uw.utils.Integral( mesh=mesh, fn=vdotv )
add_timing("Integral.__init__()", time()-ts)
ts = time()
volume_integral = uw.utils.Integral( mesh=mesh, fn=1. )
add_timing("Integral.__init__()", time()-ts)


# Get instantaneous Stokes solution
ts = time()
solver.solve()
add_timing("StokesSolver.solve()", time()-ts)
# Calculate the RMS velocity.
vrms = math.sqrt( v2sum_integral.evaluate()[0] )

# update 
dt1 = advector.get_max_dt()
dt2 = advdiff.get_max_dt()
dt = min(dt1,dt2)
# Advect using this timestep size.
ts = time()
advector.integrate(dt)
add_timing("SwarmAdvector.integrate()", time()-ts)

ts = time()
advdiff.integrate(dt)
add_timing("AdvectionDiffusion.integrate()", time()-ts)

# Save things
ts = time()
meshFileHandle = mesh.save("Mesh.h5")
add_timing("FeMesh.save()", time()-ts)
uw.barrier()
if uw.rank() == 0:
    os.remove("Mesh.h5")

ts = time()
vFH = velocityField.save("velocityField.h5")
add_timing("MeshVariable.save()", time()-ts)
uw.barrier()
if uw.rank() == 0:
    os.remove("velocityField.h5")

ts = time()
swarmFileHandle = swarm.save("Swarm.h5")
add_timing("Swarm.save()", time()-ts)

# Timing for this guy is consistently out of tolerance (like 30%).
# It is a very fast call, so not concerned.
mH = materialIndex.save("materialIndex.h5")
uw.barrier()
if uw.rank() == 0:
    os.remove("Swarm.h5")
    os.remove("materialIndex.h5")

ts = time()
fig.save()
add_timing("Figure.save()", time()-ts)
uw.barrier()
if uw.rank() == 0:
    os.remove("RT.gldb")

uw.timing.print_table(group_by="routine", tablefmt="grid")

if uw.rank() == 0:
    import numpy as np
    module_timing_data = uw.timing.get_data(group_by="routine")
    for key in timing_data.keys():
        valuescript = timing_data[key]
        valuemod    = module_timing_data[key]
        if not np.isclose(valuescript[1],valuemod[1], rtol=0.15):
            raise RuntimeError( "Timing for '{}' not within tolerance ( {}: {} ).".format(key,valuescript[1],valuemod[1]) )

# simple test for file output
fname = "timing_test.txt"
uw.timing.print_table(group_by="routine", output_file=fname)
if uw.rank() == 0:
    import os.path
    exists = os.path.isfile(fname)
    if not exists:
        raise RuntimeError( "Timing output to file does not appear to have worked." )
    os.remove(fname)


