/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

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


#ifndef __Underworld_Function_AnalyticSolDB3d_hpp__
#define __Underworld_Function_AnalyticSolDB3d_hpp__

#include "Analytic.hpp"

namespace Fn
{
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
        
    };

}

#endif
