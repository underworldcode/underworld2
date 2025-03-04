ignorelist = ['underworld', 'json', 'os', 'libUnderworld', 'glob', 'numpy', 'sys', 'os', 'time', 'control', 'LavaVuPython', 'lavavu', 're', 'lavavu_null']

import os
builddir="build"
imagedir=os.path.join(builddir,"images")
try:
    os.mkdir(builddir)
except FileExistsError:
    pass
try:
    os.mkdir(imagedir)
except FileExistsError:
    pass

# copy images in
import shutil
import os
import glob
for filename in glob.glob(os.path.join("../../../src/underworld/function/images/", '*.png')):
    shutil.copy(filename, imagedir)

done_mods = set()

def doc_module(module, modname):
    filename = modname+'.rst'
    filepath = os.path.join(builddir,filename)

    print("Generating {} for module {}".format(filepath,modname))
    import inspect
    # first gather info
    modules = {}
    classes = {}
    functions = {}
    for guy in dir(module):
        if guy[0] != "_":  # don't grab private guys
            if guy not in ignorelist:  # don't grab these
                obj = getattr(module,guy)
                if hasattr(obj,"DO_NOT_DOC"):
                    continue
                if inspect.ismodule(obj):
                    if obj.__file__ not in done_mods: 
                        done_mods.add(obj.__file__)
                        modules[guy] = obj
                    # else:
                    #     return
                elif inspect.isclass(obj):
                    classes[guy] = obj
                elif inspect.isfunction(obj):
                    functions[guy] = obj
    
    #    everything alphabetically
    from collections import OrderedDict
    modules   = OrderedDict(sorted(modules.items()))
    classes   = OrderedDict(sorted(classes.items()))
    functions = OrderedDict(sorted(functions.items()))


    # create a new file for each module
    with open(filepath, 'w') as f:
        # write title
        title = modname + " module\n"
        f.write(title)
        f.write("="*(len(title)-1)+"\n")
                
        # write out the modules docstring
        if module.__doc__:
            f.write(module.__doc__+"\n")
            f.write("\n")

        # write submodules brief
        if len(modules)>0:
            f.write("Submodules\n")
            f.write("~~~~~~~~~~~\n")
            f.write("\n")

            f.write(".. toctree::\n")
            f.write("    :titlesonly: \n")
            # f.write("    :maxdepth: 1\n")
            f.write("    :hidden: \n")
            f.write("\n")
            for key in modules.keys():
                f.write("    "+ modname + "." + key+".rst\n")
            f.write("\n")

            f.write(".. autosummary::\n")
            f.write("    :nosignatures:\n")

            f.write("\n")
            for key in modules.keys():
                f.write("    "+ modname + "." + key+"\n")
            f.write("\n")


        f.write(".. module:: {}".format(modname))
        f.write("\n\n")

        # write functions brief
        if len(functions)>0:
            f.write("Functions\n")
            f.write("~~~~~~~~~~\n")
            f.write("\n")
            f.write(".. autosummary::\n")
            f.write("    :nosignatures:\n")
            f.write("\n")
            for key in functions.keys():
                f.write("    "+ modname + "." + key+"\n")
            f.write("\n")

        # write classes brief
        if len(classes)>0:
            f.write("Classes\n")
            f.write("~~~~~~~\n")
            f.write("\n")
            f.write(".. autosummary::\n")
            f.write("    :nosignatures:\n")
            f.write("\n")
            for key in classes.keys():
                f.write("    "+ modname + "." + key+"\n")
            f.write("\n")

        # write functions
        if len(functions)>0:
            for key in functions.keys():
                funcguy = getattr(module, key)
                f.write(".. autofunction:: "+ modname + "." + key+"\n")
            f.write("\n")

        # write classes
        if len(classes)>0:
            for key in classes.keys():
                classguy = getattr(module, key)
                f.write(".. autoclass:: "+ modname + "." + key+"\n")
                f.write("    :members:\n")
                f.write("    :show-inheritance:\n")
            f.write("\n")



        # recurse
        for key in modules.keys():
            doc_module(getattr(module,key), modname+"."+key)


import underworld
import underworld.visualisation
doc_module(underworld, 'underworld')
