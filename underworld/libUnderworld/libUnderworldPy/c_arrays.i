/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module (package="underworld.libUnderworld.libUnderworldPy") c_arrays

%{
/* Includes the header in the wrapper code */
#include <cmath>
%}

/* this guy is an interface to malloc */
%include "carrays.i"
%include "cdata.i"
%array_class(double, DoubleArray)
%array_class(float, FloatArray)
%array_class(int, IntArray)
%array_class(unsigned, UnsignedArray)

