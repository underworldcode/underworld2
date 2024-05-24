/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolNL_hpp__
#define __Underworld_Function_AnalyticSolNL_hpp__

#include "Analytic.hpp"

namespace Fn
{
    class SolNL: public AnalyticCRTP<SolNL>
    {
        public:
        SolNL( double eta0, unsigned n, double r  )
        :AnalyticCRTP<SolNL>(this,2), eta0(eta0), n(n), r(r)
        {
        }
        virtual ~SolNL(){};
        double   eta0;
        unsigned n;
        double   r;
        double   kn   =n*M_PI;
        double   alpha=1./r -1.;
        
        void velocity(   const double* in, double* out )
        {
            double x,z;
            x=in[0]; z=in[1];
            out[0] = -exp(x)*cos(kn*z)*kn;
            out[1] =  exp(x)*sin(kn*z);
        };
        void pressure(   const double* in, double* out )
        {
            double x=in[0],z=in[1];
            double t2,t5,t7,t8,t9,t10,t11,t12,t13,t14;
            double t18,t20,t24,t26,t28,t30,t32,t34,t35,t39;
            double t44;
            
            double p;
            
            t2 = cos(kn * z);
            t5 = exp(x);
            t7 = exp(0.2e1 * x);
            t8 = kn * kn;
            t9 = t8 * t8;
            t10 = t2 * t2;
            t11 = t9 * t10;
            t12 = t10 * t8;
            t13 = 0.2e1 * t12;
            t14 = 0.2e1 * t8;
            t18 = pow(-t7 * (-t9 - 0.1e1 + t10 + t11 - t13 - t14) / 0.4e1, alpha);
            t20 = t9 * t8;
            t24 = t9 * alpha;
            t26 = t11 * alpha;
            t28 = t8 * alpha;
            t30 = t12 * alpha;
            t32 = 0.2e1 * alpha;
            t34 = 0.2e1 * t10 * alpha;
            t35 = alpha * t20;
            t39 = 0.1e1 - t10 - t20 - t9 - 0.3e1 * t11 + 0.3e1 * t12 + t20 * t10 + t8 + 0.6e1 * t24 - 0.6e1 * t26 + 0.10e2 * t28 + 0.6e1 * t30 + t32 - t34 - 0.2e1 * t35 + 0.2e1 * t35 * t10;
            t44 = t34 - 0.2e1 * t24 + t11 - t14 - 0.1e1 - t9 + 0.2e1 * t26 - t13 - 0.4e1 * t30 - t32 - 0.4e1 * t28 + t10;
            p = 0.2e1 * eta0 * t2 * kn * t5 * t18 * t39 / t44;
            out[0]=p;
            
        };
        void stress(     const double* in, double* out )
        {
            double x,z;
            x=in[0]; z=in[1];
            //txx
            out[0] =
            -0.4e1 * pow(pow(exp(x) * sin(kn * z) * kn * kn / 0.2e1 + exp(x) * sin(kn * z) / 0.2e1, 0.2e1) + pow(exp(x), 0.2e1) * pow(cos(kn * z), 0.2e1) * kn * kn, alpha) * eta0 * exp(x) * cos(kn * z) * kn;
            //tzz
            out[1] = 0.4e1 * pow(pow(exp(x) * sin(kn * z) * kn * kn / 0.2e1 + exp(x) * sin(kn * z) / 0.2e1, 0.2e1) + pow(exp(x), 0.2e1) * pow(cos(kn * z), 0.2e1) * kn * kn, alpha) * eta0 * exp(x) * cos(kn * z) * kn;
            //txz
            out[2] = 0.4e1 * pow(pow(exp(x) * sin(kn * z) * kn * kn / 0.2e1 + exp(x) * sin(kn * z) / 0.2e1, 0.2e1) + pow(exp(x), 0.2e1) * pow(cos(kn * z), 0.2e1) * kn * kn, alpha) * eta0 * (exp(x) * sin(kn * z) * kn * kn / 0.2e1 + exp(x) * sin(kn * z) / 0.2e1);
        };
        void strainrate( const double* in, double* out )
        {
            double x,z;
            x=in[0]; z=in[1];
            //exx
            out[0] = -exp(x) * cos(kn * z) * kn;
            //ezz
            out[1] =  exp(x) * cos(kn * z) * kn;
            //exz
            out[2] = exp(x) * sin(kn * z) * kn * kn / 0.2e1 + exp(x) * sin(kn * z) / 0.2e1;
        };
        void viscosity(  const double* in, double* out )
        {
            double t1,t2,t3,t4,t5,t9,t10,t11,t12,t16;
            double x,z;
            x=in[0]; z=in[1];
            
            t1 = exp(x);
            t2 = kn * z;
            t3 = sin(t2);
            t4 = t1 * t3;
            t5 = kn * kn;
            t9 = pow(t4 * t5 / 0.2e1 + t4 / 0.2e1, 0.2e1);
            t10 = t1 * t1;
            t11 = cos(t2);
            t12 = t11 * t11;
            t16 = pow(t9 + t10 * t12 * t5, alpha);
            
            out[0] = 0.2e1 * t16 * eta0;
            
        };
        void bodyforce(  const double* in, double* out )
        {
            double x,z;
            double t1,t2,t4,t7,t8,t9,t10,t11,t12,t13;
            double t14,t19,t20,t21,t22,t23,t26,t27,t28,t31;
            double t32,t34,t35,t37,t40,t41,t45,t48,t49,t52;
            double t58,t59,t63,t66,t68,t69,t71,t80,t82,t90;
            double t92,t94,t103,t106,t111,t115,t117,t121,t135,t149;
            double t159;
            
            out[0] = 0.;
            
            x=in[0];
            z=in[1];
            
            t1 = kn * z;
            t2 = sin(t1);
            t4 = exp(x);
            t7 = exp(0.2e1 * x);
            t8 = kn * kn;
            t9 = t8 * t8;
            t10 = cos(t1);
            t11 = t10 * t10;
            t12 = t9 * t11;
            t13 = 0.2e1 * t8;
            t14 = t11 * t8;
            t19 = pow(-t7 * (-t9 + t12 - t13 - 0.2e1 * t14 - 0.1e1 + t11) / 0.4e1, alpha);
            t20 = alpha * alpha;
            t21 = t9 * t8;
            t22 = t20 * t21;
            t23 = t11 * t11;
            t26 = t9 * t9;
            t27 = t26 * t8;
            t28 = alpha * t27;
            t31 = alpha * t8;
            t32 = t31 * t23;
            t34 = alpha * t9;
            t35 = t34 * t23;
            t37 = t20 * t27;
            t40 = alpha * t26;
            t41 = t40 * t23;
            t45 = t20 * t9;
            t48 = t26 * t9;
            t49 = t20 * t48;
            t52 = t20 * t26;
            t58 = 0.2e1 * t26 * t11;
            t59 = t8 * t23;
            t63 = 0.1e1 - 0.80e2 * t22 * t23 + 0.2e1 * t28 * t11 - 0.24e2 * t32 + 0.60e2 * t35 - 0.24e2 * t37 * t23 + 0.60e2 * t41
            + 0.20e2 * t37 * t11 + 0.60e2 * t45 * t23 - 0.4e1 * t49 * t11 + 0.60e2 * t52 * t23 - 0.16e2 * t52 * t11 + t58 - 0.6e1 * t59 + 0.4e1 * t20 * t23;
            t66 = t21 * t23;
            t68 = alpha * t21;
            t69 = t68 * t23;
            t71 = t20 * t8;
            t80 = alpha * t23;
            t82 = t11 * alpha;
            t90 = t26 * t23;
            t92 = -0.8e1 * t21 * t11 - 0.20e2 * t66 - 0.80e2 * t69 - 0.24e2 * t71 * t23 + 0.4e1 * t71 * t11 - 0.24e2 * t28 * t23 - t26
            + 0.2e1 * t27 + 0.20e2 * t52 + 0.4e1 * t80 - 0.8e1 * t82 + 0.14e2 * t31 + 0.10e2 * t34 - 0.2e1 * t48 * t11 - 0.8e1 * t20 * t11 + 0.15e2 * t90;
            t94 = t9 * t23;
            t103 = t12 * alpha;
            t106 = 0.2e1 * t11;
            t111 = 0.15e2 * t94 + t48 * t23 + 0.4e1 * t27 * t11 - 0.6e1 * t27 * t23 + 0.10e2 * t14 * alpha + 0.2e1 * t103 + t23
            + t13 - 0.16e2 * t40 - t106 + 0.2e1 * t12 - t9 + 0.4e1 * t14 + 0.4e1 * alpha + 0.20e2 * t71;
            t115 = alpha * t48;
            t117 = 0.4e1 * t21;
            t121 = t40 * t11;
            t135 = -0.2e1 * t28 + 0.4e1 * t37 + 0.40e2 * t45 + 0.2e1 * t115 - t117 - 0.12e2 * t68 + 0.4e1 * t115 * t23
            + 0.12e2 * t121 - 0.6e1 * t115 * t11 - 0.24e2 * t22 * t11 + 0.4e1 * t49 * t23 + 0.28e2 * t45 * t11 - 0.12e2 * t68 * t11 + 0.4e1 * t20 + t48 + 0.40e2 * t22;
            t149 = 0.1e1 - 0.8e1 * t32 + 0.12e2 * t35 + 0.2e1 * t41 - t58 - 0.4e1 * t59 - 0.4e1 * t66 - 0.8e1 * t69 + t26 + 0.2e1 * t80 - 0.4e1 * t82 + 0.8e1 * t31 + 0.12e2 * t34;
            t159 = t90 + 0.6e1 * t94 + 0.8e1 * t103 + t23 + 0.4e1 * t8 + 0.2e1 * t40 - t106 + 0.4e1 * t12 + 0.6e1 * t9 + 0.2e1 * alpha + t117 + 0.8e1 * t68 - 0.4e1 * t121;
            
            out[1] = -0.2e1 * eta0 * t2 * t4 * t19 * (t63 + t92 + t111 + t135) / (t149 + t159);
            
        };
    };

}

#endif
