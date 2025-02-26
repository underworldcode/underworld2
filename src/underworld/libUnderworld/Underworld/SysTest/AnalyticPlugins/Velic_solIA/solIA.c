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
#include "solIA.h"

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
			_Velic_solIA(
					pos,
					1.0, 1.0,
					0.4, 0.3,
					vel, &pressure, total_stress, strain_rate );
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

void _Velic_solIA( 
		double pos[],
		double _sigma, double _B, /* density, viscosity parameter */
		double _dx, double _x_0, /* width of dense column, centre of dense column */
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] )
{
	double Z;
	double u1,u2,u3,u4,pp,txx;
	double sum1,sum2,sum3,sum4,sum5,sum6,sum7,x,z;
	double sigma,dx;
	double del_rho;
	int n;
	double kn;
	double _C1,_C2,_C3,_C4;
	double B, Rp, R, U, VV;
	double x0,rho,a,b,r;
	
	double t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
	double t11,t12,t13,t14,t15,t16,t17,t18,t19,t20;
	double t21,t22,t23,t24,t25,t26,t27,t28,t29,t30;
	double t31,t32,t33,t34,t35,t36,t37,t38,t39,t40;
	double t41,t42,t43,t44,t45,t46,t47,t48,t49,t50;
	double t51,t52,t53,t57,t58,t59,t60,t61,t62,t63;
	double t64,t65,t66,t67,t69,t70,t72,t73,t74,t75;
	double t76,t77,t78,t81,t82,t84,t85,t88,t89,t91;
	double t92,t95,t105,t106,t108,t109,t110,t111,t112,t116;
	double t117,t119,t120,t121,t123,t126,t127,t128,t129,t130;
	double t131,t134,t135,t137,t140,t143;
	
	
	
	/*************************************************************************/
	/*************************************************************************/
	/* "background" fluid has density of zero */
	x0 = _x_0;  /* centre of dense column */
	dx = _dx;  /* total width of dense column */
	sigma = _sigma; /* density parameter */
	B = _B; /* viscosity parameter */
	/* viscosity = exp(2.0*B*z) */
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
		
		a = B*B + kn*kn;
		b = 2.0*kn*B;
		r = sqrt(a*a + b*b);
		Rp = sqrt( (r+a)/2.0 );
		R  = sqrt( (r-a)/2.0 );
		U  = Rp - B;
		VV = Rp + B;
		
		/*******************************************/
		/*            calculate del_rho            */
		/*******************************************/
		del_rho = 4.0*sigma*cos(n*M_PI*x0)*sin(n*M_PI*dx/2.0)/n/M_PI;
		/* we can have any del_rho we like here as long as it is independent of z */
		
		
		/*******************************************/
		/*         calculate the constants         */
		/*******************************************/
		
		
		t2 = sin(R);
		t3 = t2 * R;
		t4 = Rp * Rp;
		t6 = (-(double) B - Rp);
		t8 = exp((double) (2 * t6));
		t11 = R * R;
		t12 = t11 * Rp;
		t13 = 0.2e1 * R;
		t14 = cos(t13);
		t16 = exp((double) t6);
		t19 = (double) B * (double) B;
		t25 = exp(-(double) (2 * B));
		t26 = t25 * t2;
		t31 = t11 * t19;
		t34 = exp(-(double) B - 0.3e1 * Rp);
		t37 = t19 * (double) B;
		t38 = t37 * Rp;
		t41 = t11 * t4;
		t46 = t4 * Rp;
		t47 = t46 * (double) B;
		t49 = t19 * t4;
		t53 = sin(t13);
		t61 = cos(R);
		t62 = t25 * t61;
		t67 = t14 * t16;
		t70 = -t3 * t4 * (double) B * t8 + t12 * (double) B * t14 * t16 + 0.5e1 * t3 * t19 * Rp * t8 + 0.5e1 * t26 * R * t19 * Rp + 0.2e1 * t31 * t34 + 0.3e1 * t38 * t16 - 0.2e1 * t41 * t34 - 0.2e1 * t31 * t16 - t47 * t16 - 0.2e1 * t49 * t16 - 0.5e1 * R * Rp * t19 * t53 * t16 + t26 * R * t4 * (double) B + 0.2e1 * t62 * t31 - t12 * (double) B * t16 - 0.3e1 * t38 * t67;
		t72 = t11 * R;
		t75 = t61 * t11;
		t81 = t2 * t72;
		t88 = R * t46;
		t89 = t53 * t16;
		t91 = t72 * Rp;
		t109 = t47 * t67 + t26 * t72 * (double) B + 0.2e1 * t75 * t4 * t8 + 0.2e1 * t41 * t67 - t81 * (double) B * t8 + 0.2e1 * t49 * t67 + t81 * Rp * t8 + t88 * t89 + t26 * t91 - 0.2e1 * t75 * t19 * t8 - t91 * t89 + 0.3e1 * t25 * t37 * t3 - 0.3e1 * t37 * R * t2 * t8 - t3 * t46 * t8 - 0.2e1 * t62 * t41 - t26 * t88;
		t111 = kn * kn;
		t117 = exp(-0.4e1 * Rp);
		t120 = exp(-0.2e1 * Rp);
		t123 = t120 * t14;
		_C1 = del_rho * kn * (t70 + t109) / (t111 + 0.4e1 * t19) / (t31 - t41 * t117 - 0.2e1 * t49 * t120 + 0.2e1 * t49 * t123 + 0.2e1 * t41 * t123 - t41 - 0.2e1 * t31 * t120 + t31 * t117) / 0.2e1;
		
		t3 = exp(-(double) (2 * B));
		t4 = t3 * R;
		t5 = cos(R);
		t6 = t5 * Rp;
		t7 = B * B;
		t11 = Rp * Rp;
		t12 = t11 * R;
		t13 = -(double) B - Rp;
		t14 = exp(t13);
		t17 = t11 * Rp;
		t19 = 0.2e1 * R;
		t20 = sin(t19);
		t21 = t20 * t14;
		t23 = R * R;
		t24 = t23 * R;
		t25 = t24 * Rp;
		t26 = cos(t19);
		t27 = t26 * t14;
		t29 = t24 * t5;
		t31 = exp(0.2e1 * t13);
		t32 = (double) B * t31;
		t34 = sin(R);
		t35 = t23 * t34;
		t36 = t11 * t31;
		t39 = t7 * B;
		t45 = exp(-(double) B - 0.3e1 * Rp);
		t51 = R * t5;
		t57 = t34 * (double) t7;
		t60 = t3 * t24;
		t62 = t3 * t23;
		t74 = Rp * t31;
		t77 = -0.5e1 * t4 * t6 * (double) t7 + t12 * (double) B * t14 + t17 * (double) B * t21 + t25 * t27 + t29 * t32 - 0.2e1 * t35 * t36 - 0.3e1 * t4 * t5 * (double) t39 - t12 * (double) B * t45 - 0.2e1 * t35 * (double) t7 * t31 + 0.3e1 * t51 * (double) t39 * t31 - t51 * t17 * t31 - 0.4e1 * t57 * t36 - t60 * t6 - 0.2e1 * t62 * t34 * t11 - t60 * t5 * (double) B + 0.2e1 * t62 * t57 - 0.2e1 * t34 * t17 * t32 + 0.6e1 * t34 * (double) t39 * t74;
		t78 = R * Rp;
		t82 = (double) t7 * t11;
		t85 = t23 * t11;
		t106 = R * t17;
		t108 = R * (double) t39;
		t111 = t24 * (double) B;
		t128 = -0.5e1 * t78 * (double) t7 * t45 + 0.2e1 * t82 * t21 + 0.2e1 * t85 * t21 - 0.3e1 * (double) t39 * Rp * t21 - t25 * t45 + t51 * t11 * (double) B * t31 + t23 * Rp * (double) B * t20 * t14 + 0.5e1 * t51 * Rp * (double) t7 * t31 + t4 * t5 * t17 + t29 * t74 - t106 * t27 - 0.3e1 * t108 * t45 + t111 * t14 + t106 * t45 - t111 * t45 + 0.3e1 * t108 * t14 + 0.5e1 * t78 * (double) t7 * t26 * t14 - t4 * t5 * t11 * (double) B - 0.2e1 * t35 * Rp * (double) B * t31;
		t130 = (kn * kn);
		t135 = t23 * (double) t7;
		t137 = exp(-0.4e1 * Rp);
		t140 = exp(-0.2e1 * Rp);
		t143 = t140 * t26;
		_C2 = del_rho * kn * (t77 + t128) / (double) (t130 + 4 * t7) / (t135 - t85 * t137 - 0.2e1 * t82 * t140 + 0.2e1 * t82 * t143 + 0.2e1 * t85 * t143 - t85 - 0.2e1 * t135 * t140 + t135 * t137) / 0.2e1;
		
		t2 = R * Rp;
		t3 = B * B;
		t4 = sin(R);
		t7 = exp(-B - Rp);
		t11 = t4 * R;
		t15 = exp(-B - 0.3e1 * Rp);
		t19 = Rp * Rp;
		t20 = t19 * B;
		t23 = R * R;
		t24 = t3 * t23;
		t26 = exp(-0.2e1 * Rp);
		t28 = 0.2e1 * t24 * t26;
		t29 = t26 * Rp;
		t30 = t3 * B;
		t33 = t19 * Rp;
		t34 = t26 * t33;
		t36 = t3 * t19;
		t38 = 0.2e1 * t36 * t26;
		t40 = t26 * t23;
		t41 = Rp * B;
		t42 = 0.2e1 * R;
		t43 = cos(t42);
		t49 = cos(R);
		t50 = t49 * t23;
		t58 = t4 * t7;
		t60 = t23 * R;
		t63 = t23 * t19;
		t64 = t26 * t43;
		t66 = 0.2e1 * t63 * t64;
		t67 = 0.5e1 * t2 * t3 * t4 * t7 + 0.5e1 * t11 * t3 * Rp * t15 - t11 * t20 * t15 + t28 + 0.3e1 * t29 * t30 - t34 * B + t38 - 0.2e1 * t24 + t40 * t41 * t43 - 0.3e1 * t29 * t30 * t43 - 0.2e1 * t50 * t3 * t15 + 0.2e1 * t50 * t3 * t7 - R * t33 * t58 + t60 * Rp * t58 - t66;
		t69 = 0.2e1 * t36 * t64;
		t70 = t4 * t60;
		t76 = sin(t42);
		t108 = -t69 + t70 * B * t7 + t70 * Rp * t15 + t26 * R * t33 * t76 - t70 * B * t15 - t26 * t60 * Rp * t76 - t40 * t41 - t11 * t33 * t15 + t34 * B * t43 + 0.2e1 * t50 * t19 * t15 - 0.2e1 * t50 * t19 * t7 + 0.3e1 * t11 * t30 * t7 - 0.3e1 * t11 * t30 * t15 + t11 * t20 * t7 + 0.2e1 * t63 - 0.5e1 * t26 * t3 * t2 * t76;
		t110 = kn * kn;
		t116 = exp(-0.4e1 * Rp);
		_C3 = -del_rho * kn * (t67 + t108) / (t110 + 0.4e1 * t3) / (t24 - t63 * t116 - t38 + t69 + t66 - t63 - t28 + t24 * t116) / 0.2e1;
		
		t2 = ((double) B * (double) B);
		t3 = ((double) t2 * (double) B);
		t6 = ((double) Rp * (double) Rp);
		t10 = exp(-(double) (2 * Rp));
		t11 = (double) R * (double) R;
		t12 = t11 * (double) R;
		t13 = t10 * t12;
		t15 = R * Rp;
		t18 = t10 * (double) R;
		t19 = (double) t6 * (double) B;
		t21 = cos((double) R);
		t24 = exp((double) (-B - Rp));
		t28 = t12 * (double) Rp;
		t29 = t21 * t24;
		t33 = t6 * Rp;
		t34 = 2 * R;
		t35 = cos((double) t34);
		t38 = t12 * t21;
		t41 = exp((double) (-B - 3 * Rp));
		t44 = (double) B * t24;
		t48 = (double) R * t21;
		t52 = sin((double) t34);
		t53 = (double) t6 * t52;
		t59 = sin((double) R);
		t60 = t11 * t59;
		t70 = (double) (3 * t3 * R) + (double) (R * t6 * B) - t13 * (double) B - (double) (5 * t15 * t2) - t18 * t19 + 0.5e1 * (double) t15 * (double) t2 * t21 * t24 + t28 * t29 - 0.3e1 * t18 * (double) t3 - t18 * (double) t33 * t35 - t38 * (double) Rp * t41 - t38 * t44 + t38 * (double) B * t41 - t48 * t19 * t24 - 0.2e1 * t10 * (double) t2 * t53 - 0.3e1 * t48 * (double) t3 * t24 - 0.2e1 * t60 * (double) t2 * t41 - 0.2e1 * t59 * (double) t33 * t44 + 0.2e1 * t60 * (double) t2 * t24;
		t84 = (double) t6 * t24;
		t92 = t10 * t11;
		t95 = R * t33;
		t105 = Rp * t2;
		t109 = Rp * B;
		t119 = 0.2e1 * t60 * (double) t6 * t41 + 0.3e1 * t48 * (double) t3 * t41 - 0.3e1 * t10 * (double) t3 * (double) Rp * t52 + t13 * (double) Rp * t35 + 0.4e1 * (double) t2 * t59 * t84 + t48 * (double) t33 * t41 + t10 * (double) t33 * (double) B * t52 - 0.2e1 * t92 * t53 - (double) t95 * t29 + 0.2e1 * t60 * t84 + 0.6e1 * t59 * (double) t3 * (double) Rp * t24 + t48 * t19 * t41 + 0.5e1 * t18 * (double) t105 * t35 + t92 * (double) t109 * t52 - 0.5e1 * t48 * (double) t105 * t41 - 0.2e1 * t60 * (double) t109 * t24 + t12 * (double) B + (double) t95 - t28;
		t121 = (kn * kn);
		t126 = t11 * (double) t2;
		t127 = t11 * (double) t6;
		t129 = exp(-(double) (4 * Rp));
		t131 = t2 * t6;
		t134 = t10 * t35;
		_C4 = -del_rho * kn * (t70 + t119) / (double) (t121 + 4 * t2) / (t126 - t127 * t129 - 0.2e1 * (double) t131 * t10 + 0.2e1 * (double) t131 * t134 + 0.2e1 * t127 * t134 - t127 - 0.2e1 * t126 * t10 + t126 * t129) / 0.2e1;
		
		/*******************************************/
		/*       calculate the velocities etc      */
		/*******************************************/
		
		
		t1 = (kn * kn);
		t2 = (B * B);
		t8 = exp(B * z);
		t9 = t8 * t8;
		t14 = exp(U * (z - 0.1e1));
		t15 = 0.1e1 / kn;
		t17 = R * z;
		t18 = cos(t17);
		t20 = sin(t17);
		t25 = exp(-VV * z);
		u1 = -0.1e1 / (double) (t1 + 4 * t2) * del_rho / t9 + t14 * t15 * (_C1 * t18 + _C2 * t20) + t25 * t15 * (_C3 * t18 + _C4 * t20);
		
		t3 = exp((double) (U * (z - 1)));
		t4 = R * z;
		t5 = sin((double) t4);
		t6 = t5 * (double) R;
		t7 = cos((double) t4);
		t11 = (kn * kn);
		t12 = 0.1e1 / t11;
		t16 = t7 * (double) R;
		t22 = exp(-(double) (VV * z));
		t33 = ((double) B * (double) B);
		t39 = exp((double) (B * z));
		t40 = t39 * t39;
		u2 = -t3 * (-t6 + (double) U * t7) * _C1 * t12 - t3 * ((double) U * t5 + t16) * _C2 * t12 + t22 * (t6 + (double) VV * t7) * _C3 * t12 - t22 * (t16 - (double) VV * t5) * _C4 * t12 - 0.2e1 / (double) (t11 + 4 * t33) * del_rho / t40 * (double) B / kn;
		
		t5 = exp((double) (2 * B * z + U * z - U));
		t6 = R * z;
		t7 = sin((double) t6);
		t8 = t7 * (double) R;
		t9 = kn * kn;
		t11 = 0.3e1 * t8 * t9;
		t12 = cos((double) t6);
		t13 = (double) U * t12;
		t16 = U * U;
		t17 = B * t16;
		t20 = (double) B * t12;
		t21 = R * R;
		t23 = 0.2e1 * t20 * (double) t21;
		t24 = B * U;
		t28 = 0.2e1 * t20 * t9;
		t29 = t16 * U;
		t36 = t21 * R;
		t37 = t7 * (double) t36;
		t41 = 0.1e1 / t9 / kn;
		t44 = (double) U * t7;
		t47 = t12 * (double) R;
		t49 = 0.3e1 * t47 * t9;
		t50 = (double) B * t7;
		t52 = 0.2e1 * t50 * (double) t21;
		t58 = 0.2e1 * t50 * t9;
		t65 = t12 * (double) t36;
		t73 = exp((double) (z * (2 * B - VV)));
		t74 = (double) VV * t12;
		t77 = VV * VV;
		t78 = B * t77;
		t81 = B * VV;
		t84 = t77 * VV;
		t95 = (double) VV * t7;
		t112 = B * B;
		u3 = t5 * (-t11 + 0.3e1 * t13 * t9 - 0.2e1 * (double) t17 * t12 + t23 + 0.4e1 * (double) t24 * t8 - t28 - (double) t29 * t12 + 0.3e1 * t13 * (double) t21 + 0.3e1 * (double) t16 * t7 * (double) R - t37) * _C1 * t41 + t5 * (0.3e1 * t44 * t9 + t49 + t52 - 0.2e1 * (double) t17 * t7 - 0.4e1 * (double) t24 * t47 - t58 + 0.3e1 * t44 * (double) t21 - (double) t29 * t7 - 0.3e1 * (double) t16 * t12 * (double) R + t65) * _C2 * t41 + t73 * (-t11 - 0.3e1 * t74 * t9 - 0.2e1 * (double) t78 * t12 + t23 - 0.4e1 * (double) t81 * t8 - t28 + (double) t84 * t12 - 0.3e1 * t74 * (double) t21 + 0.3e1 * (double) t77 * t7 * (double) R - t37) * _C3 * t41 + t73 * (t49 - 0.3e1 * t95 * t9 - 0.2e1 * (double) t78 * t7 + t52 + 0.4e1 * (double) t81 * t47 - t58 + (double) t84 * t7 - 0.3e1 * t95 * (double) t21 - 0.3e1 * (double) t77 * t12 * (double) R + t65) * _C4 * t41 + 0.8e1 / (t9 + (double) (4 * t112)) * del_rho * (double) B;
		
		t5 = exp((double) (2 * B * z + U * z - U));
		t6 = U * U;
		t7 = R * z;
		t8 = cos((double) t7);
		t10 = R * R;
		t11 = t8 * (double) t10;
		t12 = sin((double) t7);
		t16 = kn * kn;
		t17 = t8 * t16;
		t20 = 0.1e1 / t16;
		t23 = t12 * (double) t10;
		t28 = t12 * t16;
		t36 = exp((double) (z * (2 * B - VV)));
		t37 = VV * VV;
		u4 = t5 * (-(double) t6 * t8 + t11 + 0.2e1 * (double) U * t12 * (double) R - t17) * _C1 * t20 - t5 * (-t23 + (double) t6 * t12 + 0.2e1 * (double) U * t8 * (double) R + t28) * _C2 * t20 + t36 * (-(double) t37 * t8 + t11 - 0.2e1 * (double) VV * t12 * (double) R - t17) * _C3 * t20 + t36 * (-(double) t37 * t12 + t23 + 0.2e1 * (double) VV * t8 * (double) R - t28) * _C4 * t20 + del_rho / kn;
		
		
		
		pp = (double)-(u3 + 2.0*n*M_PI*exp(2.0*B*z)*u2)*cos(n*M_PI*x); /* pressure */
		
		txx = (double)(u3 + 4*exp(2.0*B*z)*n*M_PI*u2)*cos(n*M_PI*x); /* xx stress */	    
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
		
		rho = del_rho* cos(n*M_PI*x); /* density */
		sum7 += rho;
	}/* n */
	
	/* include terms for n=0 now */
	del_rho = sigma*dx;
	rho = del_rho; /* density */
	sum7 += rho;
	
	u3 = del_rho*(z-0.5); /* zz stress */
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
		Z = exp(2.0*B*z);
		strain_rate[0] = (sum6+sum5)/(2.0*Z);
		strain_rate[1] = (sum3+sum5)/(2.0*Z);
		strain_rate[2] = (sum4)/(2.0*Z);
	}
	/* Value checks, could be cleaned up if needed. Julian Giordani 9-Oct-2006*/
//        if( fabs( sum5 - ( -0.5*(sum6+sum3) ) ) > 1e-5 ) {
//                assert(0);
//        }
	
}


