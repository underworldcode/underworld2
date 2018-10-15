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

#if 0
int main (){
   double Z,u1,u2,u3,u4,u5,u6;
   double sum1,sum2,sum3,sum4,sum5,sum6,mag,sum7,sum8,sum9,sum10,x,y,z;
   double sigma,dx,dy;
   double del_rho;
   int i,j,n,m;
   double L1,kn,km;
   double Am,Ap,Bm,Bp,C,D,E;
   double pp,txx,tyy,tyx;

   /*************************************************************************/
   //
//   x0 = 0.3; 
   dx = 0.7;
   dy = 0.4;
   sigma = 1.0;
   Z = 1.0;
//   zc = 0.6;
   //
   y=0.4;
   for (i=0;i<101;i++){
      for(j=0;j<101;j++){
	 x = ((double)i)/100.0;
	 z = ((double)j)/100.0;

	 sum1=0.0;
	 sum2=0.0;
	 sum3=0.0;
	 sum4=0.0;
	 sum5=0.0;
	 sum6=0.0;
	 sum7=0.0;
	 sum8=0.0;
	 sum9=0.0;
	 sum10=0.0;
	 
	 for(n=0;n<45;n++){
	    for(m=0;m<45;m++){

	       if ( n!=0 && m!=0 ){
		  del_rho = 4.0*sigma*sin((double)n*M_PI*dx)*sin((double)m*M_PI*dy)/(double)n/M_PI/(double)m/M_PI;
	       } else {
		  if ( n==0 && m !=0){
		     del_rho = 2.0*sigma*dx*sin((double)m*M_PI*dy)/(double)m/M_PI;
		  } else {
		     if ( n!=0 && m ==0){
			del_rho = 2.0*sigma*dy*sin((double)n*M_PI*dx)/(double)n/M_PI;
		     } else {
			del_rho = sigma*dx*dy;
		     }
		  }
	       }
	       kn = M_PI*(double)n;
	       km = M_PI*(double)m;

	       L1 = M_PI*sqrt( (double)(n*n + m*m));

	       Am = exp((z-2.0)*L1)-exp(-z*L1);
	       Ap = exp((z-2.0)*L1)+exp(-z*L1);
	       Bm = exp((z-1.0)*L1)-exp(-(z+1.0)*L1);
	       Bp = exp((z-1.0)*L1)+exp(-(z+1.0)*L1);
	       C = (exp(-z*L1)-1.0)*(exp((z-1.0)*L1)-1.0);
	       D = exp((z-1.0)*L1)-exp(-z*L1);
	       E = (1.0+exp(-L1));
	       
	       u1 = (n!=0 || m!=0) ? -( z*Am+(z-1.0)*Bm )*del_rho/( 2*Z*L1*E*E ) - C*del_rho/(Z*L1*L1*E) : 0.0;
	       if ( m !=0 ){
		  u2 =  ( z*Ap+(z-1.0)*Bp )*del_rho*km/( 2*Z*L1*L1*E*E ) - D*del_rho*km/(2*Z*L1*L1*L1*E);
		  u5 =  ( z*Am+(z-1.0)*Bm )*del_rho*km/( L1*E*E ) + C*del_rho*km/(L1*L1*E);
	       } else {
		  u2 = 0.0;
		  u5 = 0.0;
	       }
	       if ( n !=0 ){
		  u3 =  ( z*Ap+(z-1.0)*Bp )*del_rho*kn/( 2*Z*L1*L1*E*E ) - D*del_rho*kn/(2*Z*L1*L1*L1*E);
		  u6 =  ( z*Am+(z-1.0)*Bm )*del_rho*kn/( L1*E*E ) + C*del_rho*kn/(L1*L1*E);
	       } else {
		  u3 = 0.0;
		  u6 = 0.0;
	       }
	       u4 = (n!=0 || m!=0) ? -( z*Ap+(z-1.0)*Bp )*del_rho/( E*E ) + 2.0*D*del_rho/(L1*E) : 0.0*del_rho*(z-0.5) ;

	       pp = -u4-2.0*Z*(kn*u3+km*u2);
	       
	       txx = (-pp +2.0*Z*kn*u3)*cos(n*M_PI*x)*cos(m*M_PI*y);
	       tyy = (-pp +2.0*Z*km*u2)*cos(n*M_PI*x)*cos(m*M_PI*y);
	       tyx = -Z*(km*u3+kn*u2)*sin(n*M_PI*x)*sin(m*M_PI*y);
	       
	       pp *= cos(n*M_PI*x)*cos(m*M_PI*y);
	       //printf("u2 = %0.7g n=%d m=%d --> L2 = %0.7g  --> L1 = %0.7g\n",u2,n,m,L2,L1);
	       u1 *= cos(n*M_PI*x)*cos(m*M_PI*y); 
		    
	       sum1 += u1;
	       u2 *= cos(n*M_PI*x)*sin(m*M_PI*y);
	       sum2 += u2;
	       u3 *= sin(n*M_PI*x)*cos(m*M_PI*y);
	       sum3 += u3;
	       u4 *= cos(n*M_PI*x)*cos(m*M_PI*y);
	       sum4 += u4;
	       u5 *= cos(n*M_PI*x)*sin(m*M_PI*y);
	       sum5 += u5;
	       u6 *= sin(n*M_PI*x)*cos(m*M_PI*y);
	       sum6 += u6;

	       sum7 += pp; /* total pressure */
	       sum8 += txx;
	       sum9 += tyy;
	       sum10 += tyx;
	       // u5 = (double)-2*n*M_PI*(Z*u2 + u3)*cos(n*M_PI*x); /* pressure */

	       //u6 = (double)(u3*2*n*M_PI + 4*Z*n*M_PI*u2)*cos(n*M_PI*x); /* xx stress */	    
	       //sum5 +=u5;
	       //sum6 +=u6;

	       //u1 *= cos(n*M_PI*x); /* z velocity */
	       //sum1 += u1;
	       //u2 *= sin(n*M_PI*x); /* x velocity */
	       //sum2 += u2;
	       //u3 *= 2*n*M_PI*cos(n*M_PI*x); /* zz stress */
	       //sum3 += u3;
	       //u4 *= 2*n*M_PI*sin(n*M_PI*x); /* zx stress */
	       //sum4 += u4;

	    }/* n */
	 }/* m */
	 //if(z<zc){
	 //sum7 += sigma*dx; /* n=0 density term */
	 //sum5 += sigma*dx*(0.5-z); /* n=0 (hydrostatic) pressure term */
	 //sum3 += -sigma*dx*(0.5-z);/* n=0 (hydrostatic) tzz term */
	 //} else {
	 //sum5 += sigma*dx*(0.5-zc); /* n=0 (hydrostatic) pressure term */
	 //sum3 += -sigma*dx*(0.5-zc);/* n=0 (hydrostatic) tzz term */	    
	 //}
	
	 mag=sqrt(sum1*sum1+sum2*sum2+sum3*sum3);
	 printf("%0.7g %0.7g %0.7g %0.7g %0.7g %0.7g %0.7g %0.7g %0.7g %0.7g %0.7g %0.7g %0.7g\n",x,z,sum1,sum2,sum3,sum4,sum5,sum6,sum7,sum8,sum9,sum10,mag);
      }/* j */
      // printf("%0.10g\n",exp(400.0));
      printf("\n");
   }/* i */
   return 0;
}
#endif

