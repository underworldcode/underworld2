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

# **This is a minimal test for population control**

import underworld as uw
import underworld.visualisation as vis 

mesh = uw.mesh.FeMesh_Cartesian()


def vanilla_swarm(mesh):
    swarm = uw.swarm.Swarm(mesh)
    swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,4))
    return swarm
def draw_swarm(mesh,swarm):
    fig = vis.Figure()
    fig.append( vis.objects.Points(swarm, colourBar=False, pointSize=4.))
    fig.append( vis.objects.Mesh(mesh))
    return fig.show()


# generated swarm
swarm = vanilla_swarm(mesh)
draw_swarm( mesh, swarm )

swarm.particleGlobalCount

# test deleting particles limited by max
swarm = vanilla_swarm(mesh)
population_control = uw.swarm.PopulationControl(swarm,deleteThreshold=1.0,splitThreshold=1.0,maxDeletions=15,maxSplits=0)
population_control.repopulate()
if swarm.particleGlobalCount != 16:
    raise RuntimeError("Incorrect number of particles deleted during population control")
# test deleting particles limited by threshold
swarm = vanilla_swarm(mesh)
population_control = uw.swarm.PopulationControl(swarm,deleteThreshold=0.07,splitThreshold=1.0,maxDeletions=150,maxSplits=0)
population_control.repopulate()
if swarm.particleGlobalCount != 64:
    raise RuntimeError("Incorrect number of particles deleted during population control")

# test splitting particles limited by max
swarm = vanilla_swarm(mesh)
population_control = uw.swarm.PopulationControl(swarm,deleteThreshold=0.,splitThreshold=0.,maxDeletions=0,maxSplits=9999)
population_control.repopulate()
if swarm.particleGlobalCount != 512:
    raise RuntimeError("Incorrect number of particles deleted during population control")
# test deleting particles limited by threshold
swarm = vanilla_swarm(mesh)
population_control = uw.swarm.PopulationControl(swarm,deleteThreshold=0.,splitThreshold=0.07,maxDeletions=0,maxSplits=9999)
population_control.repopulate()
if swarm.particleGlobalCount != 320:
    raise RuntimeError("Incorrect number of particles deleted during population control")

# quick aggressive test
swarm = vanilla_swarm(mesh)
population_control = uw.swarm.PopulationControl(swarm,deleteThreshold=0.,splitThreshold=1.,maxDeletions=0,maxSplits=0, aggressive=True, particlesPerCell=40)
population_control.repopulate()
if swarm.particleGlobalCount != 640:
    raise RuntimeError("Incorrect number of particles deleted during population control")
