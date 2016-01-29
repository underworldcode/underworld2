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

#include "Function.hpp"

Fn::Input::func Fn::Input::getFunction( IOsptr sample_input )
{
    // return the trival (pass through) lambda
    return [](IOsptr input)->IOsptr { return input;};
}


Fn::SafeMaths::func Fn::SafeMaths::getFunction( IOsptr sample_input )
{
    // get function.. nothing to test
    func _func = _fn->getFunction( sample_input );
    
    // create and return the lambda
    return [_func](IOsptr input)->IOsptr {

        // clear all errors
        std::feclearexcept(FE_ALL_EXCEPT);
        // perform func
        IOsptr _output = _func(input);
        // check for errors
        if(       std::fetestexcept(FE_DIVBYZERO) )
            throw std::runtime_error("Divide by zero encountered while evaluating function.");
        else if ( std::fetestexcept(FE_INVALID)   )
            throw std::runtime_error("Invalid domain was encountered while evaluating function.");
        else if ( std::fetestexcept(FE_OVERFLOW)  )
            throw std::runtime_error("Value overflow error was encountered while evaluating function.");
        else if ( std::fetestexcept(FE_UNDERFLOW) )
            throw std::runtime_error("Value underflow error was encountered while evaluating function.");
        
        // ok, we got this far, let's continue
        return _output;
    };

}

#include <limits>   

Fn::MinMax::func Fn::MinMax::getFunction( IOsptr sample_input )
{
    // get function.. nothing to test
    func _func = _fn->getFunction( sample_input );
    
    std::shared_ptr<const FunctionIO> output = _func(sample_input);
    unsigned fn_out_size = output->size();
    if (_size < 0) {
        _size = fn_out_size;
    } else if ( _size != fn_out_size )
        throw std::runtime_error("MinMax subject function's return size appears to have changed.\n\
                                  Please reset MinMax, or create a new one.");

    if ( _size == 1 ) {
        return [_func,this](IOsptr input)->IOsptr {

            // perform func
            IOsptr _output = _func(input);
            
            // get
            double val = _output->at<double>();
            if      ( val < _minVal ) _minVal = val;
            else if ( val > _maxVal ) _maxVal = val;
            

            return _output;
        };
    } else {
    // create and return the lambda
        return [_func,this](IOsptr input)->IOsptr {

            // perform func
            IOsptr _output = _func(input);
            
            // get
            double val = 0;
            for(unsigned ii=0; ii<this->_size; ii++)
                val += std::pow(_output->at<double>(ii),2);
            
            if      ( val < _minVal ) _minVal = val;
            else if ( val > _maxVal ) _maxVal = val;
            
            return _output;
        };
    }

}

void Fn::MinMax::reset()
{
    _minVal = std::numeric_limits<double>::max();
    _maxVal = std::numeric_limits<double>::lowest();
}

double Fn::MinMax::getMin()
{
    if (_size == 1)
        return _minVal;
    else
        return std::sqrt(_minVal);
}

double Fn::MinMax::getMax()
{
    if (_size == 1)
        return _maxVal;
    else
        return std::sqrt(_maxVal);
}

double Fn::MinMax::getMinGlobal()
{
    double localVal = getMin();
    double globalVal;
    (void)MPI_Allreduce( &localVal,  &globalVal, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD );
    
    return globalVal;
}

double Fn::MinMax::getMaxGlobal()
{
    double localVal = getMax();
    double globalVal;
    (void)MPI_Allreduce( &localVal,  &globalVal, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD );
    
    return globalVal;
}

