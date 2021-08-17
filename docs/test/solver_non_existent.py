'''
This test simply configures the solver class for an underlying
solver type that does not exist, and ensures that the appropriate
error message is flagged to the user.  

This simulates the behaviour that a user might encounter if they 
attmpted to use `mumps` (for example) on a system where `mumps`
isn't installed.
'''

expectedmsg1 = "Error encountered. Full restart recommended as exception safety not guaranteed. Error message:\n" \
              "An error was encountered during the PETSc solve. You should refer to the PETSc\n" \
              "error message for details. Note that if you are running within Jupyter, this error\n" \
              "message will only be visible in the console window." 
expectedmsg2 = "Error encountered. Full restart recommended as exception safety not guaranteed. Error message:\n" \
              "An error was encountered during the PETSc solver setup. You should refer to the PETSc\n" \
              "error message for details. Note that if you are running within Jupyter, this error\n" \
              "message will only be visible in the console window." 

import underworld as uw
mesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0") )


# Heat solve test

temperatureField = mesh.add_variable( nodeDofCount=1 )
temperatureField.data[:] = 0.
botWalls = mesh.specialSets["Bottom_VertexSet"]
topWalls = mesh.specialSets[   "Top_VertexSet"]
bcWalls = botWalls + topWalls
tempBC = uw.conditions.DirichletCondition( variable=temperatureField, indexSetsPerDof=(bcWalls,) )
temperatureField.data[botWalls] = 1.0
temperatureField.data[topWalls] = 0.0
heatequation = uw.systems.SteadyStateHeat(temperatureField = temperatureField, 
                                            fn_diffusivity = 1.0, 
                                                conditions = tempBC)
heatsolver = uw.systems.Solver(heatequation)
heatsolver.options.EnergySolver.pc_factor_mat_solver_type="NON_EXISTENT_SOLVER"
flagged_error=True
try:
    heatsolver.solve()
    flagged_error=False
except RuntimeError as e:
    if str(e)!=expectedmsg1:
        raise RuntimeError(f"Wrong error message encountered.\n\nExpected:\n\n{expectedmsg1}\n\nEncountered:\n\n{e}\n\n")

if not flagged_error:
    raise RuntimeError("Heat solver didn't flag error for unknown solver.")


# Stokes solve test

velocityField    = mesh.add_variable(         nodeDofCount=2 )
pressureField    = mesh.subMesh.add_variable( nodeDofCount=1 )
velocityField.data[:] = [0.,0.]
pressureField.data[:] = 0.
iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]
freeslipBC = uw.conditions.DirichletCondition( variable      = velocityField, 
                                               indexSetsPerDof = (iWalls, jWalls) )
stokes = uw.systems.Stokes(    velocityField = velocityField, 
                               pressureField = pressureField,
                               conditions    = freeslipBC,
                               fn_viscosity  = 1., 
                               fn_bodyforce  = (0.,-1.) )
stokessolver = uw.systems.Solver( stokes )
stokessolver.options.A11.__dict__.clear()
stokessolver.options.A11.ksp_type="preonly"
stokessolver.options.A11.pc_type="lu"
stokessolver.options.A11._mg_active=False
stokessolver.options.A11.pc_factor_mat_solver_type="NON_EXISTENT_SOLVER"

flagged_error=True
try:
    stokessolver.solve()
    flagged_error=False
except RuntimeError as e:
    if str(e)!=expectedmsg2:
        raise RuntimeError(f"Wrong error message encountered.\n\nExpected:\n\n{expectedmsg2}\n\nEncountered:\n\n{e}\n\n")

if not flagged_error:
    raise RuntimeError("Heat solver didn't flag error for unknown solver.")
