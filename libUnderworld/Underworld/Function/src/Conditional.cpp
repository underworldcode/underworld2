/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <typeindex>
#include <sstream>

#include "Conditional.hpp"

void Fn::Conditional::insert( Function* condition, Function* value )
{
    _clause.push_back( std::pair<Function*,Function*>(condition,value) );
}

Fn::Conditional::func Fn::Conditional::getFunction( IOsptr sample_input )
{

    std::vector< std::pair<Fn::Function::func,Fn::Function::func> > _funcfuncArray;
    // ensure correct other funcs
    unsigned outputSize =-1;
    std::type_index outputType = typeid(NULL);
    for (unsigned ii=0; ii<_clause.size(); ii++ )
    {
        // get condition func
        auto condfunc = _clause.at(ii).first->getFunction( sample_input );
        // check if returns bool
        auto boolio = std::dynamic_pointer_cast<const IO_bool>(condfunc( sample_input ));
        if (!boolio){
            std::stringstream ss;
            ss << "Issue with clause " << ii << " of conditional function.\n";
            ss << "Condition function does not appear to return 'bool' result.";
            throw std::invalid_argument( ss.str() );
        }
        if (boolio->size() != 1){
            std::stringstream ss;
            ss << "Issue with clause " << ii << " of conditional function.\n";
            ss << "Condition function appears to return non-scalar result (n=" << boolio->size() << ").\n";
            throw std::invalid_argument( ss.str() );
        }
        // get consequent func
        auto consfunc = _clause.at(ii).second->getFunction( sample_input );
        // check if returns bool
        auto io = std::dynamic_pointer_cast<const FunctionIO>(consfunc( sample_input ));
        if (outputSize != -1){
            if (outputSize != (int)io->size()){
                std::stringstream ss;
                ss << "Issue with clause " << ii << " of conditional function.\n";
                ss << "Consequent function appers to return result of size " << io->size() << ".\n";
                ss << "Previous function returned result of size " << outputSize << ".\n";
                ss << "All consequent function must return results of identical size.";
                throw std::invalid_argument( ss.str() );
            }
            if (outputType != io->dataType()){
                std::stringstream ss;
                ss << "Issue with clause " << ii << " of conditional function.\n";
                ss << "Consequent function appers to return result of type " << functionio_get_type_name(io->dataType()) << ".\n";
                ss << "Previous function returned result of type " << functionio_get_type_name(outputType) << ".\n";
                ss << "All consequent function must return results of identical type.";
                throw std::invalid_argument( ss.str() );
            }
        }
        else{
          outputSize = io->size();
          outputType = io->dataType();
        }
        _funcfuncArray.push_back( std::pair<Fn::Function::func,Fn::Function::func>(condfunc,consfunc) );
    }

    // check that some func has been set!
    if (outputSize == -1)
      throw std::invalid_argument( "It does not appear that any clauses have been set for the conditional function." );

    unsigned totalClauses = _funcfuncArray.size();
    return [_funcfuncArray, totalClauses](IOsptr input)->IOsptr {
      for (unsigned ii=0; ii<totalClauses; ii++){
        bool result = (_funcfuncArray[ii].first(input))->at<bool>();
        if (result){
          return _funcfuncArray[ii].second(input);
        }
      }
      // something aint right
      throw std::runtime_error( "Reached end of conditional statement. At least one of the clause conditions must evaluate to 'True'." );
    };
}
