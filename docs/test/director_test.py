# ---
# jupyter:
#   jupytext:
#     text_representation:
#       extension: .py
#       format_name: light
#       format_version: '1.5'
#       jupytext_version: 1.13.1
#   kernelspec:
#     display_name: Python 3
#     language: python
#     name: python3
# ---

import underworld as uw
import underworld.visualisation as vis
import numpy as np

# create periodic mesh
mesh = uw.mesh.FeMesh_Cartesian(minCoord=(-1,-1), maxCoord=(1,1), periodic=(True,False))

velocity = mesh.add_variable( 2)

# setup shear
velocity.data[:] = (0.,0.)
velocity.data[:,0] = 2.*mesh.data[:,1]

fig = vis.Figure()
fig.append( vis.objects.VectorArrows(mesh,velocity, scaling=0.05))
fig.show()

swarm = uw.swarm.Swarm(mesh)
normal = swarm.add_variable('double',2)
swarm.populate_using_layout(uw.swarm.layouts.PerCellSpaceFillerLayout(swarm,20))

fig.append(vis.objects.Points(swarm,pointSize=3., pointType=1, colourBar=False))

fig.show()

# initialise to point upwards
normal.data[:] = (1.,0.)

# cast to mesh so we can see
cast_var = mesh.add_variable(2)
cast = uw.utils.MeshVariable_Projection(cast_var,normal)

# viz 
normal_fig = vis.Figure()
normal_fig.append(vis.objects.VectorArrows(mesh,cast_var,scaling=0.001))

cast.solve()
normal_fig.show()

# just use advector system to get some reasonable dt
advector = uw.systems.SwarmAdvector(velocity,swarm)
dt = advector.get_max_dt()
# dt = 0.05

# get grad on all particles.. particles are not moving, and velocity is fixed, so only need to do this once.
velgrad = velocity.fn_gradient.evaluate(swarm)

time = 0.
maxtime = 100.
while time<maxtime:
    # expected angle
    angle = 0.5 * np.pi - np.arctan2(1.,2.*time)
    # director angles
    angleDirector = np.arctan2(-normal.data[:,1],normal.data[:,0])
    # angle errors
    angleError = angle - angleDirector

    # calculate $\dot n_i = -L_{ji} n_j$
    n_dot_x = -(velgrad[:,0]*normal.data[:,0] + velgrad[:,2]*normal.data[:,1])
    n_dot_y = -(velgrad[:,1]*normal.data[:,0] + velgrad[:,3]*normal.data[:,1])
    # apply forward euler
    normal.data[:,0] += dt*n_dot_x
    normal.data[:,1] += dt*n_dot_y

    time += dt
    
    # check if any angles not as expected
    if (np.abs(angleError) > 1e-15).any():
        raise RuntimeError("Director test failed.")

cast.solve()
normal_fig.show()


