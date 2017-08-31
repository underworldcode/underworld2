/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <cfenv>
#include <cmath>
#include <mpi.h>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstring>

#include "Function.hpp"
#include "FunctionIO.hpp"
#include "MinMax.hpp"

#include <limits>

Fn::MinMax::func Fn::MinMax::getFunction( IOsptr sample_input )
{
    // get function.. nothing to test
    func _func = _fn->getFunction( sample_input );
    
    const FunctionIO* output;
    output = _func(sample_input);
    if (output->size() > 1 && !_fn_norm)
        throw std::invalid_argument("Function provided to `min_max` class does not return scalar results. "\
                                    "You must also provide a function which calculates the required norm like quantity "\
                                    "via the `fn_norm` parameter.");

    func _func_norm=NULL;
    if (_fn_norm)
    {
        _func_norm = _fn_norm->getFunction( sample_input );
        output = _func_norm(sample_input);
        
        if (output->size()>1)
        {
            std::stringstream ss;
            ss << "Norm function appears to return a result of size " << output->size() << ", where a scalar result it required.";
            throw std::invalid_argument( ss.str() );
        }
    }

    func _func_auxiliary=NULL;
    if (_fn_auxiliary)
    {
        // ok, if we are provided with an aux function, lets get some things...  first the func
        _func_auxiliary = _fn_auxiliary->getFunction( sample_input );
        // now, we need to get examples of the functionIO data out of the func.
        // we also need to take clones of this data to keep a persistent record of the results.
        // also, we only clone the typed version of this functionio, because deeper children classes
        // won't play nice with memcpy to the raw data, which is kinda necessary for MPIness.
        // we wrap all this in smart pointers for shits and giggles.
        _fn_auxiliary_io_min        = std::shared_ptr<FunctionIO>(dynamic_cast<FunctionIO*>(_func_auxiliary(sample_input)->cloneType()));
        _fn_auxiliary_io_max        = std::shared_ptr<FunctionIO>(dynamic_cast<FunctionIO*>(_func_auxiliary(sample_input)->cloneType()));
    }
    
    return [_func,_func_norm,_func_auxiliary,this](IOsptr input)->IOsptr {
        
        // perform func
        IOsptr _output = _func(input);
        
        double val;
        if(_func_norm)
        {  // use norm func if necessary
            IOsptr _output_norm = _func_norm(input);
            val = _output_norm->at<double>();
        } else
        {  //  else just use standard input
            val = _output->at<double>();
        }
        

        if      ( val < _minVal )
        {
            _minVal = val;
            if(_func_auxiliary)
            {
                // eval aux function if necessary
                const FunctionIO* _func_aux_io = _func_auxiliary(input);
                // copy raw data
                std::memcpy(_fn_auxiliary_io_min->dataRaw(), _func_aux_io->dataRaw(), _func_aux_io->size()*_func_aux_io->_dataSize);
            }
        }
        else if ( val > _maxVal )
        {
            _maxVal = val;
            if(_func_auxiliary)
            {
                // eval aux function if necessary
                const FunctionIO* _func_aux_io = _func_auxiliary(input);
                // copy raw data
                std::memcpy(_fn_auxiliary_io_max->dataRaw(), _func_aux_io->dataRaw(), _func_aux_io->size()*_func_aux_io->_dataSize);
            }
        }
        
        return _output;
    };
    
}

void Fn::MinMax::reset()
{
    _minVal = std::numeric_limits<double>::max();
    _maxVal = std::numeric_limits<double>::lowest();
    _min_rank = -1;
    _max_rank = -1;
    _fn_auxiliary_io_min        = std::shared_ptr<FunctionIO>(NULL);
    _fn_auxiliary_io_max        = std::shared_ptr<FunctionIO>(NULL);

}

double Fn::MinMax::getMin()
{
    return _minVal;
}

double Fn::MinMax::getMax()
{
    return _maxVal;
    
}

double Fn::MinMax::getMinGlobal()
{
    // create struct which is necessary for MPI_DOUBLE_INT type
    struct {
        double val;
        int   rank;
    } localVal, globalVal;
    
    // write val to reduce
    localVal.val = getMin();
    // write second data which is shared.. in this case, just the current rank
    MPI_Comm_rank( MPI_COMM_WORLD, &(localVal.rank) );
    
    int err = MPI_Allreduce( &localVal,  &globalVal, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD );
    
    if (err != MPI_SUCCESS)
        throw std::runtime_error("Unknown MPI communication error occurred during MinMax operation. Please contact developers.");

    // record rank where min was found to class
    _min_rank = globalVal.rank;
    
    // return global min
    return globalVal.val;
}

double Fn::MinMax::getMaxGlobal()
{
    // create struct which is necessary for MPI_DOUBLE_INT type
    struct {
        double val;
        int   rank;
    } localVal, globalVal;
    
    // write val to reduce
    localVal.val = getMax();
    // write second data which is shared.. in this case, just the current rank
    MPI_Comm_rank( MPI_COMM_WORLD, &(localVal.rank) );
    
    int err = MPI_Allreduce( &localVal,  &globalVal, 1, MPI_DOUBLE_INT, MPI_MAXLOC, MPI_COMM_WORLD );
    
    if (err != MPI_SUCCESS)
        throw std::runtime_error("Unknown MPI communication error occurred during MinMax operation. Please contact developers.");

    // record rank where max was found to class
    _max_rank = globalVal.rank;
    
    return globalVal.val;
}

FunctionIO* Fn::MinMax::getMinAux()
{
    if (!_fn_auxiliary)
        throw std::runtime_error("Operation not valid. Auxiliary function was never set for this MinMax object.");
    return _fn_auxiliary_io_min.get();
}
FunctionIO* Fn::MinMax::getMaxAux()
{
    if (!_fn_auxiliary)
        throw std::runtime_error("Operation not valid. Auxiliary function was never set for this MinMax object.");
    return _fn_auxiliary_io_max.get();
}

