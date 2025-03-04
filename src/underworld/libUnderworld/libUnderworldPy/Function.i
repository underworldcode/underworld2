/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#if (SWIG_VERSION <= 0x040000)
%module (package="underworld.libUnderworld.libUnderworldPy") Function
#else
%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") Function
#endif

%include "exception.i"
%import "StGermain.i"
%import "StgDomain.i"
%import "StgFEM.i"
%import "PICellerator.i"

%{
/* Includes the header in the wrapper code */
#define SWIG_FILE_WITH_INIT
#include <vector>
#include <cmath>
#include <string>
#include <functional>
#include <typeindex>
#include <Underworld/Function/src/FunctionIO.hpp>
#include <Underworld/Function/src/Function.hpp>
#include <Underworld/Function/src/SafeMaths.hpp>
#include <Underworld/Function/src/CustomException.hpp>
#include <Underworld/Function/src/MinMax.hpp>
#include <Underworld/Function/src/Count.hpp>
#include <Underworld/Function/src/Constant.hpp>
#include <Underworld/Function/src/SwarmVariableFn.hpp>
#include <Underworld/Function/src/ParticleFound.hpp>
#include <Underworld/Function/src/FeVariableFn.hpp>
#include <Underworld/Function/src/GradFeVariableFn.hpp>
#include <Underworld/Function/src/Map.hpp>
#include <Underworld/Function/src/Unary.hpp>
#include <Underworld/Function/src/Binary.hpp>
    
#include <Underworld/Function/src/Analytic.hpp>
#include <Underworld/Function/src/AnalyticSolA.hpp>
#include <Underworld/Function/src/AnalyticSolB.hpp>
#include <Underworld/Function/src/AnalyticSolC.hpp>
#include <Underworld/Function/src/AnalyticSolCx.hpp>
#include <Underworld/Function/src/AnalyticSolDA.hpp>
#include <Underworld/Function/src/AnalyticSolDB2d.hpp>
#include <Underworld/Function/src/AnalyticSolDB3d.hpp>
#include <Underworld/Function/src/AnalyticSolH.hpp>
#include <Underworld/Function/src/AnalyticSolHA.hpp>
#include <Underworld/Function/src/AnalyticSolKx.hpp>
#include <Underworld/Function/src/AnalyticSolKz.hpp>
#include <Underworld/Function/src/AnalyticSolM.hpp>
#include <Underworld/Function/src/AnalyticSolNL.hpp>

#include <Underworld/Function/src/Tensor.hpp>
#include <Underworld/Function/src/IOIterators.hpp>
#include <Underworld/Function/src/Query.hpp>
#include <Underworld/Function/src/Shape.hpp>
#include <Underworld/Function/src/Relational.hpp>
#include <Underworld/Function/src/Conditional.hpp>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>
}


%}

%include "Underworld/Function/src/FunctionIO.hpp"
%include "Underworld/Function/src/Function.hpp"
%include "Underworld/Function/src/SafeMaths.hpp"
%include "Underworld/Function/src/CustomException.hpp"
%include "Underworld/Function/src/MinMax.hpp"
%include "Underworld/Function/src/Count.hpp"
%include "Underworld/Function/src/Unary.hpp"
%include "Underworld/Function/src/Binary.hpp"
%include "Underworld/Function/src/Constant.hpp"
%include "Underworld/Function/src/SwarmVariableFn.hpp"
%include "Underworld/Function/src/ParticleFound.hpp"
%include "Underworld/Function/src/FeVariableFn.hpp"
%include "Underworld/Function/src/GradFeVariableFn.hpp"
%include "Underworld/Function/src/Map.hpp"
%include "Underworld/Function/src/Tensor.hpp"

%include "Underworld/Function/src/Analytic.hpp"
%include "Underworld/Function/src/AnalyticSolA.hpp"
%include "Underworld/Function/src/AnalyticSolB.hpp"
%include "Underworld/Function/src/AnalyticSolC.hpp"
%include "Underworld/Function/src/AnalyticSolCx.hpp"
%include "Underworld/Function/src/AnalyticSolDA.hpp"
%include "Underworld/Function/src/AnalyticSolDB2d.hpp"
%include "Underworld/Function/src/AnalyticSolDB3d.hpp"
%include "Underworld/Function/src/AnalyticSolH.hpp"
%include "Underworld/Function/src/AnalyticSolHA.hpp"
%include "Underworld/Function/src/AnalyticSolKx.hpp"
%include "Underworld/Function/src/AnalyticSolKz.hpp"
%include "Underworld/Function/src/AnalyticSolM.hpp"
%include "Underworld/Function/src/AnalyticSolNL.hpp"

%include "Underworld/Function/src/IOIterators.hpp"
%include "Underworld/Function/src/Query.hpp"
%include "Underworld/Function/src/Shape.hpp"
%include "Underworld/Function/src/Relational.hpp"
%include "Underworld/Function/src/Conditional.hpp"


%template(IO_bool)     FunctionIOTyped<bool, char>;
%template(IO_char)     FunctionIOTyped<char, char>;
%template(IO_short)    FunctionIOTyped<short, short>;
%template(IO_unsigned) FunctionIOTyped<unsigned, unsigned>;
%template(IO_int)      FunctionIOTyped<int, int>;
%template(IO_float)    FunctionIOTyped<float, float>;
%template(IO_double)   FunctionIOTyped<double, double>;

%template(MathUnary_cos)    Fn::MathUnary<std::cos>;
%template(MathUnary_sin)    Fn::MathUnary<std::sin>;
%template(MathUnary_tan)    Fn::MathUnary<std::tan>;
%template(MathUnary_acos)   Fn::MathUnary<std::acos>;
%template(MathUnary_asin)   Fn::MathUnary<std::asin>;
%template(MathUnary_atan)   Fn::MathUnary<std::atan>;
%template(MathUnary_cosh)   Fn::MathUnary<std::cosh>;
%template(MathUnary_sinh)   Fn::MathUnary<std::sinh>;
%template(MathUnary_tanh)   Fn::MathUnary<std::tanh>;
%template(MathUnary_acosh)  Fn::MathUnary<std::acosh>;
%template(MathUnary_asinh)  Fn::MathUnary<std::asinh>;
%template(MathUnary_atanh)  Fn::MathUnary<std::atanh>;
%template(MathUnary_exp)    Fn::MathUnary<std::exp>;
%template(MathUnary_log)    Fn::MathUnary<std::log>;
%template(MathUnary_log2)   Fn::MathUnary<std::log2>;
%template(MathUnary_log10)  Fn::MathUnary<std::log10>;
%template(MathUnary_sqrt)   Fn::MathUnary<std::sqrt>;
%template(MathUnary_erf)    Fn::MathUnary<std::erf>;
%template(MathUnary_erfc)   Fn::MathUnary<std::erfc>;
%template(MathUnary_abs)    Fn::MathUnary<std::abs>;

%template(SolACRTP)    Fn::AnalyticCRTP<Fn::SolA>;
%template(SolBCRTP)    Fn::AnalyticCRTP<Fn::SolB>;
%template(SolCCRTP)    Fn::AnalyticCRTP<Fn::SolC>;
%template(SolCxCRTP)   Fn::AnalyticCRTP<Fn::SolCx>;
%template(SolDACRTP)   Fn::AnalyticCRTP<Fn::SolDA>;
%template(SolHCRTP)    Fn::AnalyticCRTP<Fn::SolH>;
%template(SolHACRTP)   Fn::AnalyticCRTP<Fn::SolHA>;
%template(SolKxCRTP)   Fn::AnalyticCRTP<Fn::SolKx>;
%template(SolKzCRTP)   Fn::AnalyticCRTP<Fn::SolKz>;
%template(SolMCRTP)    Fn::AnalyticCRTP<Fn::SolM>;
%template(SolNLCRTP)   Fn::AnalyticCRTP<Fn::SolNL>;
%template(SolDB2dCRTP) Fn::AnalyticCRTP<Fn::SolDB2d>;
%template(SolDB3dCRTP) Fn::AnalyticCRTP<Fn::SolDB3d>;

%template(Relational_less         ) Fn::MathRelational<std::less< double > >;
%template(Relational_less_equal   ) Fn::MathRelational<std::less_equal< double > >;
//%template(Relational_equal_to     ) Fn::MathRelational<std::equal_to< double > >;
//%template(Relational_not_equal_to ) Fn::MathRelational<std::not_equal_to< double > >;
%template(Relational_greater      ) Fn::MathRelational<std::greater< double > >;
%template(Relational_greater_equal) Fn::MathRelational<std::greater_equal< double > >;
%template(Relational_logical_and  ) Fn::MathRelational<std::logical_and< bool > >;
%template(Relational_logical_or   ) Fn::MathRelational<std::logical_or< bool > >;
%template(Relational_logical_xor  ) Fn::MathRelational< Fn::logical_xor< bool > >;
//%template(Relational_logical_not  ) Fn::MathRelational<std::logical_not<bool>>;
