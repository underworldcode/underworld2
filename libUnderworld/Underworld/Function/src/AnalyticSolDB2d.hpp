/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

/*
 SolDB2d  from:
 
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


#ifndef __Underworld_Function_AnalyticSolDB2d_hpp__
#define __Underworld_Function_AnalyticSolDB2d_hpp__

#include "Analytic.hpp"

namespace Fn
{
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
    };
}

#endif
