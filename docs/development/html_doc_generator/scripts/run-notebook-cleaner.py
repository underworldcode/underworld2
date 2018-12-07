#! /usr/bin/env python

#

import sys
import io
import os
import glob
import shutil
import json


try:
    # Jupyter >= 4
    from nbformat import read, write, NO_CONVERT
except ImportError:
    # IPython 3
    try:
        from IPython.nbformat import read, write, NO_CONVERT
    except ImportError:
        # IPython < 3
        from IPython.nbformat import current

        def read(f, as_version):
            return current.read(f, 'json')

        def write(nb, f):
            return current.write(nb, f, 'json')

def remove_outputs(nb):
    """remove the outputs from a notebook"""

    import copy

    nb2 = copy.deepcopy(nb)

    # v3 and v4 differ significantly
    if(nb2["nbformat"] ==3 ):
        for ws in nb2["worksheets"]:
            for cell in ws["cells"]:
                if cell["cell_type"] == 'code':
                    cell.outputs = []
                    prompt_number = 0

    elif (nb2["nbformat"] == 4 ):
            for cell in nb2["cells"]:
                if cell["cell_type"] == 'code':
                    cell["outputs"] = [ ]
                    cell["execution_count"] = None

    return nb2

# Note, this should trawl through subdirectories
# Also - where does this actually get executed
# And by which commands

fails = 0


for path, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith('.ipynb'):
                print os.path.join(path, file)

                try:
                    with io.open(file, 'r', encoding='utf8') as f:
                        nb = read(f, as_version=NO_CONVERT)

                    nb2 = remove_outputs(nb)
                    shutil.copy(file, "{}.backup".format(file))
                    try:
                        with io.open("{}".format(file), 'w', encoding='utf8') as f:
                            write(nb2, f)
                        print 'stripped NB v {} file "{}"'.format(nb.nbformat, file)
                    except:
                        print "Unable to write file"
                        shutil.copy("{}.backup".format(file), file)
                        print json.dumps( nb2, indent=2 )

                except:
                    print "Warning: Notebook {} was not stripped of output data, please check before committing".format(file)
                    print "         Over-ride using 'git commit --no-verify' if this warning can be ignored "
                    fails += 1

if(fails):
    sys.exit(-1)
