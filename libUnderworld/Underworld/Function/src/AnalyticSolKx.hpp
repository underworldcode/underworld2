/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolKx_hpp__
#define __Underworld_Function_AnalyticSolKx_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solKx/solKx.h"
}

namespace Fn
{
    class SolKx: public AnalyticCRTP<SolKx>
    {
        public:
        SolKx( double sigma, double nx, double nz, double B )
        :AnalyticCRTP<SolKx>(this,2), sigma(sigma), nx(nx), nz(nz), B(B)
        {
        }
        virtual ~SolKx(){};
        double   sigma;
        double   nx;
        unsigned nz;
        double   B;
        void velocity(   const double* in, double* out ) { _Velic_solKx( in, sigma, nx, nz, B,  out, NULL, NULL, NULL, NULL ); };
        void pressure(   const double* in, double* out ) { _Velic_solKx( in, sigma, nx, nz, B, NULL,  out, NULL, NULL, NULL ); };
        void stress(     const double* in, double* out ) { _Velic_solKx( in, sigma, nx, nz, B, NULL, NULL,  out, NULL, NULL ); };
        void strainrate( const double* in, double* out ) { _Velic_solKx( in, sigma, nx, nz, B, NULL, NULL, NULL,  out, NULL ); };
        void viscosity(  const double* in, double* out ) { _Velic_solKx( in, sigma, nx, nz, B, NULL, NULL, NULL, NULL,  out ); };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.;
            out[1] = sigma*cos( M_PI*nx*in[0] ) * sin( M_PI*nz*in[1] );
        };
    };
}

#endif
