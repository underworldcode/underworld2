import subprocess
import pytest
import glob
import ntpath
from inspect import getsourcefile

wdir = ntpath.dirname(getsourcefile(lambda:0))+"/../UWGeodynamics/examples/"

# get ipynb scripts to test
scripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob(wdir+"/*.ipynb"))]

@pytest.mark.parametrize('script', scripts)
def test_script_execution(script):
    subprocess.run(["pytest", "--nbmake", script], check=True)
