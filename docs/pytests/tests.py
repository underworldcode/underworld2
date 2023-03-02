import subprocess
import pytest
import glob
import ntpath
import sys
from inspect import getsourcefile


wdir = ntpath.dirname(getsourcefile(lambda:0))+"/../test/"

# get python scripts to test
pyscripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob(wdir+"/*.py"))]

@pytest.mark.parametrize('pyscript', pyscripts)
def test_python_execution(pyscript):
    cp = subprocess.run([sys.executable, pyscript], check=True)
    assert cp.returncode == 0

# get ipynb scripts to test
ipynbscripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob(wdir+"/*.ipynb"))]

@pytest.mark.parametrize('ipynbscript', ipynbscripts)
def test_ipynb_execution(ipynbscript):
    cp = subprocess.run(["pytest", "--nbmake", ipynbscript], check=True)
    assert cp.returncode == 0
