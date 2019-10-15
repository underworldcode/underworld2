/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolDA_hpp__
#define __Underworld_Function_AnalyticSolDA_hpp__

#include "Analytic.hpp"

extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solDA/solDA.h"
}

namespace Fn
{
    class SolDA: public AnalyticCRTP<SolDA>
    {
        public:
        SolDA( double sigma_0, double x_c, double x_w, double eta_A, double eta_B, double z_c, int nmodes )
            :AnalyticCRTP<SolDA>(this,2), sigma_0(sigma_0), x_c(x_c), x_w(x_w), eta_A(eta_A), eta_B(eta_B), z_c(z_c), nmodes(nmodes)
        {
            if( z_c < 0.0 || z_c > 1.0 )
                throw std::invalid_argument( "Valid range of z_c values is [0,1]. Provided value was " + std::to_string(z_c));
            minp = x_c - 0.5*x_w;
            maxp = x_c + 0.5*x_w;
            if( minp < 0.0 || maxp > 1.0 )
                throw std::invalid_argument( "Density perturbation is outside [0,1]. Check input parameters.");
        };
        virtual ~SolDA(){};
        double   sigma_0;
        double   x_c;
        double   x_w;
        double   eta_A;
        double   eta_B;
        double   z_c;
        int      nmodes;
        double   minp,maxp;
        void velocity(   const double* in, double* out ) { _Velic_solDA( in, sigma_0, eta_A, eta_B, z_c, x_w, x_c,  out, NULL, NULL, NULL, nmodes ); };
        void pressure(   const double* in, double* out ) { _Velic_solDA( in, sigma_0, eta_A, eta_B, z_c, x_w, x_c, NULL,  out, NULL, NULL, nmodes ); };
        void stress(     const double* in, double* out ) { _Velic_solDA( in, sigma_0, eta_A, eta_B, z_c, x_w, x_c, NULL, NULL,  out, NULL, nmodes ); };
        void strainrate( const double* in, double* out ) { _Velic_solDA( in, sigma_0, eta_A, eta_B, z_c, x_w, x_c, NULL, NULL, NULL,  out, nmodes ); };
        void viscosity(  const double* in, double* out )
        {
            *out = ( in[1] < z_c ) ? eta_A : eta_B;
        };
        void bodyforce(  const double* in, double* out )
        {
            out[0] = 0.;
            if( (in[0] >= minp) && (in[0] <= maxp) )
                out[1] = -sigma_0;
            else
                out[1] = 0.;
        };
    };
}

#endif
