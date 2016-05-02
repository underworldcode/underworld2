
# coding: utf-8

# Blankenbach Benchmark Case 1
# ======
# 
# Steady isoviscous thermal convection
# ----
# 
# Two-dimensional, incompressible, bottom heated, steady isoviscous thermal convection in a 1 x 1 box, see case 1 of Blankenbach *et al.* 1989 for details.
# 
# **This example introduces:**
# 1. Loading/Saving variables to disk.
# 2. Defining analysis tools.
# 3. Finding a steady state.
# 
# **Keywords:** Stokes system, advective diffusive systems, analysis tools
# 
# **References**
# 
# B. Blankenbach, F. Busse, U. Christensen, L. Cserepes, D. Gunkel, U. Hansen, H. Harder, G. Jarvis, M. Koch, G. Marquart, D. Moore, P. Olson, H. Schmeling and T. Schnaubelt. A benchmark comparison for mantle convection codes. Geophysical Journal International, 98, 1, 23–38, 1989
# http://onlinelibrary.wiley.com/doi/10.1111/j.1365-246X.1989.tb05511.x/abstract
# 

# In[1]:

import underworld as uw
from underworld import function as fn
import glucifer
import math
import numpy as np


# Setup parameters
# -----

# In[2]:

# Set domain dimension & size
dim = 2
boxHeight = 1.0
boxLength = 1.0
# Set grid resolution.
res = 128
# Set max & min temperautres
tempMin = 0.0
tempMax = 1.0


# Choose which Rayleigh number, see case 1 of Blankenbach *et al.* 1989 for details.

# In[3]:

case = "a" 
if(case=="a"):
    Ra=1.e4
    eta0=1.e23
elif(case=="b"):
    Ra=1.e5
    eta0=1.e22
else: 
    Ra=1.e6
    eta0=1.e21


# Set input and output file directory 

# In[4]:

inputPath  = 'BBInput/'
outputPath = 'BBOutput/'
# Make output directory if necessary.
if uw.rank()==0:
    import os
    if not os.path.exists(outputPath):
        os.makedirs(outputPath)


# Create mesh and variables
# ------

# In[5]:

mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = (res, res), 
                                 minCoord    = (0., 0.), 
                                 maxCoord    = (boxLength, boxHeight))

velocityField       = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=dim )
pressureField       = uw.mesh.MeshVariable( mesh=mesh.subMesh, nodeDofCount=1 )
temperatureField    = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=1 )
temperatureDotField = uw.mesh.MeshVariable( mesh=mesh,         nodeDofCount=1 )

# initialise velocity, pressure and temperatureDot field
velocityField.data[:]       = [0.,0.]
pressureField.data[:]       = 0.
temperatureField.data[:]    = 0.
temperatureDotField.data[:] = 0.


# Set up material parameters and functions
# -----
# 
# Set values and functions for viscosity, density and buoyancy force.

# In[6]:

# Set a constant viscosity.
viscosity = 1.

# Create our density function.
densityFn = Ra * temperatureField

# Define our vertical unit vector using a python tuple (this will be automatically converted to a function).
z_hat = ( 0.0, 1.0 )

# A buoyancy function.
buoyancyFn = densityFn * z_hat


# Set initial temperature field
# -----
# The initial temperature field can be loaded from a pre-run steady state data set ( ``LoadFromFile = True`` ) or set to a sinusodial perterbation ( ``LoadFromFile = False`` ).

# In[7]:

# Steady state temperature field to be loaded from data file.
LoadFromFile = True


# **If loading steady state data set**
# 
# Data is stored in h5 format from a 64\*64 grid resolution model.  Data has been saved for 3 different Rayleigh numbers, $Ra = 10^4$, $10^5$ or $10^6$. 
# 
# Once loaded the data will need to be re-meshed onto a new grid, unless the new resolution is also 64\*64.
# 
# For more information on using meshes see the user guide.
# 

# In[8]:

if(LoadFromFile == True):
    # Setup mesh and temperature field for 64*64 data file.
    mesh64 = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                       elementRes  = (64, 64), 
                                       minCoord    = (0., 0.), 
                                       maxCoord    = (boxLength, boxHeight),
                                       partitioned = False )
    temperatureField64  = uw.mesh.MeshVariable( mesh=mesh64, nodeDofCount=1 ) 
    
    # read in saved steady state temperature field data
    if( case == "a" ):
        temperatureField64.load(inputPath+'tempfield_inp_64_Ra1e4.h5')
        print('Loading 64*64 for Ra = 1e4')
    elif( case == "b" ):
        temperatureField64.load(inputPath+'tempfield_inp_64_Ra1e5.h5')
        print('Loading 64*64 for Ra = 1e5')
    else:
        temperatureField64.load(inputPath+'tempfield_inp_64_Ra1e6.h5')
        print('Loading 64*64 for Ra = 1e6')
        
    if( res==64 ): # no remeshing needed, copy directly
        temperatureField.data[:] = temperatureField64.data[:]
    else: # remeshing needed
        temperatureField.data[:] = temperatureField64.evaluate(mesh)


# **If using sinusodial perturbation**

# In[9]:

if(LoadFromFile == False):
    temperatureField.data[:] = 0.
    pertStrength = 0.1
    deltaTemp = tempMax - tempMin
    for index, coord in enumerate(mesh.data):
        pertCoeff = math.cos( math.pi * coord[0]/boxLength ) * math.sin( math.pi * coord[1]/boxLength )
        temperatureField.data[index] = tempMin + deltaTemp*(boxHeight - coord[1]) + pertStrength * pertCoeff
        temperatureField.data[index] = max(tempMin, min(tempMax, temperatureField.data[index]))


# **Show initial temperature field**
# 
# Save to file if running in parallel.

# In[10]:

fig = glucifer.Figure()
fig.append( glucifer.objects.Surface(mesh, temperatureField) )
fig.show()


# Create boundary conditions
# ----------
# 
# Set temperature boundary conditions on the bottom ( ``MinJ`` ) and top ( ``MaxJ`` ).

# In[11]:

for index in mesh.specialSets["MinJ_VertexSet"]:
    temperatureField.data[index] = tempMax
for index in mesh.specialSets["MaxJ_VertexSet"]:
    temperatureField.data[index] = tempMin


# Construct sets for the both horizontal and vertical walls. Combine the sets of vertices to make the ``I`` (left and right side walls) and ``J`` (top and bottom walls) sets. Note that both sets contain the corners of the box.

# In[12]:

iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]

freeslipBC = uw.conditions.DirichletCondition( variable        = velocityField, 
                                               indexSetsPerDof = (iWalls, jWalls) )
tempBC     = uw.conditions.DirichletCondition( variable        = temperatureField, 
                                               indexSetsPerDof = (jWalls,) )


# System setup
# -----
# 
# **Setup a Stokes system**
# 

# In[13]:

stokesPIC = uw.systems.Stokes( velocityField = velocityField, 
                               pressureField = pressureField,
                               conditions    = [freeslipBC,],
                               fn_viscosity   = viscosity, 
                               fn_bodyforce   = buoyancyFn )
# get the default stokes equation solver
solver = uw.systems.Solver( stokesPIC )


# **Create an advection diffusion system**
# 

# In[14]:

advDiff = uw.systems.AdvectionDiffusion( phiField       = temperatureField, 
                                         phiDotField    = temperatureDotField, 
                                         velocityField  = velocityField, 
                                         fn_diffusivity = 1.0, 
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

# In[15]:

nuTop    = uw.utils.Integral( fn=temperatureField.fn_gradient[1], 
                              mesh=mesh, integrationType='Surface', 
                              surfaceIndexSet=mesh.specialSets["MaxJ_VertexSet"])

nuBottom = uw.utils.Integral( fn=temperatureField,               
                              mesh=mesh, integrationType='Surface', 
                              surfaceIndexSet=mesh.specialSets["MinJ_VertexSet"])


# In[16]:

nu = - nuTop.evaluate()[0]/nuBottom.evaluate()[0]
print('Nusselt number = {0:.6f}'.format(nu))


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

# In[17]:

def FindVrms(vel_field, mesh):

    intVdotV = uw.utils.Integral( fn.math.dot( vel_field, vel_field ), mesh )
    return ( math.sqrt( intVdotV.evaluate()[0] ) )

vrms = FindVrms(velocityField, mesh)
print('Initial vrms = {0:.3f}'.format(vrms))


# Main simulation loop
# -----

# If the initial conditions are loaded from file then this loop will only take a single step. If you would like to run the entire simulation from a small perturbation then change the ``LoadFromFile`` variable above to equal ``False``. Warning: the simulation will take a long time to get to steady state.

# In[18]:

#initialise time, step, output arrays
time = 0.
step = 0
timeVal = []
vrmsVal = []

# starting from steady state == True
if(LoadFromFile == True):
    steps_end = 1
else:
    steps_end = 5000

# output frequency
steps_output = min(100, steps_end/10)
steps_output = max(steps_output, 1)
iloop      = True
epsilon    = 1.e-8

velplotmax = 0.0
nuLast     = -1.0


# In[19]:

# Perform steps.
while iloop == True:

    # Solving the Stokes system.
    solver.solve()
    # Determining the maximum timestep for advancing the a-d system.
    dt = advDiff.get_max_dt()
    if step == 0:
        dt = 0.
    # Advect using this timestep size. 
    advDiff.integrate(dt)
    
    # Calculate & store the RMS velocity and Nusselt number.
    vrms = FindVrms( velocityField, mesh )
    nu   = - nuTop.evaluate()[0]/nuBottom.evaluate()[0]
    vrmsVal.append(vrms)
    timeVal.append(time)
    velplotmax = max(vrms, velplotmax)

    # print output statistics 
    if step%(steps_end/steps_output) == 0:
        if(uw.rank()==0):
            print('steps = {0:6d}; time = {1:.3e}; v_rms = {2:.3f}; Nu = {3:.3f}; Rel change = {4:.3e}'
              .format(step, time, vrms, nu, abs((nu - nuLast)/nu)))
    # Check loop break conditions.
    if(abs((nu - nuLast)/nu) < epsilon):
        iloop = False
        if(uw.rank()==0):
            print('steps = {0:6d}; time = {1:.3e}; v_rms = {2:.3f}; Nu = {3:.3f}; Rel change = {4:.3e}'
              .format(step, time, vrms, nu, abs((nu - nuLast)/nu)))
    nuLast = nu
    if step>=steps_end:
        iloop = False

    # Increment time and timestep counter
    time += dt
    step += 1


# Post analysis
# -----
# 
# **Benchmark values**
# 
# The time loop above outputs $v_{rms}$ and $Nu$ as general statistics for the system. For comparison, the benchmark values for the RMS velocity and Nusselt number are shown below for different Rayleigh numbers. All benchmark values shown below were determined in Blankenbach *et al.* 1989 by extroplation of numerical results. 
# 
# 
# | $Ra$          | $v_{rms}$   | $Nu$  |   $q_1$  | $q_2$  |
# | ------------- |:-------------:|:-----:|:-----:|:-----:|
# | 10$^4$ | 42.865 |  4.884 | 8.059 | 0.589 |
# | 10$^5$ | 193.215 | 10.535 | 19.079 | 0.723 |
# | 10$^6$ | 833.990 | 21.972 | 45.964 | 0.877 |
# 
# 
# For practicality, quick tests can be run to see that the results are close enough. Tests run using underworld gave
# 
# | $Ra$          | $v_{rms}$   | $Nu$  |
# | ------------- |:-------------:|:-----:|
# | 10$^4$ | 42.858 |  4.848 |
# | 10$^5$ | 193.449 | 10.248 |
# | 10$^6$ | 844.218 | 19.660 |
# 

# **Resulting pressure field**
# 
# Use the same method as above to plot the new temperature field. This can also be used to plot the pressure field, or any other data structures of interest.

# In[20]:

figtemp = glucifer.Figure()
figtemp.append( glucifer.objects.Surface( mesh, pressureField ) )
figtemp.show()


# **Plot the velocity vector field**
# 
# For this example the velocity field is interesting to see. This is visualised in two ways, firstly plotting a surface colour map of the velocity magnitude, and secondly the velocity vectors at points on the mesh. For aesthetics the vector arrows are scaled by a little more than the maximum $v_{rms}$ value found in the time loop above.

# In[21]:

fig2 = glucifer.Figure()
velmagfield = uw.function.math.sqrt( uw.function.math.dot( velocityField, velocityField ) )
fig2.append( glucifer.objects.VectorArrows(mesh, velocityField/(2.5*velplotmax), arrowHead=0.2, scaling=0.1) )
fig2.append( glucifer.objects.Surface(mesh, temperatureField) )
fig2.show()


# Parallel friendly post analysis
# ----
# 
# When running underworld in parallel the data of each mesh variable is spread across all the processors. However often we will want to calculate a quantity based on data at specific points that may not all be on the same processor.
# 
# A solution is presented here which consists of saving the data from all processors to file, then reloading the mesh variable data using a new non-partitioned mesh. This enables all the data to be available to each processor. We will the carry out the post analysis using the first processor.

# **Save temperature, pressure and velocity data**
# 
# Save the basic mesh variable data to files using the HDF5 format. This is the same file type as is loaded above.

# In[22]:

mesh.save(outputPath+"mesh.h5")
temperatureField.save(outputPath+'tempfield.h5')
pressureField.save(outputPath+'presfield.h5')
velocityField.save(outputPath+'velfield.h5')


# **Construct new mesh and variable on non-partitioned mesh**
# 
# Read saved mesh variable data into a new mesh variable where the information is not partitioned across multiple processors. This means that we can use a single processor to access all the data and calculate some quantities of interest.

# In[23]:

# build a non-partitioned mesh with same box size
mesh0 = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                  elementRes  = (res, res), 
                                  minCoord    = (0., 0.), 
                                  maxCoord    = (boxLength, boxHeight),
                                  partitioned = False ) 
# load previous mesh coordinate data onto new non-partitioned mesh
mesh0.load(outputPath+'mesh.h5')

# load T, P and V data onto the new mesh
# note that pressure is always on the submesh
temperatureField0  = uw.mesh.MeshVariable( mesh=mesh0, nodeDofCount=1 ) 
pressureField0     = uw.mesh.MeshVariable( mesh=mesh0.subMesh, nodeDofCount=1 ) 
velocityField0     = uw.mesh.MeshVariable( mesh=mesh0, nodeDofCount=dim ) 
temperatureField0.load(outputPath+"tempfield.h5")
pressureField0.load(outputPath+"presfield.h5")
velocityField0.load(outputPath+"velfield.h5")


# **Temperature gradient**
# 
# The final benchmarks in the Blankenbach paper involve the temperature gradient in the vertical direction ($\frac{\partial T}{\partial z}$). This is easy to find using the underworld functions, as shown below.
# 

# In[24]:

if(uw.rank()==0):
    tempgradField = temperatureField0.fn_gradient
    vertTGradField = - boxHeight * tempgradField[1] / tempMax # scaled for direct benchmarking below


# **More benchmark values**
# 
# 
# The vertical temperature gradient (above) is set up to be non-dimensional as per Blankenbach et al 1989. To compare to the benchmark values in their work the gradient is compared at the corners of the simulation box: $q_1$ at $x = 0$, $z = h$; $q_2$ at $x = l$, $z = h$; $q_3$ at $x = l$, $z = 0$; $q_4$ at $x = 0$, $z = 0$. Where $h$ = Box_Height and $l$ = Box_Length and the non-dimensional gradient field is given by
# \\[
# q = \frac{-h}{\Delta T} \left( \frac{\partial T}{\partial z} \right)
# \\]
# 
# Provided the simulation is run to steady-state with sufficent resolution then the $q$ values should be close to the benchmark values given again below for different Rayleigh numbers.
# 
# | $Ra$          |    $q_1$  | $q_2$  |
# | ------------- |:-----:|:-----:|
# | 10$^4$ | 8.059 | 0.589 |
# | 10$^5$ | 19.079 | 0.723 |
# | 10$^6$ | 45.964 | 0.877 |
# 
# The underworld values for $128\times128$ close to convergence are
# 
# | $Ra$          |    $q_1$  | $q_2$  |
# | ------------- |:-----:|:-----:|
# | 10$^4$ | 8.020 | 0.589 |
# | 10$^5$ | 18.409 | 0.719 |
# | 10$^6$ | 36.769 | 0.831 |
# 

# In[25]:

if(uw.rank()==0):
    q1 = vertTGradField.evaluate( (0., boxHeight))[0][0]
    q2 = vertTGradField.evaluate( (boxLength, boxHeight))[0][0]
    q3 = vertTGradField.evaluate( (boxLength, 0.))[0][0]
    q4 = vertTGradField.evaluate( (0., 0.))[0][0]

    print('Rayleigh number = {0:.1e}'.format(Ra))
    print('q1 = {0:.3f}; q2 = {1:.3f}'.format(q1, q2))
    print('q3 = {0:.3f}; q4 = {1:.3f}'.format(q3, q4))


# **Save time and rms values**
# 
# The following command uses the ``numpy`` package save to text file function to output all $v_{RMS}$ values as a function of time. This is particularly useful if you have run the simulation from the perturbed initial condition rather than the saved data file, as you can see the system coming to steady state.
# 
# The format for this text file is:
# 
#     timeVal[0], vrmsVal[0]
#     timeVal[1], vrmsVal[1]
#     ...
#     timeVal[N], vrmsVal[N]

# In[26]:

if(uw.rank()==0):
    np.savetxt(outputPath+'vrms.txt', np.c_[timeVal, vrmsVal], header="Time, VRMS" )


# **Calculate stress values for benchmark comparison**
# 
# 
# Determine stress field for whole box in dimensionless units (King 2009)
# \begin{equation}
# \tau_{ij} = \eta \frac{1}{2} \left[ \frac{\partial v_j}{\partial x_i} + \frac{\partial v_i}{\partial x_j}\right]
# \end{equation}
# which for vertical normal stress becomes
# \begin{equation}
# \tau_{zz} = \eta \frac{1}{2} \left[ \frac{\partial v_z}{\partial z} + \frac{\partial v_z}{\partial z}\right] = \eta \frac{\partial v_z}{\partial z}
# \end{equation}
# which is implemented for the whole box in the functions defined below.

# In[27]:

# get topography from non-partitioned stress tensor
if(uw.rank()==0):
    stresstensorFn = 2.* stokesPIC.fn_viscosity*fn.tensor.symmetric( velocityField0.fn_gradient ) - pressureField0
    verticalStressFn  = stresstensorFn[1]
    stress_zz_top     = -verticalStressFn.evaluate(mesh0.specialSets["MaxJ_VertexSet"])

# subtract the average value for benchmark.
    mean_sigma_zz_top = np.mean(stress_zz_top)
    sigma_zz_top      = stress_zz_top - mean_sigma_zz_top


# Dimensionalise the stress from the vertical normal stress at the top of the box (King 2009)
# 
# $$
#     \sigma_{t} = \frac{\eta_0 \kappa}{\rho g h^2}\tau _{zz} \left( x, z=h\right)
# $$
# 
# where all constants have been defined above. Finally calculate the topography, defined using $h = \sigma_{top} / (\rho g)$.

# In[28]:

# Set parameters in SI units
if(uw.rank()==0):
    grav = 10        # m.s^-2
    height = 1.e6    # m 
    rho  = 4.0e3     # g.m^-3
    kappa  = 1.0e-6  # m^2.s^-1

# dimensionalise 
    dim_sigma_zz_top  = (eta0 * kappa / (height*height)) * sigma_zz_top

# find topography in [m]
    topography        = dim_sigma_zz_top / (rho * grav) 


# **Calculate x-coordinate at zero stress**
# 
# Calculate the zero point for the stress along the x-axis at the top of the box using the **interpolation function** from ``numpy``. Note that ``numpy`` requires that the first array input for ``np.interp`` must be increasing, so the negative of the topography is used.

# In[29]:

if(uw.rank()==0):
    xCoordFn = fn.input()[0]
    x = xCoordFn.evaluate(mesh0.specialSets["MinJ_VertexSet"])
    xIntercept = np.interp(0.0,-1.0*topography[:, 0],x[:, 0])


# **Topography comparison**
# 
# Topography of the top boundary calculated in the left and right corners as given in Table 9 of Blankenbach et al 1989.
# 
# | $Ra$          |    $\xi_1$  | $\xi_2$  |  $x$ ($\xi = 0$) |
# | ------------- |:-----------:|:--------:|:--------------:|
# | 10$^4$  | 2254.02   | -2903.23  | 0.539372          |
# | 10$^5$  | 1460.99   | -2004.20  | 0.529330          |
# | 10$^6$  | 931.96   | -1283.80  | 0.506490          |
# 
# Underworld values for an "almost" converged $128\times128$ benchmark run
# 
# | $Ra$          |    $\xi_1$  | $\xi_2$  |  $x$ ($\xi = 0$) |
# | ------------- |:-----------:|:--------:|:--------------:|
# | 10$^4$  | 2243.93   | -2909.12  | 0.540373          |
# | 10$^5$  | 1451.52   | -2005.83  | 0.530246          |
# | 10$^6$  | 911.30    | -1282.47  | 0.509696          |
# 

# In[30]:

if(uw.rank()==0):
    e1 = float(topography[0])
    e2 = float(topography[len(topography)-1])
    print('Rayleigh number = {0:.1e}'.format(Ra))
    print('Topography[x=0],[x=max] = {0:.2f}, {1:.2f}'.format(e1, e2))
    print('x(topo=0) = {0:.6f}'.format(xIntercept))
    # output a summary file with benchmark values (useful for parallel runs)
    np.savetxt(outputPath+'summary.txt', [Ra, e1, e2, xIntercept, q1, q2, q3, q4])


# In[ ]:



