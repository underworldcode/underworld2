/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Analytic_h__
#define __Underworld_Function_Analytic_h__

#include <stddef.h>
#include <string>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include "Underworld/SysTest/AnalyticPlugins/Velic_solCx/solCx.h"
#include "Underworld/SysTest/AnalyticPlugins/Velic_solKx/solKx.h"
#include "Underworld/SysTest/AnalyticPlugins/Velic_solKz/solKz.h"
}

#include "Function.hpp"
/* If a solution is added here you need to add a template line
    like
   %template(SolCxCRTP) Fn::AnalyticCRTP<Fn::SolCx>;
    to
   libUnderworld/libUnderworldPy/Function.i
    and a python interface to
   underworld/function/analytic.py
*/

namespace Fn {

    template <typename T, int outsize, FunctionIO::IOType outtype>
    class _Analytic: public Function
    {
        public:
            typedef void (T::*membFuncType)(const double*, double*);
            // second constructor argument is a pointer to a member function (*F) of T
            _Analytic(T *sol, membFuncType memberFunc): _sol(sol), memberFunc(memberFunc){};
            virtual func getFunction( IOsptr sample_input )
            {
                // setup output
                std::shared_ptr<IO_double> _output = std::make_shared<IO_double>(outsize, outtype);

                std::shared_ptr<const IO_double> iodouble = std::dynamic_pointer_cast<const IO_double>(sample_input);
                T* solGuy = _sol;
                membFuncType memberFuncGuy = memberFunc;
                if ( iodouble && ((iodouble->size()==2)||(iodouble->size()==3)) ){
                    return [_output,solGuy, memberFuncGuy](IOsptr input)->IOsptr {
                        std::shared_ptr<const IO_double> iodouble = debug_dynamic_cast<const IO_double>(input);            

                        (solGuy->*memberFuncGuy)(iodouble->data(),_output->data());

                        return debug_dynamic_cast<const FunctionIO>(_output);
                    };
                }
                // if we get here, something aint right
                throw std::invalid_argument("Function does not appear to be compatible with provided input type.");
            };
        private:
            T* _sol;
            membFuncType memberFunc;
    };

    template <class T>
    class AnalyticCRTP
    {
        public:
            AnalyticCRTP(T* selfGuy, unsigned dim)
            {
                if (dim == 2)
                {
                    velocityFn   = new _Analytic<T, 2, FunctionIO::Vector         >(selfGuy,&T::velocity);
                    pressureFn   = new _Analytic<T, 1, FunctionIO::Scalar         >(selfGuy,&T::pressure);
                    stressFn     = new _Analytic<T, 3, FunctionIO::SymmetricTensor>(selfGuy,&T::stress);
                    strainRateFn = new _Analytic<T, 3, FunctionIO::SymmetricTensor>(selfGuy,&T::strainrate);
                    viscosityFn  = new _Analytic<T, 1, FunctionIO::Scalar         >(selfGuy,&T::viscosity);
                    bodyForceFn  = new _Analytic<T, 2, FunctionIO::Vector         >(selfGuy,&T::bodyforce);
                }
                else if (dim == 3)
                {
                    velocityFn   = new _Analytic<T, 3, FunctionIO::Vector         >(selfGuy,&T::velocity);
                    pressureFn   = new _Analytic<T, 1, FunctionIO::Scalar         >(selfGuy,&T::pressure);
                    stressFn     = new _Analytic<T, 6, FunctionIO::SymmetricTensor>(selfGuy,&T::stress);
                    strainRateFn = new _Analytic<T, 6, FunctionIO::SymmetricTensor>(selfGuy,&T::strainrate);
                    viscosityFn  = new _Analytic<T, 1, FunctionIO::Scalar         >(selfGuy,&T::viscosity);
                    bodyForceFn  = new _Analytic<T, 3, FunctionIO::Vector         >(selfGuy,&T::bodyforce);
                } else
                    throw std::invalid_argument("Solution appears to have invalid dimensionality.");
            };
            Function *velocityFn;
            Function *pressureFn;
            Function *stressFn;
            Function *strainRateFn;
            Function *viscosityFn;
            Function *bodyForceFn;
            ~AnalyticCRTP()
            {
                delete velocityFn;
                delete pressureFn;
                delete stressFn;
                delete strainRateFn;
                delete viscosityFn;
                delete bodyForceFn;
            }
    };
    
    // we use the CRTP pattern here so that each sol only is required to provide the minimal interface below.
    class SolCx: public AnalyticCRTP<SolCx>
    {
        public:
            SolCx( double viscosityA, double viscosityB, double xc, unsigned nz )
                :AnalyticCRTP<SolCx>(this,2), viscosityA(viscosityA), viscosityB(viscosityB), xc(xc), nz(nz)
            {
                if( xc < 0.0 || xc > 1.0 )
                    throw std::invalid_argument( "Valid range of xc values is [0,1]. Provided value was " + std::to_string(xc));
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

    class SolKx: public AnalyticCRTP<SolKx>
    {
        public:
            SolKx( double sigma, double nx, unsigned nz, double B )
                :AnalyticCRTP<SolKx>(this,2), sigma(sigma), nx(nx), nz(nz), B(B)
            {
            }
            virtual ~SolKx(){};
            double   sigma;
            double   nx;
            unsigned nz;
            double   B;
            void velocity(   const double* in, double* out ) { _Velic_solKx( in, sigma, nz, nx, B,  out, NULL, NULL, NULL, NULL ); };
            void pressure(   const double* in, double* out ) { _Velic_solKx( in, sigma, nz, nx, B, NULL,  out, NULL, NULL, NULL ); };
            void stress(     const double* in, double* out ) { _Velic_solKx( in, sigma, nz, nx, B, NULL, NULL,  out, NULL, NULL ); };
            void strainrate( const double* in, double* out ) { _Velic_solKx( in, sigma, nz, nx, B, NULL, NULL, NULL,  out, NULL ); };
            void viscosity(  const double* in, double* out ) { _Velic_solKx( in, sigma, nz, nx, B, NULL, NULL, NULL, NULL,  out ); };
            void bodyforce(  const double* in, double* out )
            {
                out[0] = 0.;
                out[1] = sigma*cos( M_PI*nx*in[0] ) * sin( M_PI*nz*in[1] );
            };
    };

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
    };//class

  /*
    SolDB2d and solDB3d from:

    @ARTICLE{2004IJNMF..46..183D,
   author = {{Dohrmann}, C.~R. and {Bochev}, P.~B.},
    title = "{A stabilized finite element method for the Stokes problem based on polynomial pressure projections}",
  journal = {International Journal for Numerical Methods in Fluids},
 keywords = {Stokes equations, stabilized mixed methods, equal-order interpolation, inf-sup condition},
     year = 2004,
    month = sep,
   volume = 46,
    pages = {183-201},
      doi = {10.1002/fld.752},
   adsurl = {http://adsabs.harvard.edu/abs/2004IJNMF..46..183D},
  adsnote = {Provided by the SAO/NASA Astrophysics Data System}
  }
  */
  class SolDB2d: public AnalyticCRTP<SolDB2d>
  {
  public:
    SolDB2d()
      :AnalyticCRTP<SolDB2d>(this,2)
      {
      }
    virtual ~SolDB2d(){};
      
    void bodyforce(  const double* in, double* out )
      {
        double x=in[0],z=in[1];
        double fx;
        double fz;

        fx = -z - 0.1e1 + 0.3e1 * x * x * z * z;
        out[0]=fx;

        fz = -0.1e1 + 0.3e1 * x + 0.2e1 * pow(x, 0.3e1) * z;
        out[1]=fz;

      };
    void viscosity(  const double* in, double* out )
      {
        out[0]=1.0;
      };
    void pressure(const double* in, double* out )
      {
        double x=in[0],z=in[1];
        double p,t2,t4;

        t2 = x * x;
        t4 = z * z;
        p = x * z + x + z + t2 * x * t4 - 0.4e1 / 0.3e1;
        out[0]=p;

      };
    void strainrate( const double* in, double* out )
      {
        double x=in[0],z=in[1];
        double exx;
        double ezz;
        double exz;

        exx = 0.1e1 + 0.2e1 * x - 0.2e1 * z + 0.3e1 * x * x - 0.3e1 * z * z + 0.2e1 * x * z;
        out[0]=exx;

        ezz = -0.1e1 - 0.2e1 * x + 0.2e1 * z - 0.3e1 * x * x + 0.3e1 * z * z - 0.2e1 * x * z;
        out[1]=ezz;

        exz = -x - 0.6e1 * x * z + x * x / 0.2e1 - z - z * z / 0.2e1;
        out[2]=exz;

      };
    void stress(     const double* in, double* out )
      {
        double x=in[0],z=in[1];
        double txx;
        double tzz;
        double txz;

        txx = 0.2e1 + 0.4e1 * x - 0.4e1 * z + 0.6e1 * x * x - 0.6e1 * z * z + 0.4e1 * x * z;
        out[0]=txx;

        tzz = -0.2e1 - 0.4e1 * x + 0.4e1 * z - 0.6e1 * x * x + 0.6e1 * z * z - 0.4e1 * x * z;
        out[1]=tzz;

        txz = -0.2e1 * x - 0.12e2 * x * z + x * x - 0.2e1 * z - z * z;
        out[2]=txz;

      };
    void velocity(   const double* in, double* out )
      {
        double x=in[0],z=in[1];
        double vx;
        double vz;

        vx = x + x * x - 0.2e1 * x * z + pow(x, 0.3e1) - 0.3e1 * x * z * z + x * x * z;
        out[0]=vx;

        vz = -z - 0.2e1 * x * z + z * z - 0.3e1 * x * x * z + pow(z, 0.3e1) - x * z * z;
        out[1]=vz;

      };
  };//class

  /*
    SolDB3d initially from Dohrmann/Bochev with exponential viscosity added by Burstedde.

    Burstedde, C., Stadler, G., Alisic, L., Wilcox, L., Tan, E., Gurnis, M., and Ghattas, O.: Large- 
    scale adaptive mantle convection simulation, GJI, 192, 889–906, 2013.

    @article{Burstedde01032013,
    author = {Burstedde, Carsten and Stadler, Georg and Alisic, Laura and Wilcox, Lucas C. and Tan, Eh and Gurnis, Michael and Ghattas, Omar}, 
    title = {Large-scale adaptive mantle convection simulation},
    volume = {192}, 
    number = {3}, 
    pages = {889-906}, 
    year = {2013}, 
    doi = {10.1093/gji/ggs070},
    URL = {http://gji.oxfordjournals.org/content/192/3/889.abstract}, 
    eprint = {http://gji.oxfordjournals.org/content/192/3/889.full.pdf+html}, 
    journal = {Geophysical Journal International} 
    }

   */
  class SolDB3d: public AnalyticCRTP<SolDB3d>
  {
  public:
    SolDB3d( double Beta )
    :AnalyticCRTP<SolDB3d>(this,3), Beta(Beta)
      {
      }
    virtual ~SolDB3d(){};
    double Beta;

    void bodyforce(  const double* in, double* out )
      {
        double x=in[0],y=in[1],z=in[2];
        double fx;
        double fy;
        double fz;

        fx = y * z + 0.2e1 * x * pow(y, 0.3e1) * z + 0.2e1 * Beta * (0.1e1 - 0.2e1 * x) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (0.1e1 + 0.2e1 * x + y + 0.3e1 * x * x * y) - 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (0.2e1 + 0.6e1 * x * y) + 0.2e1 * Beta * (0.1e1 - 0.2e1 * y) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (x / 0.2e1 + pow(x, 0.3e1) / 0.2e1 + y / 0.2e1 + x * y * y) - 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (0.1e1 / 0.2e1 + 0.2e1 * x * y) + 0.2e1 * Beta * (0.1e1 - 0.2e1 * z) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.3e1 / 0.2e1 * z - 0.5e1 * x * y * z) - 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.3e1 / 0.2e1 - 0.5e1 * x * y);
        out[0]=fx;

        fy = 0.2e1 * Beta * (0.1e1 - 0.2e1 * x) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (x / 0.2e1 + pow(x, 0.3e1) / 0.2e1 + y / 0.2e1 + x * y * y) - 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (0.1e1 / 0.2e1 + 0.3e1 / 0.2e1 * x * x + y * y) + x * z + 0.3e1 * x * x * y * y * z + 0.2e1 * Beta * (0.1e1 - 0.2e1 * y) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (0.1e1 + x + 0.2e1 * y + 0.2e1 * x * x * y) - 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (0.2e1 + 0.2e1 * x * x) + 0.2e1 * Beta * (0.1e1 - 0.2e1 * z) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.3e1 / 0.2e1 * z - 0.5e1 / 0.2e1 * x * x * z) - 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.3e1 / 0.2e1 - 0.5e1 / 0.2e1 * x * x);
        out[1]=fy;

        fz = 0.2e1 * Beta * (0.1e1 - 0.2e1 * x) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.3e1 / 0.2e1 * z - 0.5e1 * x * y * z) + 0.10e2 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * y * z + 0.2e1 * Beta * (0.1e1 - 0.2e1 * y) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.3e1 / 0.2e1 * z - 0.5e1 / 0.2e1 * x * x * z) + x * y + x * x * pow(y, 0.3e1) + 0.2e1 * Beta * (0.1e1 - 0.2e1 * z) * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.2e1 - 0.3e1 * x - 0.3e1 * y - 0.5e1 * x * x * y);
        out[2]=fz;

      };
    void viscosity(  const double* in, double* out )
      {
        double x=in[0],y=in[1],z=in[2];
        double eta;

        eta = exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z)));
        out[0]=eta;

      };
    void pressure(const double* in, double* out )
      {
        double x=in[0],y=in[1],z=in[2];
        double p,t3,t4;

        t3 = x * x;
        t4 = y * y;
        p = x * y * z + t3 * t4 * y * z - 0.5e1 / 0.32e2;
        out[0]=p;

      };
    void strainrate( const double* in, double* out )
      {
        double x=in[0],y=in[1],z=in[2];
        double exx;
        double eyy;
        double ezz;
        double exy;
        double exz;
        double eyz;

        exx = 0.1e1 + 0.2e1 * x + y + 0.3e1 * x * x * y;
        out[0]=exx;

        eyy = 0.1e1 + x + 0.2e1 * y + 0.2e1 * x * x * y;
        out[1]=eyy;

        ezz = -0.2e1 - 0.3e1 * x - 0.3e1 * y - 0.5e1 * x * x * y;
        out[2]=ezz;

        exy = x / 0.2e1 + pow(x, 0.3e1) / 0.2e1 + y / 0.2e1 + x * y * y;
        out[3]=exy;

        exz = -0.3e1 / 0.2e1 * z - 0.5e1 * x * y * z;
        out[4]=exz;

        eyz = -0.3e1 / 0.2e1 * z - 0.5e1 / 0.2e1 * x * x * z;
        out[5]=eyz;

      };
    void stress(     const double* in, double* out )
      {
        double x=in[0],y=in[1],z=in[2];
        double txx;
        double tyy;
        double tzz;
        double txy;
        double txz;
        double tyz;

        txx = 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (0.1e1 + 0.2e1 * x + y + 0.3e1 * x * x * y);
        out[0]=txx;

        tyy = 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (0.1e1 + x + 0.2e1 * y + 0.2e1 * x * x * y);
        out[1]=tyy;

        tzz = 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.2e1 - 0.3e1 * x - 0.3e1 * y - 0.5e1 * x * x * y);
        out[2]=tzz;

        txy = 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (x / 0.2e1 + pow(x, 0.3e1) / 0.2e1 + y / 0.2e1 + x * y * y);
        out[3]=txy;

        txz = 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.3e1 / 0.2e1 * z - 0.5e1 * x * y * z);
        out[4]=txz;

        tyz = 0.2e1 * exp(0.1e1 - Beta * (x * (0.1e1 - x) + y * (0.1e1 - y) + z * (0.1e1 - z))) * (-0.3e1 / 0.2e1 * z - 0.5e1 / 0.2e1 * x * x * z);
        out[5]=tyz;

      };
    void velocity(   const double* in, double* out )
      {
        double x=in[0],y=in[1],z=in[2];
        double vx;
        double vy;
        double vz;

        vx = x + x * x + x * y + pow(x, 0.3e1) * y;
        out[0]=vx;

        vy = y + x * y + y * y + x * x * y * y;
        out[1]=vy;

        vz = -0.2e1 * z - 0.3e1 * x * z - 0.3e1 * y * z - 0.5e1 * x * x * y * z;
        out[2]=vz;

      };

  };//class SolDB3d
}

#endif // __Underworld_Function_Analytic_h__

