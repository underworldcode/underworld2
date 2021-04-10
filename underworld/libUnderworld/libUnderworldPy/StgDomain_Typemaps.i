/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%{
/* Includes the header in the wrapper code */
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

%}

/* general Coord input map */
%typemap(in) (Coord coord) (double incoord[3]) {
	int i;
	if (!PyTuple_Check($input)) {
		PyErr_SetString(PyExc_ValueError,"Expected a tuple");
		return NULL;
	}
	int tupSize = PyTuple_Size($input);
	if ( !(tupSize == 2) && !(tupSize == 3)  ) {
		char* errstring;
		asprintf(&errstring, "Size mismatch (%d). Expected a tuple of size 2 or 3 for coord", tupSize );
        PyErr_SetString(PyExc_ValueError, errstring);
        free(errstring);
        return NULL;
	}
	for (i = 0; i < tupSize; i++) {
        PyObject *o = PyTuple_GetItem($input,i);
        if (PyNumber_Check(o)) {
			incoord[i] = (double) PyFloat_AsDouble(o);
	    } else {
			PyErr_SetString(PyExc_ValueError,"Tuple elements must be numbers");
			return NULL;
        }
	}
	$1 = (double*)&incoord;
}
//
///* Coord min/max output */
//%typemap(argout) Coord min, Coord max {
//	PyObject *tuple = PyTuple_New(3);
//	PyTuple_SetItem(tuple,0,PyFloat_FromDouble($1[0]));
//	PyTuple_SetItem(tuple,1,PyFloat_FromDouble($1[1]));
//	PyTuple_SetItem(tuple,2,PyFloat_FromDouble($1[2]));
//	%append_output(tuple);
//}
//
//%typemap(in,numinputs=0) Coord min (double temp[3]), Coord max (double temp[3]) {
//        $1 = &temp;
//}


