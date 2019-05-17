ignorelist = ['underworld', 'glucifer', 'json', 'os', 'libUnderworld', 'glob', 'numpy', 'sys', 'os', 'time', 'control', 'LavaVuPython', 'lavavu', 're']

done_mods = set()

def doc_module(module, modname):
    filename = modname+'.rst'
    print("Generating {} for module {}".format(filename,modname))
    import inspect
    # first gather info
    modules = {}
    classes = {}
    functions = {}
    for guy in dir(module):
        if guy[0] != "_":  # don't grab private guys
            if guy not in ignorelist:  # don't grab these
                obj = getattr(module,guy)
                if inspect.ismodule(obj):
                    if obj.__file__ not in done_mods: 
                        print(obj.__file__)
                        done_mods.add(obj.__file__)
                        modules[guy] = obj
                    # else:
                    #     return
                elif inspect.isclass(obj):
                    classes[guy] = obj
                elif inspect.isfunction(obj):
                    functions[guy] = obj
    
    # create a new file for each module
    with open(filename, 'w') as f:
        # write title
        title = modname + " module\n"
        f.write(title)
        f.write("="*(len(title)-1)+"\n")
        
        # write out the modules docstring
        if module.__doc__:
            f.write(module.__doc__+"\n")
            f.write("\n")

        f.write("Module Summary\n")
        f.write("--------------\n")

        # write submodules brief
        if len(modules)>0:
            f.write("submodules:\n")
            f.write("~~~~~~~~~~~\n")
            f.write("\n")

            f.write(".. toctree::\n")
            f.write("    :maxdepth: 1\n")

#            f.write(".. autosummary::\n")
#            f.write("    :nosignatures:\n")
#            f.write("    :toctree:\n")

            f.write("\n")
            for key in modules.keys():
                f.write("    "+ modname + "." + key+"\n")
            f.write("\n")


        # write functions brief
        if len(functions)>0:
            f.write("functions:\n")
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
            f.write("classes:\n")
            f.write("~~~~~~~~\n")
            f.write("\n")
            f.write(".. autosummary::\n")
            f.write("    :nosignatures:\n")
            f.write("\n")
            for key in classes.keys():
                f.write("    "+ modname + "." + key+"\n")
            f.write("\n")

        f.write("Module Details\n")
        f.write("--------------\n")

        # write functions
        if len(functions)>0:
            f.write("functions:\n")
            f.write("~~~~~~~~~~\n")
            for key in functions.keys():
                funcguy = getattr(module, key)
                f.write(".. autofunction:: "+ modname + "." + key+"\n")
            f.write("\n")

        # write classes
        if len(classes)>0:
            f.write("classes:\n")
            f.write("~~~~~~~~\n")
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
doc_module(underworld, 'underworld')

import glucifer
doc_module(glucifer, 'glucifer')
