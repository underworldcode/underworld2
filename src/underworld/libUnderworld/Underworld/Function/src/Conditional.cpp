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
    // are there any clauses?
    if (_clause.size() == 0)
        throw std::invalid_argument( _pyfnerrorheader+"It does not appear that any clauses have been set for the conditional function." );

    unsigned outputSize =(unsigned)-1;
    std::type_index outputType = typeid(NULL);
    
    std::vector< std::pair<Fn::Function::func,Fn::Function::func> > _funcfuncArray(_clause.size());
    // test function returned values...
    // all the 'condition functions' must return type bool.
    // all the 'consequent functions' must return types identical to each other.

    for (unsigned ii=0; ii<_clause.size(); ii++ )
    {
        // get condition func
        auto condfunc = _clause.at(ii).first->getFunction( sample_input );
        // check if returns bool
        auto boolio = dynamic_cast<const IO_bool*>(condfunc( sample_input ));
        if (!boolio){
            std::stringstream ss;
            ss << "Issue with clause " << ii << " of conditional function.\n";
            ss << "Condition function does not appear to return 'bool' result.";
            throw std::invalid_argument( _pyfnerrorheader+ss.str() );
        }
        if (boolio->size() != 1){
            std::stringstream ss;
            ss << "Issue with clause " << ii << " of conditional function.\n";
            ss << "Condition function appears to return non-scalar result (n=" << boolio->size() << ").\n";
            throw std::invalid_argument( _pyfnerrorheader+ss.str() );
        }
        
        // get consequent func
        auto consfunc = _clause.at(ii).second->getFunction( sample_input );
        
        // add condfunc,consfunc to array
        _funcfuncArray[ii] =  std::pair<Fn::Function::func,Fn::Function::func>(condfunc,consfunc) ;

        try {
            auto io = dynamic_cast<const FunctionIO*>(consfunc( sample_input ));
            outputSize = io->size();
            outputType = io->dataType();
        } catch (const std::domain_error& e) {
            // let's continue the loop on domain errors.  the logic being that
            // conditionals may be used to evaluate multiple functions, some of
            // which are not valid across the entire domain, the usual example
            // being swarms which are not defined everywhere.
            // functions cannot be evaluated outside their domain, hence
            // we continue without checking the output size/type. this will
            // instead be done during the heavy loop.
            continue;
        }
    }

    if(outputSize == (unsigned)-1) // at least one of the consequent functions should have evaluated successfully.
        throw std::invalid_argument(_pyfnerrorheader+"It seems that none of the consequent functions were able to be successfully evaluated during testing.");
    
    
    
    unsigned totalClauses = _funcfuncArray.size();
    std::vector<bool> clauseTested(_clause.size(),false);

    return [_funcfuncArray, totalClauses, clauseTested, outputSize, outputType, this] (IOsptr input) mutable ->IOsptr
    {
        for (unsigned ii=0; ii<totalClauses; ii++)
        {
            bool result = (_funcfuncArray[ii].first(input))->at<bool>();
            if (result)
            {
                auto io = _funcfuncArray[ii].second(input);
                // let's test first if not done already
                if (!clauseTested[ii])
                {
                    clauseTested[ii] = true;

                    if (outputSize != (int)io->size()){
                        std::stringstream ss;
                        ss << "Issue with clause " << ii << " of conditional function.\n";
                        ss << "Consequent function appears to return result of size " << io->size() << ".\n";
                        ss << "Previous function returned result of size " << outputSize << ".\n";
                        ss << "All consequent function must return results of identical size.";
                        throw std::invalid_argument( _pyfnerrorheader+ss.str() );
                    }
                    if (outputType != io->dataType()){
                        std::stringstream ss;
                        ss << "Issue with clause " << ii << " of conditional function.\n";
                        ss << "Consequent function appears to return result of type " << functionio_get_type_name(io->dataType()) << ".\n";
                        ss << "Previous function returned result of type " << functionio_get_type_name(outputType) << ".\n";
                        ss << "All consequent function must return results of identical type.";
                        throw std::invalid_argument( _pyfnerrorheader+ss.str() );
                    }
                }

                return io;
            }
        }
        // something aint right
        throw std::runtime_error( _pyfnerrorheader+"Reached end of conditional statement. At least one of the clause conditions must evaluate to 'True'." );
    };
}
