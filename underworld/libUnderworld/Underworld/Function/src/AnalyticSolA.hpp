/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolA_hpp__
#define __Underworld_Function_AnalyticSolA_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solA/solA.h"
}

namespace Fn
{
    class SolA: public AnalyticCRTP<SolA>
    {
        public:

        double   sigma;
        double   Z;
        int      n;
        double   m;

        SolA( double sigma, double Z, unsigned n, double m )
            :AnalyticCRTP<SolA>(this,2), sigma(sigma), Z(Z), n(n), m(m) {}
        virtual ~SolA(){};


        void velocity(   const double* in, double* out ) { _Velic_solA( in, sigma, Z, n, M_PI*m,  out, NULL, NULL, NULL ); };
        void pressure(   const double* in, double* out ) { _Velic_solA( in, sigma, Z, n, M_PI*m, NULL,  out, NULL, NULL ); };
        void stress(     const double* in, double* out ) { _Velic_solA( in, sigma, Z, n, M_PI*m, NULL, NULL,  out, NULL ); };
        void strainrate( const double* in, double* out ) { _Velic_solA( in, sigma, Z, n, M_PI*m, NULL, NULL, NULL,  out ); };
        void viscosity(  const double* in, double* out )
        {
            *out = Z;
        };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.;
            out[1] = sigma*cos( M_PI*(double)n*in[0] ) * sin( M_PI*m*in[1] );
        };
    };
}

#endif
