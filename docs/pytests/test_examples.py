# test_examples.py

import pathlib
import subprocess
import pytest
import glob
import ntpath

scripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob("docs/examples/*.py"))]

@pytest.mark.parametrize('script', scripts)
def test_script_execution(script):
    subprocess.run(["python", script])