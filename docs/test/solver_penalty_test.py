#!/usr/bin/env python3
'''
This script simply configures a system with compressibilty and
ensures that it flags a warning if the user attempts to use a 
penalty solver. 
'''

import underworld as uw
from underworld import function as fn

mesh = uw.mesh.FeMesh_Cartesian("Q1/DQ0", (2,2), (0.,0.), (1.,1.))
velocityField = uw.mesh.MeshVariable(mesh,2)
pressureField = uw.mesh.MeshVariable(mesh.subMesh,1)
stokesSystem = uw.systems.Stokes(velocityField,pressureField,1.,(0.,0.),fn_one_on_lambda=1.)
solver=uw.systems.Solver(stokesSystem)
solver.set_penalty(1.)

# catch warning to check it was generated correctly. 
import warnings
with warnings.catch_warnings(record=True) as w:
    # Cause all warnings to always be triggered.
    warnings.simplefilter("always")
    # Trigger warning.
    solver.solve()
    # Verify message exists
    assert len(w) >= 1
    # Verify message is correct
    assert uw.systems._bsscr.StokesSolver._warning_penalty_compressibility == str(w[0].message.args[0])
    # if broken, check the generated message:
    # print(str(w[-1].message))
