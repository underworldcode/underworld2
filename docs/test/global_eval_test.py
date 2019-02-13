
# coding: utf-8

# In[2]:

import underworld as uw
mesh = uw.mesh.FeMesh_Cartesian(elementRes=(32,32))
var = uw.mesh.MeshVariable(mesh,2)


# In[8]:

var.data[:mesh.data_nodegId.shape[0],0] = mesh.data_nodegId[:,0]
var.data[:mesh.data_nodegId.shape[0],1] = mesh.data_nodegId[:,0]
uw.libUnderworld.StgFEM._FeVariable_SyncShadowValues( var._cself )

# In[12]:

# import glucifer
# fig = glucifer.Figure()
# fig.append(glucifer.objects.Surface(mesh,var[0]))
# fig.show()


# In[15]:

import numpy as np
inarr = np.array( ( (0.1,0.1),(0.2,0.2),(0.3,0.3),(0.4,0.4),(0.5,0.5),(0.6,0.6),(0.7,0.7),(0.8,0.8),(0.9,0.9) ))
expectarr = np.array([[108.8,108.8],[217.6,217.6],[326.4,326.4],[435.2,435.2],[544.,544.],[652.8,652.8],[761.6,761.6],[870.4,870.4],[979.2,979.2]])
# In[16]:
out = var.evaluate_global(inarr)
if uw.mpi.rank == 0:
    if not np.allclose( out, expectarr ):
        raise RuntimeError("Error in global evaluation. Return results not as expected.")



# In[ ]:



