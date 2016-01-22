class MeshVariable(object):
    def __init__(self, *args, **kwargs):
        raise RuntimeError( "Note that the interface has changed.\n"\
                            "The MeshVariable class is now availabe within the 'mesh' module:\n"\
                            "   meshvar = uw.mesh.MeshVariable(mesh, 1)" )