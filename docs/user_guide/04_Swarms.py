
# coding: utf-8

# Particle swarms
# ============
# 
# In Underworld a swarm is a set of unique particles which store locations along with any other variable the user cares to add. The particles are advected about the simulation domain in response to the velocity field solution of a Stokes system (see the **Systems** user guide).
# 
# Underworld uses swarms of particles to carry material information such as viscosity and density as the material moves around the simulation domain. This allows users to track material properties of a slab as it subducts, for example. 
# 
# In this notebook we will examine how to create, setup and use particle swarms. While finite element variables are defined and updated on the mesh, particle swarms are free to advect anywhere inside the domain. This makes them useful for a variety of tasks as follows.
# 
# 
# **This notebook is broken down into the following examples:**
# 1. creating simple global swarms with different layouts
# 2. adding particles to a swarm based on a coordinate list
# 3. directly editing swarms
# 4. adding swarm particle variables
# 5. creating shapes using swarms
# 
# **Keywords:** swarms, particles, shapes
# 

# In[1]:

import underworld as uw
import glucifer

import mpi4py
comm = mpi4py.MPI.COMM_WORLD


# Setting up a global swarm
# ----------
# 
# Set up a random distribution of particles over a given mesh. The density of particles per cell controls the total number of particles.

# **Create a mesh**

# In[2]:

mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = (4, 4), 
                                 minCoord    = (0., 0.), 
                                 maxCoord    = (1., 1.))


# **Initialise a swarm**
# 
# Initialise a swarm on a given mesh object.

# In[3]:

swarm = uw.swarm.Swarm( mesh=mesh )


# **Create a layout object**
# 
# Create a layout object that will populate the swarm across the whole domain of the mesh. To populate the mesh the density of particles is required, e.g. the number of particles per cell.
# 
# The ``GlobalSpaceFillerLayout`` layout (default) fills the domain with particles in a quasi-random pattern. It utilises Sobol sequences to generate global particle locations which are more uniformly distributed than that achieved by a purely random generator.

# In[4]:

swarmLayout = uw.swarm.layouts.GlobalSpaceFillerLayout( swarm=swarm, particlesPerCell=20 )


# **Active the layout**

# In[5]:

swarm.populate_using_layout( layout=swarmLayout )


# **Plot resulting swarm from ``GlobalSpaceFillerLayout``**

# In[6]:

fig1 = glucifer.Figure()
fig1.append( glucifer.objects.Points(swarm=swarm, pointSize=5, colourBar=False) )
fig1.show()


# **Layout types: Gauss points**
# 
# We demonstrate the ``PerCellGaussLayout`` type. This puts a particle on each of the Gauss points within each element in the mesh. The number of points depends on the order of integration with a current maximum of 5 in each cell in each dimension.

# In[7]:

swarm = uw.swarm.Swarm( mesh=mesh )
swarmLayout = uw.swarm.layouts.PerCellGaussLayout( swarm=swarm, gaussPointCount=4 )
swarm.populate_using_layout( layout=swarmLayout )
fig2 = glucifer.Figure()
fig2.append( glucifer.objects.Points(swarm=swarm, pointSize=5, colourBar=False) )
fig2.append( glucifer.objects.Mesh(mesh) )
fig2.show()


# **Layout types: random points per cell**
# 
# While the global layout uses a uniform quasi-random distribution of points, the ``PerCellRandomLayout`` layout uses a purely random distribution. This will include some clumping as a true random distribution does, so it is often not used in cases where a uniform spread of particles is desired.

# In[8]:

swarm = uw.swarm.Swarm( mesh=mesh )
swarmLayout = uw.swarm.layouts.PerCellRandomLayout(swarm=swarm, particlesPerCell = 10)
swarm.populate_using_layout( layout=swarmLayout )
fig3 = glucifer.Figure()
fig3.append( glucifer.objects.Points(swarm=swarm, pointSize=5, colourBar=False) )
fig3.append( glucifer.objects.Mesh(mesh) )
fig3.show()


# **Layout types: quasi-random points per cell**
# 
# The ``PerCellSpaceFillerLayout`` uses a uniform quasi-random distribution of points as the global layout does, but for single cells.

# In[9]:

swarm = uw.swarm.Swarm( mesh=mesh )
swarmLayout = uw.swarm.layouts.PerCellSpaceFillerLayout( swarm=swarm, particlesPerCell = 10)
swarm.populate_using_layout( layout=swarmLayout )
fig4 = glucifer.Figure()
fig4.append( glucifer.objects.Points(swarm=swarm, pointSize=5, colourBar=False) )
fig4.append( glucifer.objects.Mesh(mesh) )
fig4.show()


# Creating a swarm from a coordinate list
# ---------
# 
# We can use the swarm particle method, with the above plotting method, to visualise the mesh nodes. 
# As an example, let us create two swarms and arbitrarily set the particle coordinates to coincide with the nodes on the mesh and sub mesh.
# 
# First, lets look at what the data for the first 5 points looks like for the mesh.

# In[10]:

mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = (4, 4), 
                                 minCoord    = (0., 0.), 
                                 maxCoord    = (1., 1.))
print(mesh.data[0:5])


# Since these 5 entries are the x, y (in 2D) coordinates for each mesh point, then they can be used directly to set the positions for the swarm particles

# In[11]:

# setup a new swarm. At this point there is no data in the swarm
swarmMesh = uw.swarm.Swarm( mesh=mesh )
# setup an array containing the position data from the mesh data
swarmCoords = mesh.data
# use this array to add particles to the newly created swarm
swarmMesh.add_particles_with_coordinates( swarmCoords )

# setup a new swarm
swarmSubMesh = uw.swarm.Swarm( mesh=mesh ) # note this must be the whole mesh
# setup an array containing the position data from the sub mesh data
swarmCoords = mesh.subMesh.data
# use this array to add particles to the newly created swarm
swarmSubMesh.add_particles_with_coordinates( swarmCoords )


# The final output from this will be a linear array with the same number of entries as there are nodes in the dQ0 sub mesh (shown above).
# 
# **Plot meshes**
# 
# Now we can plot these two swarms to see the Q1 linear mesh (4 nodes per element) and the dQ0 sub mesh (single node inside each element).

# In[12]:

fig5 = glucifer.Figure()
fig5.append( glucifer.objects.Points(swarm=swarmMesh,    pointSize=20, colourBar = False) )
fig5.append( glucifer.objects.Points(swarm=swarmSubMesh, pointSize=10, colourBar = False) )
fig5.show()


# Modifying particle positions
# -----
# 
# Here we will demonstrate how to edit the coordinates for a single swarm particle.
# 
# **Create a new swarm using particle coordinates**

# In[13]:

# initialise a swarm
swarmCustom = uw.swarm.Swarm( mesh=mesh )
# setup an array containing the position data from the mesh data
swarmCoords = mesh.data
# use this array to add particles to the newly created swarm
swarmCustom.add_particles_with_coordinates(swarmCoords)




# **Look at the swarm data**
# 
# Examine the position data stored inside the swarm objects.

# In[14]:

# This is interesting in parallel

print "Processor number: ", comm.rank

print('Head of swarm positions')
print(swarmCustom.particleCoordinates.data[0:5])
print('Tail of swarm positions')
print(swarmCustom.particleCoordinates.data[-5:])
iMid = len(swarmCustom.particleCoordinates.data)/2
print('Mid-way particle = {0:4d}'.format(iMid))
xCoord = swarmCustom.particleCoordinates.data[iMid][0]
yCoord = swarmCustom.particleCoordinates.data[iMid][1]
print('x = {0:.2f}; y = {1:.2f}'.format(xCoord,yCoord))

comm.barrier()


# **Plot current positions for swarm particles**
# 

# In[15]:

fig6 = glucifer.Figure()
fig6.append( glucifer.objects.Points(swarm=swarmCustom, pointSize=10, colourBar = False) )
fig6.show()


# **Shift a single particle**
# 
# We want to move a single particle, say the one to the right of the bottom left corner. The particles are numbered from bottom left to top right (increasing with increasing x and y) because we used the mesh data coordinates. Therefore, the bottom left corner is particle index 0, and the one to the right of this is particle index 1. 
# 
# In the command below, the first index is the particle index, and the second is the coordinate (0 is x, 1 is y)

# In[16]:

# modify particle 1's y coordinate (index=1) by adding 0.1 to it using the += operator
swarmCustom.particleCoordinates.data[1][1] += 0.1


# **Replot with shifted particle**

# In[17]:

fig7 = glucifer.Figure()
fig7.append( glucifer.objects.Points(swarm=swarmCustom, pointSize=10, colourBar = False) )
fig7.show()


# **Caution regarding swarm objects**
# 
# Using the '=' operator to assign the swarm to another variable merely creates an alias to the original
# (Also note that using *copy* in python will not copy the swarm object correctly).
# 
# Hence the following command will have the effect of undoing the move of particle 1 in the original swarm, even though it superficially looks like we are working on a new swarm.

# In[18]:

swarmCustomMod = swarmCustom

swarmCustomMod.particleCoordinates.data[1][1] -=  0.1


# **Replot the original edited swarm**
# 
# Note that the particle has moved back to it's position even though it may have appeared that``swarmCustom`` was not modified directly.

# In[19]:

fig8 = glucifer.Figure()
fig8.append( glucifer.objects.Points(swarm=swarmCustom, pointSize=10, colourBar = False) )
fig8.show()


# Adding particle variables
# ---------
# 
# In this example we will add a new variable for the particles and choose it's value based on the temperature at the particle's spatial coordinates. 

# **Create new variable for swarm object**
# 
# Firstly, create a new variable (a 'float' in this case) attached to the swarm object made in the previous example.

# In[20]:

swarmVariable = swarmCustom.add_variable( dataType="float", count=1 )


# **Make a temperature field example**

# In[21]:

temperatureField = uw.mesh.MeshVariable( mesh=mesh, nodeDofCount=1 )
# arbitrarily set the temperature to (1.0-y)
for index, coord in enumerate(mesh.data):
    temperatureField.data[index] = (1.0 - coord[1])


# Set the newly created variable to integer values based on the temperature at each index. Since the swarm particles directly line up with the mesh for the temperature FE variable then a direct copy is possible.

# In[22]:

swarmVariable.data[:] = temperatureField.data[:]
print(swarmVariable.data[0:10])


# **Plot swarm particles using new variable as the colour**

# In[23]:

fig9 = glucifer.Figure()
fig9.append( glucifer.objects.Points( swarm=swarmCustom, pointSize=20, fn_colour=swarmVariable, colourBar = True,
                                colours="red green blue") )
fig9.show()


# Making shapes
# -----
# 
# In this example we will make a few different shapes using a single particle swarm and adding a new variable to denote each shape. By using an additional variable for each particle any information can be carried by the particles, for example densities and viscosities. These can then be evaluated on the mesh points, allowing interaction between the swarms and the mesh variables - e.g. allowing the temperature field to be coupled to viscosity. This coupling will be discussed more in the next notebook, **Functions**.
# 
# For more information on how this method can be used to set material parameters to particles see the **StokesSinker** example.

# **Create a new high resolution mesh**

# In[24]:

res = 64
mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                 elementRes  = (res, res), 
                                 minCoord    = (0., 0.), 
                                 maxCoord    = (1., 1.) )


# **Create a swarm with quasi-random positions on the mesh**

# In[25]:

# initialise a swarm. Note this must be the whole mesh.
swarm = uw.swarm.Swarm( mesh=mesh )
# add a data variable which will store an index to determine material
swarmIndex = swarm.add_variable( dataType="int", count=1 )
# create a layout object that will populate the swarm across the whole domain
swarmLayout = uw.swarm.layouts.GlobalSpaceFillerLayout( swarm=swarm, particlesPerCell=20 )
# activate the layout object
swarm.populate_using_layout( layout=swarmLayout )


# **Plot the swarm**

# In[26]:

fig10 = glucifer.Figure()
fig10.append( glucifer.objects.Points(swarm=swarm, pointSize=2.0, colourBar = False) )
fig10.show()


# **Define a shape: circle**
# 
# We want to have a definition of a circle based on the position of each swarm particle. 
# 
# Here we use loops to go through each particle in the swarm manually and allocate an index depending on the particle's spatial coordinates. There is an easier (and faster) way to create shapes using functions and conditional statements, but that will be left until the user guide on **Functions**.

# Firstly, specify parameters for the circle

# In[27]:

circleRadius = 0.1
circleCentre = (0.5, 0.5)


# Use the location of each particle to set the index depending on whether the position is inside the circle or not.

# In[28]:



print(len(swarm.particleCoordinates.data))


# In[29]:

for pID in range(len(swarm.particleCoordinates.data)):
    x = swarm.particleCoordinates.data[pID][0]
    z = swarm.particleCoordinates.data[pID][1]
    xx = x - circleCentre[0]
    zz = z - circleCentre[1]
    condition = (xx*xx + zz*zz < circleRadius**2)
    if(condition == True):  # inside the circle
        swarmIndex.data[pID] = 1
    else:
        swarmIndex.data[pID] = 0


# Plot the modified swarm based on the index number

# In[30]:

fig11 = glucifer.Figure()
fig11.append( glucifer.objects.Points( swarm=swarm, fn_colour=swarmIndex, colours='blue red', 
                               colourBar = False, pointSize=2.0 ) )
fig11.show()


# **Define a shape: Box**
# 
# Use the same method as above to define a box and use this to set the particle swarm index.

# In[31]:

squareCentre = (0.5, 0.5)
squareWidth  = 0.1


# Use the location of each particle to set the index depending on if the position is inside the square.

# In[32]:

for pID in range(len(swarm.particleCoordinates.data)):
    x = swarm.particleCoordinates.data[pID][0]
    z = swarm.particleCoordinates.data[pID][1]
    xx = x - squareCentre[0]
    zz = z - squareCentre[1]
    xCond = (xx*xx) < (squareWidth/2.0)**2
    zCond = (zz*zz) < (squareWidth/2.0)**2
    condition = (xCond & zCond) # use the & operator to return true only if both conditions are satisfied
    if(condition == True):  # inside the square
        swarmIndex.data[pID] = 1
    else:
        swarmIndex.data[pID] = 0


# Plot the modified swarm based on the index number

# In[33]:

fig12 = glucifer.Figure()
fig12.append( glucifer.objects.Points( swarm=swarm, fn_colour=swarmIndex, colours='blue red', 
                               colourBar = False, pointSize=2.0 ) )
fig12.show()


# In[ ]:



