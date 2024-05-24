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
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
}
#include "FunctionIO.hpp"
#include "Function.hpp"
#include "MeshCoordinate.hpp"
#include "ParticleCoordinate.hpp"
#include "FEMCoordinate.hpp"

namespace Fn {

// Iterator type objects, for traversing some source as inputs to function object.
// As function objects require FunctionIO inputs, a FunctionIO object is created as
// a proxy from the source.  So, for example, for MeshIndexSet sources, a MeshCoordinate
// FunctionIO object is created, and it's value is updated to reflect the current index
// of the IndexSet as iteration proceeds. 
class IOIterator
{
    public:
        virtual IOIterator& operator++(int){ _position++; _setNewIO(); return *this;};
        Function::IOsptr get(){ return debug_dynamic_cast<const FunctionIO*>(_io.get()); };
        virtual void reset(){};
        unsigned size() const{ return _size; };
    protected:
        IOIterator(unsigned size=0): _io(NULL), _size(size), _position(0) {reset();};
        virtual void _setNewIO(){};
        std::shared_ptr<FunctionIO> _io;
        virtual ~IOIterator(){};
        unsigned _size;
        unsigned _position;

    
};


// this iterator class iterates over a single FunctionIO object passed in from python
class FunctionIOIter: public IOIterator
{
public:
    FunctionIOIter( FunctionIO* fIO ): IOIterator(1), _fIO(fIO) {};
    virtual void reset(){ _position = 0; _io=std::shared_ptr<FunctionIO>(_fIO->clone()); };
    virtual void _setNewIO(){ _io = NULL;};
private:
    FunctionIO* _fIO;
    
};

    
// this iterator class iterates over mesh index set objects
class MeshIndexSet: public IOIterator
{
    public:
        MeshIndexSet( IndexSet* indexSet, void* mesh );
        virtual ~MeshIndexSet(){ if (_indexArray) free(_indexArray); };
        virtual void reset();
    private:
        virtual void  _setNewIO();
        IndexSet* _indexSet;
        void* _mesh;
        Index* _indexArray;
    
};

// this iterator class iterates over numpy arrays
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

// this iterator class iterates over swarm particle global coordinates
class SwarmInput: public IOIterator
{
    public:
        SwarmInput( void* positionVariable );
        virtual ~SwarmInput(){};
        virtual void reset();
    private:
        virtual void  _setNewIO();
        void* _positionVariable;
    
};

// this iterator class iterates over integration swarm coordinates
class IntegrationSwarmInput: public IOIterator
{
    public:
        IntegrationSwarmInput( void* _intSwarm );
        virtual ~IntegrationSwarmInput(){};
        virtual void reset();
    private:
        static void* _CheckIsIntegrationSwarm(void* intSwarm);
        virtual void  _setNewIO();
        IntegrationPointsSwarm* _intSwarm;
        std::shared_ptr<ParticleCoordinate> _localCoord;
};




}

#endif
