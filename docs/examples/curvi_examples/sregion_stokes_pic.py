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
# ## Demo of buoyancy driven stokes flow in an Spherical Region

# %%
import underworld as uw
import underworld.visualisation as vis
import numpy as np
from underworld import function as fn

# %%
# boundary conditions available "BC_FREESLIP, "BC_NOSLIP, "BC_LIDDRIVEN"
bc_wanted = 'BC_FREESLIP'
output_path = 'pic_output'

maxsteps = 5
elementRes=(12,20,20)
#elementRes = (16,24,24)

# inner / outer stokes solve tolerances
ITOL=1e-4
OTOL=1e-3

# %%
mesh = uw.mesh.FeMesh_SRegion(elementRes=elementRes, 
                              radialLengths=(3.0,6.))

# fields for stokes
vField = mesh.add_variable(nodeDofCount=mesh.dim)
pField = mesh.subMesh.add_variable(nodeDofCount=1)

# auxiliary fields rank
rField = mesh.subMesh.add_variable(nodeDofCount=1)

swarm    = uw.swarm.Swarm(mesh, particleEscape=True)
rvar     = swarm.add_variable(dataType="double", count=1) # original rank
dvar     = swarm.add_variable(dataType="double", count=1) # density

layout   = uw.swarm.layouts.PerCellSpaceFillerLayout(swarm, particlesPerCell=10)
swarm.populate_using_layout(layout)
advector = uw.systems.SwarmAdvector(velocityField=vField, swarm=swarm)

inner = mesh.specialSets["innerWall_VertexSet"]
outer = mesh.specialSets["outerWall_VertexSet"]
W     = mesh.specialSets["westWall_VertexSet"]
E     = mesh.specialSets["eastWall_VertexSet"]
S     = mesh.specialSets["southWall_VertexSet"]
N     = mesh.specialSets["northWall_VertexSet"]

all_boundary_set = mesh.specialSets["AllWalls_VertexSet"]
NS0 = N+S-(E+W)
# build corner edges node indexset
cEdge = (N&W)+(N&E)+(S&E)+(S&W)


# %%
# create checkpoint function
def checkpoint( mesh, fieldDict, swarm, swarmDict, index,
                meshName='mesh', swarmName='swarm', time=None, 
                prefix='./', enable_xdmf=True):
    import os
    # Check the prefix is valid
    if prefix is not None:
        if not prefix.endswith('/'): prefix += '/' # add a backslash
        if not os.path.exists(prefix) and uw.mpi.rank==0:
            print("Creating directory: ",prefix)
            os.makedirs(prefix)
        uw.mpi.barrier() 
       
    # initialise internal time
    if time is None and not hasattr(checkpoint, '_time'):
        checkpoint.time = 0
    # use internal time
    if time is None:
        time = checkpoint.time + 1
    
    if not isinstance(index, int):
        raise TypeError("'index' is not of type int")        
    ii = str(index)
    
    if mesh is not None:
        
        # Error check the mesh and fields
        if not isinstance(mesh, uw.mesh.FeMesh):
            raise TypeError("'mesh' is not of type uw.mesh.FeMesh")
        if not isinstance(fieldDict, dict):
            raise TypeError("'fieldDict' is not of type dict")
        for key, value in fieldDict.items():
            if not isinstance( value, uw.mesh.MeshVariable ):
                raise TypeError("'fieldDict' must contain uw.mesh.MeshVariable elements")


        # see if we have already saved the mesh. It only needs to be saved once
        if not hasattr( checkpoint, 'mH' ):
            checkpoint.mH = mesh.save(prefix+meshName+".h5")
        mh = checkpoint.mH

        for key,value in fieldDict.items():
            filename = prefix+key+'-'+ii
            handle = value.save(filename+'.h5')
            if enable_xdmf: value.xdmf(filename, handle, key, mh, meshName, modeltime=time)
        
    # is there a swarm
    if swarm is not None:
        
        # Error check the swarms
        if not isinstance(swarm, uw.swarm.Swarm):
            raise TypeError("'swarm' is not of type uw.swarm.Swarm")
        if not isinstance(swarmDict, dict):
            raise TypeError("'swarmDict' is not of type dict")
        for key, value in swarmDict.items():
            if not isinstance( value, uw.swarm.SwarmVariable ):
                raise TypeError("'fieldDict' must contain uw.swarm.SwarmVariable elements")
    
        sH = swarm.save(prefix+swarmName+"-"+ii+".h5")
        for key,value in swarmDict.items():
            filename = prefix+key+'-'+ii
            handle = value.save(filename+'.h5')
            if enable_xdmf: value.xdmf(filename, handle, key, sH, swarmName, modeltime=time)


# %%
# setup dField and rField
rField.data[:] = uw.mpi.rank
rvar.data[:] = uw.mpi.rank

# z_hat, the gravity unit vector
z_hat  = -1.0*mesh.fn_unitvec_radial()

# dvar is the density anomaly
inds = (swarm.data[:,0]**2 + swarm.data[:,1]**2 + (swarm.data[:,2]-4.5)**2) < 0.75**2
if inds.size != 0:
    dvar.data[inds] = 1.

bodyForceFn = dvar * z_hat


# %%
# xdmf output
fieldDict = {'velocity':vField,
             'pressure':pField,
             'rank':rField}

swarmDict = {'orank':rvar,
             'density':dvar}

# %%
fig = vis.Figure()
fig.append(vis.objects.Mesh(mesh, segmentsPerEdge=1))
fig.append(vis.objects.Surface(mesh, pField, onMesh=True ))
if uw.utils.is_kernel(): fig.show()

# %%
# zero all dofs of vField
vField.data[...] = 0.

if bc_wanted == "BC_NOSLIP":
    # no slip
    vBC = uw.conditions.CurvilinearDirichletCondition( variable=vField, indexSetsPerDof=(all_boundary_set,all_boundary_set,all_boundary_set))

elif bc_wanted == "BC_FREESLIP":
    # free-slip
    
    vField.data[cEdge.data] = (0.,0.,0.)
    vBC = uw.conditions.CurvilinearDirichletCondition( variable=vField,
                                                   indexSetsPerDof=(inner+outer,E+W+cEdge,NS0+cEdge),
                                                   basis_vectors = (mesh._e1, mesh._e2, mesh._e3) )
elif bc_wanted == "BC_LIDDRIVEN":
    # lid-driven case
        
    # build driving node indexset & apply velocities with zero radial component
    drivers = outer - (N+S+E+W)
    vField.data[drivers.data] = (0.,1.,1.)
    
    # build corner edges node indexset and apply velocities with zero non-radial components
    cEdge = (N&W)+(N&E)+(S&E)+(S&W)
    vField.data[cEdge.data] = (0.,0.,0.)
    
    # apply altogether.
    NS0 = N+S - (E+W)
    vBC = uw.conditions.CurvilinearDirichletCondition( variable=vField,
                                                  indexSetsPerDof=(inner+outer,drivers+E+W+cEdge,drivers+NS0+cEdge), # optional, can include cEdge on the 3rd component
                                                  basis_vectors = (mesh._e1, mesh._e2, mesh._e3) )
else:
    raise ValueError("Can't find an option for the 'bc_wanted' = {}".format(bc_wanted))

# %%
stokesSLE = uw.systems.Stokes( vField, pField, 
                               fn_viscosity=1.0, fn_bodyforce=bodyForceFn, 
                               conditions=vBC, _removeBCs=False)

# %%
solver = uw.systems.Solver(stokesSLE)
solver.options.scr.ksp_monitor=''
solver.set_inner_rtol(ITOL)
solver.set_outer_rtol(OTOL)

# %%
solver.solve()
# must re-orient boundary vectors
ignore = uw.libUnderworld.Underworld.AXequalsX( stokesSLE._rot._cself, stokesSLE._velocitySol._cself, False)

# %%
# xdmf output
checkpoint(mesh, fieldDict, swarm, swarmDict, index=0, prefix=output_path, time=0)
# %%
step = 1
time = 0
while step < maxsteps:
    # advect particles
    dt = advector.get_max_dt()
    time = time + dt
    advector.integrate(dt)
    vField.data[all_boundary_set] = (0.,0.,0.)
    solver.solve()
    # must re-orient boundary vectors
    ignore = uw.libUnderworld.Underworld.AXequalsX( stokesSLE._rot._cself, stokesSLE._velocitySol._cself, False)
    checkpoint(mesh, fieldDict, swarm, swarmDict, index=step, time=time, prefix=output_path)

    step += 1

# %%
vdotv = fn.math.dot(vField,vField)
vrms = np.sqrt( mesh.integrate(vdotv)[0] / mesh.integrate(1.)[0] )
if uw.mpi.rank == 0 and step == 5:
    rtol = 1e-3
    expected = 1.67223e-2
    error = np.abs(vrms - expected)
    rerr = error / expected
    print("Model vrms / Expected vrms: {:.5e} / {:.5e}".format(vrms,expected))
    if rerr > rtol:
        es = "Model vrms greater the test tolerance. {:.4e} > {:.4e}".format(rerr, rtol)
        raise RuntimeError(es)

# %%
# figV = vis.Figure()
# figV.append(vis.objects.Mesh(mesh, segmentsPerEdge=1))
# figV.append(vis.objects.Surface(mesh, vdotv, onMesh=True))
# # figV.append(vis.objects.VectorArrows(mesh, vField, autoscale=True, onMesh=True))
# # figV.append(vis.objects.VectorArrows(mesh, vField, onMesh=True))

# if uw.mpi.size == 1: figV.window()
