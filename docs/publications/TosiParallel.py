
# coding: utf-8

# Viscoplastic thermal convection in a 2-D square box: Tosi et al. 2015
# =====
# 
# This series of notebooks generates benchmark models from the Tosi et al. (2015) in Underworld2. The Underworld2 results are then directly compared to the results from Table 2 of Tosi et al. (2015) the paper for each case. The notebooks are broken into a general notebook for reproducing the models from scratch and 5 post-analysis notebooks which look at the results Underworld2 gives for each case after running to equilibrium. 
# 
# Parallel computer friendly version
# -------
# 
# This general notebook contains the conditions for each of the cases in Tose et al. (2015) as well as being written in a way that is parallel computer friendly. This means that to run across multiple processors download this notebook as a python file and use this on your cluster of choice. Or use the supplied ``TosiParallel.py`` python script and run with a command like:
# 
#     mpirun -np 4 python TosiParallel.py 
# 
# To run case 5b modify the case 5a numbers to use $\sigma_\gamma$ = 3 to 5 with increment of 0.1.
# 
# References
# -----
# 
# 1. Tosi, Nicola and Stein, Claudia and Noack, Lena and H&uuml;ttig, Christian and Maierov&aacute;, Petra and Samuel, Henri and Davies, DR and Wilson, CR and Kramer, SC and Thieulot, Cedric and others. 2015. _A community benchmark for viscoplastic thermal convection in a 2-D square box_.
# 

# Load python functions needed for underworld. Some additional python functions from os, math and numpy used later on.

# In[ ]:

import underworld as uw
import math
from underworld import function as fn
import time
import numpy as np
import os


# Set physical constants and parameters, including the Rayleigh number (*Ra*) and viscosity constants ($\eta^*$ and $\eta_T$) which are common to all Tosi et al cases.

# In[ ]:

res = 128        # simulation resolution (horizontal and vertical directions)
RA  = 1e2        # Rayleigh number
ETA_T = 1e5      # common viscosity constant
TS  = 0          # surface temperature
TB  = 1          # bottom boundary temperature (melting point)
ETA0 = 1e-3      # viscosity constant eta^* in Tosi et al
TMAX = 3.0
IMAX = 1000


# Select which case of Tosi et al (2015) to use. Must be between 1 and 5.

# In[ ]:

CASE = 2


# Setup a dictonary containing viscosity and stress values for each case.

# In[ ]:

case_dict = {}
case_dict[1] = {}
case_dict[1]['ETA_Y'] = 1.
case_dict[1]['YSTRESS'] = 1.
case_dict[2] = {}
case_dict[2]['ETA_Y'] = 1.
case_dict[2]['YSTRESS'] = 1.
case_dict[3] = {}
case_dict[3]['ETA_Y'] = 10.
case_dict[4] = {}
case_dict[4]['ETA_Y'] = 10.
case_dict[4]['YSTRESS'] = 1.
case_dict[5] = {}
case_dict[5]['ETA_Y'] = 10.
case_dict[5]['YSTRESS'] = 4.


# Set viscosity constants based on which Tosi et al case is being used.

# In[ ]:

ETA_Y = case_dict[CASE]['ETA_Y']
YSTRESS = case_dict[CASE]['YSTRESS']


# Set input and output file directories

# In[ ]:

# input file directory
inputPath  = 'TosiInput/case' + str(CASE) + "/"

# output files
outputPath = 'TosiOutput/'
filePath   = outputPath+'files'

# make directories if they don't exist
if not os.path.isdir(outputPath):
    os.makedirs(outputPath)
if not os.path.isdir(filePath):
    os.makedirs(filePath)


# Create mesh and finite element variables
# ------
# 
# Set up mesh and field variables that are solved on the mesh. See user guides for details of this process.

# In[ ]:

mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = (res, res), 
                                 minCoord    = (0., 0.), 
                                 maxCoord    = (1., 1.))

velocityField       = uw.fevariable.FeVariable( feMesh=mesh,         nodeDofCount=2 )
pressureField       = uw.fevariable.FeVariable( feMesh=mesh.subMesh, nodeDofCount=1 )
temperatureField    = uw.fevariable.FeVariable( feMesh=mesh,         nodeDofCount=1 )
temperatureDotField = uw.fevariable.FeVariable( feMesh=mesh,         nodeDofCount=1 )


# Set initial conditions and boundary conditions
# ----------
# 
# **Initial and boundary conditions**
# 
# Either set by perturbation function or load data from file.

# In[ ]:

LoadFromFile = True
savedRes = 128


# **If loading from file**
# 
# Read (``savedRes`` $\times$ ``savedRes``) resolution data for $P$, $v$ and $T$ fields as well as existing summary statistics data. These are converted into lists so that the main time loop below will append with new values.
# 

# In[ ]:

if(LoadFromFile == True):
# set up mesh for savedRes*savedRes data file
    meshSaved = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                       elementRes  = (savedRes, savedRes), 
                                       minCoord    = (0., 0.), 
                                       maxCoord    = (1., 1.) )
    temperatureFieldSaved    = uw.fevariable.FeVariable( feMesh=meshSaved,         nodeDofCount=1 )
    temperatureDotFieldSaved = uw.fevariable.FeVariable( feMesh=meshSaved,         nodeDofCount=1 )
    pressureFieldSaved       = uw.fevariable.FeVariable( feMesh=meshSaved.subMesh, nodeDofCount=1 )
    velocityFieldSaved       = uw.fevariable.FeVariable( feMesh=meshSaved,         nodeDofCount=2 )

    temperatureFieldSaved.load(    inputPath+'temperatureField.h5' )
    temperatureDotFieldSaved.load( inputPath+'temperatureDotField.h5' )
    pressureFieldSaved.load(       inputPath+'pressureField.h5')
    velocityFieldSaved.load(       inputPath+'velocityField.h5')
    
    if(res==savedRes): # no remeshing needed, copy data directly
        temperatureField.data[:]    = temperatureFieldSaved.data[:]
        pressureField.data[:]       = pressureFieldSaved.data[:]
        velocityField.data[:]       = velocityFieldSaved.data[:]
        temperatureDotField.data[:] = temperatureDotFieldSaved.data[:]
    else: # remeshing needed
        temperatureField.data[:]    = temperatureFieldSaved.evaluate( mesh )
        pressureField.data[:]       = pressureFieldSaved.evaluate( mesh.subMesh )
        velocityField.data[:]       = velocityFieldSaved.evaluate( mesh )
        temperatureDotField.data[:] = temperatureDotFieldSaved.evaluate( mesh )
        
    data = np.loadtxt( inputPath+'Summary.txt', unpack=True )
    stepsVal = data[0].tolist()
    timeVal = data[1].tolist()
    viscdiscVal = data[2].tolist()
    nu0Val = data[3].tolist()
    nu1Val = data[4].tolist()
    Avg_tempVal = data[5].tolist()
    vrmsVal = data[6].tolist()
    Rms_surfVal = data[7].tolist()
    Max_vx_surfVal = data[8].tolist()
    GravworkVal = data[9].tolist()
    etamaxVal = data[10].tolist()
    etaminVal = data[11].tolist() 


# In[ ]:

if(LoadFromFile == False):
# Initialise data.. Note that we are also setting boundary conditions here
    velocityField.data[:]  = [0.,0.]
    pressureField.data[:]  = 0.
    temperatureDotField.data[:] = 0.
    temperatureField.data[:] = 0.
# Setup temperature initial condition via numpy arrays
    A = 0.01
#Note that width = height = 1
    for index, coord in enumerate(mesh.data):
        pertCoeff = (1- coord[1]) + A*math.cos( math.pi * coord[0] ) * math.sin( math.pi * coord[1] )
        temperatureField.data[index] = pertCoeff;    
    stepsVal = []
    timeVal = []
    viscdiscVal = []
    nu0Val = []
    nu1Val = []
    Avg_tempVal = []
    vrmsVal = []
    Rms_surfVal = []
    Max_vx_surfVal = []
    GravworkVal = []
    etamaxVal = []
    etaminVal = []


# **Boundary conditions**
# 
# This step is to ensure that the temperature boundary conditions are satisfied, as the initial conditions above may have been set to different values on the boundaries.

# In[ ]:

for index in mesh.specialSets["MinJ_VertexSet"]:
    temperatureField.data[index] = TB
for index in mesh.specialSets["MaxJ_VertexSet"]:
    temperatureField.data[index] = TS


# **Conditions on the boundaries**
# 
# Construct sets for the both horizontal and vertical walls. Combine the sets of vertices to make the I (left and right side walls) and J (top and bottom walls) sets. Note that both sets contain the corners of the box.

# In[ ]:

iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]
freeslipBC = uw.conditions.DirichletCondition( variable      = velocityField, 
                                               nodeIndexSets = (iWalls, jWalls) )
tempBC     = uw.conditions.DirichletCondition( variable      = temperatureField, 
                                               nodeIndexSets = (jWalls,) )


# Set up material parameters and functions
# -----
# 

# **Viscosity field**
# 
# The viscosity is a function of temperature ($T$), the vertical coordinate ($z$) and the strain rate ($\dot{\epsilon}$) and is given by
# 
# $$
#     \eta(T, z, \dot{\epsilon}) = 2 \left( \frac{1}{\eta_{lin}(T,z)} + \frac{1}{\eta_{plast}(\dot{\epsilon})} \right)^{-1}
# $$
# 
# where the linear part is
# 
# $$
#     \eta_{lin}(T, z) = \exp(-\gamma_T T + \gamma_z z)
# $$
# 
# with $\gamma_T = \ln(\Delta \eta_T)$ and $\gamma_z = \ln(\Delta \eta_z)$. The nonlinear part is
# 
# $$
#     \eta_{plast}(\dot{\epsilon}) = \eta^* + \frac{\sigma_{\gamma}}{\sqrt{\dot{\epsilon} : \dot{\epsilon}}}
# $$
# 
# where $\eta^*$ is the effective viscosity at high stresses and $\sigma_{\gamma}$ is the yield stress. The values of $\Delta \eta_T$, $\Delta \eta_z$, $\eta^*$ and $\sigma_{\gamma}$ are assumed to be constant with values dependent on the particular case.

# **Create variables required for plasticity calculations**

# In[ ]:

secinv = fn.tensor.second_invariant( fn.tensor.symmetric( velocityField.gradientFn ) )
coordinate = fn.coord()


# **Setup viscosity functions**
# 
# Remember to use floats everywhere when setting up functions

# In[ ]:

viscosityl1 = fn.math.exp(math.log(ETA_T)*-1.*temperatureField)
viscosityl2 = fn.math.exp((math.log(ETA_T)*-1.*temperatureField) + (1.-coordinate[1])*math.log(ETA_Y))

#Von Mises effective viscosity
viscosityp = ETA0 + YSTRESS/(secinv/math.sqrt(0.5)) #extra factor to account for underworld second invariant form

if CASE == 1:
    viscosityFn = viscosityl1
elif CASE == 2:
    viscosityFn = 2./(1./viscosityl1 + 1./viscosityp)
elif CASE == 3:
    viscosityFn = viscosityl2
else:
    viscosityFn = 2./(1./viscosityl2 + 1./viscosityp)


# **Add functions for density and buoyancy**

# In[ ]:

densityFn = RA*temperatureField

# define our vertical unit vector using a python tuple (this will be automatically converted to a function)
z_hat = ( 0.0, 1.0 )

# now create a buoyancy force vector using the density (FEvariable) and the vertical unit vector. 
buoyancyFn = densityFn * z_hat


# Build the Stokes system and solvers
# ------
# 
# Setup the Stokes system with linear or nonlinear visocity viscosity depending on case.

# In[ ]:

stokesPIC = uw.systems.Stokes( velocityField = velocityField, 
                               pressureField = pressureField,
                               conditions    = [freeslipBC,],
                               viscosityFn   = fn.exception.SafeMaths( viscosityFn ), 
                               bodyForceFn   = buoyancyFn )


# Setup solver for the Stokes system of equations

# In[ ]:

solver = uw.systems.Solver( stokesPIC )


# We do one solve for initial pressure and velocity with linear viscosity to get the initial strain rate invariant. This solve step also calculates a 'guess' of the the velocity field based on the linear system, which is used later in the non-linear solver.

# In[ ]:

solver.solve( nonLinearIterate=False )


# Create an advective-diffusive system
# -----
# 
# Setup the system in underworld by flagging the temperature and velocity field variables.

# In[ ]:

advDiff = uw.systems.AdvectionDiffusion( temperatureField, temperatureDotField, velocityField,
                                         diffusivity=1., conditions=[tempBC,] )


# Metrics for benchmark
# =====
# 
# Define functions to be used in the time loop. For cases 1-4, participants were asked to report a number of diagnostic quantities to be measured after reaching steady state:
# 
# * Average temperature
# $$  \langle T \rangle  = \int^1_0 \int^1_0 T \, dxdy $$
# * Top and bottom Nusselt numbers
# $$Nu_{top/bot} = - \left. \int^1_0 \frac{\partial T}{\partial y} \right|_{y=1/y=0} \, dx$$
# * RMS velocity over the whole domain, surface and max velocity at surface
# $$u_{RMS} = \left( \int_0^1 \int_0^1 \left( u_x^2 + u_y^2 \right) dx dy \right)^{1/2}$$
# $$u_{RMS}^{surf} = \left( \left. \int_0^1 u_x^2\right|_{y=1} dx \right)^{1/2}$$
# $$u_{max}^{surf} = \left. \max(u_x)\right|_{y=1}$$
# * max and min viscosity over the whole domain
# $$\eta_{max/min} = \max / \min \left( \eta\left(x,y\right) \right)$$
# * average rate of work done against gravity
# $$\langle W \rangle = \int^1_0 \int^1_0 T u_y \, dx dy$$
# * and the average rate of viscous dissipation
# $$\langle \Phi \rangle = \int^1_0 \int^1_0 \tau_{ij} \dot \epsilon_{ij} \, dx dy$$
# 
# * In steady state, if thermal energy is accurately conserved, the difference between $\langle W \rangle$ and $\langle \Phi \rangle / Ra$ must vanish, so also reported is the percentage error: 
# 
# $$ \delta = \frac{\lvert \langle W \rangle - \frac{\langle \Phi \rangle}{Ra} \rvert}{max \left(  \langle W \rangle,  \frac{\langle \Phi \rangle}{Ra}\right)} \times 100% $$

# **Setup volume integrals used in metric functions**

# In[ ]:

tempint = uw.utils.Integral( temperatureField, mesh )
areaint = uw.utils.Integral( 1.,               mesh )

v2int   = uw.utils.Integral( fn.math.dot(velocityField,velocityField), mesh )

dwint   = uw.utils.Integral( temperatureField*velocityField[1], mesh )

sinner = fn.math.dot( secinv, secinv )
vdint = uw.utils.Integral( (4.*viscosityFn*sinner), mesh )


# **Setup surface integrals used in metric functions**

# In[ ]:

rmsSurfInt = uw.utils.Integral( fn=velocityField[0]*velocityField[0], mesh=mesh, integrationType='Surface', 
                          surfaceIndexSet=mesh.specialSets["MaxJ_VertexSet"])
nuTop      = uw.utils.Integral( fn=temperatureField.gradientFn[1],    mesh=mesh, integrationType='Surface', 
                          surfaceIndexSet=mesh.specialSets["MaxJ_VertexSet"])
nuBottom   = uw.utils.Integral( fn=temperatureField.gradientFn[1],    mesh=mesh, integrationType='Surface', 
                          surfaceIndexSet=mesh.specialSets["MinJ_VertexSet"])


# **Define diagnostic functions using integrals**

# In[ ]:

def avg_temp():
    return tempint.evaluate()[0]/areaint.evaluate()[0]

def nusseltTB(temp_field, mesh):
    return -nuTop.evaluate()[0], -nuBottom.evaluate()[0]

def rms():
    return math.sqrt(v2int.evaluate()[0]/areaint.evaluate()[0])

def rms_surf():
    return math.sqrt(rmsSurfInt.evaluate()[0])

def max_vx_surf(velfield, mesh):
    vuvelxfn = fn.view.min_max(velfield[0])
    vuvelxfn.evaluate(mesh.specialSets["MaxJ_VertexSet"])
    return vuvelxfn.max_global()

def gravwork(workfn):
    return workfn.evaluate()[0]

def viscdis(vdissfn):
    return vdissfn.evaluate()[0]

def visc_extr(viscfn):
    vuviscfn = fn.view.min_max(viscfn)
    vuviscfn.evaluate(mesh)
    return vuviscfn.max_global(), vuviscfn.min_global()


# Main simulation loop
# ------
# 
# The main time stepping loop begins here. Before this the time and timestep are initialised to zero and the output statistics arrays are set up. Also the frequency of outputting basic statistics to the screen is set in steps_output.
# 
# Since getting each case to equilibrium takes a long time the default number of steps to do is kept short. Edit this if you wish to go through the entire process yourself.

# In[ ]:

steps_todo = 10


# Initialise steps and time. Set to zero unless loading values from file.

# In[ ]:

try:
    realtime = timeVal[-1]
    step = int(stepsVal[-1])
except:
    realtime = 0.0
    step = 0
steps_end = step + steps_todo
steps_output = 100
steps_output = max(steps_output,1)


# In[ ]:

# initialise timer for computation
start = time.clock()
# start loop
while step < steps_end:
    # Solve non-linear system
    solver.solve(nonLinearIterate=True)
    dt = advDiff.get_max_dt()
    if step == 0:
        dt = 0.
    # Advect using this timestep size   
    advDiff.integrate(dt)
    # Increment
    realtime += dt
    step += 1
    # Calculate the Metrics, only on 1 of the processors:
    Avg_temp = avg_temp()
    Rms = rms()
    Rms_surf = rms_surf()
    Max_vx_surf = max_vx_surf(velocityField, mesh)
    Gravwork = gravwork(dwint)
    Viscdis = viscdis(vdint)
    nu1, nu0 = nusseltTB(temperatureField, mesh) # return top then bottom
    etamax, etamin = visc_extr(viscosityFn)
    # store results
    stepsVal.append(       step)
    timeVal.append(        realtime )
    viscdiscVal.append(    Viscdis )
    nu0Val.append(         nu0 )
    nu1Val.append(         nu1 )
    Avg_tempVal.append(    Avg_temp )
    vrmsVal.append(        Rms )
    Rms_surfVal.append(    Rms_surf )
    Max_vx_surfVal.append( Max_vx_surf )
    GravworkVal.append(    Gravwork )
    etamaxVal.append(      etamax )
    etaminVal.append(      etamin )

    # output fields and summary info to file (output graphics can also be added here)
    if uw.rank()==0:
        if (step % steps_output == 0) or (step == steps_end-1):
            print 'step =',step, '; CPU time =', time.clock()-start, '; vrms = ',Rms
            # Temperature
            fnametemp = "temperatureField_" + str(step).zfill(6) + ".h5"
            fullpath = os.path.join(outputPath + "files/" + fnametemp)
            temperatureField.save(fullpath)
            # Temperature time derivative
            fnametemp = "temperatureDotField_" + str(step).zfill(6) + ".h5"
            fullpath = os.path.join(outputPath + "files/" + fnametemp)
            temperatureDotField.save(fullpath)
            # Pressure
            fnametemp = "pressureField_" + str(step).zfill(6) + ".h5"
            fullpath = os.path.join(outputPath + "files/" + fnametemp)
            pressureField.save(fullpath)
            # Velocity
            fnametemp = "velocityField_" + str(step).zfill(6) + ".h5"
            fullpath = os.path.join(outputPath + "files/" + fnametemp)
            velocityField.save(fullpath)

            np.savetxt( outputPath + 'Summary.txt', np.c_[stepsVal, timeVal, viscdiscVal, nu0Val, 
                                                          nu1Val, Avg_tempVal, vrmsVal, Rms_surfVal,
                                                         Max_vx_surfVal, GravworkVal, etamaxVal, etaminVal], 
                   header="Steps, Time, visc, Nusselt bottom/top, av_T, vrms, surf_rms, surf_max, grav, etamax, etamin" )


# Save final fields - useful for continuing runs.

# In[ ]:

velocityField.save(outputPath + 'velocityField.h5')
pressureField.save(outputPath + 'pressureField.h5')
temperatureField.save(outputPath + 'temperatureField.h5')
temperatureDotField.save(outputPath + 'temperatureDotField.h5')

