# test_examples.py

from .utils import _notebook_run
import pytest
import glob
import ntpath

scripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob("docs/examples/*.ipynb"))]

@pytest.mark.parametrize('script', scripts)
def test_script_execution(script):
    _notebook_run(script)
