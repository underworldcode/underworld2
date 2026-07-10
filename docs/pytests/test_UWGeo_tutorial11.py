import sys
import subprocess
import pytest
import ntpath
from inspect import getsourcefile

wdir = ntpath.dirname(getsourcefile(lambda:0))+"/../UWGeodynamics/tutorials/"

tutorials = [
    #    "Tutorial_1_ThermoMechanical_Model.ipynb",
    #    "Tutorial_2_Melt.ipynb",
    #    "Tutorial_3_SandboxExtension_static_mesh.ipynb",
    #    "Tutorial_3B_SandboxExtension_deform_mesh.ipynb",
    #    "Tutorial_4_NumericalSandboxCompression.ipynb",
    #    "Tutorial_5_Convergence_Model.ipynb",
    #    "Tutorial_6_Simple_Surface_Processes.ipynb",
    #    "Tutorial_6_1_sedimentation_erosion_rates.ipynb",
    #    "Tutorial_6_2_diffusive_surface.ipynb",
    #    "Tutorial_6_3_3Dsedimentation_erosion_rates.ipynb",
    #    "Tutorial_7_3D_Lithospheric_Model.ipynb",
    #    "Tutorial_8_Subduction_ViscoElastic.ipynb",
    #    "Tutorial_9_passive_margins.ipynb",
    #    "Tutorial_10_Thrust_Wedges.ipynb",
    "Tutorial_11_Coupling_with_Badlands.ipynb",
]

scripts = [pytest.param(wdir + t, id=t) for t in tutorials]


@pytest.mark.parametrize("script", scripts)
def test_script_execution(script):
    subprocess.run([sys.executable, "-m", "pytest", "--nbmake", script], check=True)
