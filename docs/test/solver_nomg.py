#!/usr/bin/env python
#!/bin/env python
'''
This script simply executes a solver, and ensures the expected number of iterations are performed. 
An exception is thrown otherwise.
'''

import underworld as uw
from underworld import function as fn

res=33
mesh = uw.mesh.FeMesh_Cartesian("Q1/DQ0", (res,res), (0.,0.), (1.,1.))

velocityField = uw.mesh.MeshVariable(mesh,2)
velocityField.data[:] = (0.,0.)
pressureField = uw.mesh.MeshVariable(mesh.subMesh,1)
pressureField.data[:] = 0.

# freeslip
IWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
JWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]
freeslip = uw.conditions.DirichletCondition(velocityField, (IWalls, JWalls))

# We are going to make use of one of the existing analytic solutions so that we may easily
# obtain functions for a viscosity profile and forcing terms.
# Exact solution solCx with defaults
sol = fn.analytic.SolCx()
stokesSystem = uw.systems.Stokes(velocityField,pressureField,sol.fn_viscosity,sol.fn_bodyforce,conditions=[freeslip,])
#Run the BSSCR Solver
solver=uw.systems.Solver(stokesSystem)
solver.set_inner_method("nomg")
print(solver.options.A11._mg_active)
solver.solve()
stats=solver.get_stats()
solver.print_stats()
from libUnderworld import petsc
petsc.OptionsPrint()

# if 4 != stats.pressure_its:
#     raise RuntimeError("Test returned wrong number of pressure iterations.")
# if 27 != stats.velocity_presolve_its:
#     raise RuntimeError("Test returned wrong number of velocity pre solve iterations.")
# if -1 != stats.velocity_pressuresolve_its:  # -1 will be returned if this stat isn't supported.
#     if stats.velocity_pressuresolve_its < 80:
#         raise RuntimeError("Test returned wrong number of velocity pressure solve iterations.")
# if 24 != stats.velocity_backsolve_its:
#     raise RuntimeError("Test returned wrong number of velocity back solve iterations.")

if stats.pressure_its > 5:
    raise RuntimeError("Test appears to require too many pressure iterations. Iteration count = {}.".format(stats.pressure_its))
if stats.velocity_presolve_its > 24:
    raise RuntimeError("Test appears to require too many velocity pre solve iterations. Iteration count = {}.".format(stats.velocity_presolve_its))
if -1 != stats.velocity_pressuresolve_its:  # -1 will be returned if this stat isn't supported.
    if stats.velocity_pressuresolve_its > 80 :
        raise RuntimeError("Test appears to require too many velocity pressure solve iterations. Iteration count = {}.".format(stats.velocity_pressuresolve_its))
if stats.velocity_backsolve_its > 30:
    raise RuntimeError("Test appears to require too many velocity back solve iterations. Iteration count = {}.".format(stats.velocity_backsolve_its))
