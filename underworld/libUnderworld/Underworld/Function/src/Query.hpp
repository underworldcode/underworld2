/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __Underworld_Function_Query_hpp__
#define __Underworld_Function_Query_hpp__

#include <exception>
#include <Python.h>

#include "FunctionIO.hpp"
#include "Function.hpp"
#include "IOIterators.hpp"


namespace Fn {

class Query
{
    public:
        Query( Function& function ): _function(function){};
        PyObject* query( IOIterator& iterator );
    private:
        Function& _function;
};

}

#endif