#!/usr/bin/env python
#!/bin/env python
'''
This script simply executes a solver, and ensures the expected number of iterations are performed. 
An exception is thrown otherwise.
'''

import underworld as uw
from underworld import function as fn

res=32
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
# can optionally set penalty this way
solver=uw.systems.Solver(stokesSystem)
solver.options.A11.ksp_converged_reason=''
#solver.options.mg.pc_mg_type="additive"
#solver.options.mg.pc_mg_type="full"
#solver.options.mg.pc_mg_type="kaskade"
#solver.options.mg.pc_mg_type="multiplicative"
solver.options.mg_accel.mg_accelerating_smoothing=0
solver.options.mg_accel.mg_accelerating_smoothing_view=1
#solver.options.main.penalty=1000.0
#solver.options.main.help=''
solver.solve()
stats=solver.get_stats()
solver.print_stats()

if 3 != stats.pressure_its:
    raise RuntimeError("Test returned wrong number of pressure iterations: should be 3")
if 6 != stats.velocity_presolve_its:
    raise RuntimeError("Test returned wrong number of velocity pre solve iterations: should be 6")
if 15 != stats.velocity_pressuresolve_its:
    raise RuntimeError("Test returned wrong number of velocity pressure solve iterations: should be 15")
if 6 != stats.velocity_backsolve_its:
    raise RuntimeError("Test returned wrong number of velocity back solve iterations: should be 6")
