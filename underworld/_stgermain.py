##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import os as _os
import xml.etree.cElementTree as _ET
from . import libUnderworld
import abc
from collections import defaultdict
import underworld as uw

class LeftOverParamsChecker(object):
    # This class simply checks for any left over args or parameters.
    # All objects need to inherit from it (usually indirectly) for the
    # testing to occur. 
    def __init__(self, *args, **kwargs):
        if len(args)>0:
            raise RuntimeError("There were left over arguments. args = [{}]\n".format(args)+\
                               "Please check the function/method required arguments.")
        if len(kwargs)>0:
            raise RuntimeError("There were left over keyword arguments. kwargs = [{}]\n".format(kwargs)+\
                               "Please check the function/method parameter names.")

class Save(LeftOverParamsChecker, metaclass=abc.ABCMeta):
    """
    Objects that inherit from this class are able to save their
    data to disk at the provided filename.
    """
    @abc.abstractmethod
    def save(self, filename):
        """ All children should define this method which returns the 
        c iterator object """
        pass

class Load(LeftOverParamsChecker, metaclass=abc.ABCMeta):
    """
    Objects that inherit from this class are able to load their
    data from disk at the provided filename.
    """
    @abc.abstractmethod
    def load(self, filename):
        """ All children should define this method which returns the 
        c iterator object """
        pass

class StgClass(LeftOverParamsChecker):
    """ 
    This class is useful for instances where a python initialiser generates StGermain
    class objects using public construcors. It serves these main purposes:
    
        1. Consistent naming conventions. All classes which inherit from StgClass
           must report a _cself attribute, so that we can be confident third party
           objects which need access to the c pointer know where to find it.
        2. Manages life cycle of object through custom __del__ method, which calls
           the underlying c delete methods.
        3. Objects are locked to prevent deletion from within C.
        
    StGermain class objects can inherit directly from this class, or it can be used on the 
    fly by passing the _cself parameter to the constructor.
    
    """
    _live_objects = set()
    def __init__(self, _cself=None, delSelf=True, **kwargs):
        if _cself:
            self._cself = _cself
        if not hasattr(self, '_cself'):
            raise RuntimeError("Failure during object creation. Object with class '{}' does not appear to have set a value for '_cself'.".format(type(r)))

        # ok, add lock
        self._delSelf = delSelf
        if self._delSelf:
            libUnderworld.StGermain.Stg_Class_Lock(self._cself)
        
        self._live_objects.add(self._cself.name)
#        print("creating {},{}".format(self._cself.name,self._cself.type))
#        import ipdb
#        ipdb.set_trace()
        super(StgClass, self).__init__(**kwargs)

    
    def __del__(self):
        if hasattr(self, "_delSelf") and self._delSelf:
            self._live_objects.remove(self._cself.name)
#            print("deleting {},{}".format(self._cself.name,self._cself.type))
#            print("deleting {},{}".format(self._cself.name,self._cself.type))
            libUnderworld.StGermain.Stg_Class_Unlock(self._cself)
            libUnderworld.StGermain.Stg_Class_Delete(self._cself)

class _SetupClass(abc.ABCMeta):
    '''
    This metaclass allows us to invoke a _setup method after the __init__ method.
    Borrowed from:
    http://stackoverflow.com/questions/22261763/in-python-is-it-possible-to-write-a-method-that-will-be-automatically-called-aft
    '''
    def __call__(cls, *args, **kwargs):
        # create the instance as normal.  this will invoke the class's
        # __init__'s as expected
        import time
        from . import timing
        timing._incrementDepth()
        ts = time.time()
        self = super(_SetupClass, cls).__call__(*args, **kwargs)

        # this steps through the MRO in ascending order (so that child
        # classes can be confident their bases are "set up").  A corresponding
        # teardown() method should probably be `reversed(cls.__mro__)`
        for base in reversed(cls.__mro__):
            _setup = vars(base).get('_setup')
            # in the general case, we have to use the descriptor protocol
            # to setup methods/staticmethods/classmethods properly
            if hasattr(_setup, '__get__'):
                _setup = _setup.__get__(self, cls)
            if callable(_setup):
                _setup()

        te = time.time()
        timing._decrementDepth()
        timing.log_result( te-ts, cls.__name__+".__init__()")
        return self


class StgCompoundComponent(StgClass, metaclass=_SetupClass):
    """ 
    This class ties multiple StGermain components together into a single python object.
    The life cycle of the objects (construction/build/destruction) are handled automatically.
    
    Any StGermain component instantiated by this class is also 'locked' to prevent deletion 
    from within a C routine.
    
    """
    
    def __new__(cls, *args, **kwargs):
        """
        Creates stgermain instances of underlying objects.
        
        The list of required instances is provided as a dictionary ('_objectsDict') on the class, 
        with the entry key being the object name, and entry value being the object type.
        
        A second class data member ('_selfObjectName') provides the object name 
        which should be considered the object 'self'.
        
        For example:
        class Drawing(_stgermain.StgCompoundComponent):
            _selfObjectName = "_dr"  # child should set this
            _objectsDict = { "_cm": "lucColourMap",
                             "_dr": None            }
                             
        Note that this example is for an abstract class ('Drawing'), so it does not define 
        a type for '_dr', but instead defers to child classes to define the type of '_dr'.
        
        Args:
            objectsDictOverrule (dict): If provided, this will overwrite any objects 
                                        in the class level _objectDicts. Useful for
                                        programatically modifying behaviour.
            

        Returns:
            New created instance of child class.
        
        """
        # ok, lets walk the mro (method resolution order), and build up full object dictionary
        # note, we walk in reverse so that children class objects will overwrite the parents (for polymorphism).
        fullObjDict = {}
        for basecls in reversed(cls.mro()):
            try:
                fullObjDict.update(basecls._objectsDict)
            except:
                pass
        # ok, overwrite any objects if necessary
        if "objectsDictOverrule" in kwargs:
            fullObjDict.update(kwargs["objectsDictOverrule"])
        
        for key, value in fullObjDict.items():
            if value == None:
                raise RuntimeError("Failure during object creation. Class '{}' has asked that an object '{}' be created, but has not defined a type for this object.\n"
                                   "This probably means a parent class has mandated that this object be created, but the child class "
                                   "has not defined the object type (as required). Or you are trying to create an instance of an abstract class.".format(cls.__name__,key))
        # lets go ahead and create the python instance of this object
        self = object.__new__(cls)

        import string
        import random
        self._id = "".join(random.choice(string.ascii_uppercase + string.digits) for _ in range(8))

        # ok, create stgermain objects
        # first rearrange to create stg compatible dictionary
        newObjDict = {}
        for compName, compType in fullObjDict.items():
            newObjDict[self._id + "_" + compName] = { "Type": compType }
        fullDictionary = {"components": newObjDict}

        # create
        self._objpointerDict = StgCreateInstances(fullDictionary)
        for objName, objPointer in self._objpointerDict.items():
            # lets create an easy access attribute. note here we strip out the id part to make it user friendly
            setattr(self, objName.replace(self._id+"_",""), objPointer)
            # ok, lets, now wrap these in the python StgClass class
            setattr(self, objName.replace(self._id+"_","pystgclass_"), StgClass(_cself=objPointer))
        
        # now lets get a 'cself'.  this is the corresponding clib 'self' item.
        if not hasattr(self, '_selfObjectName'):
            raise RuntimeError("Failure during object creation. Class '{}' does not appear to have set a value for '_selfObjectName'.".format(cls.__name__))
        if not hasattr(self, self._selfObjectName):
            raise RuntimeError("Failure during object creation. 'cself' object seemingly has name '{}', but Class '{}' does not appear to create an object with this name.".format(self._selfObjectName,cls.__name__))

        # and set cself
        self._cself = getattr(self,self._selfObjectName )
        
        self._setupDone = False                
        return self

    def __init__(self, **kwargs):
        """ 
        Initialisation function.  Child methods should acquire necessary parameters for
        underlying StGermain construct/build/initialise methods to complete object intialisation.
        
        """
        # note that we set delSelf to false, as deletion will occur when the other StgClass
        # object (recorded on _swarm attribute for example, as set in the __new__ function), disappears.
        super(StgCompoundComponent, self).__init__(delSelf=False, **kwargs)

    def _setup(self):
        """
        The StGermain Construct/Build/Initialise routines are called here. 
        Note that this method is called after all __init__ methods have been 
        executed. This is facilitated by the _SetupClass metaclass.
        """
        if not self._setupDone:
            # create a recursive default dict to use for component dictionary
            # the defaultdict allows children to add to the componentDictionary without knowing if a
            # particular key already exists. 
            l=lambda:defaultdict(l)
            componentDictionary = l()
            # ok... let child classes fill AssignFromXML dictionary as necessary
            self._add_to_stg_dict(componentDictionary)
            # if empty, lets add something to force AssignFromXML phase to execute
            if len(componentDictionary.keys()) == 0:
                componentDictionary[self._cself.name]["DummyKey"] = "DummyValue"
            fullDictionary = {"components": componentDictionary}
            StgConstruct(fullDictionary)
            # lets build
            for compName, compPtr in self._objpointerDict.iteritems():
                libUnderworld.StGermain.Stg_Component_Build( compPtr, None, False )
            # lets initialise
            for compName, compPtr in self._objpointerDict.iteritems():
                libUnderworld.StGermain.Stg_Component_Initialise( compPtr, None, False )
            self._setupDone = True

    @abc.abstractmethod
    def _add_to_stg_dict(self,componentDictionary):
        """ This function needs to be set by child class.
            It allows each child class to enter values into the component dictionary, and then call the parent _add_to_stg_dict method.
            
            Args:
                componentDictionary (defaultDict) : The component dictionary each class needs to fill out
            Returns:
                None
        """

def _dictToUWElementTree(inputDict):
    # lets create root element
    root = _ET.Element('StGermainData')
    root.attrib['xmlns'] = 'http://www.vpac.org/StGermain/XML_IO_Handler/Jun2003'

    # now to add subElements
    for k, v in inputDict.items():
        _itemToElement(v, k, root)

    return root

def _itemToElement(inputItem, inputItemName, inputEl):
    itemType = type(inputItem)
    if issubclass(itemType,(list,tuple)):
        subEl = _ET.SubElement(inputEl, 'list')
        if inputItemName != '':
            subEl.attrib['name'] = inputItemName
        for item in inputItem:
            _itemToElement(item, '', subEl)
    elif issubclass(itemType,dict):
        subEl = _ET.SubElement(inputEl, 'struct')
        if inputItemName != '':
            subEl.attrib['name'] = inputItemName
        for k, v in inputItem.items():
            _itemToElement(v, k, subEl)
    elif issubclass(itemType,(str, float, int, bool, unicode)):
        subEl = _ET.SubElement(inputEl, 'param')
        if inputItemName != '':
            subEl.attrib['name'] = inputItemName
        subEl.text = str(inputItem)
    elif not inputItem:
        subEl = _ET.SubElement(inputEl, 'param')
        if inputItemName != '':
            subEl.attrib['name'] = inputItemName
        subEl.text = "\t"
    else:
        raise TypeError("Unknown type encountered. key={}, value={}".format(inputItemName,inputItem))

def SetStgDictionaryFromPyDict( pyDict, stgDict ):
    """
       Sets the provided python dictionary as the StGermain dictionary.
       This routines can only be utilised after the Init phase has been completed, and
       should usually be run before the Construct phase.

       Args:
       pyDict (dict): Python dictionary to build a StGermain dictionary from.
       stgDict (Swig StGermain Dictionary*):  Pointer to StGermain dictionary to add python dictionary contents to.

       Returns:
       Nothing.
       """
    root = _dictToUWElementTree(pyDict)
    xmlString = _ET.tostring(root, encoding = 'utf-8', method = 'xml')
    ioHandler = libUnderworld.StGermain.XML_IO_Handler_New()
    libUnderworld.StGermain.IO_Handler_ReadAllFromBuffer( ioHandler, xmlString, stgDict, None )
    libUnderworld.StGermain.Stg_Class_Delete( ioHandler )

    return


def LoadModules( pyUWDict ):
    """
       Loads any Toolboxes found within provided dictionary.

       Args:
           pyUWDict (dict): Python version of underworld root dictionary.

       Returns:
           Nothing.
    """
    stgRootDict = libUnderworld.StGermain.Dictionary_New()
    SetStgDictionaryFromPyDict( pyUWDict, stgRootDict )
    libUnderworld.StGermain.ModulesManager_Load( libUnderworld.StGermain.GetToolboxManagerInstance(), stgRootDict, "" )
    libUnderworld.StGermain.Stg_Class_Delete( stgRootDict )

def StgCreateInstances( pyUWDict ):
    """
        Creates instances for all components within pyUWDict.
        
        Args:
        pyUWDict (dict): Underworld root type dictionary containing components and plugins.
        
        Returns:
        pointerDict (dict): Dictionary mapping component names to stg pointer.
    """
    if not isinstance(pyUWDict, dict):
        raise TypeError("object passed in must be of python type 'dict' or subclass")

    stgRootDict = libUnderworld.StGermain.Dictionary_New()
    SetStgDictionaryFromPyDict( pyUWDict, stgRootDict )

    stgCompDict = libUnderworld.StGermain.Dictionary_Entry_Value_AsDictionary( libUnderworld.StGermain.Dictionary_Get( stgRootDict, "components" ) )

    cf = libUnderworld.StGermain.Stg_ComponentFactory_New( stgRootDict, stgCompDict )

    # lets create instances of components
    libUnderworld.StGermain.Stg_ComponentFactory_CreateComponents( cf )

    libUnderworld.StGermain.Stg_Class_Delete(cf)
    libUnderworld.StGermain.Stg_Class_Delete(stgRootDict)

    pointerDict = {}
    # lets go ahead and construct component
    if "components" in pyUWDict:
        for compName, compDict in pyUWDict["components"].iteritems():
            compPointer = libUnderworld.StGermain.LiveComponentRegister_Get( libUnderworld.StGermain.LiveComponentRegister_GetLiveComponentRegister(), compName )
            pointerDict[compName] = compPointer

    return pointerDict


def StgConstruct( pyUWDict ):
    """
       Calls the construct phase for all components & plugins found in provided dictionary.

       Args:
         pyUWDict (dict): Underworld root type dictionary containing components and plugins.

       Returns:
         Nothing
    """
    if not isinstance(pyUWDict, dict):
        raise TypeError("object passed in must be of python type 'dict' or subclass")

    stgRootDict = libUnderworld.StGermain.Dictionary_New()
    SetStgDictionaryFromPyDict( pyUWDict, stgRootDict )
    # now lets de-alias
    libUnderworld.StGermain.DictionaryUtils_AliasDereferenceDictionary( stgRootDict )

    stgCompDict = libUnderworld.StGermain.Dictionary_Entry_Value_AsDictionary( libUnderworld.StGermain.Dictionary_Get( stgRootDict, "components" ) )

    cf = libUnderworld.StGermain.Stg_ComponentFactory_New( stgRootDict, stgCompDict )

    # lets go ahead and construct component
    if "components" in pyUWDict:
        for compName, compDict in pyUWDict["components"].iteritems():
            compPointer = libUnderworld.StGermain.LiveComponentRegister_Get( libUnderworld.StGermain.LiveComponentRegister_GetLiveComponentRegister(), compName )
            libUnderworld.StGermain.Stg_Component_AssignFromXML( compPointer, cf, None, False )
    if "plugins" in pyUWDict:
        for guy in pyUWDict["plugins"]:
            compPointer = libUnderworld.StGermain.LiveComponentRegister_Get( libUnderworld.StGermain.LiveComponentRegister_GetLiveComponentRegister(), guy["Type"] )
            libUnderworld.StGermain.Stg_Component_AssignFromXML( compPointer, cf, None, False )

    libUnderworld.StGermain.Stg_Class_Delete(cf)
    libUnderworld.StGermain.Stg_Class_Delete(stgRootDict)

def GetLiveComponent(compName):
    """
       Returns component with provided name if found within live component register.  Otherwise returns None.

       Args:
       compName (str):  Name of component to return.
       Returns:
       component (Swig Ptr):  Returns a pointer to the component object.  If not found, returns None.
    """
    if not isinstance(compName, str):
        raise TypeError("object passed in must be of python type 'str' or subclass")

    try:
        return libUnderworld.StGermain.LiveComponentRegister_Get( libUnderworld.StGermain.LiveComponentRegister_GetLiveComponentRegister(), compName )
    except:
        print("Component \'%s\' not found in the live component register." % compName)
        return None
