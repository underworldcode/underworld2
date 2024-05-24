/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolC_hpp__
#define __Underworld_Function_AnalyticSolC_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solC/solC.h"
}

namespace Fn
{
    class SolC: public AnalyticCRTP<SolC>
    {
        public:
        SolC( double sigma, double viscosity, double xc, int nmodes)
        :AnalyticCRTP<SolC>(this,2), sigma(sigma), visc(viscosity), xc(xc), nmodes(nmodes)
        {
            if( xc < 0.0 || xc > 1.0 )
            throw std::invalid_argument( "Valid range of xc values is [0,1]. Provided value was " + std::to_string(xc));
        }
        virtual ~SolC(){};
        double   sigma;
        double   visc;
        double   xc;
        int nmodes;
        void velocity(   const double* in, double* out ) { _Velic_solC( in, sigma, visc, xc,  out, NULL, NULL, NULL, nmodes ); };
        void pressure(   const double* in, double* out ) { _Velic_solC( in, sigma, visc, xc, NULL,  out, NULL, NULL, nmodes ); };
        void stress(     const double* in, double* out ) { _Velic_solC( in, sigma, visc, xc, NULL, NULL,  out, NULL, nmodes ); };
        void strainrate( const double* in, double* out ) { _Velic_solC( in, sigma, visc, xc, NULL, NULL, NULL,  out, nmodes ); };
        void viscosity(  const double* in, double* out ) { *out = visc; };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.;
            out[1] = ( *in < xc ) ? -sigma : 0.;
        };
    };
}

#endif
