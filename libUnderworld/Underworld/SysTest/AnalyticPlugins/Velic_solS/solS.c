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


void _Velic_solS( 
		double pos[],
		int _n, double _eta,
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] );


#ifndef NOSHARED
/*
int main( int argc, char **argv )
{
	int i,j;
	double pos[2], vel[2], pressure, total_stress[3], strain_rate[3];
	double x,z;
	
	for (i=0;i<129;i++){
		for(j=0;j<129;j++){
			x = i/128.0;
			z = j/128.0;
			
			pos[0] = x;
			pos[1] = z;
			solS( pos, 1, 1.0, vel, &pressure, total_stress, strain_rate );
			
			printf("%0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f \n",
					pos[0],pos[1],	// 1, 2
					vel[0],vel[1], 	// 3, 4
					pressure, 		// 5
					total_stress[0], total_stress[1], total_stress[2],	// 6,7,8
			strain_rate[0], strain_rate[1], strain_rate[2] );	// 9,10,11
			
		}
		printf("\n");
	}
	
	return 0;
}
*/
#endif 

void _Velic_solS( 
		double pos[],						/* coordinates */
		int _n, double _eta,				/* wavenumber in x, viscosity */
		double vel[], double* presssure, 				/* output: velocity, pressure */
		double total_stress[], double strain_rate[] ) 	/* ouput: total stresss, strain rate */ 
{
	double Z;
	double E;
	double e_nPI;
	double e_2nPI;
	double e_4nPI;
	double n;
	double A,B,C,D;
	double x,y,vx,vy;
	double p;
	double t_xx,t_yy,t_xy;
	double e_xx,e_yy,e_xy;
	double du_dx, dv_dy, du_dy, dv_dx;
	
	
	Z = _eta;
	n = _n;
	
	if( n == 0 ) {
		printf("n must be non-zero \n" );
		abort();
	}
	
	
	/* Constants */
	e_nPI = exp( n * M_PI );
	e_2nPI = e_nPI * e_nPI;
	e_4nPI = e_2nPI * e_2nPI;
	
	E = (4.0 * n * n * M_PI * M_PI + 2.0 ) * e_2nPI - e_4nPI - 1.0;
	
	A = ( e_2nPI - 1.0 )* e_nPI / E;
	B = - A;
	
	C =   ( 2.0 * n * M_PI - e_2nPI + 1.0 ) * e_nPI / E;
	D = - ( 2.0 * n * M_PI * e_2nPI - e_2nPI + 1.0 ) * e_nPI / E;
	
	
	x = pos[0];
	y = pos[1];
	
	/* velocity */
	vx = sin( n * M_PI * x ) * 
		( ( A * n * M_PI + C + C * n * M_PI * y) * exp( n * M_PI * y ) 
		- ( B * n * M_PI - D + D * n * M_PI * y ) * exp( - n * M_PI * y ) );
	
	vy = - n * M_PI * cos( n * M_PI * x ) * 
		( ( A + C * y ) * exp( n * M_PI * y ) 
		+ ( B + D * y ) * exp( - n * M_PI * y ) );
	
	/* pressure */
	p = - 2.0 * n * M_PI * cos( n * M_PI * x ) * ( C * exp( n * M_PI * y ) + D * exp( - n * M_PI * y ) );
	
	/* strain rate */
	du_dx = n*M_PI*cos(n*M_PI*x) * 
		( ( A*n*M_PI + C + C*n*M_PI*y) * exp(n*M_PI*y) 
		- ( B*n*M_PI - D + D*n*M_PI*y) * exp(-n*M_PI*y) );
	
	dv_dx = n*n*M_PI*M_PI*sin(n*M_PI*x) * 
		( ( A + C*y ) * exp(n*M_PI*y) 
		+ ( B + D*y ) * exp(-n*M_PI*y) );
	
	du_dy = sin(n*M_PI*x) * 
		( ( A*n*n*M_PI*M_PI + 2.0*C*n*M_PI + C*n*n*M_PI*M_PI*y ) * exp(n*M_PI*y) 
		+ ( B*n*n*M_PI*M_PI - 2.0*D*n*M_PI + D*n*n*M_PI*M_PI*y ) * exp(-n*M_PI*y) );
	
	dv_dy = - n*M_PI*cos(n*M_PI*x) * 
		( ( A*n*M_PI + C + C*n*M_PI*y ) * exp(n*M_PI*y) 
		+ ( -B*n*M_PI + D - D*n*M_PI*y ) * exp(-n*M_PI*y) );
	
	
	e_xx = du_dx; /* du/dx */
	e_yy = dv_dy; /* dv/dy */
	e_xy = 0.5 * ( du_dy + dv_dx ); /* 0.5*(du/dy + dv/dx) */
	
	
	
	/* total stress */
	t_xx = 2.0 * Z * e_xx - p;
	t_yy = 2.0 * Z * e_yy - p;
	t_xy = 2.0 * Z * e_xy;
	
	
	
	/* Output */
	if( vel != NULL ) {
		vel[0] = vx;
		vel[1] = vy;
	}
	if( presssure != NULL ) {
		(*presssure) = p;
	}
	if( total_stress != NULL ) {
		total_stress[0] = t_xx;
		total_stress[1] = t_yy;
		total_stress[2] = t_xy;
	}
	if( strain_rate != NULL ) {
		/* sigma = tau - p, tau = sigma + p, tau[] = 2*eta*strain_rate[] */
		strain_rate[0] = e_xx;
		strain_rate[1] = e_yy;
		strain_rate[2] = e_xy;
	}
	
	
	
}


