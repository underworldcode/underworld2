import subprocess
import pytest
import glob
import ntpath
import sys
import underworld as uw
from inspect import getsourcefile

wdir = ntpath.dirname(getsourcefile(lambda:0))+"/../test/"

pyscripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob(wdir+"/*.py"))]

@pytest.mark.parametrize('pyscript', pyscripts)
def test_python_execution(pyscript):
    cp = subprocess.run([sys.executable, pyscript])
    assert cp.returncode == 0

ipynbscripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob(wdir+"/*.ipynb"))]


@pytest.mark.parametrize('ipynbscript', ipynbscripts)
def test_ipynb_execution(ipynbscript):
    cp = subprocess.run(["pytest", "--nbmake", ipynbscript])
    assert cp.returncode == 0
