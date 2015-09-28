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


void _Velic_solG( 
		double pos[],
		double _sigma,
		double _eta_A, double _eta_B, 
		double _dx, double _x_0, double _z_c,
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] );

#ifndef NOSHARED
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
			_Velic_solG(
					pos,
					1.0,
					10.0, 1.0,
					0.4,0.55,0.7,
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


void _Velic_solG(
		double pos[],
		double _sigma,
		double _eta_A, double _eta_B, 
		double _dx, double _x_0, double _z_c,
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] )
{
	double Z,ZA,ZB,u1,u2,u3,u4,pp,txx;
	double u1a,u2a,u3a,u4a,u1b,u2b,u3b,u4b;
	double sum1,sum2,sum3,sum4,sum5,sum6,mag,sum7,x,z;
	double sigma;
	double del_rho;
	int n;
	double kn;
	double _C1A,_C2A,_C3A,_C4A,_C1B,_C2B,_C3B,_C4B;
	double x0,dx,rho,zc;
	
	double t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
	double t11,t12,t13,t14,t15,t16,t17,t18,t19,t20;
	double t21,t22,t23,t24,t25,t26,t27,t28,t29,t30;
	double t31,t32,t33,t34,t35,t36,t37,t38,t39,t40;
	double t41,t42,t43,t44,t45,t46,t47,t48,t49,t50;
	double t51,t52,t53,t54,t55,t56,t57,t58,t59,t60;
	double t61,t62,t63,t64,t65,t66,t67,t68,t69,t70;
	double t71,t72,t73,t74,t75,t76,t77,t78,t79,t80;
	double t81,t82,t83,t85,t86,t87,t88,t89,t90,t91;
	double t92,t93,t94,t95,t96,t97,t98,t99,t100,t101;
	double t102,t103,t104,t105,t106,t107,t108,t109,t111,t112;
	double t113,t114,t115,t116,t117,t118,t120,t121,t122,t124;
	double t125,t126,t127,t128,t129,t130,t131,t133,t134,t135;
	double t136,t137,t139,t141,t143,t144,t145,t146,t147,t148;
	double t151,t152,t153,t154,t156,t157,t158,t159,t162,t163;
	double t165,t167,t168,t190,t195;
	
	/*************************************************************************/
	/*************************************************************************/
	/*
	In layer B (the top layer) we have a dense column of density sigma
	ranging from x=0 to x= xc:
	The viscosity of layer B is ZB:
	The viscosity of layer A is ZA:
	
	*/
	/*************************************************************************/
	/*************************************************************************/
	
	/*************************************************************************/
	/*************************************************************************/
	x0 = _x_0;  /* centre of dense block in layer B */
	dx =  _dx;  /* width of dense block in layer B */
	sigma = _sigma; /* density parameter for layer B */
	ZA = _eta_A;    /* viscosity for layer A */
	ZB = _eta_B; /* viscosity for layer B */
	zc = _z_c;  /* dividing line between region B and region A */
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
	/* this one blows up when n gets around 85 for some reason... if max n is set to 75 it is fine though */
	/* fixed the convergence problem */
	for(n=1;n<85;n++){
		kn = (double) n*M_PI;
		
		/*******************************************/
		/*            calculate del_rho            */
		/*******************************************/
		del_rho = 4.0*sigma*cos(n*M_PI*x0)*sin(n*M_PI*dx/2.0)/n/M_PI;
		
		/*******************************************/
		/*         calculate the constants         */
		/*******************************************/
		
		if ( z < zc ) {
			t1 = kn * zc;
			t2 = 0.4e1 * t1;
			t3 = kn * kn;
			t5 = 0.2e1 * t3 * zc;
			t6 = zc * zc;
			t7 = t6 * t3;
			t8 = 0.2e1 * t7;
			t10 = ZA - ZB;
			t12 = 0.2e1 * zc;
			t15 = exp(-kn * (0.1e1 + t12));
			t22 = exp(kn * (t12 - 0.3e1));
			t26 = ZA + ZB;
			t29 = exp(-0.3e1 * kn);
			t34 = exp(-kn);
			t37 = 0.3e1 * t1;
			t40 = exp(-t37);
			t44 = exp(-t1);
			t50 = exp(kn * (zc - 0.4e1));
			t54 = 0.2e1 * ZA * kn * zc;
			t56 = 0.2e1 * ZB * kn;
			t57 = 0.2e1 * ZA;
			t58 = t3 * ZA;
			t60 = 0.2e1 * t58 * zc;
			t63 = 0.3e1 * ZB * zc * kn;
			t64 = t3 * ZB;
			t66 = 0.2e1 * t64 * zc;
			t68 = 0.2e1 * t64 * t6;
			t70 = 0.2e1 * t58 * t6;
			t73 = kn * (zc - 0.2e1);
			t74 = exp(t73);
			t82 = exp(kn * (0.3e1 * zc - 0.4e1));
			t87 = exp(-kn * (0.2e1 + zc));
			t92 = t10 * t10;
			t93 = t3 * t92;
			t94 = exp(-t2);
			t97 = t26 * t26;
			t100 = exp(-0.4e1 * kn);
			t103 = t3 * kn;
			t104 = t103 * t10;
			t105 = zc - 0.1e1;
			t106 = t26 * t105;
			t107 = kn * t105;
			t109 = exp(0.2e1 * t107);
			t114 = exp(0.4e1 * t107);
			t117 = t103 * zc;
			t118 = t10 * t26;
			t120 = exp(0.2e1 * t73);
			t124 = ZA * ZA;
			t125 = t3 * t124;
			t128 = ZB * ZB;
			t136 = ZA * ZB;
			t145 = exp(-0.2e1 * kn);
			t151 = exp(-0.2e1 * kn * (0.1e1 + zc));
			t156 = exp(-0.2e1 * t1);
			t165 = -0.2e1 * t93 * t94 + 0.2e1 * t3 * t97 * t100 + 0.8e1 * t104 * t106 * t109 - 0.2e1 * t93 * t114 - 0.8e1 * t117 * t118 * t120 + 0.16e2 * t3 * (-0.2e1 * t125 * zc + 0.2e1 * t7 * t128 + 0.2e1 * t7 * t124 - 0.2e1 * t128 * zc * t3 - t136 - 0.4e1 * t7 * t136 + 0.4e1 * t64 * ZA * zc) * t145 - 0.8e1 * t104 * t106 * t151 + 0.8e1 * t117 * t118 * t156 + 0.4e1 * t64 * ZA + 0.2e1 * t3 * t128 + 0.2e1 * t125;
			_C1A = (0.2e1 * (-t2 - t5 + t8 + 0.2e1 + kn) * t10 * t15 - 0.2e1 * (0.2e1 - t5 + t2 + t8 - kn) * t10 * t22 - 0.2e1 * (-0.2e1 + kn) * t26 * t29 - 0.2e1 * (kn + 0.2e1) * t26 * t34 + (t37 - 0.2e1) * t10 * t40 + (t1 + 0.2e1) * t26 * t44 + (t1 - 0.2e1) * t26 * t50 + 0.2e1 * (t54 + t56 + t57 - t60 - t63 + t66 - t68 + t70) * t74 + (t37 + 0.2e1) * t10 * t82 - 0.2e1 * (-t68 + t63 - t60 + t70 - t56 + t57 - t54 + t66) * t87) * del_rho / t165;
			
			t1 = kn * kn;
			t3 = 0.2e1 * t1 * zc;
			t4 = kn * zc;
			t5 = 0.4e1 * t4;
			t6 = zc * zc;
			t7 = t6 * t1;
			t8 = 0.2e1 * t7;
			t10 = ZA - ZB;
			t12 = zc - 0.1e1;
			t13 = kn * t12;
			t15 = exp(0.3e1 * t13);
			t19 = ZA + ZB;
			t24 = exp(0.2e1 * kn * (zc - 0.2e1));
			t26 = t1 * ZB;
			t28 = 0.2e1 * t26 * t6;
			t30 = ZB * zc * kn;
			t31 = 0.3e1 * t30;
			t32 = t1 * ZA;
			t34 = 0.2e1 * t32 * zc;
			t36 = 0.2e1 * t32 * t6;
			t38 = 0.2e1 * ZB * kn;
			t39 = 0.2e1 * ZA;
			t41 = ZA * kn * zc;
			t42 = 0.2e1 * t41;
			t44 = 0.2e1 * t26 * zc;
			t47 = exp(-0.2e1 * kn);
			t52 = exp(t13);
			t57 = exp(0.2e1 * t13);
			t64 = exp(kn * (zc - 0.3e1));
			t67 = 0.3e1 * t4;
			t71 = exp(-0.2e1 * t4);
			t76 = kn * (0.1e1 + zc);
			t77 = exp(-t76);
			t83 = exp(0.4e1 * t13);
			t86 = 0.2e1 * (0.2e1 - t3 + t5 + t8 - kn) * t10 * t15 - (t4 - 0.2e1) * t19 * t24 + 0.2e1 * (-t28 + t31 - t34 + t36 - t38 + t39 - t42 + t44) * t47 + 0.2e1 * (kn + 0.2e1) * t19 * t52 - 0.2e1 * (t42 + t38 + t39 - t34 - t31 + t44 - t28 + t36) * t57 + 0.2e1 * (-0.2e1 + kn) * t19 * t64 - (t67 - 0.2e1) * t10 * t71 - 0.2e1 * (-t5 - t3 + t8 + 0.2e1 + kn) * t10 * t77 - (t67 + 0.2e1) * t10 * t83 - 0.2e1 * ZB - t39 - t30 - t41;
			t88 = t10 * t10;
			t89 = t1 * t88;
			t90 = exp(-t5);
			t93 = t19 * t19;
			t96 = exp(-0.4e1 * kn);
			t99 = t1 * kn;
			t100 = t99 * t10;
			t101 = t19 * t12;
			t107 = t99 * zc;
			t108 = t10 * t19;
			t112 = ZA * ZA;
			t113 = t1 * t112;
			t116 = ZB * ZB;
			t124 = ZA * ZB;
			t135 = exp(-0.2e1 * t76);
			t147 = -0.2e1 * t89 * t90 + 0.2e1 * t1 * t93 * t96 + 0.8e1 * t100 * t101 * t57 - 0.2e1 * t89 * t83 - 0.8e1 * t107 * t108 * t24 + 0.16e2 * t1 * (-0.2e1 * t113 * zc + 0.2e1 * t7 * t116 + 0.2e1 * t7 * t112 - 0.2e1 * t116 * zc * t1 - t124 - 0.4e1 * t7 * t124 + 0.4e1 * t26 * ZA * zc) * t47 - 0.8e1 * t100 * t101 * t135 + 0.8e1 * t107 * t108 * t71 + 0.4e1 * t26 * ZA + 0.2e1 * t1 * t116 + 0.2e1 * t113;
			_C2A = t86 * del_rho / t147;
			
			t1 = kn * zc;
			t2 = 0.2e1 * t1;
			t3 = 0.2e1 * kn;
			t5 = ZA - ZB;
			t7 = 0.2e1 * zc;
			t10 = exp(-kn * (0.1e1 + t7));
			t17 = exp(kn * (t7 - 0.3e1));
			t20 = ZA + ZB;
			t22 = exp(-0.3e1 * kn);
			t25 = exp(-kn);
			t29 = exp(-0.3e1 * t1);
			t32 = exp(-t1);
			t36 = exp(kn * (zc - 0.4e1));
			t38 = ZA * kn;
			t40 = 0.2e1 * t38 * zc;
			t41 = 0.2e1 * t38;
			t43 = 0.2e1 * ZB * kn;
			t44 = 0.2e1 * ZA;
			t47 = 0.2e1 * ZB * zc * kn;
			t50 = kn * (zc - 0.2e1);
			t51 = exp(t50);
			t57 = exp(kn * (0.3e1 * zc - 0.4e1));
			t63 = exp(-kn * (0.2e1 + zc));
			t68 = t5 * t5;
			t69 = kn * t68;
			t71 = exp(-0.4e1 * t1);
			t74 = t20 * t20;
			t77 = exp(-0.4e1 * kn);
			t80 = kn * kn;
			t81 = t80 * t5;
			t82 = zc - 0.1e1;
			t83 = t20 * t82;
			t87 = exp(-0.2e1 * kn * (0.1e1 + zc));
			t91 = t80 * zc;
			t92 = t5 * t20;
			t94 = exp(0.2e1 * t50);
			t98 = ZA * ZA;
			t102 = zc * zc;
			t103 = t102 * t80;
			t104 = ZB * ZB;
			t112 = ZA * ZB;
			t121 = exp(-t3);
			t124 = kn * t82;
			t126 = exp(0.2e1 * t124);
			t130 = exp(-t2);
			t135 = exp(0.4e1 * t124);
			t144 = -0.2e1 * t69 * t71 + 0.2e1 * t74 * kn * t77 - 0.8e1 * t81 * t83 * t87 - 0.8e1 * t91 * t92 * t94 + 0.16e2 * kn * (-0.2e1 * t80 * t98 * zc + 0.2e1 * t103 * t104 + 0.2e1 * t103 * t98 - 0.2e1 * t104 * zc * t80 - t112 - 0.4e1 * t103 * t112 + 0.4e1 * ZB * t80 * ZA * zc) * t121 + 0.8e1 * t81 * t83 * t126 + 0.8e1 * t91 * t92 * t130 - 0.2e1 * t69 * t135 + 0.2e1 * kn * t104 + 0.2e1 * t98 * kn + 0.4e1 * t112 * kn;
			_C3A = (-0.2e1 * (t2 - 0.3e1 - t3) * t5 * t10 - 0.2e1 * (t2 + 0.3e1 - t3) * t5 * t17 + 0.2e1 * t20 * t22 - 0.2e1 * t20 * t25 - 0.3e1 * t5 * t29 + t20 * t32 - t20 * t36 + 0.2e1 * (t40 - t41 + t43 + t44 - ZB - t47) * t51 + 0.3e1 * t5 * t57 + 0.2e1 * (-t41 + t43 + ZB - t47 + t40 - t44) * t63) * del_rho / t144;
			
			t1 = ZA + ZB;
			t5 = exp(0.2e1 * kn * (zc - 0.2e1));
			t7 = ZA * kn;
			t8 = 0.2e1 * t7;
			t10 = 0.2e1 * ZB * kn;
			t13 = 0.2e1 * ZB * zc * kn;
			t15 = 0.2e1 * t7 * zc;
			t16 = 0.2e1 * ZA;
			t18 = 0.2e1 * kn;
			t19 = exp(-t18);
			t24 = exp(kn * (zc - 0.3e1));
			t27 = kn * zc;
			t28 = 0.2e1 * t27;
			t30 = ZA - ZB;
			t33 = kn * (0.1e1 + zc);
			t34 = exp(-t33);
			t38 = zc - 0.1e1;
			t39 = kn * t38;
			t41 = exp(0.2e1 * t39);
			t47 = exp(0.3e1 * t39);
			t50 = exp(-t28);
			t53 = exp(t39);
			t57 = exp(0.4e1 * t39);
			t60 = -t1 * t5 + 0.2e1 * (-t8 + t10 + ZB - t13 + t15 - t16) * t19 + 0.2e1 * t1 * t24 - 0.2e1 * (t28 - 0.3e1 - t18) * t30 * t34 + 0.2e1 * (t15 - t8 + t10 + t16 - ZB - t13) * t41 - 0.2e1 * (t28 + 0.3e1 - t18) * t30 * t47 - 0.3e1 * t30 * t50 - 0.2e1 * t1 * t53 + 0.3e1 * t30 * t57 + ZA + ZB;
			t62 = t30 * t30;
			t63 = kn * t62;
			t65 = exp(-0.4e1 * t27);
			t68 = t1 * t1;
			t71 = exp(-0.4e1 * kn);
			t74 = kn * kn;
			t75 = t74 * t30;
			t76 = t1 * t38;
			t78 = exp(-0.2e1 * t33);
			t82 = t74 * zc;
			t83 = t30 * t1;
			t87 = ZA * ZA;
			t91 = zc * zc;
			t92 = t91 * t74;
			t93 = ZB * ZB;
			t101 = ZA * ZB;
			t126 = -0.2e1 * t63 * t65 + 0.2e1 * t68 * kn * t71 - 0.8e1 * t75 * t76 * t78 - 0.8e1 * t82 * t83 * t5 + 0.16e2 * kn * (-0.2e1 * t74 * t87 * zc + 0.2e1 * t92 * t93 + 0.2e1 * t92 * t87 - 0.2e1 * t93 * zc * t74 - t101 - 0.4e1 * t92 * t101 + 0.4e1 * ZB * t74 * ZA * zc) * t19 + 0.8e1 * t75 * t76 * t41 + 0.8e1 * t82 * t83 * t50 - 0.2e1 * t63 * t57 + 0.2e1 * kn * t93 + 0.2e1 * t87 * kn + 0.4e1 * t101 * kn;
			_C4A = t60 * del_rho / t126;
		} else { 
			t1 = ZA - ZB;
			t2 = ZA * kn;
			t4 = 0.2e1 * t2 * zc;
			t5 = 0.2e1 * ZA;
			t7 = ZB * zc * kn;
			t10 = kn * zc;
			t11 = 0.4e1 * t10;
			t12 = exp(-t11);
			t14 = ZA + ZB;
			t15 = 0.4e1 * t2;
			t17 = 0.2e1 * ZB * kn;
			t21 = kn * (0.1e1 + zc);
			t23 = exp(-0.2e1 * t21);
			t25 = kn * kn;
			t26 = ZA * ZA;
			t28 = t25 * t26 * zc;
			t30 = zc * zc;
			t31 = t30 * t25;
			t32 = t31 * t26;
			t33 = 0.4e1 * t32;
			t34 = ZB * t25;
			t36 = t34 * ZA * zc;
			t39 = t26 * zc * kn;
			t40 = 0.4e1 * t39;
			t41 = ZA * ZB;
			t42 = t41 * kn;
			t44 = ZB * ZB;
			t45 = t44 * zc;
			t46 = t45 * t25;
			t48 = t41 * t10;
			t49 = 0.3e1 * t48;
			t50 = t31 * t44;
			t51 = 0.2e1 * t50;
			t52 = 0.2e1 * t41;
			t53 = t31 * t41;
			t54 = 0.6e1 * t53;
			t57 = exp(-0.2e1 * kn);
			t64 = (-t11 - 0.2e1 * t25 * zc + 0.2e1 * t31 + 0.2e1 + kn) * t1;
			t68 = exp(-kn * (0.1e1 + 0.3e1 * zc));
			t72 = -0.2e1 + kn;
			t76 = exp(kn * (zc - 0.3e1));
			t80 = t25 * kn;
			t81 = t30 * t80;
			t85 = t30 * zc;
			t91 = t45 * kn;
			t97 = 0.2e1 * t39;
			t106 = -0.2e1 * t81 * t44 + 0.2e1 * t44 * t80 * t85 + 0.2e1 * t26 * t80 * t85 + t52 + t42 + 0.2e1 * t91 + 0.8e1 * t53 + 0.4e1 * t81 * t41 - 0.4e1 * t48 + t46 + t97 - 0.2e1 * t36 - t33 - 0.4e1 * t41 * t80 * t85 - 0.4e1 * t50 - 0.2e1 * t26 * t30 * t80 + t28;
			t107 = exp(-t21);
			t112 = zc - 0.1e1;
			t113 = kn * t112;
			t115 = exp(0.2e1 * t113);
			t117 = t1 * t1;
			t120 = exp(0.3e1 * t113);
			t124 = exp(-0.2e1 * t10);
			t127 = t14 * t14;
			t131 = exp(-kn * (zc + 0.3e1));
			t133 = exp(t113);
			t137 = t1 * (t4 - t5 + t7) * t12 - t14 * (t5 + t4 - t15 - t7 + t17) * t23 + 0.2e1 * (-0.8e1 * t28 + t33 + 0.12e2 * t36 + t40 + 0.2e1 * t42 - 0.4e1 * t46 - t49 + t51 - t52 - t54) * t57 + t64 * t14 * t68 + 0.4e1 * t10 * t1 * t14 * t72 * t76 - 0.4e1 * t106 * t107 + t1 * (t5 + t4 - t15 + t7 - t17) * t115 + t117 * t72 * t120 - 0.2e1 * (t49 - t54 + t33 + t51 - t52 - t40) * t124 - t127 * t72 * t131 - t64 * t14 * t133 - t48 + 0.2e1 * t26 - t97 + t52 + t91;
			t143 = exp(-0.4e1 * kn);
			t148 = exp(0.4e1 * t113);
			t152 = t80 * ZB;
			t153 = t152 * zc;
			t154 = t1 * t14;
			t158 = exp(0.2e1 * kn * (zc - 0.2e1));
			t162 = t152 * t1;
			t163 = t14 * t112;
			t190 = -0.2e1 * t34 * t117 * t12 + 0.2e1 * t34 * t127 * t143 - 0.2e1 * t34 * t117 * t148 - 0.8e1 * t153 * t154 * t158 - 0.8e1 * t162 * t163 * t23 + 0.16e2 * t34 * (-0.2e1 * t28 + t51 + 0.2e1 * t32 - 0.2e1 * t46 - t41 - 0.4e1 * t53 + 0.4e1 * t36) * t57 + 0.8e1 * t162 * t163 * t115 + 0.8e1 * t153 * t154 * t124 + 0.2e1 * t25 * t44 * ZB + 0.2e1 * t34 * t26 + 0.4e1 * t25 * t44 * ZA;
			_C1B = t137 * del_rho / t190;
			
			t1 = ZA - ZB;
			t2 = t1 * t1;
			t3 = kn + 0.2e1;
			t5 = kn * zc;
			t6 = 0.4e1 * t5;
			t7 = exp(-t6);
			t9 = kn * kn;
			t12 = zc * zc;
			t13 = t12 * t9;
			t16 = (0.2e1 - 0.2e1 * t9 * zc + t6 + 0.2e1 * t13 - kn) * t1;
			t17 = ZA + ZB;
			t19 = kn * (0.1e1 + zc);
			t21 = exp(-0.2e1 * t19);
			t24 = ZA * ZA;
			t25 = t9 * kn;
			t27 = t12 * zc;
			t33 = ZA * ZB;
			t37 = t12 * t25;
			t40 = ZB * ZB;
			t46 = t13 * t24;
			t47 = 0.4e1 * t46;
			t49 = t9 * t24 * zc;
			t50 = t13 * t33;
			t52 = ZB * t9;
			t54 = t52 * ZA * zc;
			t56 = t13 * t40;
			t58 = t40 * zc;
			t59 = t58 * t9;
			t61 = t24 * zc * kn;
			t63 = t33 * kn;
			t64 = t33 * t5;
			t68 = 0.2e1 * t33;
			t69 = 0.2e1 * t24 * t25 * t27 - 0.2e1 * t24 * t12 * t25 - 0.4e1 * t33 * t25 * t27 + 0.4e1 * t37 * t33 + 0.2e1 * t40 * t25 * t27 - 0.2e1 * t37 * t40 + t47 - t49 - 0.8e1 * t50 + 0.2e1 * t54 + 0.4e1 * t56 - t59 + 0.2e1 * t61 + t63 - 0.4e1 * t64 + 0.2e1 * t58 * kn - t68;
			t71 = exp(-0.2e1 * kn);
			t74 = ZA * kn;
			t75 = 0.4e1 * t74;
			t77 = 0.2e1 * t74 * zc;
			t78 = 0.2e1 * ZA;
			t80 = ZB * zc * kn;
			t82 = 0.2e1 * ZB * kn;
			t88 = exp(-kn * (0.1e1 + 0.3e1 * zc));
			t90 = 0.6e1 * t50;
			t91 = 0.2e1 * t56;
			t92 = 0.3e1 * t64;
			t93 = 0.4e1 * t61;
			t97 = exp(kn * (zc - 0.3e1));
			t103 = 0.2e1 * t63;
			t105 = exp(-t19);
			t108 = zc - 0.1e1;
			t109 = kn * t108;
			t111 = exp(0.2e1 * t109);
			t117 = exp(0.3e1 * t109);
			t122 = exp(-0.2e1 * t5);
			t130 = exp(-kn * (zc + 0.3e1));
			t134 = exp(t109);
			t141 = -t2 * t3 * t7 - t16 * t17 * t21 + 0.4e1 * t69 * t71 - t1 * (-t75 + t77 - t78 + t80 - t82) * t88 - 0.2e1 * (-t90 + t91 + t47 - t92 + t93 - t68) * t97 + 0.2e1 * (t47 - 0.8e1 * t49 - t90 + 0.12e2 * t54 - 0.4e1 * t59 + t91 - t93 - t103 + t92 - t68) * t105 + t16 * t17 * t111 - t1 * (t77 + t78 + t80) * t117 + 0.4e1 * t5 * t1 * t17 * t3 * t122 + t17 * (t77 + t78 - t80) * t130 + t17 * (-t75 + t77 - t78 - t80 + t82) * t134 + kn * t40 + t103 + 0.2e1 * t40 + t24 * kn + 0.2e1 * t24 + 0.4e1 * t33;
			t146 = t17 * t17;
			t148 = exp(-0.4e1 * kn);
			t153 = exp(0.4e1 * t109);
			t157 = ZB * t25;
			t158 = t157 * zc;
			t159 = t1 * t17;
			t163 = exp(0.2e1 * kn * (zc - 0.2e1));
			t167 = t157 * t1;
			t168 = t17 * t108;
			t195 = -0.2e1 * t52 * t2 * t7 + 0.2e1 * t52 * t146 * t148 - 0.2e1 * t52 * t2 * t153 - 0.8e1 * t158 * t159 * t163 - 0.8e1 * t167 * t168 * t21 + 0.16e2 * t52 * (-0.2e1 * t49 + t91 + 0.2e1 * t46 - 0.2e1 * t59 - t33 - 0.4e1 * t50 + 0.4e1 * t54) * t71 + 0.8e1 * t167 * t168 * t111 + 0.8e1 * t158 * t159 * t122 + 0.2e1 * t9 * t40 * ZB + 0.2e1 * t52 * t24 + 0.4e1 * t9 * t40 * ZA;
			_C2B = t141 * del_rho / t195;
			
			t1 = 0.2e1 * ZA;
			t3 = ZA - ZB;
			t4 = (t1 + ZB) * t3;
			t5 = kn * zc;
			t7 = exp(-0.4e1 * t5);
			t9 = ZA + ZB;
			t13 = kn * (0.1e1 + zc);
			t15 = exp(-0.2e1 * t13);
			t17 = ZA * ZB;
			t19 = 0.6e1 * t17 * t5;
			t20 = ZB * ZB;
			t21 = zc * t20;
			t22 = t21 * kn;
			t23 = 0.2e1 * t22;
			t24 = ZA * ZA;
			t26 = t24 * zc * kn;
			t27 = 0.4e1 * t26;
			t29 = 0.2e1 * kn;
			t30 = exp(-t29);
			t33 = t3 * t9;
			t34 = 0.2e1 * t5;
			t35 = t34 - 0.3e1 - t29;
			t39 = exp(-kn * (0.1e1 + 0.3e1 * zc));
			t44 = exp(kn * (zc - 0.3e1));
			t48 = kn * kn;
			t51 = 0.2e1 * t48 * t24 * zc;
			t52 = zc * zc;
			t53 = t52 * t48;
			t55 = 0.2e1 * t53 * t24;
			t56 = ZB * t48;
			t59 = 0.4e1 * t56 * ZA * zc;
			t61 = 0.4e1 * t53 * t17;
			t63 = 0.2e1 * t21 * t48;
			t65 = 0.2e1 * t53 * t20;
			t69 = exp(-t13);
			t72 = zc - 0.1e1;
			t73 = kn * t72;
			t75 = exp(0.2e1 * t73);
			t77 = t3 * t3;
			t79 = exp(0.3e1 * t73);
			t82 = exp(-t34);
			t85 = t9 * t9;
			t88 = exp(-kn * (zc + 0.3e1));
			t90 = exp(t73);
			t94 = -t4 * t7 - t9 * (t1 - ZB) * t15 + 0.2e1 * (-t19 + t23 + t27 - t17) * t30 - t33 * t35 * t39 - 0.4e1 * t5 * t33 * t44 + 0.4e1 * (-t51 + t55 + t59 - t61 - t63 + t65 - 0.3e1 * t26 - 0.3e1 * t22 + t19 - t17) * t69 + t4 * t75 - t77 * t79 + 0.2e1 * (-t19 + t23 + t27 + t17) * t82 + t85 * t88 + t33 * t35 * t90 + 0.2e1 * t24 - t20 + t17;
			t96 = ZB * kn;
			t101 = exp(-0.4e1 * kn);
			t105 = t56 * t3;
			t106 = t9 * t72;
			t117 = t56 * zc;
			t121 = exp(0.2e1 * kn * (zc - 0.2e1));
			t129 = exp(0.4e1 * t73);
			t141 = -0.2e1 * t96 * t77 * t7 + 0.2e1 * t96 * t85 * t101 - 0.8e1 * t105 * t106 * t15 + 0.16e2 * t96 * (-t51 + t65 + t55 - t63 - t17 - t61 + t59) * t30 + 0.8e1 * t105 * t106 * t75 - 0.8e1 * t117 * t33 * t121 + 0.8e1 * t117 * t33 * t82 - 0.2e1 * t96 * t77 * t129 + 0.2e1 * t96 * t24 + 0.2e1 * t20 * ZB * kn + 0.4e1 * t20 * kn * ZA;
			_C3B = t94 * del_rho / t141;
			
			t1 = ZA - ZB;
			t2 = t1 * t1;
			t3 = kn * zc;
			t5 = exp(-0.4e1 * t3);
			t6 = t2 * t5;
			t7 = ZA + ZB;
			t8 = t1 * t7;
			t9 = 0.2e1 * t3;
			t10 = 0.2e1 * kn;
			t11 = t9 + 0.3e1 - t10;
			t13 = kn * (0.1e1 + zc);
			t15 = exp(-0.2e1 * t13);
			t18 = ZA * ZB;
			t20 = 0.6e1 * t18 * t3;
			t21 = kn * kn;
			t22 = ZB * t21;
			t25 = 0.4e1 * t22 * ZA * zc;
			t26 = ZB * ZB;
			t27 = t26 * zc;
			t29 = 0.2e1 * t27 * t21;
			t30 = ZA * ZA;
			t32 = t30 * zc * kn;
			t34 = zc * zc;
			t35 = t34 * t21;
			t37 = 0.2e1 * t35 * t30;
			t39 = 0.2e1 * t35 * t26;
			t41 = 0.4e1 * t35 * t18;
			t44 = 0.2e1 * t21 * t30 * zc;
			t45 = t27 * kn;
			t48 = exp(-t10);
			t51 = 0.2e1 * ZA;
			t53 = (t51 + ZB) * t1;
			t57 = exp(-kn * (0.1e1 + 0.3e1 * zc));
			t59 = 0.2e1 * t45;
			t60 = 0.4e1 * t32;
			t64 = exp(kn * (zc - 0.3e1));
			t68 = exp(-t13);
			t71 = zc - 0.1e1;
			t72 = kn * t71;
			t74 = exp(0.2e1 * t72);
			t78 = exp(0.3e1 * t72);
			t80 = exp(-t9);
			t81 = t8 * t80;
			t85 = t7 * (t51 - ZB);
			t88 = exp(-kn * (zc + 0.3e1));
			t90 = exp(t72);
			t93 = t6 + t8 * t11 * t15 - 0.4e1 * (-t18 - t20 + t25 - t29 + 0.3e1 * t32 + t37 + t39 - t41 - t44 + 0.3e1 * t45) * t48 - t53 * t57 + 0.2e1 * (-t20 + t59 + t60 - t18) * t64 + 0.2e1 * (-t20 + t59 + t60 + t18) * t68 - t8 * t11 * t74 + t53 * t78 - 0.4e1 * t3 * t81 - t85 * t88 + t85 * t90 - 0.2e1 * t18 - t26 - t30;
			t95 = ZB * kn;
			t98 = t7 * t7;
			t100 = exp(-0.4e1 * kn);
			t104 = t22 * t1;
			t105 = t7 * t71;
			t116 = t22 * zc;
			t120 = exp(0.2e1 * kn * (zc - 0.2e1));
			t127 = exp(0.4e1 * t72);
			t139 = -0.2e1 * t95 * t6 + 0.2e1 * t95 * t98 * t100 - 0.8e1 * t104 * t105 * t15 + 0.16e2 * t95 * (-t44 + t39 + t37 - t29 - t18 - t41 + t25) * t48 + 0.8e1 * t104 * t105 * t74 - 0.8e1 * t116 * t8 * t120 + 0.8e1 * t116 * t81 - 0.2e1 * t95 * t2 * t127 + 0.2e1 * t95 * t30 + 0.2e1 * t26 * ZB * kn + 0.4e1 * t26 * kn * ZA;
			_C4B = t93 * del_rho / t139;
			
			
		}
		
		/*******************************************/
		/*       calculate the velocities etc      */
		/*******************************************/
		
		if ( z < zc ) {
			t4 = exp(-kn * z);
			t10 = exp(kn * (z - zc));
			u1=u1a = (_C1A + z * _C3A) * t4 + (_C2A + z * _C4A) * t10;
			
			t1 = kn * z;
			t2 = exp(-t1);
			t6 = exp(kn * (z - zc));
			t10 = 0.1e1 / kn;
			u2=u2a = t2 * _C1A - t6 * _C2A + t2 * (-0.1e1 + t1) * _C3A * t10 - t6 * (0.1e1 + t1) * _C4A * t10;
			
			t1 = ZA * kn;
			t3 = exp(-kn * z);
			t8 = exp(kn * (z - zc));
			u3=u3a = -0.2e1 * t1 * t3 * _C1A + 0.2e1 * t1 * t8 * _C2A - 0.2e1 * t1 * t3 * z * _C3A + 0.2e1 * t1 * t8 * z * _C4A;
			
			t2 = kn * z;
			t6 = exp(-t2);
			t14 = exp(kn * (z - zc));
			u4=u4a = -0.2e1 * ZA * (-_C3A + kn * _C1A + t2 * _C3A) * t6 - 0.2e1 * ZA * (_C4A + kn * _C2A + t2 * _C4A) * t14;
			
			Z=ZA;
			del_rho = 0.0;
			
		} else {
			t5 = exp(kn * (zc - z));
			t11 = exp(kn * (z - 0.1e1));
			t15 = kn * kn;
			u1=u1b = (_C1B + z * _C3B) * t5 + (_C2B + z * _C4B) * t11 - del_rho / ZB / t15;
			
			t3 = exp(kn * (zc - z));
			t7 = exp(kn * (z - 0.1e1));
			t9 = kn * z;
			t12 = 0.1e1 / kn;
			u2=u2b = t3 * _C1B - t7 * _C2B + t3 * (-0.1e1 + t9) * _C3B * t12 - t7 * (0.1e1 + t9) * _C4B * t12;
			
			t1 = ZB * kn;
			t4 = exp(kn * (zc - z));
			t9 = exp(kn * (z - 0.1e1));
			u3=u3b = -0.2e1 * t1 * t4 * _C1B + 0.2e1 * t1 * t9 * _C2B - 0.2e1 * t1 * t4 * z * _C3B + 0.2e1 * t1 * t9 * z * _C4B;
			
			t2 = kn * z;
			t8 = exp(kn * (zc - z));
			t17 = exp(kn * (z - 0.1e1));
			u4=u4b = -0.2e1 * ZB * (-_C3B + kn * _C1B + t2 * _C3B) * t8 - 0.2e1 * ZB * (_C4B + kn * _C2B + t2 * _C4B) * t17 + 0.1e1 / kn * del_rho;
			
			Z=ZB;
		}
		
		
		
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
		
		rho = del_rho* cos(n*M_PI*x); /* density */
		sum7 += rho;
	}/* n */
	
	/* include terms for n=0 now */
	if( z >= zc ){
		del_rho = sigma*dx;
		rho = del_rho; /* density */
		sum7 += rho;
		u3 = del_rho*(z-zc); /* zz stress */
		sum3 += u3;
		
		txx = u3; /* xx stress */
		pp = -u3; /* pressure */
		sum5 += pp;
		sum6 += txx;
		
	}
	
	mag=sqrt(sum1*sum1+sum2*sum2);
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
		if( z<zc ) {
			Z = ZA;
		}
		else {
			Z = ZB;
		}
		strain_rate[0] = (sum6+sum5)/(2.0*Z);
		strain_rate[1] = (sum3+sum5)/(2.0*Z);
		strain_rate[2] = (sum4)/(2.0*Z);
	}
	/* Value checks, could be cleaned up if needed. Julian Giordani 9-Oct-2006*/
        if( fabs( sum5 - ( -0.5*(sum6+sum3) ) ) > 1e-5 ) {
                assert(0);
        }
	
	
}


