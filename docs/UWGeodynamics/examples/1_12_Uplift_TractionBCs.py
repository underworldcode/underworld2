# ---
# jupyter:
#   jupytext:
#     formats: ipynb,py:percent
#     text_representation:
#       extension: .py
#       format_name: percent
#       format_version: '1.3'
#       jupytext_version: 1.17.1
#   kernelspec:
#     display_name: Python 3 (ipykernel)
#     language: python
#     name: python3
# ---

# %% [markdown]
# ## Recreation of the Underworld model using traction BCs
#
# Click [here](../../examples/08_Uplift_TractionBCs.ipynb) for the original model.

# %%
from underworld import UWGeodynamics as GEO
from underworld import visualisation as vis

# %%
u = GEO.UnitRegistry

# %%
# build reference units
KL_meters   = 100e3 * u.meter
K_viscosity = 1e16  * u.pascal * u.second
K_density   = 3.3e3 * u.kilogram / (u.meter)**3

# compute dependent scaling units 
KM_kilograms = K_density * KL_meters**3
KT_seconds   = KM_kilograms / ( KL_meters * K_viscosity )
K_substance  = 1. * u.mole
Kt_degrees   = 1. * u.kelvin

# Characteristic values of the system
GEO.scaling_coefficients["[length]"]      = KL_meters.to_base_units()
GEO.scaling_coefficients["[temperature]"] = Kt_degrees.to_base_units()
GEO.scaling_coefficients["[time]"]        = KT_seconds.to_base_units()
GEO.scaling_coefficients["[mass]"]        = KM_kilograms.to_base_units()

# %%
Model = GEO.Model(elementRes=(100,60), 
                  minCoord=( 0 * u.km  , 0 * u.km),
                  maxCoord=( 100 * u.km, 60 * u.km))

# %%
Model.outputDir="1_12_Uplift_TractionBCs"

# %%
layer_h = 0.6 * Model.maxCoord[1]

# %%
# lithostaticPressure = GEO.nd(-Model.gravity[1] * K_density * layer_h)
lithostaticPressure = -Model.gravity[1] * K_density * layer_h

# %%
air = Model.add_material(name="air", shape=GEO.shapes.Layer2D(top=Model.top, bottom=0.0))
background = Model.add_material(name="background", shape=GEO.shapes.Layer2D(top=layer_h, bottom=Model.bottom))

# %%
# if GEO.size == 1:
#     Fig = vis.Figure(figsize=(1200,400))
#     Fig.Points(Model.swarm, Model.materialField, fn_size=2.0)
#     Fig.save("Figure_1.png")
#     Fig.show()

# %%
air.density =  0. * u.kilogram / u.metre**3
background.density = 3300. * u.kilogram / u.metre**3

air.viscosity         = 1e22 * u.pascal * u.second
background.viscosity  = 1e22 * u.pascal * u.second

air.compressibility   = 1./( 1e11 * u.Pa *u.sec )
background.compressibility = 0.

# %%
if GEO.size == 1:
    Fig = vis.Figure(figsize=(1200,400))
    Fig.Points(Model.swarm, Model.materialField, fn_size=2)
    # Fig.Mesh(Model.mesh)
    Fig.VectorArrows(Model.mesh, Model.velocityField)
    Fig.show()

# %%
# traction perturbation parameters
import numpy as np
xp    = GEO.nd( 50e3 * u.meter)
width = GEO.nd( 3e3  * u.meter)
for ii in Model.bottom_wall:
    coord = Model.mesh.data[ii]
    P = lithostaticPressure*(1.+0.2*np.exp((-1/width*(coord[0]-xp)**2)))
    Model.tractionField.data[ii] = [0.0,GEO.nd(P)] # important to non dimensionalise this

# %%
# # visualise the bottom stress condition
# if GEO.size == 1:
#     GEO.uw.utils.matplotlib_inline()
#     import matplotlib.pyplot as pyplot
#     import matplotlib.pylab as pylab
#     pyplot.ion()
#     pylab.rcParams[ 'figure.figsize'] = 12, 6
#     pyplot.title('Prescribed traction component normal to base wall')
#     km_scaling  = GEO.dimensionalise(1,u.kilometer)
#     MPa_scaling = GEO.dimensionalise(1,u.MPa)
#     pyplot.xlabel('X coordinate - (x{}km)'.format(km_scaling.magnitude))
#     pyplot.ylabel('Normal basal traction MPa - (x{:.3e}MPa)'.format(MPa_scaling.magnitude))
    
#     xcoord = Model.mesh.data[Model.bottom_wall.data][:,0]          # x coordinate
#     stress = Model.tractionField.data[Model.bottom_wall.data][:,1] # 2nd component of the traction along the bottom wall
    
#     pyplot.plot( xcoord, stress, 'o', color = 'black', label='numerical') 
#     pyplot.show()

# %%
Model.set_velocityBCs(left=[0.,None], right=[0.,None], top=[None, 0.])
Model.set_stressBCs(bottom=[0., Model.tractionField[1]])

# %%
Model.run_for(nstep=10)

# %%
if GEO.size==1:
    Fig = vis.Figure(figsize=(1200,400))
    Fig.Points(Model.swarm, Model.materialField, fn_size=2.0)
    # Fig.Mesh(Model.mesh)
    Fig.VectorArrows(Model.mesh, Model.velocityField)
    Fig.show()

# %%
# TODO, 1e-18 is close to zero so overwrite the atol, and rtol
if not np.isclose( Model.stokes_SLE.velocity_rms(),  1.3497493646857656e-18, rtol=1e-5, atol=0):
    raise RuntimeError("Velocity is not as expected")
