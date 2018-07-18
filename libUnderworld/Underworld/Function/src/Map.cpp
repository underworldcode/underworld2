/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <sstream>
#include "Map.hpp"


Fn::Map::Map(Function* keyFunc, Function* defaultFunc)
    : Function(), _keyFunc(keyFunc), _defaultFunc(defaultFunc)
{};


void Fn::Map::insert( unsigned key, Function* value )
{
    if (key >= _funcArray.size())
    {
        _funcArray.resize(key+1);
        _isIndexInMap.resize(key+1);
    }
    _funcArray[key] = value;
    _isIndexInMap[key] = true;
    
}

Fn::Map::func Fn::Map::getFunction( IOsptr sample_input )
{
    // get key function
    auto _keyFuncFunc = _keyFunc->getFunction( sample_input );
    // now do test eval
    auto keyfuncout   = _keyFuncFunc( sample_input );
    if (keyfuncout->size() != 1)
        throw std::invalid_argument( "Error setting up 'map' function.\nKey function appears to return a non-scalar value." );

    // ensure correct default func
    int outputSize = -1;
    
    Fn::Function::func _defaultFuncFunc;
    if (_defaultFunc)
    {
        // get key function
        _defaultFuncFunc = _defaultFunc->getFunction( sample_input );
        // check if output castable to IO_double
        auto _defaultFuncOut = dynamic_cast<const IO_double*>(_defaultFuncFunc( sample_input ) );
        if (!_defaultFuncOut)
            throw std::invalid_argument( "Error setting up 'map' function.\nDefault function does not appear to return a 'double' value, as required. "
                                         "Note that where the defaut Function you have constructed uses Python numeric objects, those objects "
                                         "must be of 'float' type (so for example '2.' instead of '2').");

        outputSize = _defaultFuncOut->size();
    }

    std::vector<Fn::Map::func> _funcfuncArray(_funcArray.size());

    // ensure correct other funcs
    for (unsigned ii=0; ii<_isIndexInMap.size(); ii++ )
    {
        if (_isIndexInMap[ii])
        {
            // get func
            _funcfuncArray[ii] = _funcArray[ii]->getFunction( sample_input );
            // check if returns double
            auto doubleio = dynamic_cast<const IO_double*>(_funcfuncArray[ii]( sample_input ));
            if (!doubleio)
            {
                std::stringstream ss;
                ss << "Error setting up 'map' function.\n"
                ss << "Function with key " << ii << " does not appear to return a 'double' value. "
                                                    "The 'map' function currently only supports 'double' return values. "
                                                    "Note that where the Function you have constructed uses Python numeric "
                                                    "objects, those objects must be of 'float' type (so for example '2.'  "
                                                    "instead of '2').";
                throw std::invalid_argument( ss.str());
            }
            if (outputSize != -1)
            {
                if (outputSize != (int)doubleio->size())
                {
                    std::stringstream ss;
                    ss << "Error setting up 'map' function.\n"
                    ss << "Function with key " << ii <<" appears to return output\n";
                    ss << "of different size (" << doubleio->size() << ") to previous or default\n";
                    ss << "function (" << outputSize << ").";
                    throw std::invalid_argument( ss.str() );
                }

            }
            else
            {
                outputSize = doubleio->size();
            }
            
        }
    }

    // check that some func has been set!
    if (outputSize == -1)
        throw std::invalid_argument( "It does not appear that any functions have been set for the 'map' function." );

    // grab copy of this guy
    std::vector<bool>  isIndexInMap = _isIndexInMap;
    return [_keyFuncFunc, _defaultFuncFunc, _funcfuncArray, isIndexInMap](IOsptr input)->IOsptr {
        // evaluate key
        auto keyio = _keyFuncFunc( input );
        const unsigned key = keyio->at<unsigned>();
        
        if (key<isIndexInMap.size() && isIndexInMap[key])
            // ok, we have func, eval
            return _funcfuncArray[key](input);
        else if ( _defaultFuncFunc )
            // no func for key, so use default
            return _defaultFuncFunc(input);
        else
        {
            std::stringstream ss;
            ss << "Error evaluating 'map' function.\n"
            ss << "Key function evaluates to key (" << key << ") which\n";
            ss << "does not appear to map to any functions, and no default function has been set.";
            // something aint right
            throw std::runtime_error( ss.str() );
        }
    };
}
