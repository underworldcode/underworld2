/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_AnalyticSolM_hpp__
#define __Underworld_Function_AnalyticSolM_hpp__

#include "Analytic.hpp"

namespace Fn
{
    class SolM: public AnalyticCRTP<SolM>
    {
        public:
        SolM( double eta0, unsigned m, unsigned n, double r  )
        :AnalyticCRTP<SolM>(this,2), eta0(eta0), m(m), n(n), r(r)
        {
        }
        virtual ~SolM(){};
        double   eta0;
        unsigned m;
        unsigned n;
        double   r;
        double km=m*M_PI;
        double kn=n*M_PI;
        double kr=r*M_PI;
        
        void velocity(   const double* in, double* out )
        {
            out[0] = -sin(km*in[0])*kn*cos(kn*in[1]);
            out[1] =  cos(km*in[0])*km*sin(kn*in[1]);
        };
        void pressure(   const double* in, double* out )
        {
            double x=in[0],z=in[1];
            double p;
            double t2,t4,t5,t6,t7,t8,t10,t11,t15,t16;
            double t18,t23,t27,t28,t29,t32,t33,t34,t44;
            
            t2 = cos(kn * z);
            t4 = kr * kr;
            t5 = t4 * eta0;
            t6 = km * km;
            t7 = kr * x;
            t8 = cos(t7);
            t10 = km * x;
            t11 = cos(t10);
            t15 = kn * kn;
            t16 = t15 * t11;
            t18 = t6 * t11;
            t23 = t6 * t6;
            t27 = t18 * t15;
            t28 = t11 * t23;
            t29 = eta0 * t8;
            t32 = sin(t7);
            t33 = t32 * kr;
            t34 = sin(t10);
            t44 = 0.2e1 * t5 * t6 * t8 * t11 + t16 * t5 + t18 * t5 - t6 * t15 * t11 * eta0 - t23 * eta0 * t11 + t16 * t4 - t27 - t28 - t29 * t27 - t29 * t28 + t33 * eta0 * t34 * t6 * km + t18 * t4 - t33 * eta0 * t34 * t15 * km;
            p = t2 * kn * t44 / km / (-t4 + t6);
            out[0]=p;
            
        };
        void stress(     const double* in, double* out )
        {
            out[0] = -2.*(1.+cos(kr*in[0]))*eta0*cos(km*in[0])*km*cos(kn*in[1])*kn; //txx
            out[1] =  2.*(1.+cos(kr*in[0]))*eta0*cos(km*in[0])*km*cos(kn*in[1])*kn; //tzz
            out[2] =  2.*(1.+cos(kr*in[0]))*eta0*(1./2*sin(km*in[0])*sin(kn*in[1])*kn*kn-1./2*sin(km*in[0])*km*km*sin(kn*in[1])); //txz
        };
        void strainrate( const double* in, double* out )
        {
            out[0] = -cos(km*in[0])*km*cos(kn*in[1])*kn; //exx
            out[1] =  cos(km*in[0])*km*cos(kn*in[1])*kn; //ezz
            out[2] = 1./2*sin(km*in[0])*sin(kn*in[1])*kn*kn-1./2*sin(km*in[0])*km*km*sin(kn*in[1]); //exz
        };
        void viscosity(  const double* in, double* out )
        {
            out[0] = (1.+cos(kr*in[0]))*eta0+1.;
        };
        void bodyforce(  const double* in, double* out )
        {
            double x,z;
            out[0] = 0.;
            
            x=in[0];
            z=in[1];
            
            out[1] = -sin(kn*z)*eta0*kr*(km-kn)*(km+kn)*(km*km-kr*kr+kn*kn)*sin(km*x)*sin(kr*x)/(-kr+km)/(kr+km)  +
            (  km*eta0*(km*km*km*km-3*kn*kn*kr*kr+2*km*km*kn*kn-kr*kr*km*km+kn*kn*kn*kn)*cos(kr*x)/(-kr+km)/(kr+km) +
             (km*km+kn*kn)*(km*km+kn*kn)*(1+eta0)/km )*cos(km*x)*sin(kn*z);
        };
    };

}

#endif
