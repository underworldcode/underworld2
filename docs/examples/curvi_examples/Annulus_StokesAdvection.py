# ---
# jupyter:
#   jupytext:
#     formats: ipynb,py:percent
#     text_representation:
#       extension: .py
#       format_name: percent
#       format_version: '1.3'
#       jupytext_version: 1.4.2
#   kernelspec:
#     display_name: Python 3
#     language: python
#     name: python3
# ---

# %% [markdown]
# ## Swarm advection with lid Driven free slip boundary conditions in annulus geometry

# %%
import underworld as uw
from underworld import function as fn
import underworld.visualisation as vis
import math, os
import numpy as np
from mpi4py import MPI

# %%
# Set simulation box size.
boxHeight = 1.0
boxLength = 2.0
# Set the resolution.
res = 2
# Set min/max temperatures.
tempMin = 0.0
tempMax = 1.0

comm = MPI.COMM_WORLD
outputDir = 'outputWithSwarm/'
        
if uw.mpi.rank == 0:
    step = 1
    while os.path.exists(outputDir):
        outputDir = outputDir.split("_")[0]+"_"+str(step).zfill(3)+'/'
        step     += 1
    
    os.makedirs(outputDir)
    outF = open(outputDir+'/output.dat', 'w')
    
store = vis.Store(outputDir+'/viz')

# build annulus mesh - handles deforming a recangular mesh and applying periodic dofs
mesh                = uw.mesh.FeMesh_Annulus(elementRes=(10,60), radialLengths=(4.,6.))
velocityField       = mesh.add_variable( nodeDofCount=2 )
pressureField       = mesh.subMesh.add_variable( nodeDofCount=1 )
vmag                = fn.math.sqrt(fn.math.dot( velocityField, velocityField ))

# %%
# Set viscosity to be a constant.
viscosity = 1.
buoyancyFn = (0.,0.0)

# %%
# TODO: reuse only the vertex sets corresponding to the boundaries.
lower = mesh.specialSets["MinI_VertexSet"]
upper = mesh.specialSets["MaxI_VertexSet"]

# (vx,vy) -> (vn,vt) (normal, tangential)
velocityField.data[ upper.data ] = [0.0,10.0]
velBC = uw.conditions.CurvilinearDirichletCondition( variable    = velocityField,
                                                indexSetsPerDof  = (lower+upper, upper),
                                                basis_vectors    = (mesh.bnd_vec_normal, mesh.bnd_vec_tangent))


# %%
swarm    = uw.swarm.Swarm(mesh, particleEscape=True)
tvar     = swarm.add_variable(dataType="double", count=1) # theta position
layout   = uw.swarm.layouts.PerCellSpaceFillerLayout(swarm, particlesPerCell=10)

swarm.populate_using_layout(layout)
# get initial theta coordinates and save into tvar
x,y = np.split(swarm.particleCoordinates.data, indices_or_sections=2,axis=1)
if uw.mpi.size == 1:
    tvar.data[:] = 180 / np.pi * np.arctan2(y,x)
else:
    tvar.data[:] = uw.mpi.rank

# add an advector
advector = uw.systems.SwarmAdvector(velocityField=velocityField, swarm=swarm)

# %%
fig = vis.Figure(store=store)
fig.append( vis.objects.Mesh( mesh ))
fig.append(vis.objects.Points(swarm, fn_colour=tvar, fn_size=4, colours="blue red"))
# fig.append( vis.objects.VectorArrows(mesh, velocityField))
fig.show()

# %%
stokesSLE = uw.systems.Stokes( velocityField = velocityField, 
                               pressureField = pressureField,
                               conditions    = velBC,
                               fn_viscosity  = viscosity, 
                               fn_bodyforce  = buoyancyFn,
                               _removeBCs    = False )      # _removeBC is required

# %%
solver = uw.systems.Solver(stokesSLE)
# using a direct method here reports in petsc warnings, because the solution is a nullspace I think
solver.solve() # results in velocity solution being mixed
# re-rotate and unmix
ierr = uw.libUnderworld.Underworld.AXequalsX( stokesSLE._rot._cself, stokesSLE._velocitySol._cself, False)

# %%
fig.show()

# %%
i=0
t0 = MPI.Wtime()
t_adv = 0.;
t_save = 0.;
while i < 30:
    t_adv = MPI.Wtime()
    # advect particles and count
    advector.integrate(advector.get_max_dt())
    t_adv = MPI.Wtime() - t_adv
    globalCount = swarm.particleGlobalCount
    
    # update 
    i += 1
    store.step = i
    t_save = MPI.Wtime()
    fig.save()
    t_save = MPI.Wtime() - t_save
    
    # print diagnostics
    if uw.mpi.rank == 0:
        outF.write("{0}, {1}, {2:.3e}, {3:.3e}\n".format(i, globalCount, t_adv, t_save))
    swarm.save(outputDir+'swarm.'+(str(i).zfill(5))+'.h5')

if uw.mpi.rank == 0:
    outF.close()

# %%
if uw.utils.is_kernel():
    vis = vis.lavavu.Viewer(database=store.filename)
#     vis["pointsize"]=3.
    vis.control.Panel()
    vis.control.ObjectList()
    vis.control.TimeStepper()
    vis.control.show()

# %%
# import os
# filename = "v.h5"
# mH = mesh.save("./annulus.h5")
# if os.path.exists(filename):
#     os.remove(filename)
# fH = velocityField.save(filename, mH)
# velocityField.xdmf('v.xdmf', fH, 'velocity', mH, 'mesh', 0)
