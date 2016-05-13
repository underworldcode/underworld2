
# coding: utf-8

# Lemiale et al 2008
# =====
# 
# Shear banding analysis of plastic models formulated for incompressible viscous flows
# -----
# 
# 
# Uses underworld to simulate the deformation and failure of the lithosphere coupled with the mantle convection. Aim to reproduce the results for the angles of the shear bands for different initial parameters given in table 2 of [Lemiale et al 2008](http://www.sciencedirect.com/science/article/pii/S0031920108001933).
# 
# This example has a complicated rheology which requires solving by an iterative method to get the viscosity of the plastic region. This plastic region also has a memory, meaning that the plastic strain is calculated by the previous strain rate integrated up to the current time.
# 
# This model requires high resolution to accurately reproduce the benchmark values, and so is ideally suited to parallel computing. However the quick analysis at the end (a single angle of the shear band) naturally lends itself to serial processing. To address this the simulation is run in parallel to get the resulting stress field, which is then remapped to a single processor which performs the analysis.
# 
# 
# **This example covers the concepts of:**
# 1. Complex rheologies and yielding
# 2. iterative viscosity functions
# 3. shear band models
# 4. running post analysis on a single processor after a multiple procesor simulation (also runs in serial)
# 
# **Keywords:** particle swarms, Stokes system, advective diffusive systems, complex rheology, parallel analysis
# 
# **References**
# 
# 1. V. Lemiale, H.-B. Mühlhaus, L. Moresi, and J. Stafford, Shear banding analysis of plastic models formulated for incompressible viscous flows. Physics of the Earth and Planetary Interiors (2008) 171: 177-186.
# http://www.sciencedirect.com/science/article/pii/S0031920108001933
# 

# In[1]:

import numpy as np
import math
import os
import time as cpuTime
import underworld as uw
from underworld import function as fn
from underworld import meshvariable as meshvariable
import glucifer


# Set benchmark variables from table 2 of Lemiale et al 2008

# In[2]:

tanPhi = 0.2
iExtension = True
cinf = 3.95


# **Set parameters from Lemiale et al 2008 (table 1)**
# 
# NOTE: this is a quick version, so there is a resolution factor to reduce the resolution down to a bare minimum that still shows the banding effect. The angles will be different for low resolution runs compared to the benchmark values given in table 2 of Lemiale et al.

# In[3]:

# set resolution etc
resfact = 4
resX = 384/resfact
resY = 128/resfact

numPartPerCell = 60 # orig 20

stressCrit = 1.0 # ad hoc test (rough division observed in simple stress field)


if(iExtension==True):
    width = 3.0  # 3 for extension
else:
    width = 4.0  # 4 for compression
maxWidth = 2.0 # maximum width for any model (=4/2) used for plotting

xmin = -width/2.
xmax = width/2.

# set viscosity parameters
etaA = 0.01 # accommodation layer - sticky air
etaW = 1.   # weak zone
etaV = 100. # viscoplastic layer


if(iExtension==True):
    cohesion = 4.   # extension
    minXv = -0.05
    maxXv = -minXv
else:
    cohesion = 20.  # compression
    minXv = 2./3.
    maxXv = -minXv

thicknessV = 0.8  # thickness of sample layer
thicknessA = 0.2  # thickness of accommodation layer
dWeak      = 0.04 # distance along each side of the weak square

rhoA = 0.
rhoV = 1. # including weak layer

phi = np.arctan(tanPhi)


# In[4]:

# As a precaution ... keep workdir. 
try:
    workdir
except NameError:
    workdir = os.path.abspath(".")

# check outputs directory exists, make it if it doesn't
outputPath = os.path.join(workdir,"outputs/")

if uw.rank() == 0:
    if not os.path.exists(outputPath):
        os.makedirs(outputPath)
    


# Create mesh and finite element variables
# ------
# 
# Note: the use of a pressure-sensitive rheology suggests that it is important to use a Q2/dQ1 element 

# In[5]:

mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = ( resX, resY), 
                                 minCoord    = ( xmin, 0.), 
                                 maxCoord    = ( xmax, 1.),
                                 periodic    = [False, False]  ) 

velocityField    = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=mesh.dim )
pressureField    = uw.mesh.MeshVariable( mesh=mesh.subMesh, nodeDofCount=1 )

velocityField.data[:] = [0.,0.]
pressureField.data[:] = 0.


# ### Boundary conditions
# 
# Pure shear with moving  walls — all boundaries are zero traction with 

# In[6]:

iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]
base   = mesh.specialSets["MinJ_VertexSet"]
top    = mesh.specialSets["MaxJ_VertexSet"]

allWalls = iWalls + jWalls

velocityBCs = uw.conditions.DirichletCondition( variable        = velocityField, 
                                                indexSetsPerDof = (iWalls, base) )

for index in mesh.specialSets["MinI_VertexSet"]:
    velocityField.data[index] = [minXv, 0.]
for index in mesh.specialSets["MaxI_VertexSet"]:
    velocityField.data[index] = [maxXv, 0.]
    


# ### Setup the material swarm
# 
# This is used for tracking deformation and history dependence of the rheology

# In[7]:

swarm         = uw.swarm.Swarm( mesh=mesh )
swarmLayout   = uw.swarm.layouts.GlobalSpaceFillerLayout( swarm=swarm, particlesPerCell=20 )
swarm.populate_using_layout( layout=swarmLayout )


# ### Create a particle advection system
# 
# Note that we need to set up one advector systems for each particle swarm (our global swarm and a separate one if we add passive tracers).

# In[8]:

advector        = uw.systems.SwarmAdvector( swarm=swarm,        velocityField=velocityField, order=2 )


# ### Add swarm variables
# 
# We are using a single material with a single rheology. We need to track the plastic strain in order to have some manner of strain-related softening (e.g. of the cohesion or the friction coefficient). For visualisation of swarm data we need an actual swarm variable and not just the computation.
# 
# Other variables are used to track deformation in the shear band etc.
# 
# **NOTE**:  Underworld needs all the swarm variables defined before they are initialised or there will be / can be memory problems (at least it complains about them !). That means we need to add the monitoring variables now, even if we don't always need them.

# In[9]:

# Tracking different materials
plasticStrain  = swarm.add_variable( dataType="double",  count=1 )
materialVariable = swarm.add_variable( dataType="int", count=1 )

# These ones are for monitoring of the shear bands
stretching = swarm.add_variable( dataType="double", count=mesh.dim)
orientation = swarm.add_variable( dataType="double", count=1)


# ### Initialise swarm variables
# 
# 

# In[10]:

# Stretching - assume an initial orientation aligned with the x-axis
stretching.data[:,0] = 1.0
stretching.data[:,1] = 0.0

# This is a work-variable for visualisation
orientation.data[:] = 0.0

# Plastic strain- initialise with zero. Weakness in bottom centre of domain is caused by low viscosity defect.
plasticStrain.data[:] = 0.0


# ### Material distribution in the domain.
# 
# 

# In[11]:

# Initialise the 'materialVariable' data to represent different materials. 
materialV = 0 # viscoplastic
materialW = 1 # weak
materialA = 2 # accommodation layer a.k.a. Sticky Air

# The particle coordinates will be the input to the function evaluate (see final line in this cell).
# We get proxy for this now using the input() function.
coord = fn.input()

# Setup the conditions list for the following conditional function. Where the
# z coordinate (coordinate[1]) is less than the perturbation, set to lightIndex.
conditions = [ (                                  coord[1] > thicknessV , materialA ),
               ( ((coord[1] < dWeak) & (coord[0]**2. < (dWeak**2.)/4.)) , materialW ),
               (                                                   True , materialV ) ]

# The actual function evaluation. Here the conditional function is evaluated at the location
# of each swarm particle. The results are then written to the materialVariable swarm variable.
materialVariable.data[:] = fn.branching.conditional( conditions ).evaluate(swarm)


# Define the density function
# ---

# In[12]:

# Here we set a density for each material - constants defined at the top
densityMap   = { materialA:rhoA, materialV:rhoV, materialW:rhoV }
densityFn    = fn.branching.map( fn_key = materialVariable, mapping = densityMap )


# Define the viscosity function
# ----
# 
# In this case, the viscosity of material which has not reached the yield criterion is simply going to be a constant. Nevertheless, it is useful to define it here as a function and write the remaining code such that it is not changed if we introduce additional materials or a dependence on another set of equations.
# 
# **Define first iteration of first timestep**
# 
# Set all viscosities to the constant values. For the plastic region this is used to calculate the effective viscosity by iterating later.
# 

# In[13]:

viscosityMap = { materialA:etaA, materialV:etaV, materialW:etaW }
viscosityFn  = fn.branching.map( fn_key = materialVariable, mapping = viscosityMap )


# Plastic viscosity
# ----
# 
# Iterative method to get the viscosity of the plastic region
# 
# \begin{equation}
#     \eta = 
#         \begin{cases}
#               \eta_0 & f < 0 \\
#               \frac{3 \alpha p + k}{\dot{\gamma}} & f \geq 0
#         \end{cases}
# \end{equation}
# 
# where we adopt a Drucker-Prager type failure criterion:
# 
# \begin{equation}
#     f = \hat{\tau} - 3 \alpha P 
# \end{equation}
# 
# with
# \begin{equation}
# \alpha = \frac{\sin(\phi)}{3} \quad \quad k = c \cos(\phi)
# \end{equation}
# and
# \begin{equation}
# c = c_0 + \left( c_\infty - c_0 \right) \min \left( 1, \frac{\gamma^P}{\gamma^0} \right)
# \end{equation}
# where $\gamma^0 = 0.1$ is a reference strain. The plastic strain ($\gamma^P$) is calculated by the previous strain rate integrated up to the current time, i.e.
# \begin{equation}
# \gamma^P_{rel} = \int \left( \dot{\gamma}^P - \beta \frac{\tau_s}{\eta} \right) dt
# \end{equation}
# where $\beta$ is a parameter between 0 and 1 to describe the diminishing and healing of strain. For now we take $\beta = 0$.
# 
# 

# In[14]:

cohesionInf     = fn.misc.constant(cinf) # from table 2 of paper
cohesion0       = fn.misc.constant(cohesion)
referenceStrain = fn.misc.constant(0.1)

cosPhi = fn.math.cos(fn.misc.constant(phi)) # phi defined at the start based on entered tanPhi
sinPhi = fn.math.sin(fn.misc.constant(phi))

alpha = sinPhi/3.0

cohesionFn =  cohesion0 + (cohesionInf - cohesion0) * fn.exception.SafeMaths( 
                                                        fn.misc.min(1.0, plasticStrain / referenceStrain ))
kFn = cohesionFn * cosPhi

# first define strain rate tensor
strainRateFn = fn.tensor.symmetric( velocityField.fn_gradient )
strainRate_2ndInvariantFn = fn.tensor.second_invariant(strainRateFn)

# DeviatoricStress
devStressFn = 2.0 * viscosityFn * strainRateFn
devStressFn_2ndInvariantFn = fn.tensor.second_invariant(devStressFn)

# Failure criterion (f as given in equation 2)
yieldStressFn = devStressFn_2ndInvariantFn - 3.0 * alpha * pressureField


# In[15]:

# iterative over 
viscIterate = viscosityFn * (3.0 * alpha * pressureField + kFn) * fn.exception.SafeMaths( 
                                                        fn.misc.max(0.0, 1./(yieldStressFn+1.0e-14) ))

# apply criterion and calculate viscosity everywhere
conditions = [ ( yieldStressFn > 3.0 * alpha * pressureField + kFn , viscIterate ),     # plastic
               (                                              True , etaV        ) ]    # viscous

# The actual function evaluation. Here the conditional function is evaluated at the location
# of each swarm particle. The results are then written to the materialVariable swarm variable.
viscPlastic = fn.branching.conditional( conditions )

viscosityMap = { materialA:etaA, materialV:viscPlastic, materialW:etaW }
viscosityFn  = fn.branching.map( fn_key = materialVariable, mapping = viscosityMap )


# Buoyancy forces
# ----
# 
# Densites of materials are different, so gravity does play a role.
# 

# In[16]:

# Define our vertical unit vector using a python tuple (this will be automatically converted to a function).
z_hat = ( 0.0, 1.0 )

# Now create a buoyancy force vector using the density (FEvariable) and the vertical unit vector. 
buoyancyFn = -densityFn*z_hat


# System setup
# -----
# 
# Setup a Stokes equation system and connect a solver up to it.  
# 
# In this example, no buoyancy forces are considered. However, to establish an appropriate pressure gradient in the material, it would normally be useful to map density from material properties and create a buoyancy force.

# In[17]:

stokesPIC = uw.systems.Stokes( velocityField = velocityField, 
                               pressureField = pressureField,
                               swarm         = swarm, 
                               conditions    = [velocityBCs,],
                               fn_viscosity  = viscosityFn, 
                               fn_bodyforce  = buoyancyFn )

solver = uw.systems.Solver( stokesPIC )

## Initial solve (drop the non-linearity the very first solve only)
penaltyNumber=1.0e6
# direct solver
if(uw.nProcs>1): # multiple processor
    solver.set_inner_method("mumps")
else:            # single processors
    solver.set_inner_method("lu")

solver.set_penalty(penaltyNumber)
solver.solve( nonLinearIterate=True )
#solver.print_stats()


# Create some diagnostic figures
# ----
# 

# In[18]:

figVelocityPressure = glucifer.Figure( figsize=(1200,400), boundingBox=((-maxWidth, 0.0, 0.0), (maxWidth, 1.0, 0.0)) )
figVelocityPressure.append( glucifer.objects.VectorArrows(mesh, velocityField, scaling=1.) )
figVelocityPressure.append( glucifer.objects.Surface(mesh, pressureField) )
#figVelocityPressure.show()


# In[19]:

# The stress is only guaranteed to be accurate when integrated across an element. Fluctuations
# within an element can be significant. Mapping to the mesh can help

meshDevStress = uw.mesh.MeshVariable( mesh, 1 )

projectorStress = uw.utils.MeshVariable_Projection( meshDevStress, fn.tensor.second_invariant(devStressFn), type=0 )
projectorStress.solve()

figMeshStress = glucifer.Figure( figsize=(1200,400), boundingBox=((-maxWidth, 0.0, 0.0), (maxWidth, 1.0, 0.0)) )
figMeshStress.append( glucifer.objects.Surface(mesh, meshDevStress  , colours="#0044BB, #777777, #FF9900") )
#figMeshStress.show()


# In[20]:

curlV = velocityField.fn_gradient[1] - velocityField.fn_gradient[2]
figStrain = glucifer.Figure( figsize=(1200,400), boundingBox=((-maxWidth, 0.0, 0.0), (maxWidth, 1.0, 0.0)) )
figStrain.append( glucifer.objects.Surface(mesh, curlV) )
figStrain.append( glucifer.objects.Points(swarm, plasticStrain, pointSize=4.0, fn_mask=materialVariable,  colours="#FF6600:0.5, #555555:0.25, Blue:0.5") )
#figStrain.show()


# Main simulation loop
# -----
# 
# For computational speed the number of steps is reduced here. To get the correct banding effect ``nsteps`` should be run to about 20 for extension models and up to 40 for compression models.

# In[21]:

# Stepping. Initialise time and timestep.
time = 0.
step = 0
nsteps = 1 # change to 20 or 40 here
xBuffer = 1.0e-5
sBuffer = 1.0e-6
ymax = 1.0
ymin = 0.0


# In[22]:

startTime = cpuTime.clock()
while step<nsteps:
    solver.solve( nonLinearIterate=True )

    dt = advector.get_max_dt()
    advector.integrate(dt)
    
    # Stretch mesh to match boundary conditions
    xmin += minXv * dt
    xmax += maxXv * dt

    # failsafe in case swarm particle is just outside domain (e.g. xmin - 1e-5)
    swarmXMin = swarm.particleCoordinates.data[:,0].min()
    swarmXMax = swarm.particleCoordinates.data[:,0].max()
    xmin = min(xmin, swarmXMin - xBuffer)
    xmax = max(xmax, swarmXMax + xBuffer)

    # if in compression then the lid must raise to conserve total volume (area in 2D)
    if(iExtension==False):
        swarmYMax = swarm.particleCoordinates.data[:,1].max()
        swarm.particleCoordinates.data[:,0] = np.maximum(xmin+sBuffer, swarm.particleCoordinates.data[:,0])
        swarm.particleCoordinates.data[:,0] = np.minimum(xmax-sBuffer, swarm.particleCoordinates.data[:,0])
        swarm.particleCoordinates.data[:,1] = np.maximum(ymin+sBuffer, swarm.particleCoordinates.data[:,1])
        swarm.particleCoordinates.data[:,1] = np.minimum(ymax-sBuffer, swarm.particleCoordinates.data[:,1])

    # create the mesh
    mesh2 = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                      elementRes  = (resX, resY), 
                                      minCoord    = ( xmin, ymin), 
                                      maxCoord    = ( xmax, ymax),
                                      periodic    = [False, False]  ) 

    with mesh.deform_mesh():
        mesh.data[:] = mesh2.data[:]
    
    swarm.update_particle_owners()

    swarmVgrad = velocityField.fn_gradient.evaluate(swarm)
  
    stretching.data[:,0] += dt * (swarmVgrad[:,0] * stretching.data[:,0] + swarmVgrad[:,1] * stretching.data[:,1])
    stretching.data[:,1] += dt * (swarmVgrad[:,2] * stretching.data[:,0] + swarmVgrad[:,3] * stretching.data[:,1])
    
    # update plastic strain on those swarm particles that yielded
    swarmYield = viscosityFn.evaluate(swarm) < etaV
    swarmStrainRateInv = strainRate_2ndInvariantFn.evaluate(swarm)
    particleIsViscPlastic = materialVariable.evaluate(swarm) == materialV
    plasticStrainIncrement = dt * np.where(swarmYield, np.where(particleIsViscPlastic, swarmStrainRateInv, 0.0) , 0.0 )
    
    plasticStrain.data[:] += plasticStrainIncrement

    if (step%5 ==0):      
#        figVelocityPressure.save_image( outputPath + "figVP-" + str(step).zfill(4))
        projectorStress = uw.utils.MeshVariable_Projection( meshDevStress, 
                                                           fn.tensor.second_invariant(devStressFn), type=0 )
        projectorStress.solve()
#        figStrain.save_image( outputPath + "figStrain-" + str(step).zfill(4))
        
    if uw.rank()==0:   
        print('step = {0:3d}; time = {1:.3e}; xmax = {2:.3f}; pmax = {3:.4f}; cpu time = {4:.2e}'
              .format(step, time, xmax, plasticStrain.evaluate(swarm).max(), cpuTime.clock()-startTime))

    time += dt
    step += 1
  


# Post simulation images
# -----
# 
# Output images for final pressure and strain.

# In[23]:

#figVelocityPressure.save_image( outputPath + "figVP-" + str(step).zfill(4))
projectorStress = uw.utils.MeshVariable_Projection( meshDevStress, 
                                                           fn.tensor.second_invariant(devStressFn), type=0 )
projectorStress.solve()
#figStrain.save_image( outputPath + "figStrain-" + str(step).zfill(4))


# In[24]:

projectorStress = uw.utils.MeshVariable_Projection( meshDevStress, fn.tensor.second_invariant(devStressFn), type=0 )
projectorStress.solve()
#figMeshStress.save_image( outputPath + "figShearBand-" + str(step).zfill(4))


# Post-analysis: Shear band angle calculation
# ----
# 
# Calculate the angle the shear band makes to the vertical direction. This is clearest to see in the second invariant of the deviatoric stress tensor.
# 
# The deviatoric stress tensor needs to be known near the weak point at the centre, bottom on the simulation domain. As this simulation is (most likely) done in parallel then not all information needed near the weak point is known to any single processor. 
# 
# Therefore we must determine the shear band angle by 
# 
# 1. first collecting all data from all processors, easiest way to do this is to save the variable of interest and the mesh it exists on. 
# 2. create new mesh which is **not** partitioned. This means each processor sees the entire mesh.
# 3. create a new variable and load the previously stored data onto it.
# 4. use the new variable for analysis on a single processor.

# **Conduct analysis on the first processor only**
# 
# Output to screen and to a summary text file.

# In[32]:

radius = 0.2
nbin = 100
theta = np.arange(0.,np.pi/2.0, np.pi/(2.*float(nbin)))
fdev = np.zeros(nbin)
xx = np.zeros(nbin)
yy = np.zeros(nbin)
fmax = 0.0
for i in range(nbin):
    xx[i] = radius*np.cos(np.pi/2.0 - theta[i])
    yy[i] = radius*np.sin(np.pi/2.0 - theta[i])
positions = np.zeros((nbin,2))
for i in range(nbin):
    positions[i][0]=xx[i]
    positions[i][1]=yy[i]

# global evaluations must be written so all processors go through this part
#    (only rank=0 needs the correct info for the call though)
fdev = meshDevStress._evaluate_global( positions )

# back to rank=0 analysis
if uw.rank()==0: 
    for i in range(nbin):
        if(fmax<fdev[i]):
            fmax = fdev[i]
            thetamax = theta[i]
    if(iExtension):
        print('Extension model parameters:')
    else:
        print('Compression model parameters:')
    thetaDeg = thetamax*180./np.pi
    print('Res = {0:3d}*{1:3d}; C0 = {2:.1f}; Cinf = {3:.2f}'.format(resX, resY, cohesion, cinf))
    print('tan(phi) = {0:.1f}; theta = {1:.1f}'.format(tanPhi, thetaDeg))
    f = open('summary.out','w')
    f.write('{0:3d} {1:3d} {2:.1f} {3:.2f} {4:.1f} {5:.1f}'.format(resX, resY, cohesion, cinf, tanPhi, thetaDeg))
    f.close()


# In[28]:

fdev


# Benchmark values
# ---
# 
# For the default values of an extension model at resolution $384\times128$ with $\tan \phi = 0.2$ and $C_\infty = 3.95$ the angle of the shear bands at the point of the perturbation is $\theta = 38\pm2$ degrees. Shear band angles for other parameters are given in table 2 of Lemiale et al. 2008.

# Notes:
# ----
# 
# 1. This notebook will also work if run on a single processor.
# 2. The angles will be different for low resolution runs compared to the benchmark values.

# In[ ]:



