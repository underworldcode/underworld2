/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolH_hpp__
#define __Underworld_Function_AnalyticSolH_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solH/solH.h"
}

namespace Fn
{
    class SolH: public AnalyticCRTP<SolH>
    {
        public:
        SolH( double sigma_0, double x_c, double y_c, double eta_0, int nmodes )
            :AnalyticCRTP<SolH>(this,3), sigma_0(sigma_0), x_c(x_c), y_c(y_c), eta_0(eta_0), nmodes(nmodes)
        {
            if( x_c < 0.0 || x_c > 1.0 || y_c < 0.0 || y_c > 1.0 )
                throw std::invalid_argument( "Density step is outside [0,1]. Check input parameters.");
        };
        virtual ~SolH(){};
        double   sigma_0;
        double   x_c;
        double   y_c;
        double   eta_0;
        int      nmodes;
        void velocity(   const double* in, double* out ) { _Velic_solH( in, sigma_0, eta_0, x_c, y_c,  out, NULL, NULL, NULL, nmodes ); };
        void pressure(   const double* in, double* out ) { _Velic_solH( in, sigma_0, eta_0, x_c, y_c, NULL,  out, NULL, NULL, nmodes ); };
        void stress(     const double* in, double* out ) { _Velic_solH( in, sigma_0, eta_0, x_c, y_c, NULL, NULL,  out, NULL, nmodes ); };
        void strainrate( const double* in, double* out ) { _Velic_solH( in, sigma_0, eta_0, x_c, y_c, NULL, NULL, NULL,  out, nmodes ); };
        void viscosity(  const double* in, double* out ) { *out = eta_0; };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.;
            out[1] = 0.;
            if( (in[0] <= x_c) && (in[1] <= y_c ) )
                out[2] = -sigma_0;
            else
                out[2] = 0.;
        };
    };
}

#endif
