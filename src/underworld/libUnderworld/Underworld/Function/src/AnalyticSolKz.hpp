/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolKz_hpp__
#define __Underworld_Function_AnalyticSolKz_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solKz/solKz.h"
}

namespace Fn
{
    class SolKz: public AnalyticCRTP<SolKz>
    {
        public:
        SolKz( double sigma, unsigned nx, double nz, double B )
        :AnalyticCRTP<SolKz>(this,2), sigma(sigma), nx(nx), nz(nz), B(B)
        {
        }
        virtual ~SolKz(){};
        double   sigma;
        unsigned nx;
        double   nz;
        double   B;
        void velocity(   const double* in, double* out ) { _Velic_solKz( in, sigma, M_PI*nz, nx, B,  out, NULL, NULL, NULL ); };
        void pressure(   const double* in, double* out ) { _Velic_solKz( in, sigma, M_PI*nz, nx, B, NULL,  out, NULL, NULL ); };
        void stress(     const double* in, double* out ) { _Velic_solKz( in, sigma, M_PI*nz, nx, B, NULL, NULL,  out, NULL ); };
        void strainrate( const double* in, double* out ) { _Velic_solKz( in, sigma, M_PI*nz, nx, B, NULL, NULL, NULL,  out ); };
        void viscosity(  const double* in, double* out )
        {
            out[0] = exp( 2.0 * B * in[ 1 ] );
        };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.;
            out[1] = sigma*cos( M_PI*nx*in[0] ) * sin( M_PI*nz*in[1] );
        };
    };

}

#endif
