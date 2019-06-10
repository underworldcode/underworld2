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

#define PI 3.14159265358979323846264338328

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
            _Velic_solCA( pos, 1.0, 1.0, 0.2, 0.4,
                    vel, &pressure, total_stress, strain_rate );
 
            printf("t_xz, e_xz look funny \n");
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

void _Velic_solCA(
		double pos[], 
		double _sigma, double _eta, 	/* Input parameters: density, viscosity */
		double _dx, double _x_0, 		/* Input parameters: width of the dense column, centre of the dense column */
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] )
{
	double Z,u1,u2,u3,u4,u5,u6;
	double _C1,_C2,_C3,_C4;
	double sum1,sum2,sum3,sum4,sum5,sum6,sum7,x,z;
	double sigma,del_rho,k,x0,dx;
	int n;
	
	double t1,t2,t3,t4,t5,t6,t7,t8,t10,t11;
	double t12,t14,t16,t21;
	
	
	/* del_rho = sigma for x-dx/2 < x < xc+dx/2 and 0 else */
	sigma = _sigma;
	Z = _eta;
	x0 = _x_0;
	dx = _dx;
	
	x = pos[0];
	z = pos[1];
	
	sum1=0.0;
	sum2=0.0;
	sum3=0.0;
	sum4=0.0;
	sum5=0.0;
	sum6=0.0;
	sum7=0.0;
	
	
	
	for(n=1;n<155;n++){
		
		k = (double)n*M_PI;
		del_rho = 4.0*sigma*cos(k*x0)*sin(k*dx/2.0)/k;
		
		t1 = exp(-k);
		t7 = k * k;
		t12 = pow(t1 + 0.1e1, 0.2e1);
		_C1 = -del_rho * (-0.2e1 + k * t1 - 0.2e1 * t1) / Z / t7 / k / t12 / 0.2e1;
		
		t1 = exp(-k);
		t6 = k * k;
		t11 = pow(t1 + 0.1e1, 0.2e1);
		_C2 = del_rho * (k + 0.2e1 + 0.2e1 * t1) / Z / t6 / k / t11 / 0.2e1;
		
		t3 = k * k;
		t5 = exp(-k);
		_C3 = del_rho / Z / t3 / (t5 + 0.1e1) / 0.2e1;
		
		t3 = k * k;
		t5 = exp(-k);
		_C4 = -del_rho / Z / t3 / (t5 + 0.1e1) / 0.2e1;
		
		/* Vz */
		t4 = exp(-k * z);
		t10 = exp(k * (z - 0.1e1));
		t14 = k * k;
		u1 = k * ((_C1 + z * _C3) * t4 + (_C2 + z * _C4) * t10 - del_rho / Z / t14 / k);
		/* Vx */
		t7 = exp(k * (z - 0.1e1));
		t14 = exp(-k * z);
		u2 = (-_C4 - (_C2 + z * _C4) * k) * t7 + (-_C3 + (_C1 + z * _C3) * k) * t14;
		/* tzz */
		t1 = Z * k;
		t8 = exp(k * (z - 0.1e1));
		t16 = exp(-k * z);
		u3 = 0.2e1 * t1 * (_C4 + (_C2 + z * _C4) * k) * t8 + 0.2e1 * t1 * (_C3 - (_C1 + z * _C3) * k) * t16;
		/* txz */
		t2 = k * k;
		t11 = exp(k * (z - 0.1e1));
		t21 = exp(-k * z);
		u4 = -Z * (0.2e1 * _C4 * k + 0.2e1 * t2 * (_C2 + z * _C4)) * t11 - Z * (-0.2e1 * _C3 * k + 0.2e1 * t2 * (_C1 + z * _C3)) * t21 + 0.1e1 / k * del_rho;
		/* txx */
		t1 = Z * k;
		t8 = exp(k * (z - 0.1e1));
		t16 = exp(-k * z);
		u6 = -0.2e1 * t1 * (_C4 + (_C2 + z * _C4) * k) * t8 - 0.2e1 * t1 * (_C3 - (_C1 + z * _C3) * k) * t16;
		/* pressure */
		t1 = Z * k;
		t4 = exp(k * (z - 0.1e1));
		t8 = exp(-k * z);
		u5 = 0.2e1 * t1 * _C4 * t4 + 0.2e1 * t1 * _C3 * t8;
		
		
		

	    u5 = u5*cos(k*x); /* pressure */
	    u6 = u6*cos(k*x); /* xx stress */	    
	    sum5 +=u5;
	    sum6 +=u6;

	    u1 *= cos(k*x); /* z velocity */
	    sum1 += u1;
	    u2 *= sin(k*x); /* x velocity */
	    sum2 += u2;
	    u3 *= cos(k*x); /* zz stress */
	    sum3 += u3;
	    u4 *= sin(k*x); /* zx stress */
	    sum4 += u4;
	    /* density */
	    sum7 += del_rho*cos(k*x);

	 }/* n */
	 /* n=0 term*/
	 sum7 += sigma*dx;
	 /* pressure 0th term integration constant is arbitrarily chosen so that this term is 0 at z=0.5 */
	 sum5 += sigma*dx*(0.5-z); /* now have total pressure */
	 sum3 += -sum5; /* now have total zz stress */
	 sum6 += -sum5; /* now have total xx stress */
	 //mag=sqrt(sum1*sum1+sum2*sum2);
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
		strain_rate[0] = (sum6+sum5)/(2.0*Z);
		strain_rate[1] = (sum3+sum5)/(2.0*Z);
		strain_rate[2] = (sum4)/(2.0*Z);
	}
	
	
}


