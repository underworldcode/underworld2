/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <string>
#include <sstream>

#include "CustomException.hpp"

Fn::CustomException::func Fn::CustomException::getFunction( IOsptr sample_input )
{
    // get functions.. nothing to test as will simply be passed through
    func func_input = _fn_input->getFunction( sample_input );
    
    // the condition function
    func func_condition = _fn_condition->getFunction( sample_input );
    // need to ensure func_condition returns a bool
    auto condition_output = dynamic_cast<const IO_bool*>(func_condition( sample_input ));
    if (!condition_output)
        throw std::invalid_argument( _pyfnerrorheader+"CustomException condition function does not appear to \
                                    return a Bool type output as required." );
    
    // print guy
    func func_print = NULL;
    if (_fn_print)
        func_print = _fn_print->getFunction( sample_input );
    
    // create and return the lambda
    return [ func_input, func_condition, func_print, this ](IOsptr input)->IOsptr {
        
        // now lets evaluate our condition function and check its output
        const IO_bool* condition_output = debug_dynamic_cast<const IO_bool*>(func_condition(input));
        if (!condition_output->at())
        {
            std::stringstream ss;
            ss << "CustomException condition function has evaluated to False for current input!";
            if (func_print){
                IOsptr print_output = func_print(input);
                ss << "\nPrint function returns the following values (cast to double precision):\n";
                ss << "    ( " << print_output->at<double>(0);
                for (unsigned ii=1; ii<print_output->size(); ii++ )
                    ss << ", " << print_output->at<double>(ii);
                ss << " )";
            }
            throw std::runtime_error(_pyfnerrorheader+ss.str());
        }
        
        // ok, we got this far, let's continue.. return the result
        return func_input(input);
    };
    
}



