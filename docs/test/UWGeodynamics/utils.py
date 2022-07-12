import subprocess
from shutil import copyfile
import os
import json
import sys


def _check_setup_only(nb):
    # Remove all cells after and including "run_for"
    val = None
    data = json.load(open(nb, "r"))
    for index, cell in enumerate(data["cells"]):
        for line in cell["source"]:
            if "Model.run_for" in line:
                val = index
    if val:
        data["cells"] = data["cells"][:val]
        with open(nb, "w") as f:
            json.dump(data, f)

def _notebook_run(script, check_setup_only=True):

    outpath = script.replace(".ipynb", "_out.ipynb" )
    copyfile(script, outpath)

    if check_setup_only:
        _check_setup_only(outpath)

    args = ["jupyter", "nbconvert",
            "--to", "python", outpath]
    subprocess.check_call(args)
    
    pyfile = outpath.split(".")[0] + ".py"
    os.remove(outpath)
    args = [sys.executable, pyfile]
    subprocess.check_call(args)
    os.remove(pyfile)


