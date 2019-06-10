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
#include "solL.h"

#if 0
int main( int argc, char **argv )
{
	int i,j;
	double pos[2], vel[2], pressure, total_stress[3], strain_rate[3];
	double x,z;
	
	for (i=0;i<101;i++){
		for(j=0;j<101;j++){
			x = i/100.0;
			z = j/100.0;
			
			pos[0] = x;
			pos[1] = z;
			_Velic_solL(
					pos,
					2.0, 1.0,
					1.0,
					vel, &pressure, total_stress, strain_rate );
			printf("t_zx fucked !! \n");
			printf("%0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f \n",
					pos[0],pos[1],
					vel[0],vel[1], pressure, 
					total_stress[0], total_stress[1], total_stress[2], 
					strain_rate[0], strain_rate[1], strain_rate[2] );
		}
		printf("\n");
	}
	
	return 0;
}
#endif


void _Velic_solL( 
		double pos[],
		double _sigma_B, double _sigma_A, /* density B, density A */
		double _eta, /* viscosity */
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] )
{
	double u1,u2,u3,u4,pp,txx,Z;
	double sum1,sum2,sum3,sum4,sum5,sum6,sum7,x,z;
	double dsigma,sigma0,sigma1;
	double del_rho;
	int n;
	double kn;
	double _C1,_C2,_C3,_C4;
	double rho;
	
	double t1,t2,t3,t4,t5,t6,t8,t9,t10,t12;
	double t14,t16,t17,t20,t23;
	
	
	
	/*************************************************************************/
	/*************************************************************************/
	
	sigma1 = _sigma_B; /* upper triangular density */
	sigma0 = _sigma_A; /* lower triangular density */
	
	dsigma = sigma1-sigma0; /* density difference */
	Z = _eta; /* viscosity */
	
	/*************************************************************************/
	/*************************************************************************/
	
	x = pos[0];
	z = pos[1];
	
	sum1=0.0;
	sum2=0.0;
	sum3=0.0;
	sum4=0.0;
	sum5=0.0;
	sum6=0.0;
	sum7=0.0;
	
	for(n=1;n<55;n++){
		kn = (double) n*M_PI;
		
		
		/*******************************************/
		/*            calculate del_rho            */
		/*******************************************/
		del_rho =  2.0*dsigma*sin(kn*(z-1.0))/kn; /* density */
		
		/*******************************************/
		/*         calculate the constants         */
		/*******************************************/
		t1 = sin(kn);
		t3 = exp(-kn);
		t4 = t3 * t3;
		t9 = kn * kn;
		t14 = pow(t3 - 0.1e1, 0.2e1);
		t17 = pow(t3 + 0.1e1, 0.2e1);
		_C1 = -dsigma * t1 * (0.2e1 * t4 * kn - t4 + 0.1e1) / t9 / Z / t14 / t17 / 0.4e1;
		
		t1 = sin(kn);
		t3 = exp(-kn);
		t4 = t3 * t3;
		t9 = kn * kn;
		t14 = pow(t3 - 0.1e1, 0.2e1);
		t17 = pow(t3 + 0.1e1, 0.2e1);
		_C2 = dsigma * t1 * t3 * (-t4 + 0.1e1 + 0.2e1 * kn) / t9 / Z / t14 / t17 / 0.4e1;
		
		t1 = sin(kn);
		t6 = exp(-kn);
		_C3 = t1 * dsigma / Z / kn / (t6 - 0.1e1) / (t6 + 0.1e1) / 0.4e1;
		
		t1 = exp(-kn);
		t2 = sin(kn);
		_C4 = t1 * t2 * dsigma / Z / kn / (t1 - 0.1e1) / (t1 + 0.1e1) / 0.4e1;
		
		/*******************************************/
		/*       calculate the velocities etc      */
		/*******************************************/
		
		
		t3 = kn * z;
		t4 = exp(-t3);
		t10 = exp(kn * (z - 0.1e1));
		t12 = kn * kn;
		t16 = sin(t3 - kn);
		u1 = (_C1 + z * _C3) * t4 + (_C2 + z * _C4) * t10 - dsigma / t12 * t16 / Z / 0.4e1;
		
		t1 = kn * z;
		t2 = exp(-t1);
		t5 = kn * (z - 0.1e1);
		t6 = exp(t5);
		t10 = 0.1e1 / kn;
		t17 = kn * kn;
		t20 = cos(t5);
		u2 = t2 * _C1 - t6 * _C2 + t2 * (t1 - 0.1e1) * _C3 * t10 - t6 * (t1 + 0.1e1) * _C4 * t10 + dsigma / t17 * t20 / Z / 0.4e1;
		
		t1 = Z * kn;
		t3 = exp(-kn * z);
		t8 = kn * (z - 0.1e1);
		t9 = exp(t8);
		t23 = cos(t8);
		u3 = -0.2e1 * t1 * t3 * _C1 + 0.2e1 * t1 * t9 * _C2 - 0.2e1 * t1 * t3 * z * _C3 + 0.2e1 * t1 * t9 * z * _C4 - 0.1e1 / kn * dsigma * t23;
		
		t2 = kn * z;
		t8 = exp(kn * (z - 0.1e1));
		t14 = exp(-t2);
		u4 = -0.2e1 * Z * (_C4 + kn * _C2 + t2 * _C4) * t8 - 0.2e1 * Z * (-_C3 + kn * _C1 + t2 * _C3) * t14;
		
		
		
		pp = (double)-(u3 + 2.0*n*M_PI*Z*u2)*cos(n*M_PI*x); /* pressure */
		
		txx = (double)(u3 + 4*Z*n*M_PI*u2)*cos(n*M_PI*x); /* xx stress */	    
		sum5 += pp;
		sum6 += txx;
		
		u1 *= cos(n*M_PI*x); /* z velocity */
		sum1 += u1;
		u2 *= sin(n*M_PI*x); /* x velocity */
		sum2 += u2;
		u3 *= cos(n*M_PI*x); /* zz stress */
		sum3 += u3;
		u4 *= sin(n*M_PI*x); /* zx stress */
		sum4 += u4;
		
		rho =  del_rho*cos(n*M_PI*x); /* density */
		sum7 += rho;
	}/* n */
	
	/* include terms for n=0 now */
	sum7 += dsigma*z+sigma0; /* density term for n=0 */
	
	u3 = 0.5*dsigma*z*z + sigma0*z; /* zz stress for n=0 */
	sum3 += u3;
	
	txx = u3; /* xx stress */
	pp = -u3; /* pressure */
	sum5 += pp;
	sum6 += txx;
	
	
	//mag=sqrt(sum1*sum1+sum2*sum2);
	//printf("%0.7g %0.7g %0.7g %0.7g %0.7g %0.7g\n",x,y,z,sum3,sum2,sum1);
	//printf("%0.7g %0.7g %0.7g\n%0.7g %0.7g %0.7g\n\n\n",x,y,z,x+sum3,y+sum2,z+sum1);
	/*printf("%0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f\n",x,z,sum1,sum2,sum3,sum4,sum5,sum6,mag,sum7);*/
	
	
	/* Output */
	if( vel != NULL ) {
		vel[0] = sum2;
		vel[1] = sum1;
	}
	if( presssure != NULL ) {
		(*presssure) = sum5;
	}
	if( total_stress != NULL ) {
		total_stress[0] = sum6;
		total_stress[1] = sum3;
		total_stress[2] = sum4;
	}
	if( strain_rate != NULL ) {
		/* sigma = tau - p, tau = sigma + p, tau[] = 2*eta*strain_rate[] */
		strain_rate[0] = (sum6+sum5)/(2.0*Z);
		strain_rate[1] = (sum3+sum5)/(2.0*Z);
		strain_rate[2] = (sum4)/(2.0*Z);
	}
	/* Value checks, could be cleaned up if needed. Julian Giordani 9-Oct-2006*/
//        if( fabs( sum5 - ( -0.5*(sum6+sum3) ) ) > 1e-5 ) {
//                assert(0);
//        }
}


