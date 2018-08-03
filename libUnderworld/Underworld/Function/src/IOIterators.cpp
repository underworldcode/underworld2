/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <sstream>
#include "IOIterators.hpp"
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
}

Fn::MeshIndexSet::MeshIndexSet( IndexSet* indexSet, void* mesh )
    : IOIterator(), _indexSet(indexSet), _mesh(mesh), _indexArray(NULL)
{
    if(!Stg_Class_IsInstance( mesh, Mesh_Type ))
        throw std::invalid_argument("Provided 'mesh' does not appear to be of 'Mesh' type.");
    if(!Stg_Class_IsInstance( indexSet, IndexSet_Type ))
        throw std::invalid_argument("Provided 'indexSet' does not appear to be of 'IndexSet' type.");
    
    // get array from index set
    IndexSet_GetMembers( _indexSet, &_size, &_indexArray );
}

void Fn::MeshIndexSet::_setNewIO()
{
    if (_position<_size)
    {
        // create io guy
        std::static_pointer_cast<MeshCoordinate>(_io)->index() = _indexArray[_position];
    } else
    {
        _io = NULL;
    }
}

void Fn::MeshIndexSet::reset()
{
    _position = 0;
    // setup first guy
    if (_position<_size) {
        // create io guy
        _io = std::shared_ptr<MeshCoordinate>(new MeshCoordinate(_mesh));
        std::static_pointer_cast<MeshCoordinate>(_io)->index() = _indexArray[_position];
    }
}


Fn::NumpyInput::NumpyInput( PyObject* arr, FunctionIO::IOType inputType )
    : IOIterator(), _arr(arr), _iter(NULL), _iternext(NULL), _dataptr(NULL),
                    _innersizeptr(NULL), _strideptr(NULL), _count(0), _data(NULL), _inputType(inputType)
{
    PyArrayObject* arrguy = (PyArrayObject*)arr;
    unsigned ndims = PyArray_NDIM(arrguy);
    if(ndims != 2)
        throw std::invalid_argument("Expecting a two dimensional numpy array.");
    npy_intp* shape = PyArray_DIMS(arrguy);
    _size = shape[0];
    _dim  = shape[1];
}

void Fn::NumpyInput::reset()
{
    _io = std::make_shared<IO_double>(_dim,_inputType);

    // setup numpy iterator
    if (_iter) {
        NpyIter_Deallocate(_iter);
        _iter = NULL;
    }

    PyArray_Descr* dtype = PyArray_DescrFromType(NPY_DOUBLE);
    _iter = NpyIter_New( (PyArrayObject*)_arr, NPY_ITER_READONLY|
                               NPY_ITER_EXTERNAL_LOOP|
                               NPY_ITER_REFS_OK,
                               NPY_KEEPORDER, NPY_UNSAFE_CASTING, dtype );
    Py_DECREF(dtype);
    
    if (_iter == NULL) {
        throw std::invalid_argument("An unknown error was encountered. Please contact developers.");
    }
    
    _iternext = NpyIter_GetIterNext(_iter, NULL);
    if (_iternext == NULL) {
        NpyIter_Deallocate(_iter);
        throw std::invalid_argument("An unknown error was encountered. Please contact developers.");
    }

    /* The location of the data pointer which the iterator may update */
    _dataptr = NpyIter_GetDataPtrArray(_iter);
    /* The location of the stride which the iterator may update */
    _strideptr = NpyIter_GetInnerStrideArray(_iter);
    /* The location of the inner loop size which the iterator may update */
    _innersizeptr = NpyIter_GetInnerLoopSizePtr(_iter);

    _count = *_innersizeptr;
    _data  = *_dataptr;
    grab();
}

void Fn::NumpyInput::grab()
{
    /* */
    /* This is a typical inner loop for NPY_ITER_EXTERNAL_LOOP */
    unsigned tots = 0;  // grab 2 values
    std::size_t dataSize = _io->_dataSize;
    do {
        /* Get the inner loop data/stride/count values */
        if (_count == 0) {
            _count = *_innersizeptr;
            _data  = *_dataptr;
        }
        npy_intp stride = *_strideptr;

        /* This is a typical inner loop for NPY_ITER_EXTERNAL_LOOP */
        while (_count && (tots<_dim)) {
            memcpy( (char*)_io->dataRaw() + tots*dataSize, _data, dataSize );
            tots++;
            _data += stride;
            _count--;
        }

        /* Increment the iterator to the next inner loop */
    } while( tots<_dim );
    
}

Fn::NumpyInput& Fn::NumpyInput::operator++(int)
{
    if( _count>0 || _iternext(_iter) )
        grab();
    else
        _io = NULL;
    return *this;
};

Fn::NumpyInput::~NumpyInput()
{
    if (_iter) {
        NpyIter_Deallocate(_iter);
        _iter = NULL;
    }
}


Fn::SwarmInput::SwarmInput( void* positionVariable )
    : IOIterator(), _positionVariable(positionVariable)
{
    if(!Stg_Class_IsInstance( positionVariable, SwarmVariable_Type ))
        throw std::invalid_argument("Provided 'positionVariable' does not appear to be of 'SwarmVariable' type.");

    // get number of particles
    _size = ((SwarmVariable*)positionVariable)->swarm->particleLocalCount;
}

void Fn::SwarmInput::_setNewIO()
{
    if (_position<_size)
    {
        // set position (ie, particleLocalId) as required
        std::static_pointer_cast<ParticleCoordinate>(_io)->index() = _position;
    } else
    {
        _io = NULL;
    }
}

void Fn::SwarmInput::reset()
{
    _position = 0;
    // setup first guy
    if (_position<_size) {
        // create io guy
        _io = std::make_shared<ParticleCoordinate>(_positionVariable);
        std::static_pointer_cast<ParticleCoordinate>(_io)->index() = _position;
    }
}



void* Fn::IntegrationSwarmInput::_CheckIsIntegrationSwarm(void* intSwarm){
    // check is type IntegrationPointsSwarm
    if(!Stg_Class_IsInstance( intSwarm, IntegrationPointsSwarm_Type ))
        throw std::invalid_argument("Provided 'integrationSwarm' does not appear to be of 'IntegrationPointsSwarm' type.");
    // lets also check that it has the correct celllayout type
    if(!Stg_Class_IsInstance( ((IntegrationPointsSwarm*)intSwarm)->cellLayout, ElementCellLayout_Type ))
        throw std::invalid_argument("Provided 'integrationSwarm' does not appear to have a cell layout of 'ElementCellLayout' type.");
    return intSwarm;
}

Fn::IntegrationSwarmInput::IntegrationSwarmInput( void* integrationSwarm )
    : IOIterator(),
      _intSwarm((IntegrationPointsSwarm*)_CheckIsIntegrationSwarm(integrationSwarm))
{
    // get number of particles
    _size = _intSwarm->particleLocalCount;
}

void Fn::IntegrationSwarmInput::_setNewIO()
{
    if (_position<_size)
    {
        // set position (ie, particleLocalId) as required
        // first get owning cell
        int owningCell = StgVariable_GetValueInt( _intSwarm->owningCellVariable->variable, _position );
        // because this is a global swarm, the cellId is the localId, so set femcoord
        std::static_pointer_cast<FEMCoordinate>(_io)->index() = owningCell;
        // now set local coord
        // note that because this is a ParticleCoordinate type, 'index' is the particleLocalId,
        // note the cellId (as is the case for ParticleInCellCoord)
        _localCoord->index() = _position;
    }
    else
    {
        _io = NULL;
    }
}

void Fn::IntegrationSwarmInput::reset()
{
    _position = 0;
    // setup first guy if necessary
    if (_position<_size) {
        // create io guy
        _localCoord = std::make_shared<ParticleCoordinate>( _intSwarm->localCoordVariable );
        _io = std::make_shared<FEMCoordinate>((void*)_intSwarm->mesh, _localCoord);
    }
}

















