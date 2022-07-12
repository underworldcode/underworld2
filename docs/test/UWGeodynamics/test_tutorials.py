# test_tutorials.py

from .utils import _notebook_run
import pytest
import glob
import ntpath

scripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob("UWGeodynamics/tutorials/*.ipynb")) if "Badlands" not in path]

@pytest.mark.parametrize('script', scripts)
def test_script_execution(script):
    _notebook_run(script)


