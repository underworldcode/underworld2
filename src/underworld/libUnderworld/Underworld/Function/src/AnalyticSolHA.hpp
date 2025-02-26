/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolHA_hpp__
#define __Underworld_Function_AnalyticSolHA_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solHA/solHA.h"
}

namespace Fn
{
    class SolHA: public AnalyticCRTP<SolHA>
    {
        public:
        SolHA( double sigma_0, double x_c, double x_w, double y_c, double y_w, double eta_0, int nmodes )
            :AnalyticCRTP<SolHA>(this,3), sigma_0(sigma_0), x_c(x_c), x_w(x_w), y_c(y_c), y_w(y_w), eta_0(eta_0), nmodes(nmodes)
        {
            minpx = x_c - 0.5*x_w;
            maxpx = x_c + 0.5*x_w;
            minpy = y_c - 0.5*y_w;
            maxpy = y_c + 0.5*y_w;
            if( minpx < 0.0 || maxpx > 1.0 || minpy < 0.0 || maxpy > 1.0 )
                throw std::invalid_argument( "Density perturbation is outside [0,1]. Check input parameters.");
        };
        virtual ~SolHA(){};
        double   sigma_0;
        double   x_c;
        double   x_w;
        double   y_c;
        double   y_w;
        double   eta_0;
        int      nmodes;
        double   minpx,maxpx,minpy,maxpy;
        void velocity(   const double* in, double* out ) { _Velic_solHA( in, sigma_0, eta_0, x_w, y_w, x_c, y_c,  out, NULL, NULL, NULL, nmodes ); };
        void pressure(   const double* in, double* out ) { _Velic_solHA( in, sigma_0, eta_0, x_w, y_w, x_c, y_c, NULL,  out, NULL, NULL, nmodes ); };
        void stress(     const double* in, double* out ) { _Velic_solHA( in, sigma_0, eta_0, x_w, y_w, x_c, y_c, NULL, NULL,  out, NULL, nmodes ); };
        void strainrate( const double* in, double* out ) { _Velic_solHA( in, sigma_0, eta_0, x_w, y_w, x_c, y_c, NULL, NULL, NULL,  out, nmodes ); };
        void viscosity(  const double* in, double* out ) { *out = eta_0; };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.;
            out[1] = 0.;
            if( (in[0] >= minpx) && (in[0] <= maxpx) && (in[1] >= minpy) && (in[1] <= maxpy) )
                out[2] = -sigma_0;
            else
                out[2] = 0.;
        };
    };
}

#endif
