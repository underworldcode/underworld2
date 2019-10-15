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
#include "solE.h"

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
			_Velic_solE(
					pos,
					1.0,
					100.0, 1.0,
					0.8, (double)M_PI, 1,
					vel, &pressure, total_stress, strain_rate );
//			printf("pressure is fucked !! \n");
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


void _Velic_solE(
		double pos[],
		double _sigma,
		double _eta_A, double _eta_B, 
		double _z_c, double _km, int _n,
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[] )
{
	double Z,ZA,ZB,u1,u2,u3,u4,pp,txx;
	double u1a,u2a,u3a,u4a,u1b,u2b,u3b,u4b;
	double sum1,sum2,sum3,sum4,sum5,sum6,sum7,x,z;
	double sigma;
	int n;
	double kn,km;
	double _C1A,_C2A,_C3A,_C4A,_C1B,_C2B,_C3B,_C4B;
	double rho,zc;
	
	double t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
	double t11,t12,t13,t14,t15,t16,t17,t18,t19,t20;
	double t21,t22,t23,t24,t25,t26,t27,t28,t29,t30;
	double t31,t32,t33,t34,t35,t36,t37,t38,t39,t40;
	double t41,t42,t43,t44,t45,t46,t47,t48,t49,t50;
	double t51,t52,t53,t54,t55,t56,t57,t58,t59,t60;
	double t61,t62,t63,t64,t65,t66,t67,t68,t69,t70;
	double t71,t72,t73,t74,t76,t77,t78,t79,t80,t81;
	double t82,t83,t84,t85,t86,t87,t88,t89,t90,t91;
	double t92,t93,t94,t95,t96,t97,t98,t99,t100,t101;
	double t102,t103,t104,t105,t106,t109,t111,t112,t113,t114;
	double t115,t116,t117,t118,t119,t120,t121,t123,t124,t126;
	double t127,t128,t130,t131,t132,t133,t134,t136,t137,t140;
	double t141,t142,t143,t145,t146,t147,t148,t149,t151,t152;
	double t155,t156,t157,t158,t160,t162,t163,t164,t165,t166;
	double t167,t169,t170,t171,t172,t173,t174,t176,t178,t179;
	double t180,t181,t184,t186,t187,t188,t189,t191,t192,t193;
	double t195,t196,t197,t198,t199,t200,t201,t203,t204,t205;
	double t237,t241,t245,t247,t250,t251,t252,t253,t256,t261;
	double t262,t263,t265,t266,t268,t269,t271,t272,t275,t276;
	double t279,t282,t286;
	
	
	/*************************************************************************/
	/*************************************************************************/
	/*
	
	rho = -sigma*sin(km*z)*cos(kn*x)
	
	The viscosity of layer B is ZB:
	The viscosity of layer A is ZA:
	
	*/
	/*************************************************************************/
	/*************************************************************************/
	
	/*************************************************************************/
	/*************************************************************************/
	sigma = _sigma; /* density parameter for layer B */
	ZA = _eta_A;    /* viscosity for layer A */
	ZB = _eta_B; /* viscosity for layer B */
	zc = _z_c;  /* dividing line between region B and region A */
	
	n = _n;
	kn = (double)n*M_PI;
	km = _km;
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
	
	
	/*******************************************/
	/*         calculate the constants         */
	/*******************************************/
	
	if ( z < zc ) {
		t3 = exp(-kn * (zc + 0.2e1));
		t4 = kn * kn;
		t5 = t4 * t4;
		t6 = t3 * t5;
		t7 = ZA * zc;
		t8 = km * zc;
		t9 = sin(t8);
		t10 = t7 * t9;
		t13 = sin(km);
		t14 = kn * t13;
		t15 = km * km;
		t16 = ZA * t15;
		t17 = 0.2e1 * zc;
		t20 = exp(-kn * (t17 + 0.3e1));
		t26 = exp(-kn * (0.4e1 * zc + 0.1e1));
		t27 = t26 * ZA;
		t28 = t27 * t4;
		t29 = zc * zc;
		t31 = t29 * t15 * t13;
		t33 = t4 * kn;
		t34 = t3 * t33;
		t35 = t34 * ZA;
		t36 = cos(t8);
		t38 = t29 * t36 * km;
		t43 = exp(-kn * (zc + 0.4e1));
		t47 = t36 * km * zc;
		t51 = kn * (t17 + 0.1e1);
		t52 = exp(-t51);
		t53 = t52 * ZA;
		t54 = t53 * t4;
		t56 = kn * t36;
		t57 = ZA * km;
		t58 = kn * zc;
		t60 = exp(-0.3e1 * t58);
		t65 = t7 * t13;
		t70 = exp(-kn * (0.3e1 * zc + 0.2e1));
		t71 = t70 * t33;
		t74 = km * ZB * zc;
		t77 = t71 * ZA;
		t81 = exp(-0.3e1 * kn);
		t83 = t81 * t4 * ZA;
		t84 = t13 * t15;
		t85 = t84 * zc;
		t88 = t29 * ZA * t9;
		t91 = ZA * t33;
		t92 = zc * t13;
		t96 = kn * ZA;
		t97 = t96 * t13;
		t98 = t15 * zc;
		t105 = t4 * ZA * t36;
		t111 = 0.4e1 * t6 * t10 - t14 * t16 * t20 - t28 * t31 + 0.4e1 * t35 * t38 - 0.2e1 * t43 * t4 * ZA * t47 + t54 * t31 + 0.2e1 * t56 * t57 * t60 + t20 * t5 * t65 + 0.4e1 * t71 * t36 * t74 - 0.4e1 * t77 * t47 - t83 * t85 - 0.4e1 * t6 * t88 + 0.2e1 * t91 * t92 * t81 + 0.2e1 * t97 * t98 * t52 - t14 * t16 * t26 + 0.2e1 * t105 * t8 * t3 - 0.4e1 * t35 * t47;
		t112 = t33 * zc;
		t113 = ZB * t9;
		t117 = t4 * t36;
		t118 = t57 * t70;
		t121 = t81 * ZA;
		t123 = t5 * t13 * t29;
		t128 = t29 * ZB * t9;
		t140 = t70 * t5;
		t143 = t57 * t3;
		t147 = ZA * t9;
		t151 = t4 * t13;
		t156 = t20 * t4 * ZA;
		t162 = t20 * ZA;
		t164 = 0.4e1 * t112 * t113 * t70 + 0.4e1 * t117 * t118 + t121 * t123 + 0.4e1 * t77 * t38 + 0.4e1 * t6 * t128 + 0.2e1 * t91 * t92 * t26 + 0.2e1 * t56 * t57 * t43 - 0.4e1 * t34 * ZB * t38 - 0.4e1 * t140 * t128 - 0.2e1 * t56 * t143 + t53 * t123 - 0.4e1 * t112 * t147 * t3 - t151 * t53 - t27 * t123 - t54 * t85 + t156 * t85 + 0.2e1 * t60 * t4 * ZA * t47 - t162 * t123;
		t172 = t33 * t13;
		t176 = t33 * t9;
		t187 = ZB * zc;
		t188 = t187 * t9;
		t200 = 0.4e1 * t34 * t36 * t74 - 0.2e1 * t105 * t8 * t70 - t172 * t162 + t84 * t27 - t172 * t53 - 0.4e1 * t176 * ZB * t70 + 0.4e1 * t176 * ZA * t70 - 0.4e1 * t176 * ZB * t3 + t151 * t162 - 0.4e1 * t6 * t188 - t151 * t27 + t151 * t121 - t172 * t121 + t84 * t53 + 0.4e1 * t176 * ZA * t3 - t172 * t27 - t84 * t121;
		t237 = -t84 * t162 - t81 * t5 * t65 + t83 * t31 - 0.4e1 * t117 * t143 - 0.4e1 * t140 * t10 - 0.4e1 * t112 * t147 * t70 + 0.4e1 * t112 * t113 * t3 + t28 * t85 - t52 * t5 * t65 - t14 * t16 * t52 + t26 * t5 * t65 + 0.2e1 * t97 * t98 * t20 - 0.2e1 * t56 * t118 + 0.4e1 * t140 * t188 - 0.4e1 * t71 * ZB * t38 + 0.4e1 * t140 * t88 - t14 * t16 * t81 - t156 * t31;
		t245 = pow(t15 + t4, 0.2e1);
		t251 = exp(-0.2e1 * kn * (zc + 0.1e1));
		t252 = ZB * t251;
		t265 = exp(-0.2e1 * kn);
		t268 = exp(-0.2e1 * t51);
		t271 = exp(-0.4e1 * t58);
		t275 = exp(-0.4e1 * kn);
		_C1A = -sigma * (t111 + t164 + t200 + t237) / ZA / kn / t245 / (-0.4e1 * t58 * t252 - 0.4e1 * t96 * t29 * t251 + 0.4e1 * kn * t29 * t252 + 0.4e1 * t96 * zc * t251 - t187 * t265 - ZB * t268 - t7 * t271 + ZB * t265 + t7 * t275 + t187 * t268) / 0.4e1;
		
		t1 = 0.2e1 * zc;
		t4 = exp(-kn * (t1 + 0.3e1));
		t5 = kn * kn;
		t7 = t4 * t5 * ZA;
		t8 = sin(km);
		t9 = km * km;
		t10 = t8 * t9;
		t11 = t10 * zc;
		t14 = kn * (t1 + 0.1e1);
		t15 = exp(-t14);
		t16 = t5 * t5;
		t18 = ZA * zc;
		t19 = t18 * t8;
		t21 = kn * t8;
		t22 = ZA * t9;
		t28 = exp(-kn * (0.3e1 * zc + 0.2e1));
		t29 = t5 * kn;
		t30 = t28 * t29;
		t31 = t30 * ZA;
		t32 = km * zc;
		t33 = cos(t32);
		t35 = t33 * km * zc;
		t39 = exp(-0.3e1 * kn);
		t41 = t39 * t5 * ZA;
		t43 = t5 * t8;
		t47 = exp(-kn * (0.4e1 * zc + 0.1e1));
		t48 = ZA * t47;
		t50 = t29 * t8;
		t51 = ZA * t4;
		t54 = ZA * t15;
		t56 = sin(t32);
		t57 = t29 * t56;
		t66 = exp(-kn * (zc + 0.2e1));
		t73 = ZA * t39;
		t76 = -t7 * t11 + t15 * t16 * t19 + t21 * t22 * t15 + 0.4e1 * t31 * t35 + t41 * t11 + t43 * t48 + t50 * t51 - t10 * t48 + t50 * t54 + 0.4e1 * t57 * ZB * t28 - 0.4e1 * t57 * ZA * t28 + 0.4e1 * t57 * ZB * t66 + t43 * t54 + t10 * t51 - t43 * t51 + t50 * t73 - t10 * t54;
		t83 = zc * zc;
		t84 = t16 * t8 * t83;
		t86 = t66 * t29;
		t87 = t86 * ZA;
		t89 = t83 * t33 * km;
		t92 = t66 * t16;
		t94 = t83 * ZB * t56;
		t97 = ZA * t29;
		t98 = zc * t8;
		t102 = kn * ZA;
		t103 = t102 * t8;
		t104 = t9 * zc;
		t109 = t83 * t9 * t8;
		t113 = exp(-kn * (zc + 0.4e1));
		t121 = t83 * ZA * t56;
		t127 = t54 * t5;
		t132 = t5 * t33;
		t133 = ZA * km;
		t134 = t133 * t66;
		t140 = -0.4e1 * t57 * ZA * t66 + t50 * t48 + t10 * t73 + t51 * t84 - 0.4e1 * t87 * t89 - 0.4e1 * t92 * t94 - 0.2e1 * t97 * t98 * t47 - 0.2e1 * t103 * t104 * t15 - t41 * t109 + 0.2e1 * t113 * t5 * ZA * t35 - t47 * t16 * t19 + 0.4e1 * t92 * t121 - 0.2e1 * t103 * t104 * t4 - t127 * t109 + 0.4e1 * t86 * ZB * t89 + 0.4e1 * t132 * t134 + t7 * t109 + t21 * t22 * t47;
		t142 = kn * t33;
		t143 = kn * zc;
		t145 = exp(-0.3e1 * t143);
		t149 = t18 * t56;
		t156 = t29 * zc;
		t157 = ZB * t56;
		t162 = t28 * t16;
		t163 = zc * ZB;
		t164 = t163 * t56;
		t171 = t5 * ZA * t33;
		t178 = t133 * t28;
		t181 = ZA * t56;
		t192 = km * ZB * zc;
		t197 = -0.2e1 * t142 * t133 * t145 - 0.4e1 * t92 * t149 - t4 * t16 * t19 + 0.2e1 * t142 * t134 - 0.4e1 * t156 * t157 * t28 + t48 * t84 - 0.4e1 * t162 * t164 - 0.4e1 * t156 * t157 * t66 - 0.2e1 * t171 * t32 * t66 + 0.4e1 * t162 * t94 + t127 * t11 + 0.2e1 * t142 * t178 + 0.4e1 * t156 * t181 * t66 + t21 * t22 * t39 - 0.2e1 * t97 * t98 * t39 - 0.4e1 * t86 * t33 * t192 - 0.4e1 * t132 * t178;
		t205 = t48 * t5;
		t237 = -t54 * t84 - 0.2e1 * t145 * t5 * ZA * t35 + 0.4e1 * t92 * t164 + t205 * t109 - 0.4e1 * t31 * t89 + t21 * t22 * t4 - t73 * t84 + 0.4e1 * t156 * t181 * t28 - 0.2e1 * t142 * t133 * t113 + 0.4e1 * t162 * t149 - t205 * t11 + 0.4e1 * t87 * t35 - 0.4e1 * t162 * t121 + 0.4e1 * t30 * ZB * t89 + 0.2e1 * t171 * t32 * t28 + t39 * t16 * t19 - 0.4e1 * t30 * t33 * t192 - t43 * t73;
		t241 = exp(t143);
		t247 = pow(t9 + t5, 0.2e1);
		t252 = exp(-0.2e1 * kn * (zc + 0.1e1));
		t253 = ZB * t252;
		t266 = exp(-0.2e1 * kn);
		t269 = exp(-0.2e1 * t14);
		t272 = exp(-0.4e1 * t143);
		t276 = exp(-0.4e1 * kn);
		_C2A = -sigma * (t76 + t140 + t197 + t237) * t241 / ZA / kn / t247 / (-0.4e1 * t143 * t253 - 0.4e1 * t102 * t83 * t252 + 0.4e1 * kn * t83 * t253 + 0.4e1 * t102 * zc * t252 - t163 * t266 - ZB * t269 - t18 * t272 + ZB * t266 + t18 * t276 + t163 * t269) / 0.4e1;
		
		t1 = kn * kn;
		t2 = t1 * kn;
		t3 = ZA * t2;
		t4 = sin(km);
		t5 = zc * t4;
		t7 = exp(-0.3e1 * kn);
		t10 = kn * t4;
		t11 = km * km;
		t12 = ZA * t11;
		t16 = exp(-kn * (0.4e1 * zc + 0.1e1));
		t19 = t1 * t4;
		t20 = 0.2e1 * zc;
		t22 = kn * (t20 + 0.1e1);
		t23 = exp(-t22);
		t24 = ZA * t23;
		t26 = km * zc;
		t27 = sin(t26);
		t28 = t2 * t27;
		t31 = exp(-kn * (zc + 0.2e1));
		t32 = ZB * t31;
		t36 = cos(t26);
		t37 = t1 * ZA * t36;
		t44 = exp(-kn * (0.3e1 * zc + 0.2e1));
		t48 = t1 * t36;
		t49 = ZA * km;
		t50 = t49 * t44;
		t56 = t7 * ZA;
		t58 = t4 * t11;
		t59 = t58 * zc;
		t61 = ZB * t44;
		t66 = exp(-kn * (t20 + 0.3e1));
		t67 = ZA * t66;
		t69 = -t3 * t5 * t7 + t10 * t12 * t16 + t19 * t24 + 0.4e1 * t28 * t32 - 0.4e1 * t37 * t26 * t31 - 0.4e1 * t28 * ZA * t44 - 0.4e1 * t50 * t48 + 0.4e1 * t37 * t26 * t44 - t56 * kn * t59 + 0.4e1 * t28 * t61 - t19 * t67;
		t70 = ZA * t16;
		t72 = t2 * t4;
		t87 = km * ZB * zc;
		t90 = t19 * t70 + t72 * t67 - t58 * t70 + t72 * t24 + t72 * t56 - t58 * t24 - 0.4e1 * t28 * ZA * t31 + t72 * t70 + t58 * t56 + t58 * t67 - 0.4e1 * t44 * t1 * t36 * t87;
		t93 = t49 * t31;
		t96 = t48 * km;
		t101 = t2 * zc;
		t102 = ZA * t27;
		t106 = kn * t36;
		t109 = exp(-kn * (zc + 0.4e1));
		t113 = ZB * t27;
		t119 = kn * ZA;
		t120 = t119 * t4;
		t121 = t11 * zc;
		t126 = kn * zc;
		t128 = exp(-0.3e1 * t126);
		t132 = -t19 * t56 + 0.4e1 * t48 * t93 - 0.4e1 * t32 * t96 - t3 * t5 * t16 + 0.4e1 * t101 * t102 * t44 - 0.2e1 * t106 * t49 * t109 - 0.4e1 * t101 * t113 * t31 + t10 * t12 * t23 - t120 * t121 * t66 + 0.2e1 * t106 * t50 - 0.2e1 * t106 * t49 * t128;
		t149 = t101 * t4;
		t158 = -0.4e1 * t101 * t113 * t44 - t70 * kn * t59 + 0.4e1 * t101 * t102 * t31 - t120 * t121 * t23 + t10 * t12 * t7 + 0.4e1 * t61 * t96 + 0.2e1 * t106 * t93 - t24 * t149 - t67 * t149 + t10 * t12 * t66 + 0.4e1 * t31 * t1 * t36 * t87;
		t164 = pow(t11 + t1, 0.2e1);
		t170 = exp(-0.2e1 * kn * (zc + 0.1e1));
		t171 = ZB * t170;
		t174 = zc * zc;
		t184 = zc * ZB;
		t186 = exp(-0.2e1 * kn);
		t189 = exp(-0.2e1 * t22);
		t191 = ZA * zc;
		t193 = exp(-0.4e1 * t126);
		t197 = exp(-0.4e1 * kn);
		_C3A = -sigma * (t69 + t90 + t132 + t158) / ZA / t164 / (0.4e1 * t126 * t171 + 0.4e1 * t119 * t174 * t170 - 0.4e1 * kn * t174 * t171 - 0.4e1 * t119 * zc * t170 + t184 * t186 + ZB * t189 + t191 * t193 - ZB * t186 - t191 * t197 - t184 * t189) / 0.4e1;
		
		t1 = 0.2e1 * zc;
		t4 = exp(-kn * (t1 + 0.3e1));
		t5 = t4 * ZA;
		t6 = kn * kn;
		t7 = t6 * kn;
		t8 = t7 * zc;
		t9 = sin(km);
		t10 = t8 * t9;
		t12 = kn * t9;
		t13 = km * km;
		t14 = ZA * t13;
		t17 = km * zc;
		t18 = sin(t17);
		t19 = ZA * t18;
		t22 = exp(-kn * (zc + 0.2e1));
		t27 = exp(-0.3e1 * kn);
		t31 = cos(t17);
		t32 = t6 * ZA * t31;
		t36 = exp(-kn * (0.3e1 * zc + 0.2e1));
		t40 = t6 * t9;
		t42 = kn * (t1 + 0.1e1);
		t43 = exp(-t42);
		t44 = ZA * t43;
		t46 = t7 * t18;
		t47 = ZB * t36;
		t53 = ZB * t22;
		t56 = t7 * t9;
		t59 = t5 * t10 - t12 * t14 * t4 - 0.4e1 * t8 * t19 * t22 - t12 * t14 * t27 - 0.4e1 * t32 * t17 * t36 - t40 * t44 - 0.4e1 * t46 * t47 + 0.4e1 * t46 * ZA * t36 - 0.4e1 * t46 * t53 - t56 * t44 - t56 * t5;
		t60 = t13 * t9;
		t64 = exp(-kn * (0.4e1 * zc + 0.1e1));
		t65 = ZA * t64;
		t70 = km * ZB * zc;
		t73 = t7 * ZA;
		t74 = zc * t9;
		t77 = kn * ZA;
		t78 = t77 * t9;
		t79 = t13 * zc;
		t82 = t6 * t31;
		t83 = ZA * km;
		t84 = t83 * t22;
		t94 = ZA * t27;
		t97 = t60 * t65 - 0.4e1 * t22 * t6 * t31 * t70 + t73 * t74 * t27 + t78 * t79 * t43 - 0.4e1 * t82 * t84 - t12 * t14 * t64 - t40 * t65 + 0.4e1 * t46 * ZA * t22 - t56 * t65 - t60 * t94 - t60 * t5;
		t106 = t83 * t36;
		t113 = t60 * zc;
		t115 = kn * t31;
		t118 = ZB * t18;
		t126 = t40 * t5 + t40 * t94 - t56 * t94 + t60 * t44 + 0.4e1 * t32 * t17 * t22 + 0.4e1 * t82 * t106 - 0.4e1 * t8 * t19 * t36 + t94 * kn * t113 - 0.2e1 * t115 * t106 + 0.4e1 * t8 * t118 * t22 + 0.4e1 * t36 * t6 * t31 * t70;
		t127 = t82 * km;
		t136 = exp(-kn * (zc + 0.4e1));
		t147 = kn * zc;
		t149 = exp(-0.3e1 * t147);
		t158 = 0.4e1 * t53 * t127 + t73 * t74 * t64 - t12 * t14 * t43 + 0.2e1 * t115 * t83 * t136 - 0.4e1 * t47 * t127 - 0.2e1 * t115 * t84 + t44 * t10 + t78 * t79 * t4 + 0.2e1 * t115 * t83 * t149 + 0.4e1 * t8 * t118 * t36 + t65 * kn * t113;
		t162 = exp(t147);
		t166 = pow(t13 + t6, 0.2e1);
		t172 = exp(-0.2e1 * kn * (zc + 0.1e1));
		t173 = ZB * t172;
		t176 = zc * zc;
		t186 = zc * ZB;
		t188 = exp(-0.2e1 * kn);
		t191 = exp(-0.2e1 * t42);
		t193 = ZA * zc;
		t195 = exp(-0.4e1 * t147);
		t199 = exp(-0.4e1 * kn);
		_C4A = -sigma * (t59 + t97 + t126 + t158) * t162 / ZA / t166 / (-0.4e1 * t147 * t173 - 0.4e1 * t77 * t176 * t172 + 0.4e1 * kn * t176 * t173 + 0.4e1 * t77 * zc * t172 - t186 * t188 - ZB * t191 - t193 * t195 + ZB * t188 + t193 * t199 + t186 * t191) / 0.4e1;
		
		
		
	} else { 
		t3 = kn * (0.2e1 * zc + 0.1e1);
		t4 = exp(-t3);
		t5 = t4 * ZB;
		t6 = kn * kn;
		t8 = sin(km);
		t9 = km * km;
		t10 = t8 * t9;
		t11 = t10 * zc;
		t14 = exp(-kn);
		t15 = t14 * ZB;
		t16 = t15 * t6;
		t18 = zc * zc;
		t20 = t18 * t9 * t8;
		t23 = t10 * ZB;
		t27 = exp(-kn * (zc + 0.2e1));
		t29 = km * zc;
		t30 = cos(t29);
		t31 = t30 * km;
		t32 = t31 * ZB;
		t35 = kn * zc;
		t37 = exp(-0.3e1 * t35);
		t38 = t6 * kn;
		t39 = t37 * t38;
		t40 = ZA * zc;
		t41 = sin(t29);
		t42 = t40 * t41;
		t45 = t4 * ZA;
		t46 = t6 * t6;
		t48 = t46 * t8 * t18;
		t51 = t38 * zc;
		t59 = km * ZB * zc;
		t62 = zc * ZB;
		t63 = t62 * t41;
		t72 = exp(-kn * (0.4e1 * zc + 0.1e1));
		t73 = t72 * t38;
		t74 = t62 * t8;
		t78 = t40 * t8;
		t82 = t9 * ZB;
		t84 = t6 * t8;
		t88 = -0.4e1 * t5 * t6 * t11 - t16 * t11 + t16 * t20 + t14 * kn * t23 + 0.2e1 * t27 * kn * t32 - 0.4e1 * t39 * t42 - 0.8e1 * t45 * t48 + 0.4e1 * t51 * ZB * t41 * t27 + 0.2e1 * t37 * t6 * t30 * t59 + 0.4e1 * t39 * t63 + 0.4e1 * t45 * t51 * t8 + 0.2e1 * t73 * t74 + 0.2e1 * t4 * t46 * t78 - t14 * t8 * t82 + t15 * t84 + t72 * t8 * t82;
		t89 = ZB * t8;
		t91 = t4 * t38;
		t92 = t91 * ZB;
		t93 = t18 * zc;
		t95 = t93 * t9 * t8;
		t98 = t14 * t38;
		t100 = t72 * ZB;
		t102 = t91 * ZA;
		t106 = t4 * t46 * kn;
		t112 = t46 * zc * t8;
		t127 = t27 * t38;
		t130 = t18 * t30 * km;
		t141 = -t73 * t89 - 0.4e1 * t92 * t95 + t98 * t89 - t100 * t84 - 0.4e1 * t102 * t20 - 0.4e1 * t106 * ZA * t8 * t18 - 0.4e1 * t5 * t112 - t15 * t112 + 0.4e1 * t4 * kn * zc * t23 + t15 * t48 - t72 * kn * t23 - 0.4e1 * t51 * ZA * t41 * t27 - 0.4e1 * t127 * ZB * t130 + 0.4e1 * t106 * t89 * t18 - 0.4e1 * t91 * t74 + 0.2e1 * t37 * kn * t32;
		t147 = exp(-t35);
		t152 = exp(-0.3e1 * kn);
		t169 = t100 * t6;
		t172 = t37 * t46;
		t174 = t18 * ZA * t41;
		t180 = exp(-kn * (0.3e1 * zc + 0.2e1));
		t188 = t6 * t30 * km;
		t191 = t27 * t46;
		t193 = t18 * ZB * t41;
		t198 = t127 * ZA;
		t204 = -0.2e1 * t27 * t6 * t30 * t59 - 0.2e1 * t147 * kn * t32 - 0.2e1 * t152 * t6 * ZA * t11 - 0.4e1 * t39 * ZB * t130 - 0.4e1 * t106 * ZB * t93 * t8 + 0.4e1 * ZA * t38 * zc * t8 * t152 + t169 * t11 - t100 * t48 + 0.4e1 * t172 * t174 - 0.2e1 * t180 * t6 * t30 * t59 + t100 * t112 - 0.4e1 * t27 * ZB * t188 + 0.4e1 * t191 * t193 - 0.2e1 * t98 * t74 + 0.4e1 * t198 * t130 + 0.4e1 * t180 * ZB * t188;
		t247 = -0.8e1 * t191 * t63 - 0.4e1 * t191 * t174 - 0.8e1 * t198 * t31 * zc - t169 * t20 - 0.4e1 * t172 * t193 + 0.2e1 * t4 * t6 * ZA * t11 + 0.4e1 * t102 * t95 + 0.8e1 * t127 * t30 * t59 - 0.2e1 * t152 * t46 * t78 + 0.4e1 * t106 * ZA * t93 * t8 + 0.8e1 * t5 * t48 + 0.4e1 * t92 * t20 + 0.8e1 * t191 * t42 + 0.2e1 * t147 * t6 * t30 * t59 - 0.2e1 * t180 * kn * t32 + 0.4e1 * t39 * ZA * t130;
		t256 = pow(t9 + t6, 0.2e1);
		t261 = exp(-0.2e1 * kn * (zc + 0.1e1));
		t262 = ZB * t261;
		t265 = kn * ZA;
		t276 = exp(-0.2e1 * kn);
		t279 = exp(-0.2e1 * t3);
		t282 = exp(-0.4e1 * t35);
		t286 = exp(-0.4e1 * kn);
		_C1B = sigma * (t88 + t141 + t204 + t247) * t147 / ZB / kn / t256 / (0.4e1 * t35 * t262 + 0.4e1 * t265 * t18 * t261 - 0.4e1 * kn * t18 * t262 - 0.4e1 * t265 * zc * t261 + t62 * t276 + ZB * t279 + t40 * t282 - ZB * t276 - t40 * t286 - t62 * t279) / 0.4e1;
		
		t2 = exp(-0.2e1 * kn);
		t3 = t2 * ZB;
		t4 = kn * kn;
		t5 = t3 * t4;
		t6 = zc * zc;
		t7 = km * km;
		t9 = sin(km);
		t10 = t6 * t7 * t9;
		t13 = kn * (zc + 0.1e1);
		t15 = exp(-0.2e1 * t13);
		t16 = t4 * t4;
		t18 = t15 * t16 * kn;
		t19 = t9 * ZB;
		t27 = exp(-0.2e1 * kn * (0.2e1 * zc + 0.1e1));
		t28 = ZB * t27;
		t30 = t16 * zc * t9;
		t32 = t9 * t7;
		t33 = t32 * zc;
		t38 = exp(-kn * (0.3e1 * zc + 0.1e1));
		t39 = t4 * kn;
		t40 = t38 * t39;
		t41 = t40 * ZA;
		t42 = km * zc;
		t43 = cos(t42);
		t44 = t43 * km;
		t45 = t44 * zc;
		t49 = exp(-0.3e1 * t13);
		t53 = km * ZB * zc;
		t57 = t44 * ZB;
		t60 = exp(-t13);
		t64 = t15 * ZB;
		t68 = t6 * zc;
		t74 = t7 * ZB;
		t78 = t4 * t9;
		t84 = t2 * t39;
		t86 = t27 * t39;
		t88 = t5 * t10 + 0.4e1 * t18 * t19 * t6 + t28 * t30 - t5 * t33 - 0.8e1 * t41 * t45 - 0.2e1 * t49 * t4 * t43 * t53 + 0.2e1 * t38 * kn * t57 - 0.2e1 * t60 * kn * t57 + 0.4e1 * t64 * t4 * t33 - 0.4e1 * t18 * ZB * t68 * t9 - t2 * t9 * t74 + t27 * t9 * t74 + t3 * t78 + 0.4e1 * t18 * ZA * t68 * t9 - t84 * t19 + t86 * t19;
		t90 = t40 * ZB;
		t93 = t38 * t16;
		t95 = sin(t42);
		t96 = ZB * t6 * t95;
		t99 = zc * ZB;
		t100 = t99 * t95;
		t105 = exp(-kn * (zc + 0.3e1));
		t111 = t16 * t9 * t6;
		t113 = t15 * t39;
		t114 = t113 * ZB;
		t116 = t68 * t7 * t9;
		t120 = t6 * t43 * km;
		t124 = ZA * t6 * t95;
		t127 = t105 * t39;
		t131 = t28 * t4;
		t134 = t99 * t9;
		t140 = ZA * zc;
		t141 = t140 * t95;
		t146 = t32 * ZB;
		t149 = -t28 * t78 + 0.8e1 * t90 * t45 - 0.4e1 * t93 * t96 + 0.8e1 * t93 * t100 - 0.2e1 * t105 * t4 * t43 * t53 + t3 * t111 - 0.4e1 * t114 * t116 + 0.4e1 * t41 * t120 + 0.4e1 * t93 * t124 - 0.4e1 * t127 * ZB * t120 + t131 * t33 - t28 * t111 + 0.2e1 * t84 * t134 + 0.2e1 * t105 * kn * t57 - 0.4e1 * t40 * t141 + 0.4e1 * t15 * kn * zc * t146;
		t151 = t113 * ZA;
		t156 = t4 * t43 * km;
		t165 = t15 * ZA;
		t169 = t105 * t16;
		t188 = t39 * zc * t9;
		t197 = 0.4e1 * t151 * t116 + 0.4e1 * t38 * ZB * t156 - 0.2e1 * t86 * t134 - 0.4e1 * t151 * t10 - t2 * kn * t146 - 0.2e1 * t165 * t4 * t33 - 0.4e1 * t169 * t124 - t3 * t30 - 0.4e1 * t113 * t134 + 0.4e1 * t127 * ZA * t120 + 0.2e1 * t38 * t4 * t43 * t53 + 0.4e1 * t169 * t96 - 0.4e1 * t60 * ZB * t156 + 0.4e1 * t165 * t188 + 0.2e1 * t60 * t4 * t43 * t53 - 0.8e1 * t93 * t141;
		t198 = kn * zc;
		t200 = exp(-0.4e1 * t198);
		t201 = t200 * ZA;
		t237 = 0.2e1 * t201 * t30 + 0.4e1 * t114 * t10 - 0.4e1 * t127 * t141 + 0.4e1 * t64 * t30 + 0.4e1 * t127 * t100 - 0.2e1 * t49 * kn * t57 + 0.4e1 * t40 * t100 + t27 * kn * t146 - t131 * t10 - 0.2e1 * t165 * t30 - 0.4e1 * t90 * t120 + 0.8e1 * t165 * t111 - 0.4e1 * t18 * ZA * t9 * t6 - 0.8e1 * t64 * t111 + 0.4e1 * t201 * t188 + 0.2e1 * t201 * t4 * t33;
		t245 = pow(t7 + t4, 0.2e1);
		t250 = kn * ZA;
		t263 = exp(-0.4e1 * kn);
		_C2B = -sigma * (t88 + t149 + t197 + t237) / kn / ZB / t245 / (0.4e1 * t198 * t64 + 0.4e1 * t250 * t6 * t15 - 0.4e1 * kn * t6 * t64 - 0.4e1 * t250 * zc * t15 + t99 * t2 + t28 + t140 * t200 - t3 - t140 * t263 - t99 * t27) / 0.4e1;
		
		t4 = exp(-kn * (0.4e1 * zc + 0.1e1));
		t5 = t4 * ZB;
		t7 = sin(km);
		t8 = km * km;
		t9 = t7 * t8;
		t10 = t9 * zc;
		t12 = exp(-kn);
		t14 = t9 * ZB;
		t18 = kn * (0.2e1 * zc + 0.1e1);
		t19 = exp(-t18);
		t20 = kn * kn;
		t21 = t20 * kn;
		t23 = zc * ZB;
		t24 = t23 * t7;
		t27 = kn * zc;
		t29 = exp(-0.3e1 * t27);
		t31 = km * zc;
		t32 = cos(t31);
		t33 = t32 * km;
		t34 = t33 * ZB;
		t41 = exp(-kn * (zc + 0.2e1));
		t45 = t29 * t21;
		t46 = sin(t31);
		t50 = t19 * ZA;
		t51 = t21 * zc;
		t56 = exp(-0.3e1 * kn);
		t61 = kn * ZA;
		t67 = t19 * ZB;
		t68 = t20 * t20;
		t70 = zc * zc;
		t71 = t68 * t7 * t70;
		t77 = km * ZB * zc;
		t80 = t19 * t20;
		t81 = t80 * ZB;
		t83 = t70 * t8 * t7;
		t86 = t4 * t21;
		t88 = t80 * ZA;
		t91 = t20 * t7;
		t93 = t12 * ZB;
		t96 = t8 * ZB;
		t100 = t7 * ZB;
		t102 = -t5 * kn * t10 - t12 * kn * t14 + 0.4e1 * t19 * t21 * t24 - 0.2e1 * t29 * kn * t34 - 0.4e1 * t20 * ZA * t32 * t31 * t41 - 0.4e1 * t45 * t23 * t46 - 0.6e1 * t50 * t51 * t7 - 0.2e1 * t56 * ZA * kn * t10 + 0.2e1 * t61 * t7 * t8 * zc * t19 - 0.4e1 * t67 * t71 + 0.4e1 * t41 * t20 * t32 * t77 - 0.4e1 * t81 * t83 - t86 * t24 - 0.4e1 * t88 * t10 + t5 * t91 - t93 * t91 + t12 * t7 * t96 - t4 * t7 * t96 + t86 * t100;
		t103 = t12 * t21;
		t105 = t29 * t20;
		t120 = ZA * zc;
		t158 = exp(-kn * (0.3e1 * zc + 0.2e1));
		t162 = exp(-t27);
		t167 = -t103 * t100 + 0.4e1 * t105 * ZA * t33 * zc + t93 * kn * t10 - 0.2e1 * t41 * kn * t34 - 0.4e1 * t51 * ZB * t46 * t41 - 0.4e1 * t19 * t68 * t120 * t7 + 0.4e1 * t67 * t68 * zc * t7 + t4 * kn * t14 + 0.4e1 * t88 * t83 + 0.4e1 * t50 * t71 + 0.4e1 * t45 * t120 * t46 - 0.2e1 * ZA * t21 * zc * t7 * t56 - 0.4e1 * t19 * kn * zc * t14 + 0.4e1 * t81 * t10 + 0.4e1 * t51 * ZA * t46 * t41 - 0.4e1 * t105 * t32 * t77 + 0.2e1 * t158 * kn * t34 + 0.2e1 * t162 * kn * t34 + t103 * t24;
		t173 = pow(t8 + t20, 0.2e1);
		t179 = exp(-0.2e1 * kn * (zc + 0.1e1));
		t180 = ZB * t179;
		t193 = exp(-0.2e1 * kn);
		t196 = exp(-0.2e1 * t18);
		t199 = exp(-0.4e1 * t27);
		t203 = exp(-0.4e1 * kn);
		_C3B = -sigma * (t102 + t167) * t162 / ZB / t173 / (0.4e1 * t27 * t180 + 0.4e1 * t61 * t70 * t179 - 0.4e1 * kn * t70 * t180 - 0.4e1 * t61 * zc * t179 + t23 * t193 + ZB * t196 + t120 * t199 - ZB * t193 - t120 * t203 - t23 * t196) / 0.4e1;
		
		t2 = kn * (zc + 0.1e1);
		t4 = exp(-0.2e1 * t2);
		t5 = t4 * ZA;
		t6 = kn * kn;
		t7 = t6 * kn;
		t9 = sin(km);
		t10 = t7 * zc * t9;
		t17 = exp(-0.2e1 * kn * (0.2e1 * zc + 0.1e1));
		t19 = km * km;
		t20 = t9 * t19;
		t21 = t20 * ZB;
		t25 = exp(-kn * (zc + 0.3e1));
		t26 = t25 * t7;
		t27 = zc * ZB;
		t28 = km * zc;
		t29 = sin(t28);
		t30 = t27 * t29;
		t33 = kn * zc;
		t35 = exp(-0.4e1 * t33);
		t36 = t35 * ZA;
		t39 = t4 * ZB;
		t40 = t6 * t6;
		t42 = t40 * zc * t9;
		t47 = t17 * ZB;
		t49 = t20 * zc;
		t52 = exp(-0.3e1 * t2);
		t54 = cos(t28);
		t55 = t54 * km;
		t56 = t55 * ZB;
		t62 = exp(-kn * (0.3e1 * zc + 0.1e1));
		t66 = t5 * t6;
		t70 = exp(-0.2e1 * kn);
		t72 = t19 * ZB;
		t74 = t6 * t9;
		t76 = t17 * t7;
		t77 = t9 * ZB;
		t79 = t70 * t7;
		t81 = ZB * t70;
		t85 = t62 * t6;
		t88 = km * ZB * zc;
		t91 = t39 * t6;
		t94 = t27 * t9;
		t96 = 0.6e1 * t5 * t10 + t17 * kn * t21 + 0.4e1 * t26 * t30 + 0.2e1 * t36 * t10 + 0.4e1 * t39 * t42 - 0.4e1 * t5 * t42 - t47 * kn * t49 - 0.2e1 * t52 * kn * t56 + 0.2e1 * t62 * kn * t56 - 0.4e1 * t66 * t49 - t70 * t9 * t72 - t47 * t74 + t76 * t77 - t79 * t77 + t81 * t74 + t17 * t9 * t72 + 0.4e1 * t85 * t54 * t88 + 0.4e1 * t91 * t49 + t79 * t94;
		t98 = t55 * zc;
		t105 = zc * zc;
		t106 = t40 * t9 * t105;
		t112 = ZA * zc;
		t113 = t112 * t29;
		t116 = t25 * t6;
		t121 = t105 * t19 * t9;
		t137 = t62 * t7;
		t148 = exp(-t2);
		t155 = -0.4e1 * t85 * ZA * t98 + 0.2e1 * t36 * kn * t49 + 0.4e1 * t5 * t106 + 0.2e1 * t25 * kn * t56 - 0.4e1 * t26 * t113 + 0.4e1 * t116 * ZA * t98 + 0.4e1 * t66 * t121 - 0.4e1 * t39 * t106 + t81 * kn * t49 - 0.4e1 * t4 * t7 * t94 - 0.4e1 * t116 * t54 * t88 - 0.2e1 * t5 * kn * t49 - 0.4e1 * t137 * t113 + 0.4e1 * t4 * kn * zc * t21 - 0.4e1 * t91 * t121 - t70 * kn * t21 - 0.2e1 * t148 * kn * t56 + 0.4e1 * t137 * t30 - t76 * t94;
		t160 = pow(t19 + t6, 0.2e1);
		t165 = kn * ZA;
		t178 = exp(-0.4e1 * kn);
		_C4B = -sigma * (t96 + t155) / ZB / t160 / (-0.4e1 * t33 * t39 - 0.4e1 * t165 * t105 * t4 + 0.4e1 * kn * t105 * t39 + 0.4e1 * t165 * zc * t4 - t27 * t70 - t47 - t112 * t35 + t81 + t112 * t178 + t27 * t17) / 0.4e1;
		
	}
	
	/*******************************************/
	/*       calculate the velocities etc      */
	/*******************************************/
	
	if ( z < zc ) {
		t4 = exp(-kn * z);
		t10 = exp(kn * (z - zc));
		t12 = kn * kn;
		t15 = sin(km * z);
		t18 = km * km;
		t20 = pow(t18 + t12, 0.2e1);
		u1=u1a = kn * ((_C1A + z * _C3A) * t4 + (_C2A + z * _C4A) * t10 + sigma * t12 * t15 / ZA / t20);
		
		t6 = exp(-kn * z);
		t14 = exp(kn * (z - zc));
		t16 = kn * kn;
		t19 = cos(km * z);
		t23 = km * km;
		t25 = pow(t23 + t16, 0.2e1);
		u2=u2a = ((_C1A + z * _C3A) * kn - _C3A) * t6 + (-_C4A - (_C2A + z * _C4A) * kn) * t14 - sigma * t16 * t19 * km / ZA / t25;
		
		t1 = kn * ZA;
		t7 = exp(-kn * z);
		t16 = exp(kn * (z - zc));
		t19 = kn * kn;
		t23 = cos(km * z);
		t25 = km * km;
		t27 = pow(t25 + t19, 0.2e1);
		u3=u3a = 0.2e1 * t1 * (-(_C1A + z * _C3A) * kn + _C3A) * t7 + 0.2e1 * t1 * (_C4A + (_C2A + z * _C4A) * kn) * t16 + 0.2e1 * t19 * kn * sigma * t23 * km / t27;
		
		t2 = kn * kn;
		t10 = exp(-kn * z);
		t21 = exp(kn * (z - zc));
		t25 = sin(km * z);
		t27 = km * km;
		t28 = 0.1e1 / ZA;
		t31 = pow(t27 + t2, 0.2e1);
		t32 = 0.1e1 / t31;
		t35 = t2 * t2;
		u4=u4a = -ZA * (-0.2e1 * _C3A * kn + 0.2e1 * t2 * (_C1A + z * _C3A)) * t10 - ZA * (0.2e1 * t2 * (_C2A + z * _C4A) + 0.2e1 * _C4A * kn) * t21 - ZA * (-sigma * t2 * t25 * t27 * t28 * t32 + t35 * sigma * t25 * t28 * t32);
		
		
		Z=ZA;
		
		
	} else {
		
		t5 = exp(kn * (zc - z));
		t11 = exp(kn * (z - 0.1e1));
		t13 = kn * kn;
		t16 = sin(km * z);
		t19 = km * km;
		t21 = pow(t19 + t13, 0.2e1);
		u1=u1b = kn * ((_C1B + z * _C3B) * t5 + (_C2B + z * _C4B) * t11 + sigma * t13 * t16 / ZB / t21);
		
		t7 = exp(kn * (zc - z));
		t15 = exp(kn * (z - 0.1e1));
		t17 = kn * kn;
		t20 = cos(km * z);
		t24 = km * km;
		t26 = pow(t24 + t17, 0.2e1);
		u2=u2b = ((_C1B + z * _C3B) * kn - _C3B) * t7 + (-_C4B - (_C2B + z * _C4B) * kn) * t15 - sigma * t17 * t20 * km / ZB / t26;
		
		t1 = ZB * kn;
		t8 = exp(kn * (zc - z));
		t17 = exp(kn * (z - 0.1e1));
		t20 = kn * kn;
		t24 = cos(km * z);
		t26 = km * km;
		t28 = pow(t26 + t20, 0.2e1);
		u3=u3b = 0.2e1 * t1 * (-(_C1B + z * _C3B) * kn + _C3B) * t8 + 0.2e1 * t1 * (_C4B + (_C2B + z * _C4B) * kn) * t17 + 0.2e1 * t20 * kn * sigma * t24 * km / t28;
		
		t2 = kn * kn;
		t11 = exp(kn * (zc - z));
		t22 = exp(kn * (z - 0.1e1));
		t26 = sin(km * z);
		t28 = km * km;
		t29 = 0.1e1 / ZB;
		t32 = pow(t28 + t2, 0.2e1);
		t33 = 0.1e1 / t32;
		t36 = t2 * t2;
		u4=u4b = -ZB * (-0.2e1 * _C3B * kn + 0.2e1 * t2 * (_C1B + z * _C3B)) * t11 - ZB * (0.2e1 * t2 * (_C2B + z * _C4B) + 0.2e1 * _C4B * kn) * t22 - ZB * (-sigma * t2 * t26 * t28 * t29 * t33 + t36 * sigma * t26 * t29 * t33);
		
		Z=ZB;
	}
	
	
	
	pp = (double)-(u3 + 2.0*n*M_PI*Z*u2)*cos(n*M_PI*x); /* pressure */
	
	txx = (double)(u3 + 4*Z*n*M_PI*u2)*cos(n*M_PI*x); /* xx stress */
	//printf( "() = %g\n", (u3 + 2.0*n*M_PI*Z*u2) );
	//printf( "cos = %g\n", cos(n*M_PI*x) );
	//printf( "pp = %g\n", pp );
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
	
	rho = -sigma*sin(km*z)*cos(n*M_PI*x); /* density */
	sum7 += rho;
	
	//mag=sqrt(sum1*sum1+sum2*sum2);
	/****************************************************/
	/****************************************************/
//	printf("%0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f %0.7f\n",x,z,sum1,sum2,sum3,sum4,sum5,sum6,mag,sum7);
	
	
	/* Output */
	if( vel != NULL ) {
		vel[0] = sum2;
		vel[1] = sum1;
	}
	if( presssure != NULL ) {
		//printf( "pressure = %g\n", *presssure );
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


