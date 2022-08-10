import subprocess
import pytest
import glob
import ntpath
import sys
import underworld as uw
wdir = ntpath.dirname(str(uw.__path__[0]))+"/docs/user_guide/"

scripts = [pytest.param(path, id=ntpath.basename(path)) for path in sorted(glob.glob(wdir+"/*.ipynb"))]

@pytest.mark.parametrize('script', scripts)
def test_script_execution(script):
    subprocess.run(["pytest", "--nbmake", script])
    # subprocess.run([sys.executable, script])
