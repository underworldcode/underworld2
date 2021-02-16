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

# %%
mesh = uw.mesh.FeMesh_SRegion(elementRes=(6,12,12), 
                              radialLengths=(3.0,6.))

dField = mesh.add_variable(nodeDofCount=1)
vField = mesh.add_variable(nodeDofCount=mesh.dim)
pField = mesh.subMesh.add_variable(nodeDofCount=1)

inner = mesh.specialSets["innerWall_VertexSet"]
outer = mesh.specialSets["outerWall_VertexSet"]
W     = mesh.specialSets["westWall_VertexSet"]
E     = mesh.specialSets["eastWall_VertexSet"]
S     = mesh.specialSets["southWall_VertexSet"]
N     = mesh.specialSets["northWall_VertexSet"]

allWalls = mesh.specialSets["AllWalls_VertexSet"]
NS0 = N+S-(E+W)
# build corner edges node indexset
cEdge = (N&W)+(N&E)+(S&E)+(S&W)


# %%
# create checkpoint function
def checkpoint( mesh, fieldDict, swarm, swarmDict, index,
                meshName='mesh', swarmName='swarm', 
                prefix='./', enable_xdmf=True):
    import os
    # Check the prefix is valid
    if prefix is not None:
        if not prefix.endswith('/'): prefix += '/' # add a backslash
        if not os.path.exists(prefix) and uw.mpi.rank==0:
            print("Creating directory: ",prefix)
            os.makedirs(prefix)
        uw.mpi.barrier() 
            
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
            if enable_xdmf: value.xdmf(filename, handle, key, mh, meshName)
        
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
            if enable_xdmf: value.xdmf(filename, handle, key, sH, swarmName)


# %%
# xdmf output
fieldDict = {'velocity':vField,
             'normal':mesh._e2,
             'radial':mesh._e1,
             'tangent':mesh._e3,
             'temperature':dField}
checkpoint(mesh, fieldDict, None, None, index=0, prefix='output')

# %%
fig = vis.Figure()
fig.append(vis.objects.Mesh(mesh, segmentsPerEdge=1))
fig.append(vis.objects.Surface(mesh, dField, onMesh=True ))
fig.show()

# %%
# zero all dofs of vField
vField.data[...] = 0.

if bc_wanted == "BC_NOSLIP":
    # no slip
    vBC = uw.conditions.CurvilinearDirichletCondition( variable=vField, indexSetsPerDof=(allWalls,allWalls,allWalls))

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
z_hat  = -1.0*mesh.fn_unitvec_radial()

# %%
inds = (mesh.data[:,0]**2 + mesh.data[:,1]**2 + (mesh.data[:,2]-4.5)**2) < 1.5**2
dField.data[inds] = 1.

# %%
bodyForceFn = dField * z_hat

# %%
stokesSLE = uw.systems.Stokes( vField, pField, 
                               fn_viscosity=1.0, fn_bodyforce=bodyForceFn, 
                               conditions=vBC, _removeBCs=False)

# %%
stokesSolver = uw.systems.Solver(stokesSLE)
if uw.mpi.size == 1:
    stokesSolver.set_inner_method("mumps")

# %%
stokesSolver.solve()
# must re-orient boundary vectors
ignore = uw.libUnderworld.Underworld.AXequalsX( stokesSLE._rot._cself, stokesSLE._velocitySol._cself, False)

# %%
vdotv = fn.math.dot(vField,vField)
vrms = np.sqrt( mesh.integrate(vdotv)[0] / mesh.integrate(1.)[0] )
if uw.mpi.rank == 0:
    rtol = 1e-3
    expected = 6.89257e-02
    error = np.abs(vrms - expected)
    rerr = error / expected
    print("Model vrms / Expected vrms: {:.5e} / {:.5e}".format(vrms,expected))
    if rerr > rtol:
        es = "Model vrms greater the test tolerance. {:.4e} > {:.4e}".format(error, rtol)
        raise RuntimeError(es)

# %%
figV = vis.Figure()
figV.append(vis.objects.Mesh(mesh, segmentsPerEdge=1))
figV.append(vis.objects.Surface(mesh, vdotv, onMesh=True))
# figV.append(vis.objects.VectorArrows(mesh, vField, autoscale=True, onMesh=True))
# figV.append(vis.objects.VectorArrows(mesh, vField, onMesh=True))

if uw.mpi.size == 1: figV.window()

# %%
# xdmf output
fieldDict = {'velocity':vField,
             'density':dField}
checkpoint(mesh, fieldDict, None, None, index=1, prefix='output')

# %%
