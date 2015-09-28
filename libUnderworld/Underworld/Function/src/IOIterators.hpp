/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __Underworld_Function_IOIterators_hpp__
#define __Underworld_Function_IOIterators_hpp__


#include <exception>
#include <Python.h>
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
}
#include "FunctionIO.hpp"
#include "Function.hpp"
#include "MeshCoordinate.hpp"
#include "ParticleCoordinate.hpp"

namespace Fn {

class IOIterator
{
    public:
        virtual IOIterator& operator++(int) =0;
        Function::IOsptr get(){ return debug_dynamic_cast<const FunctionIO>(_io); };
        virtual void reset() =0;
        unsigned size() const{ return _size; };
    protected:
        IOIterator(): _io(NULL), _size(0) {};
        std::shared_ptr<FunctionIO> _io;
        virtual ~IOIterator(){};
        unsigned _size;
};

class MeshIndexSet: public IOIterator
{
    public:
        MeshIndexSet( IndexSet* indexSet, void* mesh );
        virtual IOIterator& operator++(int){ _position++; _setNewIO(); return *this;};
        virtual ~MeshIndexSet(){ if (_indexArray) free(_indexArray); };
        virtual void reset();
    private:
        void _setNewIO();
        IndexSet* _indexSet;
        void* _mesh;
        Index* _indexArray;
        unsigned _position;
    
};

class NumpyInput: public IOIterator
{
    public:
        NumpyInput( PyObject* arr, FunctionIO::IOType inputType );
        virtual ~NumpyInput();
        virtual void reset();
        void grab();
        virtual NumpyInput& operator++(int);
    private:
        PyObject* _arr;
        unsigned _dim;
        NpyIter* _iter;
        NpyIter_IterNextFunc *_iternext;
        char** _dataptr;
        npy_intp* _innersizeptr;
        npy_intp* _strideptr;
        npy_intp _count;
        char* _data;
        FunctionIO::IOType _inputType;
};

class SwarmInput: public IOIterator
{
    public:
        SwarmInput( void* positionVariable );
        virtual IOIterator& operator++(int){ _position++; _setNewIO(); return *this;};
        virtual ~SwarmInput(){};
        virtual void reset();
    private:
        void _setNewIO();
        void* _positionVariable;
        unsigned _position;
    
};




}

#endif