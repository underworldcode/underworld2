/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "solA.h"

//int main() {
//   double sigma = 1.0;
//   double Z = 1.0;
//   double km = M_PI;
//   int    n = 1;
//   double pos[2], velocity[2], pressure, Tstress[3], strainRate[3],jp;
//   int    i, j;   
//
//   for (i=0;i<101;i++){
//      for(j=0;j<101;j++){
//         pos[0] = i/100.0;
//         pos[1] = j/100.0;
//
//         _Velic_solA(
//            pos,
//            sigma,
//            Z,
//            n,
//            km,
//            velocity,
//            &pressure,
//            Tstress,
//            strainRate );
//
//         printf(
//            "%.7g %.7g %.7g %.7g %.7g %.7g %.7g %.7g %.7g %.7g %.7g %.7g\n",
//            pos[0], pos[1],
//            velocity[0], velocity[1],
//            pressure,
//            jp, 
//            strainRate[0], strainRate[1], strainRate[2],
//            Tstress[0], Tstress[1], Tstress[2] );
//      }
//   }
//   return 0;
//}

void _Velic_solA(
   const double* pos,
   double  sigma,
   double  Z,
   int     n,
   double  km,
   double* velocity,
   double* pressure,
   double* Tstress,
   double* strainRate ) {
   
   double u1,u2,u3,u4,pp,txx;
   double _C1,_C2,_C3,_C4;
   double sum1,sum2,sum3,sum4,sum5,sum6,sum7,mag,x,z;
   double ss,ss_z,ss_zz,ss_zzz,e_zz,e_xx,e_xz;
   double t1,t3,t4,t5,t6,t7,t9,t10,t14,t15;
   double t16,t17,t18,t19,t20,t21,t23,t24,t25,t26,t27,t29;
   double kn;

   kn = (double)n*M_PI;
   x = pos[0];
   z = pos[1];

   sum1=0.0;
   sum2=0.0;
   sum3=0.0;
   sum4=0.0;
   sum5=0.0;
   sum6=0.0;
   sum7=0.0;

   t1 = sin(km);
   t3 = kn * kn;
   t4 = exp(-kn);
   t5 = t4 * t4;
   t10 = km * km;
   t17 = pow(t10 + t3, 0.2e1);
   t21 = pow(t4 - 0.1e1, 0.2e1);
   t24 = pow(t4 + 0.1e1, 0.2e1);
   _C1 = t1 * sigma * (t3 * t5 + t3 - 0.2e1 * kn * t5 + 0.2e1 * kn + t10 * t5 + t10) * t4 / Z / t17 / t21 / t24 / 0.2e1;

   t1 = sin(km);
   t3 = kn * kn;
   t4 = exp(-kn);
   t5 = t4 * t4;
   t10 = km * km;
   t16 = pow(t10 + t3, 0.2e1);
   t20 = pow(t4 - 0.1e1, 0.2e1);
   t23 = pow(t4 + 0.1e1, 0.2e1);
   _C2 = -t1 * sigma * (t3 * t5 + t3 - 0.2e1 * kn * t5 + 0.2e1 * kn + t10 * t5 + t10) / Z / t16 / t20 / t23 / 0.2e1;

   t1 = sin(km);
   t3 = exp(-kn);
   t6 = km * km;
   t7 = kn * kn;
   _C3 = -t1 * sigma * t3 / Z / (t6 + t7) / (t3 - 0.1e1) / (t3 + 0.1e1) / 0.2e1;

   t1 = sin(km);
   t5 = km * km;
   t6 = kn * kn;
   t9 = exp(-kn);
   _C4 = -t1 * sigma / Z / (t5 + t6) / (t9 - 0.1e1) / (t9 + 0.1e1) / 0.2e1;

   t4 = exp(-kn * z);
   t10 = exp(kn * (z - 0.1e1));
   t14 = sin(km * z);
   t15 = km * km;
   t16 = kn * kn;
   t18 = pow(t15 + t16, 0.2e1);
   ss = (_C1 + z * _C3) * t4 + (_C2 + z * _C4) * t10 + kn * sigma * t14 / t18 / Z;

   t6 = exp(-kn * z);
   t14 = exp(kn * (z - 0.1e1));
   t18 = cos(km * z);
   t20 = km * km;
   t21 = kn * kn;
   t23 = pow(t20 + t21, 0.2e1);
   ss_z = (-(_C1 + z * _C3) * kn + _C3) * t6 + (_C4 + (_C2 + z * _C4) * kn) * t14 + kn * sigma * t18 * km / t23 / Z;

   t3 = kn * kn;
   t9 = exp(-kn * z);
   t19 = exp(kn * (z - 0.1e1));
   t23 = sin(km * z);
   t25 = km * km;
   t27 = pow(t25 + t3, 0.2e1);
   ss_zz = ((_C1 + z * _C3) * t3 - 0.2e1 * _C3 * kn) * t9 + (0.2e1 * _C4 * kn + (_C2 + z * _C4) * t3) * t19 - kn * sigma * t23 * t25 / t27 / Z;

   t3 = kn * kn;
   t4 = t3 * kn;
   t10 = exp(-kn * z);
   t20 = exp(kn * (z - 0.1e1));
   t24 = cos(km * z);
   t26 = km * km;
   t29 = pow(t26 + t3, 0.2e1);
   ss_zzz = (-(_C1 + z * _C3) * t4 + 0.3e1 * _C3 * t3) * t10 + (0.3e1 * _C4 * t3 + (_C2 + z * _C4) * t4) * t20 - kn * sigma * t24 * t26 * km / t29 / Z;

   /* u1 = Vz, u2 = Vx, u3 = tzz, u4 = tzx, pp = pressure */

   u1 = kn*ss;
   u2 = -ss_z;
   pp = Z*(ss_zzz-kn*kn*ss_z)/kn;
   u3 = 2.0*kn*ss_z - pp;
   u4 = -Z*(ss_zz + kn*kn*ss);
   txx = -2.0*Z*kn*ss_z - pp;
   
   u1 *= cos(kn*x); /* z velocity */
   sum1 += u1;
   u2 *= sin(kn*x); /* x velocity */
   sum2 += u2;
   u3 *= cos(kn*x); /* zz stress */
   sum3 += u3;
   u4 *= sin(kn*x); /* zx stress */
   sum4 += u4;
   txx *= cos(kn*x); /* xx stress */
   sum6 += txx;
   pp *= cos(kn*x); /* pressure */
   sum5 += pp;

   e_zz = kn*ss_z*cos(kn*x);  /* zz rate of strain */
   e_xx = -e_zz;  /* xx rate of strain */
   e_xz = -0.5*(ss_zz+kn*kn*ss)*sin(kn*x); /* xz rate of strain */

//   ss *= sin(kn*x); /* stream function */
   /* density/temp */
//   sum7 += -sin(km*z)*cos(kn*x);
//   mag=sqrt(sum1*sum1+sum2*sum2);

   // printf("%0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f\n",x,z,sum1,sum2,sum3,sum4,sum5,sum6,mag,sum7,e_zz,e_xx,e_xz,ss);

   if ( velocity != NULL ) {
      velocity[0] = sum2;
      velocity[1] = sum1;
   }
   if( pressure != NULL ) {
      *pressure = sum5;
   }
   if( Tstress != NULL ) {
      Tstress[0] = sum6;
      Tstress[1] = sum3;
      Tstress[2] = sum4; /*2*Z*e_xz;*/
   }
   if( strainRate != NULL ) {
      strainRate[0] = e_xx;
      strainRate[1] = e_zz;
      strainRate[2] = e_xz;
   }
   /*
   if ( fabs( sum3 - ( 2*Z*e_zz - sum5 ) ) > 1e-4 ) {
      printf("%g is not within tolerance 1e-4\n", sum3 - ( 2*Z*e_zz + sum5 ) );
      assert(0);
   }
   if( fabs( sum5 - ( -0.5*(sum6 + sum3) ) ) > 1e-4 ) {
      printf("error is %g, which is outside tolerance of 1e-4\n", fabs( sum5 - ( -0.5*(sum6 + sum3) )) );
      assert(0);
   }
   */
}
