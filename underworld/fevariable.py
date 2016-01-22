''' DEPRECATE
'''
class FeVariable(object):
    def __init__(self, *args, **kwargs):
        raise RuntimeError("Note that the 'FeVariable' class has been renamed to \n" \
                           "'MeshVariable' and is accessible via the 'meshvariable' module.")
