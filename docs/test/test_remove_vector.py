
# coding: utf-8

# This Notebook demonstrates how to remove an unwanted vector from a given vector field.
# 
# 2 tests are run:
#  - One for a cartesian box with an applied velocity field over it
#  - A Stokes solution within an annulus geometry.
# 
# The removal of a vector projected in the direction of another vector is how we can subtract away a Null Space contribution from a solution vectors.

# In[ ]:


import underworld as uw
import numpy as np
from underworld import function as fn
uw.matplotlib_inline()
import glucifer

with_vis=False # Set to True for pictorial results... it helps


# In[ ]:


def performRemoval(f1, f2):
    """
    Given 2 vector fields f1 & f2 find the scalar projection of f1 in the direction of f2
    
    Returns the scalar projection removed
    """
    # make eqNum objects and SolutionVectors
    eqNum   = uw.systems.sle.EqNumber( f1, False )
    eqNum2  = uw.systems.sle.EqNumber( f2, False )
    
    aVec = uw.systems.sle.SolutionVector(f1, eqNum)
    bVec = uw.systems.sle.SolutionVector(f2, eqNum2)

    # load fields onto vectors
    uw.libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector(aVec._cself)
    uw.libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector(bVec._cself)

    # remove one solution vector from another and update field
    projection = uw.libUnderworld.StgFEM.SolutionVector_RemoveVectorSpace(aVec._cself, bVec._cself)
    uw.libUnderworld.StgFEM.SolutionVector_UpdateSolutionOntoNodes(aVec._cself)
    
    return projection


# In[ ]:


# create mesh and required fields
mesh   = uw.mesh.FeMesh_Cartesian(elementRes=(10,10))
vField = mesh.add_variable(nodeDofCount=2)
oField = mesh.add_variable(nodeDofCount=2)

vField.data[:] = [1.,1.]    # the given vector
oField.data[:] = [1.,0.]    # the unwanted vector

# use dot product to test
fn_dot      = fn.math.dot(vField,oField)
initial_dot = mesh.integrate(fn_dot)[0]


# In[ ]:


scene = glucifer.Figure()
scene.Surface(mesh, fn_dot, onMesh=False)
scene.VectorArrows(mesh, vField)
if with_vis: scene.show()


# In[ ]:


projection = performRemoval(vField,oField)
final_dot  = mesh.integrate(fn_dot)[0]


# In[ ]:


assert final_dot < initial_dot, "Error, unexpected output from removing the one field from another"
assert np.allclose((projection,final_dot), (1.,0.)), "Error, unexpected output from removing the one field from another"


# In[ ]:


if with_vis: scene.show()


# ### Complex example with annulus

# In[ ]:


# a more intense example with an annulus geometry
annulus = uw.mesh.FeMesh_Annulus(elementRes=(16,120), 
                                  radialLengths=(2.0,4.), angularExtent=(0.0,360.0),
                                  periodic = [False, True])
pField = uw.mesh.MeshVariable(annulus.subMesh, nodeDofCount=1)
vField = uw.mesh.MeshVariable(annulus, nodeDofCount=2)

# velocity boundary conditions for solid body rotation 
outer = annulus.specialSets["MaxI_VertexSet"]
inner = annulus.specialSets["MinI_VertexSet"]
vField.data[outer.data] = [0.,10]
vBC = uw.conditions.RotatedDirichletCondition( variable=vField, 
                                               indexSetsPerDof=(inner+outer, outer),
                                               basis_vectors=(annulus.bnd_vec_normal,annulus.bnd_vec_tangent))

fig = glucifer.Figure()
fig.Mesh(annulus, segmentsPerEdge=1)
fig.Surface(annulus, fn.math.dot(vField,vField), onMesh=True )
fig.VectorArrows(annulus, vField)

stokesSLE = uw.systems.Stokes( vField, pField, 
                               fn_viscosity=1.0, fn_bodyforce=[0.,0.], 
                               conditions=vBC, _removeBCs=False)
stokesSolver = uw.systems.Solver(stokesSLE)
stokesSolver.solve()
# must realign the velocity solution
uw.libUnderworld.Underworld.AXequalsX( stokesSLE._rot._cself, stokesSLE._velocitySol._cself, False)
if with_vis : fig.show()

# make a copy of vField
v0Field = vField.copy() ; v0Field.data[:] = vField.data[:]

# use dot product to test
fn_dot      = fn.math.dot(v0Field,vField)
initial_dot = annulus.integrate(fn_dot)[0]

# remove the vector and update the SolutionVector
projection = uw.libUnderworld.StgFEM.SolutionVector_RemoveVectorSpace(stokesSLE._velocitySol._cself, stokesSLE._vnsVec._cself)
uw.libUnderworld.StgFEM.SolutionVector_UpdateSolutionOntoNodes(stokesSLE._velocitySol._cself)

final_dot = annulus.integrate(fn_dot)[0]
if with_vis: fig.show()


# In[ ]:


assert final_dot < initial_dot, "Error, unexpected output from removing the one field from another {} {}".format(final_dot, initial_dot)
if not np.allclose((projection,final_dot), (2.5,0.), atol=1e-3):
    raise RuntimeError("Error, unexpected output from removing the one field from another. Error measures {} {}".format(projection, final_dot))

