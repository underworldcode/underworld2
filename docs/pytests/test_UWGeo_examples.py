import sys
import subprocess
import pytest
import glob
import ntpath
from inspect import getsourcefile

wdir = ntpath.dirname(getsourcefile(lambda:0))+"/../UWGeodynamics/examples/"

# 3D notebooks are too slow for the default test run. Marked as long_tests
# so they are skipped unless --run-long is passed.
LONG_NOTEBOOKS = {
    "1_11_StokesSinker3D.ipynb",
    "1_21_3D_ColumnsTractionBottom.ipynb",
    "1_23_05_FreeSurface_3D_Relaxation.ipynb",
    "1_24_Define_3D_volumes.ipynb",
    "3D_RisingBall_FreeSurface.ipynb",
}

# get ipynb scripts to test
scripts = []
for path in sorted(glob.glob(wdir+"/*.ipynb")):
    name = ntpath.basename(path)
    params = {"id": name}
    if name in LONG_NOTEBOOKS:
        params["marks"] = pytest.mark.long_tests
    scripts.append(pytest.param(path, **params))

@pytest.mark.parametrize('script', scripts)
def test_script_execution(script):
    subprocess.run([sys.executable, "-m", "pytest", "--nbmake", script], check=True)
