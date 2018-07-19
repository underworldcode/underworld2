/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Analytic_hpp__
#define __Underworld_Function_Analytic_hpp__

#include <string>

#include "Function.hpp"

/*
 
This template class uses a CRTP pattern. It's ugly, but tries to do the heavy lifting
so that for an additional solution, only a minimal set of concrete methods needs to
be specified. See AnalyticSolCx.hpp for an example of usage.
 
When a new solution concrete class is added (for example see `AnalyticSolCx.hpp`), you
need to add a template line similar to
   %template(SolCxCRTP) Fn::AnalyticCRTP<Fn::SolCx>;
within
   libUnderworld/libUnderworldPy/Function.i
This actually asks the compiler to generate a concrete class _implementation_ for
the provided template variable (`Fn::SolCx` in this case). You will also need
to `%include` the solution header within Function.i so that swig generates
the bindings, and `#includes` the header within Function.i so that the generated
the bindings can access the declarations.

Finally, go ahead and add the user facing Python interface within
   underworld/function/analytic.py
Docstrings and doctests are also fun!
 
*/

namespace Fn {

    template <typename T, int outsize, FunctionIO::IOType outtype>
    class _Analytic: public Function
    {
        public:
            // Note the syntax here is a pointer to a class member function
            //(a 'method' in Python speak). We template on this.
            typedef void (T::*membFuncType)(const double*, double*);
            // second constructor argument is the pointer to a member function (*F) of T
            _Analytic(T *sol, membFuncType memberFunc): _sol(sol), memberFunc(memberFunc){};
            virtual func getFunction( IOsptr sample_input )
            {
                // setup output
                std::shared_ptr<IO_double> _output_sp = std::make_shared<IO_double>(outsize, outtype);
                IO_double* _output = _output_sp.get();

                const IO_double* iodouble = dynamic_cast<const IO_double*>(sample_input);
                T* solGuy = _sol;
                membFuncType memberFuncGuy = memberFunc;
                if ( iodouble && ((iodouble->size()==2)||(iodouble->size()==3)) ){
                    return [_output, _output_sp, solGuy, memberFuncGuy](IOsptr input)->IOsptr {
                        const IO_double* iodouble = debug_dynamic_cast<const IO_double*>(input);            

                        (solGuy->*memberFuncGuy)(iodouble->data(),_output->data());

                        return debug_dynamic_cast<const FunctionIO*>(_output);
                    };
                }
                // if we get here, something aint right
                throw std::invalid_argument(_pyfnerrorheader+"Function does not appear to be compatible with provided input type.");
            };
        private:
            T* _sol;
            membFuncType memberFunc;
    };

    template <class T>
    class AnalyticCRTP
    {
        public:
            AnalyticCRTP(T* selfGuy, unsigned dim)
            {
                if (dim == 2)
                {
                    velocityFn   = new _Analytic<T, 2, FunctionIO::Vector         >(selfGuy,&T::velocity);
                    pressureFn   = new _Analytic<T, 1, FunctionIO::Scalar         >(selfGuy,&T::pressure);
                    stressFn     = new _Analytic<T, 3, FunctionIO::SymmetricTensor>(selfGuy,&T::stress);
                    strainRateFn = new _Analytic<T, 3, FunctionIO::SymmetricTensor>(selfGuy,&T::strainrate);
                    viscosityFn  = new _Analytic<T, 1, FunctionIO::Scalar         >(selfGuy,&T::viscosity);
                    bodyForceFn  = new _Analytic<T, 2, FunctionIO::Vector         >(selfGuy,&T::bodyforce);
                }
                else if (dim == 3)
                {
                    velocityFn   = new _Analytic<T, 3, FunctionIO::Vector         >(selfGuy,&T::velocity);
                    pressureFn   = new _Analytic<T, 1, FunctionIO::Scalar         >(selfGuy,&T::pressure);
                    stressFn     = new _Analytic<T, 6, FunctionIO::SymmetricTensor>(selfGuy,&T::stress);
                    strainRateFn = new _Analytic<T, 6, FunctionIO::SymmetricTensor>(selfGuy,&T::strainrate);
                    viscosityFn  = new _Analytic<T, 1, FunctionIO::Scalar         >(selfGuy,&T::viscosity);
                    bodyForceFn  = new _Analytic<T, 3, FunctionIO::Vector         >(selfGuy,&T::bodyforce);
                } else
                    throw std::invalid_argument(_pyfnerrorheader+"Solution appears to have invalid dimensionality.");
            };
            Function *velocityFn;
            Function *pressureFn;
            Function *stressFn;
            Function *strainRateFn;
            Function *viscosityFn;
            Function *bodyForceFn;
            ~AnalyticCRTP()
            {
                delete velocityFn;
                delete pressureFn;
                delete stressFn;
                delete strainRateFn;
                delete viscosityFn;
                delete bodyForceFn;
            }
    };
}

#endif

