import pathlib
import subprocess
import pytest
import glob
import ntpath
import sys
import underworld as uw
wdir = ntpath.dirname(str(uw.__path__[0]))+"/docs/test/"

pyscripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob(wdir+"/*.py"))]

@pytest.mark.parametrize('pyscript', pyscripts)
def test_python_execution(pyscript):
    subprocess.run([sys.executable, pyscript])


ipynbscripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob(wdir+"/*.ipynb"))]


@pytest.mark.parametrize('ipynbscript', ipynbscripts)
def test_ipynb_execution(ipynbscript):
    subprocess.run(["pytest", "--nbmake", ipynbscript])
    # subprocess.run([sys.executable, ipynbscript])
