
# coding: utf-8

# In[1]:

import underworld as uw
import math
from underworld import function as fn
import glucifer.pylab as plt

linearMesh = uw.mesh.FeMesh_Cartesian("Q2/dQ1", (32,32), (0.,0.), (1.,1.))

constantMesh = linearMesh.subMesh

velocityField = uw.fevariable.FeVariable(linearMesh,2)
velocityField.data[:] = (0.,0.)

pressureField = uw.fevariable.FeVariable(constantMesh,1)
pressureField.data[:] = 0.

# freeslip
IWalls = linearMesh.specialSets["MinI_VertexSet"] + linearMesh.specialSets["MaxI_VertexSet"]
JWalls = linearMesh.specialSets["MinJ_VertexSet"] + linearMesh.specialSets["MaxJ_VertexSet"]
freeslip = uw.conditions.DirichletCondition(velocityField, (IWalls, JWalls))


solA = fn.analytic.SolCx()

stokesSystem = uw.systems.Stokes(velocityField,pressureField,solA.viscosityFn,solA.bodyForceFn,conditions=[freeslip,], rtolerance=1.e-5)


# In[3]:

# can optionally set penalty this way
solver=uw.systems.Solver(stokesSystem, penalty=1.0)


# In[4]:


solver.options.A11.ksp_rtol=1e-4
solver.options.scr.ksp_rtol=1e-3

solver.options.main.Q22_pc_type='uwscale'


# In[5]:

#solver.options.mg.active=False
solver.options.A11.set_direct()
solver.options.A11.list()


# In[6]:

solver.solve()



