/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
}

#include "Query.hpp"


PyObject* Fn::Query::query( IOIterator& iterator )
{
    // get number of outputs
    iterator.reset();
    unsigned size = iterator.size();
    if ( size == 0 )
        return Py_None;
    
    // grab the function and evaluate
    auto func = _function.getFunction( iterator.get() );
    auto io = func( iterator.get() );
    unsigned iosize = io->size();

    npy_intp dims[2] = { size, iosize };
    
    int numtype=-1;
    int sizeitem = io->_dataSize;
    if(         dynamic_cast<const IO_char*>(io) ){
        numtype = NPY_BYTE;
    } else if ( dynamic_cast<const IO_short*>(io) ){
        numtype = NPY_SHORT;
    } else if ( dynamic_cast<const IO_int*>(io) ){
        numtype = NPY_INT;
    } else if ( dynamic_cast<const IO_float*>(io) ){
        numtype = NPY_FLOAT;
    } else if ( dynamic_cast<const IO_double*>(io) ){
        numtype = NPY_DOUBLE;
    } else if ( dynamic_cast<const IO_bool*>(io) ){
        numtype = NPY_BOOL;
    } else
        throw std::invalid_argument("Query function does not appear to produce a valid output.");
    
    // allocate numpy array
    PyObject* pyobj = PyArray_New(&PyArray_Type, 2, dims, numtype, NULL, NULL, sizeitem, (int)NULL, NULL);
 
    // setup numpy iterator for output
    NpyIter* iter;
    NpyIter_IterNextFunc *iternext;
    char** dataptr;
    npy_intp* innersizeptr;
//    npy_intp* strideptr;

    iter = NpyIter_New( (PyArrayObject*)pyobj, NPY_ITER_WRITEONLY|
                               NPY_ITER_EXTERNAL_LOOP|
                               NPY_ITER_REFS_OK,
                               NPY_KEEPORDER, NPY_NO_CASTING, NULL);
    if (iter == NULL) {
        Py_DECREF(pyobj);
        throw std::invalid_argument("An unknown error was encountered. Please contact developers.");
    }
    
    iternext = NpyIter_GetIterNext(iter, NULL);
    if (iternext == NULL) {
        NpyIter_Deallocate(iter);
        Py_DECREF(pyobj);
        throw std::invalid_argument("An unknown error was encountered. Please contact developers.");
    }

    /* The location of the data pointer which the iterator may update */
    dataptr = NpyIter_GetDataPtrArray(iter);
    /* The location of the stride which the iterator may update */
    //strideptr = NpyIter_GetInnerStrideArray(iter);
    /* The location of the inner loop size which the iterator may update */
    innersizeptr = NpyIter_GetInnerLoopSizePtr(iter);

    /* The iteration loop */
    do {
        /* Get the inner loop data/stride/count values */
        char* data = *dataptr;
        //npy_intp stride = *strideptr;
        npy_intp count = *innersizeptr;

        /* This is a typical inner loop for NPY_ITER_EXTERNAL_LOOP */
        unsigned todo = count;
        while (todo) {
            // evaluate
            io = func( iterator.get() );
            // copy result into numpy array
            memcpy( data, (char*)io->dataRaw(), sizeitem*iosize);
            // increment numpy array ptr
            data += sizeitem*iosize;
            // increment iterator object (note c++ ++ overload)
            iterator++;
            // how many left todo?
            todo-=iosize;
        }

        /* Increment the iterator to the next inner loop */
    } while(iternext(iter));

    NpyIter_Deallocate(iter);

    return pyobj;
}