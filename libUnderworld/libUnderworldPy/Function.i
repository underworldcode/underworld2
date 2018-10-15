/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
%module Function

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
#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/SafeMaths.hpp>
#include <Underworld/Function/CustomException.hpp>
#include <Underworld/Function/MinMax.hpp>
#include <Underworld/Function/Constant.hpp>
#include <Underworld/Function/SwarmVariableFn.hpp>
#include <Underworld/Function/ParticleFound.hpp>
#include <Underworld/Function/FeVariableFn.hpp>
#include <Underworld/Function/GradFeVariableFn.hpp>
#include <Underworld/Function/Map.hpp>
#include <Underworld/Function/Unary.hpp>
#include <Underworld/Function/Binary.hpp>
    
#include <Underworld/Function/Analytic.hpp>
#include <Underworld/Function/AnalyticSolA.hpp>
#include <Underworld/Function/AnalyticSolB.hpp>
#include <Underworld/Function/AnalyticSolC.hpp>
#include <Underworld/Function/AnalyticSolCx.hpp>
#include <Underworld/Function/AnalyticSolDA.hpp>
#include <Underworld/Function/AnalyticSolDB2d.hpp>
#include <Underworld/Function/AnalyticSolDB3d.hpp>
#include <Underworld/Function/AnalyticSolH.hpp>
#include <Underworld/Function/AnalyticSolHA.hpp>
#include <Underworld/Function/AnalyticSolKx.hpp>
#include <Underworld/Function/AnalyticSolKz.hpp>
#include <Underworld/Function/AnalyticSolM.hpp>
#include <Underworld/Function/AnalyticSolNL.hpp>

#include <Underworld/Function/Tensor.hpp>
#include <Underworld/Function/IOIterators.hpp>
#include <Underworld/Function/Query.hpp>
#include <Underworld/Function/Shape.hpp>
#include <Underworld/Function/Relational.hpp>
#include <Underworld/Function/Conditional.hpp>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
}


%}

%include "Function/FunctionIO.hpp"
%include "Function/Function.hpp"
%include "Function/SafeMaths.hpp"
%include "Function/CustomException.hpp"
%include "Function/MinMax.hpp"
%include "Function/Unary.hpp"
%include "Function/Binary.hpp"
%include "Function/Constant.hpp"
%include "Function/SwarmVariableFn.hpp"
%include "Function/ParticleFound.hpp"
%include "Function/FeVariableFn.hpp"
%include "Function/GradFeVariableFn.hpp"
%include "Function/Map.hpp"
%include "Function/Tensor.hpp"

%include "Function/Analytic.hpp"
%include "Function/AnalyticSolA.hpp"
%include "Function/AnalyticSolB.hpp"
%include "Function/AnalyticSolC.hpp"
%include "Function/AnalyticSolCx.hpp"
%include "Function/AnalyticSolDA.hpp"
%include "Function/AnalyticSolDB2d.hpp"
%include "Function/AnalyticSolDB3d.hpp"
%include "Function/AnalyticSolH.hpp"
%include "Function/AnalyticSolHA.hpp"
%include "Function/AnalyticSolKx.hpp"
%include "Function/AnalyticSolKz.hpp"
%include "Function/AnalyticSolM.hpp"
%include "Function/AnalyticSolNL.hpp"

%include "Function/IOIterators.hpp"
%include "Function/Query.hpp"
%include "Function/Shape.hpp"
%include "Function/Relational.hpp"
%include "Function/Conditional.hpp"


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
