##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
Miscellaneous functions.
"""
import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function


class map(_Function):
    """  
    This function performs a map to other functions. 
    The user provides a python dictionary which maps unsigned integers
    keys to underworld funcions.
    The user must also provide a 'key function', which is an underworld
    function which returns the unsigned int values.
    """


    def __init__(self, keyFunc, mappingDict=None, defaultFunc=None, *args, **kwargs):
        # error check mapping
        if mappingDict and not isinstance(mappingDict, dict):
            raise TypeError("'mapping' object passed in must be of python type 'dict'")

        keyFunc = _Function._CheckIsFnOrConvertOrThrow(keyFunc)

        defaultFunc = _Function._CheckIsFnOrConvertOrThrow(defaultFunc)
        if defaultFunc == None:
            defaultFuncCself = None
        else:
            defaultFuncCself = defaultFunc._fncself
        # create instance
        self._fncself = _cfn.Map( keyFunc._fncself, defaultFuncCself )

        self._keyFunc     = keyFunc
        self._mappingDict = mappingDict

        # build parent
        super(map,self).__init__(argument_fns=[keyFunc,defaultFunc],**kwargs)
        
        self._map = {}

        for key, value in mappingDict.iteritems():
            if not isinstance(key, int) or key < 0:
                raise ValueError("Key '{}' not valid. Mapping keys must be unsigned integers.".format(key))
            funcVal = _Function._CheckIsFnOrConvertOrThrow(value)
            if funcVal == None:
                raise ValueError("'None' is not valid for mapped functions.")
            
            self._underlyingDataItems.update(funcVal._underlyingDataItems) # update dictionary
            # insert mapping and keep handles in py dict
            self._map[key] = funcVal
            self._fncself.insert( key, funcVal._fncself )

class conditional(_Function):
    """  
    This function performs an 'if' conditional statement.
    The user provides a set of  tuples of (conditionFn, actionFn). Each
    tuple provides a clause within the if statement.  The function loops
    through the clauses, stopping at the first conditionFn which returns 
    'true'. It executes its corresponding actionFn, and returns the results.
    
    If non of the provided clauses return a 'True' result, and exception is raised.
    """


    def __init__(self, clauseList, *args, **kwargs):
        # error check mapping
        if not isinstance(clauseList, (list,tuple)):
            raise TypeError("'clauseList' object passed in must be of python type 'list' or 'tuple'")

        self._clauseList = []
        funcSet = set()
        for clause in clauseList:
            if not isinstance(clause, (list,tuple)):
                raise TypeError("Clauses within the clauseList must be of python type 'list' or 'tuple'")
            if len(clause) != 2:
                raise ValueError("Clauses tuples must be of length 2.")
            conditionFn = _Function._CheckIsFnOrConvertOrThrow(clause[0])
            funcSet.add(conditionFn)
            actionFn    = _Function._CheckIsFnOrConvertOrThrow(clause[1])
            funcSet.add(actionFn)
            self._clauseList.append( (conditionFn,actionFn) )
        
        # build parent
        self._fncself = _cfn.Conditional()

        super(conditional,self).__init__(argument_fns=funcSet,**kwargs)

        # insert clause into c object now
        for clause in self._clauseList:
            self._fncself.insert( clause[0]._fncself, clause[1]._fncself )



