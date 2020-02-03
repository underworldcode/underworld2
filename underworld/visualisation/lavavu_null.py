#Dummy package for lavavu if real package not found
def is_ipython():
    return False

def is_notebook():
    return False

class Control(object):

    def __init__(self, *args, **kwargs):
        super(Control, self).__init__(**kwargs)

    #Undefined methods supported directly as LavaVu commands
    def __getattr__(self, key):
        #__getattr__ called if no attrib/method found
        def any_method(*args, **kwargs):
            return None

        return any_method

class Viewer(dict):

    def __init__(self, *args, **kwargs):
        super(Viewer, self).__init__(**kwargs)
        self.control = Control()

    #Undefined methods supported directly as LavaVu commands
    def __getattr__(self, key):
        #__getattr__ called if no attrib/method found
        def any_method(*args, **kwargs):
            return None

        return any_method

    @property
    def objects(self):
        return Objects()

    @property
    def figures(self):
        return Objects()

    @property
    def steps(self):
        return [0]

class Objects(dict):

    def __init__(self, *args, **kwargs):

        super(Objects, self).__init__(**kwargs)

    #Undefined methods supported directly as LavaVu commands
    def __getattr__(self, key):
        #__getattr__ called if no attrib/method found
        def any_method(*args, **kwargs):
            return None

        return any_method

    def __getitem__(self, key):
        self[key] = Viewer()
        return super(Objects, self).__getitem__(key)


