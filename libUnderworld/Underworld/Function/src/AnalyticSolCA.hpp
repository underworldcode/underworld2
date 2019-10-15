/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolCA_hpp__
#define __Underworld_Function_AnalyticSolCA_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solCA/solCA.h"
}

namespace Fn
{
    class SolCA: public AnalyticCRTP<SolCA>
    {
        public:
        SolCA( double sigma, double viscosity, double xc )
        :AnalyticCRTP<SolCA>(this,2), sigma(sigma), viscosity(viscosity), xc(xc)
        {
            if( xc < 0.0 || xc > 1.0 )
            throw std::invalid_argument( "Valid range of xc values is [0,1]. Provided value was " + std::to_string(xc));
        }
        virtual ~SolCA(){};
        double   sigma;
        double   viscosity;
        double   xc;
        void velocity(   const double* in, double* out ) { _Velic_solCA( in, sigma, viscosity, xc,  out, NULL, NULL, NULL ); };
        void pressure(   const double* in, double* out ) { _Velic_solCA( in, sigma, viscosity, xc, NULL,  out, NULL, NULL ); };
        void stress(     const double* in, double* out ) { _Velic_solCA( in, sigma, viscosity, xc, NULL, NULL,  out, NULL ); };
        void strainrate( const double* in, double* out ) { _Velic_solCA( in, sigma, viscosity, xc, NULL, NULL, NULL,  out ); };
        void viscosity(  const double* in, double* out ) { *out = viscosity; };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.
            out[1] = ( *in < xc ) ? sigma : 0.;
        };
    };
}

#endif
