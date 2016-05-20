
# coding: utf-8

# ''Stokes Sinker''
# ======
# 
# Demonstration example for setting up particle swarms with different material properties. This system consists of a dense, high viscosity sphere falling through a background lower density and viscosity fluid. 
# 
# **This lesson introduces the concepts of:**
# 1. Creating particle swarms.
# 2. Associating different behaviours with different particles.
# 3. Using the ``branching.conditional`` function.
# 4. Advecting particle swarms in a Stokes system.
# 
# **Keywords:** particle swarms, Stokes system, advective diffusive systems
# 

# In[1]:

#%matplotlib inline
import matplotlib.pyplot as pyplot
import matplotlib.pylab as pylab
import numpy as np
import underworld as uw
import math
from underworld import function as fn
import glucifer
from mpi4py import MPI


# Setup parameters
# -----
# 
# Set simulation parameters for the test and position of the spherical sinker.

# In[2]:

dim = 2
# Set the resolution.
res = 64

# Set size and position of dense sphere.
sphereRadius = 0.1
sphereCentre = (0., 0.7)


# Create mesh and finite element variables
# ------

# In[3]:

mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = (res, res), 
                                 minCoord    = (-1., 0.), 
                                 maxCoord    = (1., 1.))

velocityField    = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=dim )
pressureField    = uw.mesh.MeshVariable( mesh=mesh.subMesh, nodeDofCount=1 )


# Set initial conditions and boundary conditions
# ----------
# 
# **Initial and boundary conditions**
# 
# Initialise the velocity and pressure fields to zero.

# In[4]:

velocityField.data[:] = [0.,0.]
pressureField.data[:] = 0.


# **Conditions on the boundaries**
# 
# Construct sets for the both horizontal and vertical walls to define conditons for underworld solvers.

# In[5]:

iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]

freeslipBC = uw.conditions.DirichletCondition( variable      = velocityField, 
                                               indexSetsPerDof = (iWalls, jWalls) )


# Create a particle swarm
# ------
# 
# Swarms refer to (large) groups of particles which can advect with the fluid flow. These can be used to determine 'materials' as they can carry local information such as the fluid density and viscosity.

# **Setup a swarm**
# 
# To set up a swarm of particles the following steps are needed:
# 1. Initialise and name a swarm, here called ``swarm``.
# 2. Define data variable (``materialIndex``) to store an index that will state what material a given particle is.
# 3. Populate the swarm over the whole domain using the layout command, here this is used to allocate 20 particles in each element.

# In[6]:

# Create the swarm. 
swarm = uw.swarm.Swarm( mesh=mesh )

# Add a data variable which will store an index to determine material.
materialIndex = swarm.add_variable( dataType="int", count=1 )

# Create a layout object that will populate the swarm across the whole domain.
swarmLayout = uw.swarm.layouts.GlobalSpaceFillerLayout( swarm=swarm, particlesPerCell=20 )

# Go ahead and populate the swarm.
swarm.populate_using_layout( layout=swarmLayout )


# **Define a shape**
# 
# Define a python function that mathematically describes a shape, in this case a circle offset to the centre of the sinker. Note that this returns an underworld function, which can be used in the branching condition function below.

# In[7]:

def inCircleFnGenerator(centre, radius):
    coord = fn.input()
    offsetFn = coord - centre
    return fn.math.dot( offsetFn, offsetFn ) < radius**2


# **Set material index for each swarm particle**
# 
# Use the location of each particle (stored in the 2 dimensional vector called ``coord``) to set the material type. In this case the circle function defined above will return ``True`` when the ``coord`` is within the circular sinker and ``False`` otherwise. The branching conditional function will then set the ``materialIndex`` data value for that particle to equal the ``materialHeavyIndex`` if it is inside the sinker, or the ``materialLightIndex`` otherwise.

# In[8]:

# Let's initialise the 'materialVariable' data to represent two different materials. 
materialLightIndex = 0
materialHeavyIndex = 1

# Set up the condition for being in a circle. If not in sphere then will return light index.
conditions = [ ( inCircleFnGenerator( sphereCentre, sphereRadius) , materialHeavyIndex), 
               ( True                                             , materialLightIndex) ]

# Execute the branching conditional function, evaluating at the location of each particle in the swarm.
# The results are copied into the materialIndex swarm variable.
materialIndex.data[:] = fn.branching.conditional( conditions ).evaluate(swarm)


# **Branching conditional function**
# 
# The material index is set using a underworld function called 'branching.conditional' which has the form of:
# 
#     if( [ (conditionFunction1, actionFunction 1), 
#           (conditionFunction2, actionFunction 2), 
#           ... 
#           ] )
# 
# This function first evaluates the first condition function (conditionFunction1), and if it evaluates to True, it returns the results obtained by executing the corresponding action function (actionFunction1). If it is not True then the next conditional function is tested and so on. If no condition functions return True, then the branching function will return an error. To avoid this, the last condition function may be set to True and the final action function serves the purpose of ``everything else``, for example a default background material index as in this case.

# **Define minimum y coordinate function**
# 
# Define a function that finds the minimum y coordinate value for the sinker particles. This function uses ``mpi4py`` commands to allow the model to run in parallel and return the correct minimum value to processor 0.

# In[9]:

def GetSwarmYMin():
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    nprocs = comm.Get_size()
    ymax = 1.0
    conditions = [ ( materialIndex > materialLightIndex , swarm.particleCoordinates ),
                   ( True                               , (0.0, ymax) ) ]
    positions = fn.branching.conditional( conditions ).evaluate(swarm)
    try: # in case there are no swarm particles on this processor
        ymin = np.min(positions[:,1])
    except:
        ymin = ymax
    if(rank!=0):
        # send min to proc0
        comm.send(ymin, dest=0, tag=0)
    else:
        for iProc in range(1,nprocs):
            ytemp = comm.recv(source=iProc, tag=0)
            ymin = np.min((ymin, ytemp))
    return ymin # only proc0 will have correct value in parallel runs


# **Test minimum y coordinate function**

# In[10]:

ymin = GetSwarmYMin()
if(uw.rank()==0):
    print('Minimum y value for sinker = {0:.3e}'.format(ymin))


# **Plot the particles by material**
# 
# Plot the initial positions of all swarm particles coloured by their material indices.

# In[11]:

fig1 = glucifer.Figure( figsize=(800,400) )
fig1.append( glucifer.objects.Points(swarm, materialIndex, colourBar=False, pointSize=2.0) )
fig1.show()


# Set up material parameters and functions
# -----
# 
# Here the functions for density and viscosity are set using the ``map`` function. This function evaluates a key function (here the material index), and the result (i.e. the key) is used to determine which function to evaluate to obtain the actual result (such as the particle density). 
# 
# For example if the material index of a particle is the light index number then the viscosity for that particle will be set to 1. If it had the heavy index number then it will be set to ``visc_sphere``, which can be either a function (say depending on temperature) or a constant as it is below.
# 
# The same approach is taken when setting up the density function for each particle in the swarm.

# In[12]:

# Set constants for the viscosity and density of the sinker.
viscSphere = 10.0
densitySphere = 10.0

# Here we set a viscosity value of '1.' for both materials 
mappingDictViscosity = { materialLightIndex:1., materialHeavyIndex:viscSphere }
# Create the viscosity map function.
viscosityMapFn = fn.branching.map( fn_key=materialIndex, mapping=mappingDictViscosity )
# Here we set a density of '0.' for the lightMaterial, and '1.' for the heavymaterial.
mappingDictDensity = { materialLightIndex:0., materialHeavyIndex:densitySphere }
# Create the density map function.
densityFn = fn.branching.map( fn_key=materialIndex, mapping=mappingDictDensity )

# And the final buoyancy force function.
z_hat = ( 0.0, 1.0 )
buoyancyFn = -densityFn * z_hat


# System setup
# -----
# 
# **Setup a Stokes system**

# In[13]:

stokesPIC = uw.systems.Stokes( velocityField = velocityField, 
                               pressureField = pressureField,
                               swarm         = swarm, 
                               conditions    = [freeslipBC,],
                               fn_viscosity   = viscosityMapFn, 
                               fn_bodyforce   = buoyancyFn )
solver = uw.systems.Solver( stokesPIC )


# **Create an advective diffusive system**
# 
# Note that we need to set up two advector systems, one for each particle swarm (our global swarm and the tracer particle).

# In[14]:

advector  = uw.systems.SwarmAdvector( swarm=swarm,       velocityField=velocityField, order=2 )


# Analysis tools
# -----
# 
# **RMS velocity**
# 
# Set up integrals used to calculate the RMS velocity.

# In[15]:

vdotv = fn.math.dot( velocityField, velocityField )
v2sum_integral  = uw.utils.Integral( mesh=mesh, fn=vdotv )
volume_integral = uw.utils.Integral( mesh=mesh, fn=1. )


# Main simulation loop
# -----
# 
# The main time stepping loop begins here. Before this the time and timestep are initialised to zero and the output statistics arrays are set up. Also the frequency of outputting basic statistics to the screen is set in steps_output.
# 
# Note that there are two ``advector.integrate`` steps, one for each swarm, that need to be done each time step.

# In[16]:

# Stepping. Initialise time and timestep.
time = 0.
step = 0
nsteps = 10
if(uw.rank()==0):
    tSinker = np.zeros(nsteps)
    ySinker = np.zeros(nsteps)
# Perform 10 steps
while step<nsteps:
    # Calculate and store/output at the start of each step (before solvers)

    # Determine and store tracer location.
    ymin = GetSwarmYMin()
    # Calculate the RMS velocity
    vrms = math.sqrt( v2sum_integral.evaluate()[0] / volume_integral.evaluate()[0] )
    if(uw.rank()==0):
        ySinker[step] = ymin
        tSinker[step] = time
        print('step = {0:6d}; time = {1:.3e}; v_rms = {2:.3e}; height = {3:.3e}'
              .format(step,time,vrms,ymin))
    
    # Get solution for initial configuration.
    solver.solve()
    # Retrieve the maximum possible timestep for the advection system.
    dt = advector.get_max_dt()
    # Advect using this timestep size.
    advector.integrate(dt)
    # Increment
    time += dt
    step += 1


# Post simulation analysis
# -----
# 
# **Plot tracer particle**
# 
# Plot the vertical component of the tracer particle's position as a function of time.

# In[17]:

if(uw.rank()==0):
    print('Initial position: t = {0:.3f}, y = {1:.3f}'.format(tSinker[0], ySinker[0]))
    print('Final position:   t = {0:.3f}, y = {1:.3f}'.format(tSinker[nsteps-1], ySinker[nsteps-1]))

    pylab.rcParams[ 'figure.figsize'] = 8, 6
    pyplot.plot(tSinker, ySinker)
    pyplot.xlabel('Time')
    pyplot.ylabel('Height of bottom of sinker')
    pyplot.savefig('SinkerYvsT.png')


# **Plot the final particle positions**

# In[18]:

fig1.show()


# **Plot velocity field**
# 
# Plot the velocity field in the fluid induced by the motion of the sinking ball.

# In[19]:

velplotmax=0.02
fig3 = glucifer.Figure( figsize=(800,400) )
velmagfield = uw.function.math.sqrt( uw.function.math.dot(velocityField,velocityField) )
fig3.append( glucifer.objects.VectorArrows(mesh, velocityField/(1.5*velplotmax), arrowHead=0.2, scaling=0.15) )
fig3.append( glucifer.objects.Surface(mesh, velmagfield) )
fig3.show()

