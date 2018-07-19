/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolCx_hpp__
#define __Underworld_Function_AnalyticSolCx_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solCx/solCx.h"
}

namespace Fn
{
    class SolCx: public AnalyticCRTP<SolCx>
    {
        public:
        SolCx( double viscosityA, double viscosityB, double xc, unsigned nz )
        :AnalyticCRTP<SolCx>(this,2), viscosityA(viscosityA), viscosityB(viscosityB), xc(xc), nz(nz)
        {
            if( xc < 0.0 || xc > 1.0 )
            throw std::invalid_argument( _pyfnerrorheader+"Valid range of xc values is [0,1]. Provided value was " + std::to_string(xc));
        }
        virtual ~SolCx(){};
        double   viscosityA;
        double   viscosityB;
        double   xc;
        unsigned nz;
        void velocity(   const double* in, double* out ) { _Velic_solCx( in, viscosityA, viscosityB, xc, nz,  out, NULL, NULL, NULL ); };
        void pressure(   const double* in, double* out ) { _Velic_solCx( in, viscosityA, viscosityB, xc, nz, NULL,  out, NULL, NULL ); };
        void stress(     const double* in, double* out ) { _Velic_solCx( in, viscosityA, viscosityB, xc, nz, NULL, NULL,  out, NULL ); };
        void strainrate( const double* in, double* out ) { _Velic_solCx( in, viscosityA, viscosityB, xc, nz, NULL, NULL, NULL,  out ); };
        void viscosity(  const double* in, double* out )
        {
            *out = ( *in < xc ) ? viscosityA : viscosityB;
        };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.;
            out[1] = cos( M_PI*in[0] ) * sin( M_PI*nz*in[1] );
        };
    };
}

#endif
