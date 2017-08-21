/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_MinMax_hpp__
#define __Underworld_Function_MinMax_hpp__

#include <functional>
#include <stdexcept>

#include "FunctionIO.hpp"

namespace Fn {
    
    class MinMax: public Function
    {
    public:
        MinMax( Function *fn, Function *fn_norm=NULL, Function *fn_auxiliary=NULL ):
            _fn(fn), _fn_norm(fn_norm), _fn_auxiliary(fn_auxiliary),
            _min_rank(-1), _max_rank(-1),
            _fn_auxiliary_io_min(NULL), _fn_auxiliary_io_max(NULL),
            _fn_auxiliary_io_min_global(NULL), _fn_auxiliary_io_max_global(NULL) {reset();};
        virtual ~MinMax(){};
        virtual func getFunction( IOsptr sample_input );
        double getMin();
        double getMax();
        double getMinGlobal();
        double getMaxGlobal();
        int getMinRank(){return _min_rank;};
        int getMaxRank(){return _max_rank;};
        FunctionIO* getMinAux();
        FunctionIO* getMaxAux();
        FunctionIO* getMinAuxGlobal();
        FunctionIO* getMaxAuxGlobal();
        void reset();
    protected:
        Function* _fn;
        Function* _fn_norm;
        Function* _fn_auxiliary;
        int _min_rank;
        int _max_rank;
        std::shared_ptr<FunctionIO> _fn_auxiliary_io_min;
        std::shared_ptr<FunctionIO> _fn_auxiliary_io_max;
        std::shared_ptr<FunctionIO> _fn_auxiliary_io_min_global;
        std::shared_ptr<FunctionIO> _fn_auxiliary_io_max_global;
        double _minVal;
        double _maxVal;
    };
    
}


#endif /* __Underworld_Function_MinMax_hpp__ */

