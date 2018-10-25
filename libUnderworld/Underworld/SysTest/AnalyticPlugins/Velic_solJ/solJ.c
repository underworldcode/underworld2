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
#include "solJ.h"

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
			_Velic_solJ(
					pos,
					3.0, 1.0,
					2.0, 1.0,
					0.4, 0.3,
					0.3, 0.6,
					0.8,
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


void _Velic_solJ( 
		double pos[],
		double _sigma_B ,double _sigma_A, /* density B, density A */
		double _eta_B   ,double _eta_A  , /* viscosity B, viscosity A */
		double _dx_B    ,double _dx_A   , /* width of the upper dense block, width of the lower dense block */
		double _x_0_B   ,double _x_0_A  , /* centre of the upper dense block, centre of lower dense block */
		double _z_c, /* bottom of the upper dense block */
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] )
{
	double Z,ZA,ZB,u1,u2,u3,u4,pp,txx;
	double u1a,u2a,u3a,u4a,u1b,u2b,u3b,u4b;
	double sum1,sum2,sum3,sum4,sum5,sum6,sum7,x,z;
	double sigma,sigmaA,dx,dxA;
	double del_rhoA,del_rhoB,del_rho;
	int n;
	double kn;
	double _C1A,_C2A,_C3A,_C4A,_C1B,_C2B,_C3B,_C4B;
	double x0,x0A,rho,zc,x0B,dxB,sigmaB;
	
	double t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
	double t11,t12,t13,t14,t15,t16,t17,t18,t19,t20;
	double t21,t22,t23,t24,t25,t26,t27,t28,t29,t30;
	double t31,t32,t33,t34,t35,t36,t37,t38,t39,t40;
	double t41,t42,t43,t44,t45,t46,t47,t48,t49,t50;
	double t51,t52,t53,t54,t55,t56,t57,t58,t59,t60;
	double t61,t62,t63,t64,t66,t67,t68,t69,t70,t71;
	double t72,t73,t74,t75,t76,t77,t78,t79,t80,t81;
	double t82,t83,t84,t85,t86,t87,t88,t89,t90,t91;
	double t93,t94,t95,t96,t97,t98,t99,t100,t101,t102;
	double t103,t104,t105,t106,t107,t108,t109,t110,t111,t112;
	double t113,t114,t115,t116,t117,t118,t119,t120,t121,t122;
	double t123,t124,t125,t126,t128,t129,t130,t131,t132,t133;
	double t134,t135,t136,t137,t138,t139,t140,t141,t142,t143;
	double t144,t145,t146,t147,t148,t149,t150,t151,t152,t153;
	double t154,t155,t156,t157,t158,t159,t160,t161,t162,t163;
	double t164,t165,t166,t167,t168,t169,t170,t171,t172,t173;
	double t174,t175,t176,t178,t179,t180,t181,t183,t184,t185;
	double t186,t187,t188,t189,t191,t193,t196,t197,t200,t202;
	double t203,t206,t207,t210,t211,t215,t217,t218,t219,t221;
	double t222,t223,t224,t225,t226,t227,t228,t229,t232,t233;
	double t238,t239,t243,t244,t246,t248,t250,t251,t267,t273;
	
	
	
	
	/*************************************************************************/
	/*************************************************************************/
	x0 = x0B = _x_0_B;  /* centre of dense column for layer B */
	dx = dxB = _dx_B;  /* total width of dense column  for layer B */
	sigma = sigmaB = _sigma_B; /* density parameter for layer B (sigma_1 in _Velic_solJ.pdf) */
	sigmaA = _sigma_A; /* density parameter for layer A (sigma_0 in _Velic_solJ.pdf) */
	x0A = _x_0_A; /* centre of dense column for layer A */
	dxA = _dx_A;  /* total width of dense column  for layer A (eta_0 in _Velic_solJ.pdf) */
	ZA = _eta_A;    /* viscosity for layer A (eta_1 in _Velic_solJ.pdf) */
	ZB = _eta_B; /* viscosity for layer B */
	zc = _z_c;
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
	/* convergence is good */
	for(n=1;n<155;n++){
		kn = (double) n*M_PI;
		
		/*******************************************/
		/*            calculate del_rho            */
		/*******************************************/
		del_rhoB = 4.0*sigma*cos(n*M_PI*x0)*sin(n*M_PI*dx/2.0)/n/M_PI;
		del_rhoA = 4.0*sigmaA*cos(n*M_PI*x0A)*sin(n*M_PI*dxA/2.0)/n/M_PI;
		
		/*******************************************/
		/*         calculate the constants         */
		/*******************************************/
		
		if (z < zc) {
			t1 = -ZB + ZA;
			t2 = ZA * kn;
			t3 = t2 * zc;
			t4 = 0.2e1 * ZB;
			t5 = kn * zc;
			t7 = 0.2e1 * t5 * ZB;
			t11 = exp(-0.3e1 * t5);
			t13 = ZA + ZB;
			t14 = t1 * t13;
			t16 = pow(-0.1e1 + t5, 0.2e1);
			t18 = kn * (-0.2e1 + zc);
			t20 = exp(0.2e1 * t18);
			t25 = zc - 0.1e1;
			t26 = t13 * t25;
			t27 = kn * t25;
			t29 = exp(0.2e1 * t27);
			t30 = t26 * t29;
			t33 = kn * kn;
			t34 = t33 * kn;
			t35 = ZA * ZA;
			t37 = zc * zc;
			t38 = t37 * zc;
			t40 = t34 * t37;
			t42 = ZB * ZB;
			t43 = t33 * t42;
			t44 = t43 * t37;
			t45 = 0.2e1 * t44;
			t46 = t33 * t35;
			t48 = 0.2e1 * t46 * zc;
			t51 = 0.2e1 * t46 * t37;
			t52 = ZA * ZB;
			t53 = t33 * ZA;
			t55 = t53 * t37 * ZB;
			t56 = 0.4e1 * t55;
			t57 = kn * t42;
			t58 = ZB * zc;
			t59 = t53 * t58;
			t60 = 0.4e1 * t59;
			t61 = t43 * zc;
			t62 = 0.2e1 * t61;
			t63 = t34 * ZA;
			t67 = kn * t35;
			t73 = t2 * t58;
			t75 = t57 * zc;
			t76 = t2 * ZB;
			t77 = t34 * t35 * t38 - t40 * t35 - t45 + t48 - t40 * t42 - t51 + t52 + t56 - t57 - t60 + t62 - 0.2e1 * t63 * ZB * t38 + t67 * zc + 0.2e1 * t40 * t52 + t34 * t42 * t38 - t67 - 0.2e1 * t73 + t75 + t76;
			t79 = exp(-0.2e1 * kn);
			t82 = 0.6e1 * t55;
			t83 = 0.2e1 * t52;
			t84 = 0.4e1 * t44;
			t85 = 0.3e1 * t73;
			t86 = 0.4e1 * t57;
			t87 = 0.2e1 * t76;
			t88 = 0.4e1 * t61;
			t89 = 0.6e1 * t59;
			t90 = 0.4e1 * t75;
			t91 = -t82 - t83 + t84 + t51 + t85 + t86 - t87 - t88 + t89 - t48 - t90;
			t94 = exp(-kn * (zc + 0.2e1));
			t99 = exp(-t5);
			t101 = -t82 + t51 + t90 - t86 + t87 - t85 + t84 + t89 - t88 - t83 - t48;
			t102 = exp(t18);
			t110 = exp(kn * (0.3e1 * zc - 0.4e1));
			t113 = exp(-0.2e1 * t5);
			t121 = exp(kn * (zc - 0.4e1));
			t123 = t1 * t1;
			t125 = exp(0.4e1 * t27);
			t126 = t123 * t125;
			t131 = t1 * (-t3 + t4 - t7) * t11 + 0.2e1 * t14 * t16 * t20 + 0.8e1 * kn * t1 * t30 - 0.8e1 * t77 * t79 - 0.2e1 * t91 * t94 + t13 * (t3 - t4 - t7) * t99 + 0.2e1 * t101 * t102 + t1 * (-t3 - t4 - t7) * t110 - 0.2e1 * t14 * t16 * t113 + t13 * (t3 + t4 - t7) * t121 - 0.2e1 * t126 + 0.2e1 * t35 + 0.4e1 * t52 + 0.2e1 * t42;
			t133 = ZA * del_rhoB;
			t137 = exp(-0.3e1 * kn);
			t141 = t33 * zc;
			t142 = 0.2e1 * t141;
			t143 = 0.4e1 * t5;
			t144 = t33 * t37;
			t145 = 0.2e1 * t144;
			t148 = 0.2e1 * zc;
			t151 = exp(-kn * (t148 + 0.1e1));
			t155 = t133 * t5;
			t156 = 0.2e1 * t133;
			t163 = 0.3e1 * t2 * zc * del_rhoB * ZB;
			t164 = t35 * del_rhoB;
			t166 = 0.2e1 * t144 * t164;
			t167 = t133 * ZB;
			t169 = 0.2e1 * t141 * t167;
			t172 = 0.2e1 * t133 * kn * ZB;
			t174 = 0.2e1 * t164 * t5;
			t176 = 0.2e1 * t144 * t167;
			t178 = 0.2e1 * t141 * t164;
			t179 = 0.2e1 * t164;
			t183 = 0.3e1 * t155;
			t197 = exp(kn * (t148 - 0.3e1));
			t203 = exp(-kn);
			t210 = t131 * del_rhoA - 0.2e1 * t133 * (-0.2e1 + kn) * t13 * t137 + 0.2e1 * t133 * (0.2e1 - t142 - t143 + t145 + kn) * t1 * t151 + t13 * (t155 + t156) * t99 + 0.2e1 * (-t163 + t166 + t169 + t172 + t174 - t176 - t178 + t179) * t102 + t1 * (-t156 + t183) * t11 - 0.2e1 * (-t176 + t163 + t166 - t174 - t172 - t178 + t179 + t169) * t94 + t13 * (-t156 + t155) * t121 - 0.2e1 * t133 * (-t142 + t143 + t145 + 0.2e1 - kn) * t1 * t197 - 0.2e1 * t133 * (0.2e1 + kn) * t13 * t203 + t1 * (t183 + t156) * t110;
			t211 = t63 * zc;
			t215 = exp(-t143);
			t222 = t63 * t1;
			t225 = t13 * t13;
			t227 = exp(-0.4e1 * kn);
			t238 = exp(-0.2e1 * kn * (zc + 0.1e1));
			t251 = -0.8e1 * t211 * t14 * t20 - 0.2e1 * t53 * t123 * t215 + 0.8e1 * t211 * t14 * t113 + 0.8e1 * t222 * t30 + 0.2e1 * t53 * t225 * t227 + 0.16e2 * t53 * (-t62 + t60 - t48 - t52 + t45 + t51 - t56) * t79 - 0.8e1 * t222 * t26 * t238 - 0.2e1 * t53 * t126 + 0.2e1 * t33 * t35 * ZA + 0.2e1 * t53 * t42 + 0.4e1 * t46 * ZB;
			_C1A = t210 / t251;
			
			t1 = -ZB + ZA;
			t2 = ZA * kn;
			t3 = t2 * zc;
			t4 = 0.2e1 * ZB;
			t5 = kn * zc;
			t7 = 0.2e1 * t5 * ZB;
			t11 = exp(-0.2e1 * t5);
			t13 = kn * kn;
			t14 = t13 * ZA;
			t15 = zc * zc;
			t17 = t14 * t15 * ZB;
			t18 = 0.6e1 * t17;
			t19 = ZA * ZA;
			t20 = t13 * t19;
			t22 = 0.2e1 * t20 * t15;
			t23 = ZB * ZB;
			t24 = kn * t23;
			t25 = t24 * zc;
			t26 = 0.4e1 * t25;
			t27 = 0.4e1 * t24;
			t28 = t2 * ZB;
			t29 = 0.2e1 * t28;
			t30 = zc * ZB;
			t31 = t2 * t30;
			t32 = 0.3e1 * t31;
			t33 = t13 * t23;
			t34 = t33 * t15;
			t35 = 0.4e1 * t34;
			t36 = t14 * t30;
			t37 = 0.6e1 * t36;
			t38 = t33 * zc;
			t39 = 0.4e1 * t38;
			t40 = ZA * ZB;
			t41 = 0.2e1 * t40;
			t43 = 0.2e1 * t20 * zc;
			t44 = -t18 + t22 + t26 - t27 + t29 - t32 + t35 + t37 - t39 - t41 - t43;
			t45 = zc - 0.1e1;
			t46 = kn * t45;
			t48 = exp(0.2e1 * t46);
			t51 = t13 * kn;
			t53 = t15 * zc;
			t55 = t51 * t15;
			t59 = t51 * ZA;
			t66 = 0.4e1 * t17;
			t67 = 0.4e1 * t36;
			t68 = 0.2e1 * t34;
			t69 = 0.2e1 * t38;
			t70 = kn * t19;
			t71 = t70 * zc;
			t73 = t51 * t19 * t53 - t55 * t19 + 0.2e1 * t55 * t40 - 0.2e1 * t59 * ZB * t53 + t51 * t23 * t53 - t55 * t23 + t22 - t43 - t66 + t67 + t68 - t69 + t71 - t70 + t28 - 0.2e1 * t31 + t25 - t24 - t40;
			t75 = kn * (-0.2e1 + zc);
			t76 = exp(t75);
			t79 = ZA + ZB;
			t80 = t79 * t79;
			t83 = exp(kn * (zc - 0.4e1));
			t87 = t79 * t45;
			t90 = exp(-kn * (zc + 0.2e1));
			t97 = exp(0.2e1 * t75);
			t99 = t1 * t1;
			t101 = exp(-0.3e1 * t5);
			t107 = exp(0.4e1 * t46);
			t110 = -t18 - t41 + t35 + t22 + t32 + t27 - t29 - t39 + t37 - t43 - t26;
			t112 = exp(-0.2e1 * kn);
			t116 = t1 * t79;
			t118 = pow(0.1e1 + t5, 0.2e1);
			t119 = exp(-t5);
			t126 = exp(kn * (0.3e1 * zc - 0.4e1));
			t130 = -t1 * (-t3 + t4 - t7) * t11 - 0.2e1 * t44 * t48 + 0.8e1 * t73 * t76 + 0.2e1 * t80 * t83 - 0.8e1 * kn * t1 * t87 * t90 - t79 * (t3 + t4 - t7) * t97 - 0.2e1 * t99 * t101 - t71 + t31 - t1 * (-t3 - t4 - t7) * t107 + 0.2e1 * t23 + t41 + 0.2e1 * t110 * t112 + 0.2e1 * t25 + 0.2e1 * t116 * t118 * t119 - 0.2e1 * t116 * t118 * t126;
			t132 = ZA * del_rhoB;
			t133 = 0.2e1 * t132;
			t134 = t132 * t5;
			t138 = t19 * del_rhoB;
			t139 = 0.2e1 * t138;
			t140 = t132 * ZB;
			t142 = t13 * zc;
			t143 = 0.2e1 * t142;
			t144 = 0.4e1 * t5;
			t145 = t13 * t15;
			t146 = 0.2e1 * t145;
			t150 = exp(0.3e1 * t46);
			t154 = 0.3e1 * t134;
			t161 = kn * (zc + 0.1e1);
			t162 = exp(-t161);
			t168 = exp(t46);
			t175 = t138 * t5;
			t178 = t2 * zc * del_rhoB * ZB;
			t179 = 0.3e1 * t178;
			t181 = 0.2e1 * t145 * t138;
			t183 = 0.2e1 * t142 * t140;
			t186 = 0.2e1 * t132 * kn * ZB;
			t187 = 0.2e1 * t175;
			t189 = 0.2e1 * t145 * t140;
			t191 = 0.2e1 * t142 * t138;
			t202 = exp(kn * (zc - 0.3e1));
			t206 = t130 * del_rhoA - t79 * (-t133 + t134) * t97 - t139 - 0.2e1 * t140 + 0.2e1 * t132 * (-t143 + t144 + t146 + 0.2e1 - kn) * t1 * t150 - t1 * (t154 + t133) * t107 - 0.2e1 * t132 * (0.2e1 - t143 - t144 + t146 + kn) * t1 * t162 + 0.2e1 * t132 * (0.2e1 + kn) * t79 * t168 - t1 * (-t133 + t154) * t11 - t175 - 0.2e1 * (-t179 + t181 + t183 + t186 + t187 - t189 - t191 + t139) * t48 - t178 + 0.2e1 * (-t189 + t179 + t181 - t187 - t186 - t191 + t139 + t183) * t112 + 0.2e1 * t132 * (-0.2e1 + kn) * t79 * t202;
			t207 = t59 * zc;
			t211 = exp(-t144);
			t218 = t59 * t1;
			t223 = exp(-0.4e1 * kn);
			t232 = exp(-0.2e1 * t161);
			t246 = -0.8e1 * t207 * t116 * t97 - 0.2e1 * t14 * t99 * t211 + 0.8e1 * t207 * t116 * t11 + 0.8e1 * t218 * t87 * t48 + 0.2e1 * t14 * t80 * t223 + 0.16e2 * t14 * (-t69 + t67 - t43 - t40 + t68 + t22 - t66) * t112 - 0.8e1 * t218 * t87 * t232 - 0.2e1 * t14 * t99 * t107 + 0.2e1 * t13 * t19 * ZA + 0.2e1 * t14 * t23 + 0.4e1 * t20 * ZB;
			_C2A = t206 / t246;
			
			t1 = -ZB + ZA;
			t2 = 0.2e1 * ZB;
			t4 = t1 * (-ZA - t2);
			t5 = kn * zc;
			t7 = exp(-0.3e1 * t5);
			t10 = ZA + ZB;
			t11 = zc - 0.1e1;
			t12 = t10 * t11;
			t13 = kn * t11;
			t15 = exp(0.2e1 * t13);
			t16 = t12 * t15;
			t19 = kn * kn;
			t20 = ZA * ZA;
			t21 = t19 * t20;
			t22 = zc * zc;
			t24 = 0.2e1 * t21 * t22;
			t26 = 0.2e1 * t21 * zc;
			t27 = t19 * ZA;
			t28 = zc * ZB;
			t30 = 0.4e1 * t27 * t28;
			t33 = 0.4e1 * t27 * t22 * ZB;
			t34 = ZB * ZB;
			t35 = t19 * t34;
			t37 = 0.2e1 * t35 * t22;
			t39 = 0.2e1 * t35 * zc;
			t40 = kn * t20;
			t42 = t40 * zc;
			t44 = ZA * kn;
			t46 = 0.6e1 * t44 * t28;
			t48 = 0.6e1 * t44 * ZB;
			t49 = kn * t34;
			t51 = t49 * zc;
			t53 = ZA * ZB;
			t54 = t24 - t26 + t30 - t33 + t37 - t39 + 0.3e1 * t40 - 0.3e1 * t42 + t46 - t48 + 0.3e1 * t49 - 0.3e1 * t51 - t53;
			t55 = 0.2e1 * kn;
			t56 = exp(-t55);
			t59 = t1 * t10;
			t60 = 0.2e1 * t5;
			t61 = t60 - 0.3e1;
			t62 = exp(-t60);
			t66 = t10 * (ZA - t2);
			t67 = exp(-t5);
			t70 = kn * (-0.2e1 + zc);
			t72 = exp(0.2e1 * t70);
			t78 = exp(kn * (0.3e1 * zc - 0.4e1));
			t80 = 0.2e1 * t40;
			t81 = 0.2e1 * t42;
			t82 = 0.4e1 * t49;
			t83 = 0.4e1 * t51;
			t85 = exp(t70);
			t90 = exp(kn * (zc - 0.4e1));
			t95 = exp(-kn * (zc + 0.2e1));
			t98 = t1 * t1;
			t100 = exp(0.4e1 * t13);
			t101 = t98 * t100;
			t103 = -t4 * t7 + 0.4e1 * kn * t1 * t16 + 0.4e1 * t54 * t56 + t59 * t61 * t62 + t66 * t67 - t59 * t61 * t72 + t4 * t78 + 0.2e1 * (-t80 + t48 + t81 - t82 - t53 - t46 + t83) * t85 - t66 * t90 + 0.2e1 * (-t80 + t48 + t81 - t82 + t53 - t46 + t83) * t95 - t101 + t34 + t20 + 0.2e1 * t53;
			t105 = ZA * del_rhoB;
			t108 = 0.2e1 * zc;
			t111 = exp(-kn * (t108 + 0.1e1));
			t116 = exp(-0.3e1 * kn);
			t120 = t1 * ZA;
			t126 = 0.2e1 * del_rhoB * kn * t20;
			t130 = 0.2e1 * t44 * zc * del_rhoB * ZB;
			t133 = 0.2e1 * t105 * kn * ZB;
			t134 = t20 * del_rhoB;
			t136 = 0.2e1 * t134 * t5;
			t137 = 0.2e1 * t134;
			t138 = t105 * ZB;
			t151 = exp(kn * (t108 - 0.3e1));
			t160 = exp(-kn);
			t164 = t103 * del_rhoA - 0.2e1 * t105 * t1 * (-0.3e1 - t55 + t60) * t111 + 0.2e1 * t105 * t10 * t116 + 0.3e1 * t120 * del_rhoB * t78 + 0.2e1 * (-t126 - t130 + t133 + t136 + t137 - t138) * t85 - t105 * t10 * t90 + 0.2e1 * (-t126 - t130 + t133 + t136 + t138 - t137) * t95 - 0.2e1 * t105 * t1 * (-t55 + 0.3e1 + t60) * t151 - 0.3e1 * t120 * del_rhoB * t7 + t105 * t10 * t67 - 0.2e1 * t105 * t10 * t160;
			t165 = t27 * zc;
			t169 = t27 * t1;
			t173 = exp(-0.2e1 * kn * (zc + 0.1e1));
			t178 = exp(-0.4e1 * t5);
			t187 = t10 * t10;
			t189 = exp(-0.4e1 * kn);
			t206 = -0.8e1 * t165 * t59 * t72 - 0.8e1 * t169 * t12 * t173 - 0.2e1 * t44 * t98 * t178 + 0.8e1 * t165 * t59 * t62 + 0.8e1 * t169 * t16 + 0.2e1 * t44 * t187 * t189 + 0.16e2 * t44 * (-t39 + t30 - t26 - t53 + t37 + t24 - t33) * t56 - 0.2e1 * t44 * t101 + 0.4e1 * t40 * ZB + 0.2e1 * kn * t20 * ZA + 0.2e1 * t44 * t34;
			_C3A = t164 / t206;
			
			t1 = ZB * ZB;
			t2 = kn * t1;
			t3 = t2 * zc;
			t5 = ZA * ZA;
			t6 = kn * t5;
			t8 = kn * kn;
			t9 = t8 * t1;
			t11 = 0.2e1 * t9 * zc;
			t12 = t8 * t5;
			t13 = zc * zc;
			t15 = 0.2e1 * t12 * t13;
			t16 = ZA * ZB;
			t18 = 0.2e1 * t12 * zc;
			t19 = ZA * kn;
			t21 = 0.6e1 * t19 * ZB;
			t22 = t8 * ZA;
			t25 = 0.4e1 * t22 * t13 * ZB;
			t27 = 0.2e1 * t9 * t13;
			t28 = zc * ZB;
			t30 = 0.6e1 * t19 * t28;
			t32 = t6 * zc;
			t35 = 0.4e1 * t22 * t28;
			t36 = 0.3e1 * t3 - 0.3e1 * t6 - t11 + t15 - t16 - t18 + t21 - t25 + t27 - t30 - 0.3e1 * t2 + 0.3e1 * t32 + t35;
			t38 = kn * (-0.2e1 + zc);
			t39 = exp(t38);
			t42 = -ZB + ZA;
			t43 = 0.2e1 * ZB;
			t45 = t42 * (-ZA - t43);
			t46 = kn * zc;
			t47 = 0.2e1 * t46;
			t48 = exp(-t47);
			t50 = 0.2e1 * t6;
			t51 = 0.2e1 * t32;
			t52 = 0.4e1 * t2;
			t53 = 0.4e1 * t3;
			t55 = 0.2e1 * kn;
			t56 = exp(-t55);
			t59 = t42 * t42;
			t61 = exp(-0.3e1 * t46);
			t63 = ZA + ZB;
			t67 = exp(0.2e1 * t38);
			t69 = t42 * t63;
			t70 = t47 + 0.3e1;
			t71 = exp(-t46);
			t74 = t63 * t63;
			t77 = exp(kn * (zc - 0.4e1));
			t82 = exp(kn * (0.3e1 * zc - 0.4e1));
			t85 = zc - 0.1e1;
			t86 = kn * t85;
			t88 = exp(0.4e1 * t86);
			t93 = exp(0.2e1 * t86);
			t97 = t63 * t85;
			t100 = exp(-kn * (zc + 0.2e1));
			t104 = -0.4e1 * t36 * t39 - t45 * t48 + 0.2e1 * (-t50 + t21 + t51 - t52 + t16 - t30 + t53) * t56 + t59 * t61 - t63 * (ZA - t43) * t67 - t69 * t70 * t71 - t74 * t77 + t69 * t70 * t82 + t45 * t88 - t16 - 0.2e1 * t1 + t5 + 0.2e1 * (-t50 + t21 + t51 - t52 - t16 - t30 + t53) * t93 + 0.4e1 * kn * t42 * t97 * t100;
			t106 = ZA * del_rhoB;
			t110 = kn * (zc + 0.1e1);
			t111 = exp(-t110);
			t120 = exp(0.3e1 * t86);
			t126 = exp(kn * (zc - 0.3e1));
			t132 = 0.2e1 * del_rhoB * kn * t5;
			t136 = 0.2e1 * t19 * zc * del_rhoB * ZB;
			t139 = 0.2e1 * t106 * kn * ZB;
			t140 = t5 * del_rhoB;
			t142 = 0.2e1 * t140 * t46;
			t143 = t106 * ZB;
			t144 = 0.2e1 * t140;
			t148 = exp(t86);
			t152 = t42 * ZA;
			t162 = t104 * del_rhoA - 0.2e1 * t106 * t42 * (-0.3e1 - t55 + t47) * t111 - t106 * t63 * t67 - 0.2e1 * t106 * t42 * (-t55 + 0.3e1 + t47) * t120 + 0.2e1 * t106 * t63 * t126 + 0.2e1 * (-t132 - t136 + t139 + t142 + t143 - t144) * t56 - 0.2e1 * t106 * t63 * t148 + t143 - 0.3e1 * t152 * del_rhoB * t48 + 0.2e1 * (-t132 - t136 + t139 + t142 + t144 - t143) * t93 + 0.3e1 * t152 * del_rhoB * t88 + t140;
			t163 = t22 * zc;
			t167 = t22 * t42;
			t169 = exp(-0.2e1 * t110);
			t174 = exp(-0.4e1 * t46);
			t185 = exp(-0.4e1 * kn);
			t203 = -0.8e1 * t163 * t69 * t67 - 0.8e1 * t167 * t97 * t169 - 0.2e1 * t19 * t59 * t174 + 0.8e1 * t163 * t69 * t48 + 0.8e1 * t167 * t97 * t93 + 0.2e1 * t19 * t74 * t185 + 0.16e2 * t19 * (-t11 + t35 - t18 - t16 + t27 + t15 - t25) * t56 - 0.2e1 * t19 * t59 * t88 + 0.4e1 * t6 * ZB + 0.2e1 * kn * t5 * ZA + 0.2e1 * t19 * t1;
			_C4A = t162 / t203;
		} else {
			t1 = ZA * kn;
			t2 = t1 * ZB;
			t3 = zc * ZB;
			t4 = t1 * t3;
			t6 = kn * kn;
			t7 = ZB * ZB;
			t8 = t6 * t7;
			t9 = zc * zc;
			t10 = t8 * t9;
			t12 = ZA * ZA;
			t14 = kn * t12 * zc;
			t15 = 0.2e1 * t14;
			t16 = t8 * zc;
			t17 = t6 * kn;
			t18 = t17 * t9;
			t21 = kn * t7;
			t22 = t21 * zc;
			t25 = t9 * zc;
			t35 = t6 * ZA;
			t36 = t9 * ZB;
			t37 = t35 * t36;
			t39 = t6 * t12;
			t40 = t39 * zc;
			t41 = t35 * t3;
			t43 = t39 * t9;
			t44 = 0.4e1 * t43;
			t47 = ZA * ZB;
			t48 = 0.2e1 * t47;
			t51 = t2 - 0.4e1 * t4 - 0.4e1 * t10 + t15 + t16 - 0.2e1 * t18 * t12 + 0.2e1 * t22 + 0.2e1 * t17 * t7 * t25 - 0.4e1 * t17 * ZA * ZB * t25 + 0.2e1 * t17 * t12 * t25 + 0.8e1 * t37 + t40 - 0.2e1 * t41 - t44 - 0.2e1 * t18 * t7 + t48 + 0.4e1 * t18 * t47;
			t53 = kn * (zc + 0.1e1);
			t54 = exp(-t53);
			t57 = 0.6e1 * t37;
			t58 = 0.2e1 * t10;
			t59 = 0.4e1 * t14;
			t60 = 0.3e1 * t4;
			t62 = kn * zc;
			t63 = 0.2e1 * t62;
			t64 = exp(-t63);
			t67 = -ZB + ZA;
			t69 = 0.2e1 * t1 * zc;
			t70 = 0.4e1 * t1;
			t71 = 0.2e1 * ZA;
			t72 = t62 * ZB;
			t73 = kn * ZB;
			t74 = 0.2e1 * t73;
			t77 = zc - 0.1e1;
			t78 = kn * t77;
			t80 = exp(0.2e1 * t78);
			t82 = ZA + ZB;
			t83 = t67 * t82;
			t85 = 0.2e1 * t6 * zc;
			t86 = 0.4e1 * t62;
			t87 = t6 * t9;
			t89 = 0.2e1 - t85 - t86 + 0.2e1 * t87 + kn;
			t90 = exp(t78);
			t93 = t82 * t82;
			t94 = -0.2e1 + kn;
			t98 = exp(-kn * (0.3e1 + zc));
			t100 = t67 * t67;
			t103 = exp(0.3e1 * t78);
			t107 = exp(-t86);
			t115 = exp(-0.2e1 * kn);
			t122 = exp(-kn * (0.3e1 * zc + 0.1e1));
			t128 = exp(-0.2e1 * t53);
			t134 = exp(kn * (zc - 0.3e1));
			t138 = -0.4e1 * t51 * t54 - 0.2e1 * (-t57 + t58 - t59 + t60 + t44 - t48) * t64 + t67 * (t69 - t70 + t71 + t72 - t74) * t80 - t83 * t89 * t90 - t93 * t94 * t98 + t100 * t94 * t103 + t67 * (-t71 + t69 + t72) * t107 + 0.2e1 * (0.12e2 * t41 - t48 - t57 - 0.4e1 * t16 + t59 - t60 + t58 + t44 + 0.2e1 * t2 - 0.8e1 * t40) * t115 - t15 - t4 + 0.2e1 * t12 + t22 + t48 + t83 * t89 * t122 - t82 * (-t70 + t71 + t69 + t74 - t72) * t128 + 0.4e1 * t62 * t67 * t82 * t94 * t134;
			t141 = del_rhoA * kn * ZB;
			t143 = del_rhoA * ZB;
			t144 = 0.2e1 * t143;
			t145 = t62 * t143;
			t153 = kn * (-0.2e1 + zc);
			t154 = exp(t153);
			t159 = t6 * del_rhoA;
			t162 = del_rhoA * ZA;
			t165 = zc * del_rhoA;
			t166 = t21 * t165;
			t167 = 0.2e1 * t166;
			t168 = t159 * ZA;
			t170 = 0.2e1 * t36 * t168;
			t171 = t7 * del_rhoA;
			t172 = 0.2e1 * t171;
			t178 = 0.2e1 * t87 * t171;
			t180 = t1 * t165 * ZB;
			t181 = 0.3e1 * t180;
			t186 = pow(-0.1e1 + t62, 0.2e1);
			t189 = exp(-0.3e1 * t62);
			t196 = 0.3e1 * t145;
			t200 = exp(-t62);
			t210 = exp(-kn * (zc + 0.2e1));
			t218 = t138 * del_rhoB - t82 * (0.2e1 * t141 - t144 - t145) * t128 + 0.4e1 * t143 * (0.1e1 - 0.3e1 * kn + t87 - t85 + t63) * t67 * t154 + 0.2e1 * (-0.4e1 * t7 * zc * t159 + 0.2e1 * t162 * t73 + t167 - t170 + t172 + 0.4e1 * t3 * t168 - 0.4e1 * t21 * del_rhoA + t178 - t181) * t115 - 0.4e1 * t143 * t186 * t67 * t189 - 0.2e1 * (t181 + t172 - t170 - t167 + t178) * t64 + t67 * (-t196 + t144) * t107 + 0.4e1 * t143 * t82 * t200 - 0.2e1 * t162 * ZB + 0.4e1 * t143 * (kn - 0.1e1) * t82 * t210 + t166 + t67 * (0.6e1 * t141 - t196 - t144) * t80 + t180 - t172;
			t219 = t6 * ZB;
			t221 = exp(0.4e1 * t78);
			t225 = t17 * ZB;
			t226 = t225 * zc;
			t228 = exp(0.2e1 * t153);
			t238 = t225 * t67;
			t239 = t82 * t77;
			t244 = exp(-0.4e1 * kn);
			t267 = -0.2e1 * t219 * t100 * t221 - 0.8e1 * t226 * t83 * t228 - 0.2e1 * t219 * t100 * t107 + 0.8e1 * t226 * t83 * t64 + 0.8e1 * t238 * t239 * t80 + 0.2e1 * t219 * t93 * t244 - 0.8e1 * t238 * t239 * t128 + 0.16e2 * t219 * (-0.2e1 * t16 + 0.4e1 * t41 - 0.2e1 * t40 - t47 + t58 + 0.2e1 * t43 - 0.4e1 * t37) * t115 + 0.4e1 * t35 * t7 + 0.2e1 * t39 * ZB + 0.2e1 * t6 * t7 * ZB;
			_C1B = t218 / t267;
			
			t1 = kn * kn;
			t2 = ZA * ZA;
			t3 = t1 * t2;
			t4 = zc * zc;
			t5 = t3 * t4;
			t6 = 0.4e1 * t5;
			t7 = t3 * zc;
			t9 = t1 * ZA;
			t10 = zc * ZB;
			t11 = t9 * t10;
			t13 = t4 * ZB;
			t14 = t9 * t13;
			t15 = 0.6e1 * t14;
			t16 = ZB * ZB;
			t17 = t1 * t16;
			t18 = t17 * t4;
			t19 = 0.2e1 * t18;
			t20 = t17 * zc;
			t22 = kn * t2;
			t23 = t22 * zc;
			t24 = 0.4e1 * t23;
			t25 = ZA * kn;
			t26 = t25 * ZB;
			t27 = 0.2e1 * t26;
			t28 = t25 * t10;
			t29 = 0.3e1 * t28;
			t30 = ZA * ZB;
			t31 = 0.2e1 * t30;
			t34 = kn * (zc + 0.1e1);
			t35 = exp(-t34);
			t38 = ZA + ZB;
			t39 = 0.2e1 * ZA;
			t41 = 0.2e1 * t25 * zc;
			t42 = 0.4e1 * t25;
			t43 = kn * ZB;
			t44 = 0.2e1 * t43;
			t45 = kn * zc;
			t46 = t45 * ZB;
			t49 = zc - 0.1e1;
			t50 = kn * t49;
			t51 = exp(t50);
			t53 = -ZB + ZA;
			t55 = 0.2e1 + kn;
			t57 = 0.2e1 * t45;
			t58 = exp(-t57);
			t67 = exp(-kn * (0.3e1 * zc + 0.1e1));
			t69 = t53 * t38;
			t71 = 0.2e1 * t1 * zc;
			t72 = 0.4e1 * t45;
			t73 = t1 * t4;
			t75 = -t71 + t72 + 0.2e1 * t73 + 0.2e1 - kn;
			t77 = exp(-0.2e1 * t34);
			t80 = t53 * t53;
			t82 = exp(-t72);
			t84 = t1 * kn;
			t86 = t4 * zc;
			t89 = t84 * t4;
			t108 = kn * t16;
			t111 = 0.2e1 * t84 * t2 * t86 - 0.2e1 * t89 * t2 + 0.4e1 * t89 * t30 - 0.4e1 * t84 * ZA * ZB * t86 + 0.2e1 * t84 * t16 * t86 - 0.2e1 * t89 * t16 - t7 + t6 + 0.2e1 * t11 - 0.8e1 * t14 - t20 + 0.4e1 * t18 + 0.2e1 * t23 + t26 - 0.4e1 * t28 + 0.2e1 * t108 * zc - t31;
			t113 = exp(-0.2e1 * kn);
			t117 = exp(0.2e1 * t50);
			t124 = exp(-kn * (0.3e1 + zc));
			t129 = exp(kn * (zc - 0.3e1));
			t135 = exp(0.3e1 * t50);
			t140 = 0.2e1 * (t6 - 0.8e1 * t7 + 0.12e2 * t11 - t15 + t19 - 0.4e1 * t20 - t24 - t27 + t29 - t31) * t35 + t38 * (-t39 + t41 - t42 + t44 - t46) * t51 + 0.4e1 * t45 * t53 * t38 * t55 * t58 - t53 * (-t39 - t42 + t41 - t44 + t46) * t67 - t69 * t75 * t77 - t80 * t55 * t82 + 0.4e1 * t111 * t113 + t69 * t75 * t117 + t38 * (t41 + t39 - t46) * t124 - 0.2e1 * (t6 + t19 - t15 - t29 + t24 - t31) * t129 - t53 * (t39 + t46 + t41) * t135 + t27 + 0.2e1 * t16 + t108 + t22 + 0.4e1 * t30 + 0.2e1 * t2;
			t142 = del_rhoA * ZB;
			t143 = 0.2e1 * t142;
			t145 = del_rhoA * kn * ZB;
			t147 = t45 * t142;
			t148 = 0.3e1 * t147;
			t152 = t16 * del_rhoA;
			t154 = 0.2e1 * t73 * t152;
			t155 = t1 * del_rhoA;
			t156 = t155 * ZA;
			t158 = 0.2e1 * t13 * t156;
			t159 = zc * del_rhoA;
			t161 = 0.2e1 * t108 * t159;
			t164 = 0.3e1 * t25 * t159 * ZB;
			t165 = 0.2e1 * t152;
			t169 = 0.3e1 * kn;
			t172 = 0.2e1 * zc;
			t175 = exp(-kn * (t172 + 0.1e1));
			t180 = pow(0.1e1 + t45, 0.2e1);
			t184 = exp(kn * (t172 - 0.3e1));
			t188 = exp(-t169);
			t217 = exp(-kn);
			t221 = t140 * del_rhoB - t53 * (t143 + 0.6e1 * t145 - t148) * t67 - 0.2e1 * (t154 - t158 + t161 - t164 + t165) * t129 + 0.4e1 * t142 * (t169 - t57 + t73 - t71 + 0.1e1) * t53 * t175 - 0.4e1 * t142 * t180 * t53 * t184 + 0.4e1 * t142 * t38 * t188 + t38 * (0.2e1 * t145 + t143 - t147) * t51 - t53 * (-t148 - t143) * t135 + 0.2e1 * (0.4e1 * t10 * t156 - t158 + t154 - 0.4e1 * t16 * zc * t155 - 0.2e1 * del_rhoA * ZA * t43 + t164 + 0.4e1 * t108 * del_rhoA - t161 + t165) * t35 + t38 * (-t147 - t143) * t124 - 0.4e1 * t142 * (kn + 0.1e1) * t38 * t217;
			t222 = t1 * ZB;
			t224 = exp(0.4e1 * t50);
			t228 = t84 * ZB;
			t229 = t228 * zc;
			t233 = exp(0.2e1 * kn * (-0.2e1 + zc));
			t243 = t228 * t53;
			t244 = t38 * t49;
			t248 = t38 * t38;
			t250 = exp(-0.4e1 * kn);
			t273 = -0.2e1 * t222 * t80 * t224 - 0.8e1 * t229 * t69 * t233 - 0.2e1 * t222 * t80 * t82 + 0.8e1 * t229 * t69 * t58 + 0.8e1 * t243 * t244 * t117 + 0.2e1 * t222 * t248 * t250 - 0.8e1 * t243 * t244 * t77 + 0.16e2 * t222 * (-0.2e1 * t20 + 0.4e1 * t11 - 0.2e1 * t7 - t30 + t19 + 0.2e1 * t5 - 0.4e1 * t14) * t113 + 0.4e1 * t9 * t16 + 0.2e1 * t3 * ZB + 0.2e1 * t1 * t16 * ZB;
			_C2B = t221 / t273;
			
			t1 = kn * kn;
			t2 = ZA * ZA;
			t3 = t1 * t2;
			t4 = zc * zc;
			t6 = 0.2e1 * t3 * t4;
			t8 = 0.2e1 * t3 * zc;
			t9 = t1 * ZA;
			t10 = zc * ZB;
			t12 = 0.4e1 * t9 * t10;
			t15 = 0.4e1 * t9 * t4 * ZB;
			t16 = ZB * ZB;
			t17 = t1 * t16;
			t19 = 0.2e1 * t17 * t4;
			t21 = 0.2e1 * t17 * zc;
			t22 = kn * t16;
			t23 = t22 * zc;
			t25 = ZA * kn;
			t27 = 0.6e1 * t25 * t10;
			t28 = kn * t2;
			t29 = t28 * zc;
			t31 = ZA * ZB;
			t34 = kn * (zc + 0.1e1);
			t35 = exp(-t34);
			t38 = -ZB + ZA;
			t39 = 0.2e1 * ZA;
			t41 = t38 * (t39 + ZB);
			t42 = zc - 0.1e1;
			t43 = kn * t42;
			t45 = exp(0.2e1 * t43);
			t47 = 0.4e1 * t29;
			t48 = 0.2e1 * t23;
			t50 = kn * zc;
			t51 = 0.2e1 * t50;
			t52 = exp(-t51);
			t55 = ZA + ZB;
			t59 = exp(-0.2e1 * t34);
			t61 = t38 * t55;
			t64 = exp(kn * (zc - 0.3e1));
			t68 = 0.2e1 * kn;
			t69 = -0.3e1 - t68 + t51;
			t70 = exp(t43);
			t76 = exp(-kn * (0.3e1 * zc + 0.1e1));
			t79 = t38 * t38;
			t81 = exp(0.3e1 * t43);
			t84 = exp(-0.4e1 * t50);
			t87 = exp(-t68);
			t90 = t55 * t55;
			t93 = exp(-kn * (0.3e1 + zc));
			t96 = 0.4e1 * (t6 - t8 + t12 - t15 + t19 - t21 - 0.3e1 * t23 + t27 - 0.3e1 * t29 - t31) * t35 + t41 * t45 + 0.2e1 * (t47 - t27 + t48 + t31) * t52 - t55 * (t39 - ZB) * t59 - 0.4e1 * t50 * t61 * t64 + t61 * t69 * t70 - t61 * t69 * t76 - t79 * t81 - t41 * t84 + 0.2e1 * (t47 - t27 + t48 - t31) * t87 + t90 * t93 + 0.2e1 * t2 + t31 - t16;
			t98 = del_rhoA * ZB;
			t104 = kn * (-0.2e1 + zc);
			t105 = exp(t104);
			t109 = zc * del_rhoA;
			t112 = 0.2e1 * t25 * t109 * ZB;
			t114 = 0.2e1 * t22 * t109;
			t115 = t16 * del_rhoA;
			t116 = 0.2e1 * t115;
			t118 = del_rhoA * ZA * ZB;
			t125 = exp(-0.3e1 * t50);
			t132 = t38 * del_rhoA;
			t136 = exp(-t50);
			t142 = exp(-kn * (zc + 0.2e1));
			t149 = t96 * del_rhoB + t98 * t55 * t59 + 0.2e1 * t98 * t38 * (t51 + 0.3e1) * t105 + 0.2e1 * (-t112 + t114 + t116 - t118) * t87 + 0.2e1 * t98 * t38 * (t51 - 0.3e1) * t125 + 0.2e1 * (-t112 + t114 + t118 - t116) * t52 + 0.3e1 * t132 * ZB * t84 + 0.2e1 * t98 * t55 * t136 - 0.2e1 * t98 * t55 * t142 - t115 - 0.3e1 * t132 * ZB * t45 - t118;
			t150 = t1 * ZB;
			t151 = t150 * zc;
			t153 = exp(0.2e1 * t104);
			t157 = kn * ZB;
			t159 = exp(0.4e1 * t43);
			t169 = t150 * t38;
			t170 = t55 * t42;
			t175 = exp(-0.4e1 * kn);
			t193 = -0.8e1 * t151 * t61 * t153 - 0.2e1 * t157 * t79 * t159 - 0.2e1 * t157 * t79 * t84 + 0.8e1 * t151 * t61 * t52 + 0.8e1 * t169 * t170 * t45 + 0.2e1 * t157 * t90 * t175 - 0.8e1 * t169 * t170 * t59 + 0.16e2 * t157 * (-t21 + t12 - t8 - t31 + t19 + t6 - t15) * t87 + 0.4e1 * t25 * t16 + 0.2e1 * kn * t16 * ZB + 0.2e1 * t28 * ZB;
			_C3B = t149 / t193;
			
			t1 = -ZB + ZA;
			t2 = t1 * t1;
			t3 = kn * zc;
			t5 = exp(-0.4e1 * t3);
			t6 = t2 * t5;
			t7 = ZA + ZB;
			t8 = t1 * t7;
			t9 = 0.2e1 * t3;
			t10 = exp(-t9);
			t11 = t8 * t10;
			t14 = ZA * ZA;
			t15 = kn * t14;
			t16 = t15 * zc;
			t17 = 0.4e1 * t16;
			t18 = ZA * kn;
			t19 = zc * ZB;
			t21 = 0.6e1 * t18 * t19;
			t22 = ZB * ZB;
			t23 = kn * t22;
			t24 = t23 * zc;
			t25 = 0.2e1 * t24;
			t26 = ZA * ZB;
			t30 = exp(kn * (zc - 0.3e1));
			t33 = 0.2e1 * kn;
			t34 = -t33 + 0.3e1 + t9;
			t36 = kn * (zc + 0.1e1);
			t38 = exp(-0.2e1 * t36);
			t42 = exp(-t36);
			t45 = kn * kn;
			t46 = t45 * ZA;
			t48 = 0.4e1 * t46 * t19;
			t49 = t45 * t22;
			t50 = zc * zc;
			t52 = 0.2e1 * t49 * t50;
			t55 = 0.2e1 * t49 * zc;
			t57 = t45 * t14;
			t59 = 0.2e1 * t57 * t50;
			t61 = 0.2e1 * t57 * zc;
			t64 = 0.4e1 * t46 * t50 * ZB;
			t66 = exp(-t33);
			t69 = 0.2e1 * ZA;
			t71 = t7 * (t69 - ZB);
			t72 = zc - 0.1e1;
			t73 = kn * t72;
			t74 = exp(t73);
			t78 = exp(-kn * (0.3e1 + zc));
			t81 = t1 * (t69 + ZB);
			t85 = exp(-kn * (0.3e1 * zc + 0.1e1));
			t88 = exp(0.3e1 * t73);
			t91 = exp(0.2e1 * t73);
			t95 = t6 - 0.4e1 * t3 * t11 + 0.2e1 * (t17 - t21 + t25 - t26) * t30 + t8 * t34 * t38 + 0.2e1 * (t17 - t21 + t25 + t26) * t42 - 0.4e1 * (t48 + t52 + 0.3e1 * t24 - t55 + 0.3e1 * t16 - t21 + t59 - t61 - t26 - t64) * t66 + t71 * t74 - t71 * t78 - t81 * t85 + t81 * t88 - t8 * t34 * t91 - t22 - 0.2e1 * t26 - t14;
			t97 = t1 * del_rhoA;
			t101 = del_rhoA * ZB;
			t102 = exp(-kn);
			t107 = exp(-0.3e1 * kn);
			t111 = zc * del_rhoA;
			t114 = 0.2e1 * t18 * t111 * ZB;
			t116 = 0.2e1 * t23 * t111;
			t118 = 0.2e1 * t22 * del_rhoA;
			t120 = del_rhoA * ZA * ZB;
			t131 = 0.2e1 * zc;
			t134 = exp(-kn * (t131 + 0.1e1));
			t144 = exp(kn * (t131 - 0.3e1));
			t151 = t95 * del_rhoB + 0.3e1 * t97 * ZB * t85 + 0.2e1 * t101 * t7 * t102 - 0.2e1 * t101 * t7 * t107 + 0.2e1 * (-t114 + t116 + t118 - t120) * t30 + t101 * t7 * t78 + 0.2e1 * (-t114 + t116 + t120 - t118) * t42 + 0.2e1 * t101 * t1 * (t9 - 0.3e1) * t134 - t101 * t7 * t74 + 0.2e1 * t101 * t1 * (t9 + 0.3e1) * t144 - 0.3e1 * t97 * ZB * t88;
			t152 = t45 * ZB;
			t153 = t152 * zc;
			t157 = exp(0.2e1 * kn * (-0.2e1 + zc));
			t161 = kn * ZB;
			t163 = exp(0.4e1 * t73);
			t171 = t152 * t1;
			t172 = t7 * t72;
			t176 = t7 * t7;
			t178 = exp(-0.4e1 * kn);
			t196 = -0.8e1 * t153 * t8 * t157 - 0.2e1 * t161 * t2 * t163 - 0.2e1 * t161 * t6 + 0.8e1 * t153 * t11 + 0.8e1 * t171 * t172 * t91 + 0.2e1 * t161 * t176 * t178 - 0.8e1 * t171 * t172 * t38 + 0.16e2 * t161 * (-t55 + t48 - t61 - t26 + t52 + t59 - t64) * t66 + 0.4e1 * t18 * t22 + 0.2e1 * kn * t22 * ZB + 0.2e1 * t15 * ZB;
			_C4B = t151 / t196;
			
		}
		
		/*******************************************/
		/*       calculate the velocities etc      */
		/*******************************************/
		
		if (z < zc) { 
			
			t4 = exp(-kn * z);
			t10 = exp(kn * (z - zc));
			t14 = kn * kn;
			u1 = u1a = (_C1A + z * _C3A) * t4 + (_C2A + z * _C4A) * t10 - del_rhoA / ZA / t14;
			
			t1 = kn * z;
			t2 = exp(-t1);
			t6 = exp(kn * (z - zc));
			t10 = 0.1e1 / kn;
			u2 = u2a = t2 * _C1A - t6 * _C2A + t2 * (-0.1e1 + t1) * _C3A * t10 - t6 * (0.1e1 + t1) * _C4A * t10;
			
			t1 = ZA * kn;
			t3 = exp(-kn * z);
			t8 = exp(kn * (z - zc));
			u3 = u3a = -0.2e1 * t1 * t3 * _C1A + 0.2e1 * t1 * t8 * _C2A - 0.2e1 * t1 * t3 * z * _C3A + 0.2e1 * t1 * t8 * z * _C4A;
			
			t2 = kn * z;
			t6 = exp(-t2);
			t15 = exp(kn * (z - zc));
			u4 = u4a = -0.2e1 * ZA * (-_C3A + kn * _C1A + t2 * _C3A) * t6 - 0.2e1 * ZA * (_C4A + kn * _C2A + t2 * _C4A) * t15 + 0.1e1 / kn * del_rhoA;
			
			del_rho = del_rhoA;
			Z = ZA;
			
		} else {
			
			t5 = exp(kn * (zc - z));
			t11 = exp(kn * (z - 0.1e1));
			t15 = kn * kn;
			u1 = u1b = (_C1B + z * _C3B) * t5 + (_C2B + z * _C4B) * t11 - del_rhoB / ZB / t15;
			
			t3 = exp(kn * (zc - z));
			t7 = exp(kn * (z - 0.1e1));
			t9 = kn * z;
			t12 = 0.1e1 / kn;
			u2 = u2b = t3 * _C1B - t7 * _C2B + t3 * (-0.1e1 + t9) * _C3B * t12 - t7 * (0.1e1 + t9) * _C4B * t12;
			
			t1 = kn * ZB;
			t4 = exp(kn * (zc - z));
			t9 = exp(kn * (z - 0.1e1));
			u3 = u3b = -0.2e1 * t1 * t4 * _C1B + 0.2e1 * t1 * t9 * _C2B - 0.2e1 * t1 * t4 * z * _C3B + 0.2e1 * t1 * t9 * z * _C4B;
			
			t2 = kn * z;
			t8 = exp(kn * (zc - z));
			t17 = exp(kn * (z - 0.1e1));
			u4 = u4b = -0.2e1 * ZB * (-_C3B + kn * _C1B + t2 * _C3B) * t8 - 0.2e1 * ZB * (_C4B + kn * _C2B + t2 * _C4B) * t17 + 0.1e1 / kn * del_rhoB;
			
			
			del_rho = del_rhoB;
			Z = ZB;
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
	} else {
		del_rho = sigmaA*dxA;
		rho = del_rho; /* density */
		sum7 += rho;
		u3 = del_rho*(z-zc); /* zz stress */
		sum3 += u3;
		
		txx = u3; /* xx stress */
		pp = -u3; /* pressure */
		sum5 += pp;
		sum6 += txx;
		/* i.e. u3 for n=0 is arbitrarily set to zero at z=zc */
	}
	
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
//        if( fabs( sum5 - ( -0.5*(sum6+sum3) ) ) > 1e-5 ) {
//                assert(0);
//        }
}


