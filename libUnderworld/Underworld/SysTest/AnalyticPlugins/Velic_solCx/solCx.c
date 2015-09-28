/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include "solCx.h"

#ifdef MAIN

int main( int argc, char **argv )
{
  int i,j;
  double pos[2], vel[2], pressure, total_stress[3], strain_rate[3];
  double x,z;
  int N=4;
	
  for (i=0;i<N+1;i++){
    for(j=0;j<N+1;j++){
      x = i/(double)(N);
      z = j/(double)(N);
			
      pos[0] = x;
      pos[1] = z;
      _Velic_solCx( 
        pos, 
        100.0, 1.0,
        0.5, 2,
        vel, &pressure, total_stress, strain_rate );
			
      //printf("t_xx, t_xz fucked !! \n");
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

void _Velic_solCx(
  const double pos[],
  double _eta_A, double _eta_B, 	/* Input parameters: density, viscosity A, viscosity B */
  double _x_c, int _n, 			/* Input parameters: viscosity jump location, wavenumber in x */
  double vel[], double* presssure, 
  double total_stress[], double strain_rate[] )
{

  if(_eta_A > _eta_B){
    _solCx_A(pos, _eta_A, _eta_B, _x_c, _n, vel, presssure, total_stress, strain_rate);
  }
  else{
    _solCx_B(pos, _eta_A, _eta_B, _x_c, _n, vel, presssure, total_stress, strain_rate);
  }

}


/* ZA >> ZB */
void _solCx_A(
  const double pos[],
  double _eta_A, double _eta_B, 	/* Input parameters: density, viscosity A, viscosity B */
  double _x_c, int _n, 			/* Input parameters: viscosity jump location, wavenumber in x */
  double vel[], double* presssure, 
  double total_stress[], double strain_rate[] )
{
  double Z,u1,u2,u3,u4,u5,u6,ZA,ZB,ZR;
  double sum1,sum2,sum3,sum4,sum5,sum6,mag,x,z,xc;
  double num1A,num2A,num3A,num4A,num1B,num2B,num3B,num4B,denA,denB;
  double _C1A,_C2A,_C3A,_C4A,_C1B,_C2B,_C3B,_C4B,_C1,_C2,_C3,_C4;
  int n,nx;
  double kx, kn;

  double t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
  double t11,t12,t13,t14,t15,t16,t17,t18,t19,t20;
  double t21,t22,t23,t24,t25,t26,t27,t28,t29,t30;
  double t31,t32,t33,t34,t35,t36,t37,t38,t39,t40;
  double t42,t43,t44,t45,t46,t47,t48,t49,t50,t51;
  double t52,t53,t54,t55,t56,t57,t58,t59,t60,t61;
  double t62,t63,t64,t66,t67,t69,t70,t71,t72,t73;
  double t74,t76,t77,t78,t79,t80,t81,t82,t83,t84;
  double t85,t86,t87,t88,t89,t90,t91,t92,t93,t94;
  double t95,t96,t97,t98,t99,t100,t101,t102,t103,t104;
  double t105,t106,t107,t108,t109,t110,t112,t113,t114,t115;
  double t116,t118,t119,t121,t122,t123,t124,t125,t126,t127;
  double t128,t129,t132,t134,t135,t136,t137,t138,t139,t140;
  double t141,t142,t143,t144,t146,t147,t148,t149,t150,t152;
  double t153,t154,t155,t156,t157,t159,t160,t162,t164,t166;
  double t167,t168,t169,t171,t172,t173,t175,t177,t178,t180;
  double t181,t183,t184,t185,t187,t188,t189,t190,t192,t193;
  double t194,t195,t197,t198,t199,t200,t202,t204,t208,t211;
  double t214,t215,t216,t217,t218,t220,t223,t224,t226,t229;
  double t230,t232,t233,t236,t237,t239,t243,t248,t252,t256;
  double t257,t259,t263,t267,t272,t275,t281,t284,t294,t303;
  double t305,t306,t309,t314,t322,t325,t365,t369,t380,t384;
  double t392,t414,t456;

  nx=1.0;
  kx=nx*M_PI;
  kn=_n*M_PI;
    
  ZA=_eta_A; /* left column viscosity */
  ZB=_eta_B; /* right column viscosity */
  xc = _x_c;
	
  x = pos[0];
  z = pos[1];
	
  sum1=0.0;
  sum2=0.0;
  sum3=0.0;
  sum4=0.0;
  sum5=0.0;
  sum6=0.0;
	
  ZR=ZB/ZA;

  if (x<xc){
    t1 = kn * kn;
    t4 = exp(-kn * (xc + 0.2e1));
    t5 = t1 * t4;
    t7 = xc * xc;
    t9 = kx * xc;
    t10 = cos(t9);
    t11 = kx * t7 * t10;
    t14 = t1 * kn;
    t17 = exp(kn * (xc - 0.2e1));
    t18 = t14 * t17;
    t19 = ZR * ZR;
    t21 = sin(t9);
    t22 = t19 * t7 * t21;
    t25 = t5 * t19;
    t28 = t1 * t17;
    t31 = sin(kx);
    t32 = 0.2e1 * xc;
    t35 = exp(-kn * (t32 + 0.1e1));
    t36 = t31 * t35;
    t37 = kx * kx;
    t38 = t36 * t37;
    t39 = kn * xc;
    t40 = t39 * ZR;
    t45 = exp(kn * (xc - 0.4e1));
    t46 = t1 * t45;
    t47 = xc * t21;
    t49 = kn * t45;
    t50 = t19 * t21;
    t52 = t1 * xc;
    t55 = t14 * t7;
    t58 = t1 * ZR;
    t60 = kn * ZR;
    t63 = t14 * xc;
    t66 = t37 * ZR;
    t70 = exp(kn * (t32 - 0.3e1));
    t71 = t31 * t70;
    t72 = t55 * ZR;
    t76 = exp(-0.3e1 * t39);
    t77 = kn * t76;
    t78 = t9 * t10;
    t80 = 0.8e1 * t5 * ZR * t11 + 0.4e1 * t18 * t22 - 0.4e1 * t25 * t11 - 0.4e1 * t28 * t11 + 0.2e1 * t38 * t40 + t46 * t47 + t49 * t50 - 0.4e1 * t36 * t52 + 0.2e1 * t36 * t55 - t36 * t58 - 0.2e1 * t36 * t60 - 0.2e1 * t36 * t63 - t36 * t66 + 0.2e1 * t71 * t72 + t77 * t78;
    t81 = kn * t4;
    t82 = ZR * t10;
    t83 = t82 * kx;
    t86 = t1 * t76;
    t88 = t19 * xc * t21;
    t90 = kx * t10;
    t95 = t14 * t4;
    t96 = t7 * t21;
    t99 = ZR * t21;
    t102 = exp(-t39);
    t103 = t1 * t102;
    t109 = kn * t17;
    t112 = t63 * ZR;
    t115 = t52 * ZR;
    t118 = t99 * xc;
    t125 = 0.4e1 * t81 * t83 + t86 * t88 - 0.4e1 * t81 * t90 + 0.2e1 * t5 * t47 - 0.4e1 * t95 * t96 + 0.4e1 * t5 * t99 + t103 * t47 - 0.4e1 * t5 * t50 - 0.4e1 * t18 * t47 - 0.2e1 * t109 * t99 + 0.2e1 * t36 * t112 + 0.4e1 * t36 * t115 + 0.8e1 * t18 * t118 - 0.6e1 * t28 * t118 + 0.4e1 * t95 * t88;
    t135 = t82 * xc;
    t143 = exp(kn * (0.3e1 * xc - 0.4e1));
    t144 = kn * t143;
    t150 = t37 * kn * xc;
    t154 = t37 * t7 * kn;
    t162 = kn * t102;
    t167 = t1 * t143;
    t171 = -0.4e1 * t18 * t88 + 0.8e1 * t28 * ZR * t11 - 0.4e1 * t109 * t83 - 0.8e1 * t28 * kx * t135 - 0.2e1 * t86 * t118 - t144 * t19 * t78 - 0.2e1 * t36 * t72 - 0.2e1 * t36 * t150 + 0.2e1 * t36 * t154 + t77 * t19 * t78 - 0.2e1 * t77 * ZR * t78 + t162 * t19 * t78 - 0.4e1 * t95 * t22 - 0.2e1 * t167 * t118 - t103 * t88;
    t177 = exp(-kn);
    t178 = t31 * t177;
    t194 = exp(-0.3e1 * kn);
    t195 = t31 * t194;
    t200 = 0.4e1 * t109 * t90 + t167 * t47 - 0.2e1 * t144 * t99 - t178 * t58 - 0.2e1 * t178 * t60 - t178 * t66 - t162 * t50 + t86 * t47 + 0.2e1 * t28 * t47 + 0.4e1 * t18 * t96 + 0.4e1 * t28 * t99 - 0.4e1 * t28 * t50 + 0.2e1 * t195 * t60 - t195 * t66 - t195 * t58;
    t204 = ZR * t7 * t21;
    t208 = t7 * kn * ZR;
    t236 = 0.8e1 * t95 * t204 - 0.2e1 * t38 * t208 - 0.8e1 * t5 * kx * t135 + 0.4e1 * t5 * t88 - 0.2e1 * t71 * t112 + 0.2e1 * t109 * ZR * t78 - 0.2e1 * t178 * kn - t178 * t37 + t162 * t21 + t49 * t78 - 0.6e1 * t5 * t118 + 0.2e1 * t144 * ZR * t78 + t167 * t88 + 0.4e1 * t71 * t115 + 0.4e1 * t25 * t78;
    t252 = t71 * t37;
    t256 = -0.2e1 * t71 * t154 - t144 * t78 - 0.4e1 * t5 * t11 + 0.2e1 * t81 * t78 - t162 * t78 - t178 * t1 - t49 * t21 + t36 * t1 + 0.2e1 * t36 * kn + t38 + t144 * t21 + t71 * t1 + t252 - 0.2e1 * t71 * kn - t77 * t21;
    t284 = -0.2e1 * t81 * t21 - t195 * t1 + 0.2e1 * t195 * kn - t195 * t37 + 0.2e1 * t109 * t21 - 0.2e1 * t81 * ZR * t78 + 0.4e1 * t5 * t78 + 0.2e1 * t252 * t208 + t144 * t50 + 0.2e1 * t71 * t63 - t71 * t58 - t71 * t66 - 0.4e1 * t71 * t52 + 0.2e1 * t71 * t60 - 0.2e1 * t71 * t55;
    t303 = t28 * t19;
    t314 = 0.2e1 * t77 * t99 - t77 * t50 + 0.4e1 * t95 * t47 + 0.2e1 * t81 * t99 + 0.2e1 * t71 * t150 - 0.2e1 * t252 * t40 - 0.8e1 * t95 * t118 + 0.4e1 * t28 * t88 - t46 * t88 + 0.4e1 * t28 * t78 - 0.4e1 * t303 * t11 - t49 * t19 * t78 - 0.8e1 * t18 * t204 + 0.4e1 * t303 * t78 - 0.2e1 * t109 * t78;
    num1A = -kx * (t80 + t125 + t171 + t200 + t236 + t256 + t284 + t314);

    t1 = xc * xc;
    t2 = -t1 + xc;
    t3 = 0.4e1 * t2;
    t4 = ZR * ZR;
    t7 = -0.8e1 * t2;
    t9 = 0.4e1 * t1;
    t10 = 0.4e1 * xc;
    t11 = t3 * t4 + t7 * ZR - t9 + t10;
    t13 = kn * kn;
    t14 = t13 * kn;
    t22 = 0.2e1 * xc;
    t25 = ((0.4e1 * xc - 0.4e1) * t4 + (0.4e1 - 0.6e1 * xc) * ZR + t22) * kx * t13;
    t26 = -0.1e1 + ZR;
    t27 = 0.2e1 * t26;
    t28 = t27 * kx;
    t31 = kx * xc;
    t32 = sin(t31);
    t34 = kx * kx;
    t36 = t11 * t34 * t13;
    t37 = -t22 + 0.4e1;
    t43 = cos(t31);
    t48 = exp(kn * (x - 0.2e1 - xc));
    t50 = -ZR - 0.1e1;
    t52 = t50 * kx * t13;
    t54 = -0.2e1 * t50;
    t57 = t34 * kx;
    t58 = t50 * t57;
    t60 = sin(kx);
    t64 = exp(kn * (x - 0.3e1));
    t74 = -t27 * kx;
    t88 = exp(kn * (x + xc - 0.2e1));
    t90 = t4 * xc;
    t93 = t90 - 0.2e1 * ZR * xc + xc;
    t95 = t93 * kx * t13;
    t97 = t4 - 0.2e1 * ZR + 0.1e1;
    t104 = kn * t43;
    t107 = 0.3e1 * xc;
    t110 = exp(kn * (x + t107 - 0.4e1));
    t112 = 0.2e1 * t2;
    t114 = 0.2e1 * t1;
    t115 = t112 * ZR + t114 - t22;
    t122 = ((t10 - 0.1e1) * ZR - t10 + 0.1e1) * kx * t13;
    t127 = -t26 * t57;
    t132 = exp(kn * (x - t22 - 0.1e1));
    t134 = -t90 + xc;
    t136 = t134 * kx * t13;
    t137 = 0.1e1 - t4;
    t148 = exp(kn * (x - xc));
    t160 = exp(kn * (x - t107));
    t169 = exp(kn * (x - 0.1e1));
    t173 = -t112 * ZR + t22 - t114;
    t183 = exp(kn * (x + t22 - 0.3e1));
    t195 = exp(kn * (x + xc - 0.4e1));
    num2A = ((t11 * kx * t14 + t25 + t28 * kn) * t32 + (t36 + (t37 * ZR - 0.4e1 + t22) * t34 * kn) * t43) * t48 + (t52 + t54 * kx * kn + t58) * t60 * t64 + (((-t3 * t4 - t7 * ZR - t10 + t9) * kx * t14 + t25 + t74 * kn) * t32 + (t36 + (-t37 * ZR - t22 + 0.4e1) * t34 * kn) * t43) * t88 + ((t95 + t97 * kx * kn) * t32 - t93 * t34 * t104) * t110 + (t115 * kx * t14 + t122 + (t115 * t57 + t74) * kn + t127) * t60 * t132 + ((t136 + t137 * kx * kn) * t32 - t134 * t34 * t104) * t148 + ((t95 - t97 * kx * kn) * t32 + t93 * t34 * t104) * t160 + (t52 - t54 * kx * kn + t58) * t60 * t169 + (t173 * kx * t14 + t122 + (t173 * t57 + t28) * kn + t127) * t60 * t183 + ((t136 - t137 * kx * kn) * t32 + t134 * t34 * t104) * t195;

    t3 = exp(kn * (xc - 0.2e1));
    t4 = kn * t3;
    t5 = kx * xc;
    t6 = cos(t5);
    t7 = ZR * t6;
    t8 = t7 * kx;
    t11 = kn * kn;
    t14 = exp(-kn * (xc + 0.2e1));
    t15 = t11 * t14;
    t16 = ZR * ZR;
    t18 = t16 * kx * t6;
    t21 = sin(kx);
    t22 = 0.2e1 * xc;
    t25 = exp(kn * (t22 - 0.3e1));
    t26 = t21 * t25;
    t27 = t11 * kn;
    t28 = t27 * xc;
    t29 = t28 * ZR;
    t32 = kx * kx;
    t33 = t32 * kn;
    t34 = t33 * xc;
    t37 = t26 * t32;
    t38 = kn * xc;
    t39 = t38 * ZR;
    t42 = kx * t6;
    t43 = t42 * xc;
    t48 = exp(-kn * (t22 + 0.1e1));
    t49 = t21 * t48;
    t50 = t49 * t32;
    t55 = t27 * t14;
    t56 = sin(t5);
    t57 = ZR * t56;
    t63 = t27 * t3;
    t64 = t16 * t56;
    t69 = t11 * t3;
    t72 = t56 * xc;
    t77 = kn * t14;
    t83 = exp(-0.3e1 * kn);
    t84 = t21 * t83;
    t85 = t11 * ZR;
    t89 = t32 * ZR;
    t93 = -0.4e1 * t63 * t64 + 0.8e1 * t63 * t57 + 0.4e1 * t69 * t42 + 0.4e1 * t63 * t72 + 0.2e1 * t15 * t57 + 0.2e1 * t77 * t42 - 0.2e1 * t26 * t28 + t84 * t85 + 0.8e1 * t15 * t8 + t84 * t89 - 0.2e1 * t69 * t57;
    t95 = exp(-kn);
    t96 = t21 * t95;
    t102 = exp(kn * (0.3e1 * xc - 0.4e1));
    t103 = kn * t102;
    t118 = exp(kn * (xc - 0.4e1));
    t119 = kn * t118;
    t121 = exp(-t38);
    t122 = t11 * t121;
    t127 = kn * t121;
    t129 = t11 * t118;
    t138 = t27 * ZR;
    t141 = t11 * t102;
    t146 = exp(-0.3e1 * t38);
    t147 = kn * t146;
    t149 = -0.8e1 * t69 * t8 - t127 * t42 + t129 * t64 + t49 * t89 - 0.2e1 * t49 * t28 - 0.3e1 * t49 * t85 + 0.2e1 * t26 * t33 - 0.2e1 * t26 * t138 + t141 * t64 - 0.2e1 * t141 * t57 - t147 * t42;
    t159 = t11 * t146;
    t167 = t7 * xc;
    t172 = t33 * ZR;
    t175 = t64 * xc;
    t181 = t57 * xc;
    t198 = -0.2e1 * t26 * t172 + 0.4e1 * t55 * t175 + 0.4e1 * t15 * t16 * t43 - 0.8e1 * t55 * t181 - 0.2e1 * t49 * t172 - 0.2e1 * t77 * t8 + 0.4e1 * t69 * t18 + t127 * t18 - 0.4e1 * t69 * t43 - 0.8e1 * t63 * t181 + 0.8e1 * t69 * kx * t167;
    t230 = -t96 * t32 - 0.4e1 * t63 * t56 + t84 * t11 + t37 - 0.3e1 * t26 * t11 - t50 + 0.3e1 * t49 * t11 - t96 * t11 + t122 * t56 - t129 * t56 + t84 * t32;
    num3A = -kx * (0.2e1 * t50 * t39 + 0.2e1 * t37 * t39 + t149 + 0.4e1 * t63 * t175 - t96 * t89 + 0.8e1 * t55 * t57 - t96 * t85 - t159 * t64 + 0.2e1 * t103 * t8 - 0.2e1 * t26 * t34 + 0.4e1 * t55 * t72 + 0.2e1 * t159 * t57 - 0.2e1 * t49 * t34 - t103 * t18 - 0.2e1 * t15 * t56 + t230 - t26 * t89 - t147 * t18 - t103 * t42 + 0.3e1 * t26 * t85 - 0.2e1 * t4 * t8 + t119 * t18 + t198 - t119 * t42 - 0.8e1 * t15 * kx * t167 - 0.4e1 * t69 * t16 * t43 + 0.2e1 * t69 * t56 + 0.2e1 * t26 * t29 + 0.2e1 * t49 * t33 - t159 * t56 + 0.2e1 * t49 * t27 + 0.4e1 * t15 * t43 - 0.4e1 * t15 * t18 + 0.2e1 * t49 * t29 + 0.2e1 * t4 * t42 + t93 + t141 * t56 - 0.4e1 * t15 * t42 + 0.2e1 * t26 * t27 - 0.4e1 * t55 * t56 - 0.2e1 * t49 * t138 + 0.2e1 * t147 * t8 - 0.4e1 * t55 * t64 - t122 * t64);

    t1 = 0.1e1 - xc;
    t2 = 0.4e1 * t1;
    t3 = ZR * ZR;
    t6 = -0.8e1 * t1;
    t8 = 0.4e1 * xc;
    t9 = t2 * t3 + ZR * t6 + 0.4e1 - t8;
    t11 = kn * kn;
    t12 = t11 * kn;
    t13 = t9 * kx * t12;
    t14 = -ZR + 0.1e1;
    t15 = 0.2e1 * t14;
    t19 = kx * xc;
    t20 = sin(t19);
    t22 = kx * kx;
    t25 = -t15;
    t27 = t25 * t22 * kn;
    t29 = cos(t19);
    t34 = exp(kn * (x - 0.2e1 - xc));
    t36 = -ZR - 0.1e1;
    t39 = t22 * kx;
    t42 = sin(kx);
    t46 = exp(kn * (x - 0.3e1));
    t64 = exp(kn * (x + xc - 0.2e1));
    t67 = -t3 + 0.2e1 * ZR - 0.1e1;
    t69 = t11 * t20;
    t71 = -t67;
    t73 = kn * t29;
    t74 = t71 * t22 * t73;
    t76 = 0.3e1 * xc;
    t79 = exp(kn * (x + t76 - 0.4e1));
    t83 = 0.2e1 * xc;
    t84 = 0.2e1 * t1 * ZR - 0.2e1 + t83;
    t86 = t84 * kx * t12;
    t87 = -t14;
    t88 = 0.3e1 * t87;
    t92 = t84 * t39 * kn;
    t98 = exp(kn * (x - t83 - 0.1e1));
    t100 = t3 - 0.1e1;
    t103 = -t100;
    t105 = t103 * t22 * t73;
    t109 = exp(kn * (x - xc));
    t116 = exp(kn * (x - t76));
    t118 = -t36;
    t126 = exp(kn * (x - 0.1e1));
    t136 = exp(kn * (x + t83 - 0.3e1));
    t143 = exp(kn * (x + xc - 0.4e1));
    num4A = ((t13 + t15 * kx * t11) * t20 + (t9 * t22 * t11 + t27) * t29) * t34 + (t36 * kx * t11 + t36 * t39) * t42 * t46 + ((t13 + t25 * kx * t11) * t20 + ((-t2 * t3 - ZR * t6 + t8 - 0.4e1) * t22 * t11 + t27) * t29) * t64 + (t67 * kx * t69 + t74) * t79 + (t86 + t88 * kx * t11 + t92 + t14 * t39) * t42 * t98 + (t100 * kx * t69 + t105) * t109 + (t71 * kx * t69 + t74) * t116 + (t118 * kx * t11 + t118 * t39) * t42 * t126 + (t86 - t88 * kx * t11 + t92 + t87 * t39) * t42 * t136 + (t103 * kx * t69 + t105) * t143;


    t1 = kn * kn;
    t3 = exp(-0.2e1 * kn);
    t4 = t1 * t3;
    t8 = ZR * ZR;
    t9 = xc * xc;
    t19 = exp(-0.2e1 * kn * (xc + 0.1e1));
    t20 = kn * t19;
    t21 = t8 * xc;
    t24 = kn * xc;
    t28 = exp(0.2e1 * kn * (xc - 0.2e1));
    t33 = kn * (xc - 0.1e1);
    t35 = exp(0.2e1 * t33);
    t36 = kn * t35;
    t40 = exp(-0.2e1 * t24);
    t47 = exp(-0.4e1 * t24);
    t49 = exp(0.4e1 * t33);
    t56 = exp(-0.4e1 * kn);
    t59 = 0.1e1 + 0.32e2 * t4 * ZR * xc + 0.16e2 * t4 * t8 * t9 - 0.32e2 * t4 * ZR * t9 + 0.4e1 * t20 * t21 + 0.4e1 * t24 * t28 * t8 - 0.4e1 * t36 * t21 - 0.4e1 * t24 * t40 * t8 - 0.16e2 * t4 * t21 - t47 - t49 * t8 + 0.2e1 * t47 * ZR + 0.2e1 * t49 * ZR + t56 * t8 - 0.4e1 * t36;
    t83 = 0.2e1 * t56 * ZR - 0.8e1 * t3 * ZR + 0.4e1 * t20 + 0.2e1 * ZR + t8 - t47 * t8 - 0.4e1 * t20 * xc - 0.4e1 * t20 * t8 - 0.4e1 * t24 * t28 + 0.4e1 * t36 * xc + 0.4e1 * t36 * t8 + 0.4e1 * t24 * t40 + 0.16e2 * t4 * t9 - 0.16e2 * t4 * xc + t56 - t49;
    denA = kn * (t59 + t83);

    _C1A=num1A/denA; _C2A=num2A/denA; _C3A=num3A/denA; _C4A=num4A/denA;
    Z=ZA; _C1=_C1A; _C2=_C2A; _C3=_C3A; _C4=_C4A;
  }else{
    t1 = sin(kx);
    t2 = 0.2e1 * xc;
    t5 = exp(kn * (-0.1e1 + t2));
    t6 = t1 * t5;
    t7 = kx * kx;
    t9 = t7 * xc * kn;
    t12 = kn * kn;
    t13 = kn * xc;
    t15 = exp(-0.3e1 * t13);
    t16 = t12 * t15;
    t17 = ZR * ZR;
    t19 = kx * xc;
    t20 = sin(t19);
    t21 = t17 * xc * t20;
    t24 = t12 * kn;
    t27 = exp(kn * (xc - 0.2e1));
    t28 = t24 * t27;
    t29 = ZR * t20;
    t30 = t29 * xc;
    t33 = t12 * t27;
    t34 = cos(t19);
    t35 = kx * t34;
    t36 = t35 * xc;
    t39 = t33 * t17;
    t40 = xc * xc;
    t42 = kx * t40 * t34;
    t45 = kn * t15;
    t52 = ZR * t40 * t20;
    t57 = exp(-kn * (t2 + 0.1e1));
    t58 = t1 * t57;
    t59 = t58 * t7;
    t61 = t17 * t40 * t20;
    t66 = exp(t13);
    t67 = kn * t66;
    t73 = exp(-t13);
    t74 = t12 * t73;
    t79 = kn * t27;
    t80 = ZR * t34;
    t81 = t80 * kx;
    t90 = exp(-kn);
    t91 = t1 * t90;
    t92 = t40 * xc;
    t94 = t12 * t92 * t7;
    t97 = -0.2e1 * t6 * t9 - 0.2e1 * t16 * t21 - 0.32e2 * t28 * t30 - 0.16e2 * t33 * t36 + 0.8e1 * t39 * t42 + 0.2e1 * t45 * t20 - 0.2e1 * t45 * t17 * t36 + 0.16e2 * t28 * t52 - t59 - 0.8e1 * t28 * t61 - 0.2e1 * t58 * kn + 0.2e1 * t67 * t36 - 0.16e2 * t33 * ZR * t42 - 0.4e1 * t74 * t21 - 0.2e1 * t67 * t20 - 0.8e1 * t79 * t81 + 0.12e2 * t33 * t30 + 0.32e2 * t33 * kx * t80 * xc + 0.8e1 * t91 * t94;
    t107 = exp(kn * (t2 - 0.3e1));
    t108 = t1 * t107;
    t112 = t12 * t40 * t7;
    t121 = exp(-kn * (xc + 0.2e1));
    t122 = t12 * t121;
    t125 = t12 * t66;
    t139 = t24 * t17;
    t140 = t139 * t40;
    t143 = t6 * t7;
    t147 = t91 * t17;
    t154 = t139 * xc;
    t157 = 0.4e1 * t79 * t17 * t36 - t58 * t12 - 0.4e1 * t79 * ZR * t36 - 0.4e1 * t108 * t9 - 0.8e1 * t91 * t112 + 0.4e1 * t91 * t9 - 0.2e1 * t45 * t36 - 0.4e1 * t122 * t20 - 0.2e1 * t125 * t21 + 0.16e2 * t28 * t21 + 0.4e1 * t16 * t30 + 0.2e1 * t6 * kn + 0.4e1 * t13 * t1 * t107 * t17 * t7 - 0.16e2 * t91 * t140 + t143 + 0.4e1 * t45 * ZR * t36 - 0.8e1 * t147 * t112 + 0.8e1 * t147 * t94 - 0.2e1 * t122 * t21 + 0.4e1 * t91 * t154;
    t162 = t91 * t7;
    t166 = t17 * t7;
    t168 = t17 * t20;
    t171 = ZR * t12;
    t175 = kn * t73;
    t181 = t12 * xc;
    t184 = kn * t17;
    t185 = t184 * xc;
    t189 = exp(-0.3e1 * kn);
    t190 = t1 * t189;
    t192 = t12 * t17;
    t194 = t12 * t12;
    t195 = ZR * t194;
    t199 = t24 * xc;
    t202 = t24 * t40;
    t215 = t91 * ZR;
    t220 = -0.2e1 * t67 * t17 * t36 - 0.8e1 * t162 * t13 * ZR - t6 * t166 - 0.4e1 * t79 * t168 - 0.16e2 * t91 * t171 * xc + 0.4e1 * t175 * ZR * t36 + 0.8e1 * t74 * t42 + 0.4e1 * t58 * t181 + 0.2e1 * t143 * t185 + t190 * t7 + t190 * t192 - 0.16e2 * t91 * t195 * t92 + 0.4e1 * t91 * t199 - 0.2e1 * t58 * t202 - 0.16e2 * t91 * t202 + 0.8e1 * t91 * t194 * t92 - 0.8e1 * t91 * t194 * t40 + 0.8e1 * t91 * t181 - 0.16e2 * t215 * t94 - 0.4e1 * t175 * t29;
    t223 = xc * t20;
    t226 = kn * t121;
    t232 = exp(kn * (-0.2e1 + 0.3e1 * xc));
    t233 = t12 * t232;
    t243 = t17 * kx * t34;
    t256 = t7 * ZR;
    t259 = kn * ZR;
    t272 = 0.2e1 * t58 * t199 + 0.2e1 * t122 * t223 + 0.4e1 * t226 * t35 - 0.2e1 * t233 * t223 + 0.4e1 * t79 * t29 + 0.16e2 * t28 * t223 + 0.4e1 * t122 * t168 + 0.8e1 * t79 * t243 - 0.8e1 * t74 * t223 - 0.8e1 * t33 * t223 - 0.2e1 * t16 * t223 + 0.12e2 * t74 * t30 + 0.4e1 * t175 * t168 + 0.4e1 * t91 * t256 + 0.8e1 * t91 * t259 + 0.4e1 * t91 * t171 + 0.2e1 * t125 * t223 - 0.4e1 * t190 * t259 + 0.8e1 * t33 * t168 - 0.8e1 * t33 * t29;
    t275 = t40 * t20;
    t294 = t7 * t40 * kn;
    t305 = exp(kn * (-0.3e1 + 0.4e1 * xc));
    t306 = t1 * t305;
    t309 = t24 * t73;
    t322 = kn * t232;
    t325 = -0.8e1 * t28 * t275 - 0.4e1 * t175 * t17 * t36 - 0.4e1 * t108 * t199 - 0.16e2 * t36 * t39 + 0.2e1 * t190 * t171 + 0.2e1 * t190 * t256 + 0.2e1 * t45 * t168 - 0.4e1 * t45 * t29 + 0.2e1 * t6 * t294 - 0.16e2 * t74 * ZR * t42 + 0.8e1 * t108 * t181 - 0.4e1 * t306 * t259 + 0.8e1 * t309 * t61 + 0.2e1 * t306 * t171 + t58 * t166 + 0.2e1 * t58 * t184 + t58 * t192 - 0.2e1 * t226 * t168 + 0.16e2 * t215 * t112 + 0.4e1 * t322 * t29;
    t365 = -0.8e1 * t233 * t29 - 0.4e1 * t322 * t35 - t306 * t166 + 0.2e1 * t306 * t184 - t306 * t192 + 0.2e1 * t306 * t256 + 0.2e1 * t6 * t202 - 0.4e1 * t6 * t181 - 0.2e1 * t322 * t168 + 0.4e1 * t233 * t168 + t190 * t166 - 0.2e1 * t190 * t184 - 0.2e1 * t190 * kn - 0.2e1 * t6 * t184 - t6 * t192 - 0.2e1 * t6 * t199 + 0.8e1 * t74 * t17 * t42 + 0.2e1 * t58 * t9 + 0.2e1 * t226 * t17 * t36 + 0.16e2 * t91 * t195 * t40;
    t369 = t192 * xc;
    t380 = t17 * t194;
    t384 = ZR * t24;
    t392 = t108 * t17;
    t414 = -0.4e1 * t226 * t243 - 0.4e1 * t58 * t369 - 0.2e1 * t58 * t294 - 0.2e1 * t226 * t36 + 0.2e1 * t6 * t154 + 0.2e1 * t58 * t140 + 0.8e1 * t91 * t380 * t92 - 0.8e1 * t91 * t384 * xc - 0.2e1 * t59 * t185 + 0.2e1 * t322 * t36 + 0.4e1 * t199 * t392 + t190 * t12 + 0.4e1 * t6 * t369 - 0.2e1 * t58 * t154 + 0.2e1 * t58 * t17 * t294 - 0.8e1 * t91 * t380 * t40 - t306 * t12 + 0.8e1 * t91 * t369 + 0.4e1 * t233 * t30 - 0.4e1 * t322 * ZR * t36;
    t456 = -0.2e1 * t322 * t20 + 0.32e2 * t91 * t384 * t40 + 0.8e1 * t322 * t81 + 0.2e1 * t226 * t20 + 0.4e1 * t233 * t20 + 0.8e1 * t309 * t275 - 0.8e1 * t181 * t392 + 0.4e1 * t162 * t185 - 0.2e1 * t233 * t21 + 0.8e1 * t33 * t42 - 0.16e2 * t309 * t52 - 0.2e1 * t6 * t17 * t294 + 0.2e1 * t322 * t17 * t36 + 0.2e1 * t67 * t168 + 0.2e1 * t306 * kn + t6 * t12 - 0.4e1 * t33 * t21 - t306 * t7 - 0.4e1 * t322 * t243 - 0.2e1 * t6 * t140;
    num1B = kx * (t97 + t157 + t220 + t272 + t325 + t365 + t414 + t456);

    t1 = xc * xc;
    t2 = 0.8e1 * t1;
    t3 = 0.16e2 * xc;
    t5 = ZR * ZR;
    t8 = 0.16e2 * t1;
    t11 = (-t2 + t3) * t5 + (-0.32e2 * xc + t8) * ZR - t2 + t3;
    t13 = kn * kn;
    t14 = t13 * kn;
    t16 = 0.4e1 * xc;
    t17 = t16 - 0.8e1;
    t22 = 0.8e1 * xc;
    t27 = 0.4e1 * ZR - 0.4e1 * t5;
    t31 = kx * xc;
    t32 = sin(t31);
    t34 = kx * kx;
    t37 = 0.8e1 + t16;
    t45 = cos(t31);
    t50 = exp(kn * (x - 0.2e1 - xc));
    t52 = t1 * xc;
    t53 = -t52 + t1;
    t60 = 0.8e1 * t53 * t5 - 0.16e2 * t53 * ZR - 0.8e1 * t52 + t2;
    t62 = t13 * t13;
    t72 = kx * t34;
    t74 = t5 * xc;
    t82 = ZR * xc;
    t84 = 0.4e1 * t74;
    t94 = sin(kx);
    t98 = exp(kn * (x - 0.3e1));
    t100 = 0.2e1 * xc;
    t102 = (-0.4e1 + t100) * t5;
    t103 = t102 - t100 + 0.4e1;
    t106 = 0.1e1 - t5;
    t107 = 0.2e1 * t106;
    t108 = t107 * kx;
    t113 = kn * t45;
    t118 = exp(kn * (x + xc - 0.2e1));
    t122 = t100 + 0.2e1 * t74 - 0.4e1 * t82;
    t125 = 0.4e1 * ZR;
    t126 = 0.2e1 * t5;
    t127 = 0.2e1 - t125 + t126;
    t129 = t127 * kx * kn;
    t136 = 0.3e1 * xc;
    t139 = exp(kn * (x + t136 - 0.4e1));
    t141 = t74 - xc;
    t142 = 0.2e1 * t141;
    t146 = -t107 * kx;
    t156 = exp(kn * (x - xc - 0.4e1));
    t160 = t102 - t17 * ZR - 0.4e1 + t100;
    t173 = exp(kn * (x - t136 - 0.2e1));
    t175 = 0.4e1 * t141;
    t187 = exp(kn * (x - t100 - 0.1e1));
    t189 = 0.2e1 * ZR;
    t190 = t5 - t189 + 0.1e1;
    t198 = exp(kn * (x - t16 - 0.1e1));
    t200 = -t5 - t189 - 0.1e1;
    t211 = exp(kn * (x - 0.1e1));
    t214 = 0.2e1 * t1 - 0.2e1 * xc;
    t216 = 0.2e1 * t1;
    t217 = t214 * t5 + t100 - t216;
    t220 = t16 - 0.1e1;
    t233 = exp(kn * (x + t100 - 0.3e1));
    t239 = t2 + 0.8e1 * t1 * t5 - 0.16e2 * ZR * t1;
    t263 = exp(kn * (x + xc - 0.4e1));
    t267 = -t214 * t5 + t216 - t100;
    t284 = exp(kn * (x - t100 - 0.3e1));
    num2B = ((t11 * kx * t14 + (t17 * t5 + (-0.12e2 * xc + 0.8e1) * ZR + t22) * kx * t13 + t27 * kx * kn) * t32 + (t11 * t34 * t13 + (t37 * t5 - t37 * ZR) * t34 * kn) * t45) * t50 + (t60 * kx * t62 + ((t16 - t8) * t5 + (-t22 + 0.32e2 * t1) * ZR + t16 - t8) * kx * t14 + (t60 * t72 + (-0.8e1 * t74 + (-0.4e1 + t3) * ZR - t22) * kx) * t13 + ((-0.8e1 * t82 + t16 + t84) * t72 + 0.8e1 * kx * ZR) * kn - 0.4e1 * t72 * ZR) * t94 * t98 + ((t103 * kx * t13 + t108 * kn) * t32 + t103 * t34 * t113) * t118 + ((t122 * kx * t13 + t129) * t32 - t122 * t34 * t113) * t139 + ((t142 * kx * t13 + t146 * kn) * t32 - t142 * t34 * t113) * t156 + ((t160 * kx * t13 - t127 * kx * kn) * t32 + t160 * t34 * t113) * t173 + (t175 * kx * t14 + 0.8e1 * t141 * kx * t13 + t175 * t72 * kn) * t94 * t187 + (t190 * kx * t13 + t129 + t190 * t72) * t94 * t198 + (t200 * kx * t13 + (-0.2e1 - t126 - t125) * kx * kn + t200 * t72) * t94 * t211 + (t217 * kx * t14 + (t220 * t5 + 0.1e1 - t16) * kx * t13 + (t217 * t72 + t146) * kn + t106 * t72) * t94 * t233 + ((t239 * kx * t14 + (t84 + t22 - 0.12e2 * t82) * kx * t13 - t27 * kx * kn) * t32 + (-t239 * t34 * t13 + (0.4e1 * t82 - 0.4e1 * t74) * t34 * kn) * t45) * t263 + (t267 * kx * t14 + (-t220 * t5 + t16 - 0.1e1) * kx * t13 + (t267 * t72 + t108) * kn - t106 * t72) * t94 * t284;

    t1 = sin(kx);
    t2 = exp(-kn);
    t3 = t1 * t2;
    t4 = ZR * ZR;
    t5 = kn * kn;
    t6 = t5 * t5;
    t7 = t4 * t6;
    t8 = xc * xc;
    t12 = 0.2e1 * xc;
    t15 = exp(kn * (-0.1e1 + t12));
    t16 = t1 * t15;
    t17 = t5 * kn;
    t18 = t17 * t4;
    t19 = t18 * xc;
    t22 = kn * xc;
    t23 = exp(-t22);
    t24 = t5 * t23;
    t25 = kx * xc;
    t26 = cos(t25);
    t27 = kx * t26;
    t28 = t27 * xc;
    t33 = exp(kn * (xc - 0.2e1));
    t34 = t17 * t33;
    t35 = sin(t25);
    t36 = ZR * t35;
    t37 = t36 * xc;
    t40 = kn * t23;
    t42 = ZR * kx * t26;
    t45 = t5 * t33;
    t48 = ZR * t26 * xc;
    t51 = kn * t33;
    t53 = t4 * kx * t26;
    t56 = t4 * t35;
    t57 = t56 * xc;
    t60 = kx * kx;
    t61 = t3 * t60;
    t63 = kn * t4 * xc;
    t71 = t4 * t60;
    t73 = 0.8e1 * t3 * t7 * t8 - 0.2e1 * t16 * t19 + 0.8e1 * t24 * t28 - 0.16e2 * t34 * t37 - 0.4e1 * t40 * t42 + 0.16e2 * t45 * kx * t48 + 0.4e1 * t51 * t53 + 0.8e1 * t34 * t57 + 0.4e1 * t61 * t63 - 0.8e1 * t3 * t7 * xc - 0.4e1 * t51 * t42 - t16 * t71;
    t76 = exp(-kn * (t12 + 0.1e1));
    t77 = t1 * t76;
    t80 = exp(t22);
    t81 = kn * t80;
    t86 = t17 * t23;
    t87 = t35 * xc;
    t90 = t5 * t80;
    t93 = t17 * xc;
    t97 = exp(-0.3e1 * kn);
    t98 = t1 * t97;
    t99 = t5 * t4;
    t101 = t60 * kn;
    t108 = exp(-kn * (xc + 0.2e1));
    t109 = t108 * kn;
    t113 = t5 * t8 * t60;
    t119 = t5 * ZR;
    t122 = -0.2e1 * t77 * t18 + 0.2e1 * t81 * t27 + 0.4e1 * t24 * t36 + 0.8e1 * t86 * t87 - 0.2e1 * t90 * t56 - 0.12e2 * t3 * t93 + t98 * t99 - 0.2e1 * t16 * t101 + 0.2e1 * t16 * t18 + 0.2e1 * t109 * t27 + 0.8e1 * t3 * t113 + 0.8e1 * t3 * t6 * t8 - 0.4e1 * t3 * t119;
    t124 = t101 * xc;
    t129 = t5 * t108;
    t134 = exp(kn * (t12 - 0.3e1));
    t135 = t1 * t134;
    t140 = t60 * ZR;
    t152 = exp(kn * (-0.2e1 + 0.3e1 * xc));
    t153 = kn * t152;
    t159 = exp(kn * (-0.3e1 + 0.4e1 * xc));
    t160 = t1 * t159;
    t164 = -0.2e1 * t77 * t124 + 0.8e1 * t34 * t87 + 0.2e1 * t129 * t56 - 0.4e1 * t135 * t93 + 0.2e1 * t98 * t119 + 0.2e1 * t98 * t140 + 0.4e1 * t45 * t56 - 0.4e1 * t45 * t36 - 0.4e1 * t3 * t140 - 0.2e1 * t153 * t27 - t160 * t71 - 0.16e2 * t86 * t37;
    t177 = t5 * t152;
    t188 = -t160 * t99 + 0.2e1 * t160 * t140 + 0.2e1 * t160 * t119 + t77 * t71 - 0.3e1 * t77 * t99 + 0.3e1 * t16 * t99 + 0.2e1 * t16 * t93 + 0.2e1 * t177 * t56 + t98 * t71 - 0.4e1 * t177 * t36 + 0.2e1 * t177 * t35 - t160 * t5 + 0.4e1 * t3 * t124;
    t192 = exp(-0.3e1 * t22);
    t193 = t5 * t192;
    t198 = kn * t192;
    t216 = t3 * ZR;
    t218 = t60 * t5 * xc;
    t223 = -0.2e1 * t193 * t56 + 0.4e1 * t193 * t36 - 0.2e1 * t198 * t27 + 0.2e1 * t77 * t101 - 0.4e1 * t24 * t56 - 0.2e1 * t77 * t93 - 0.4e1 * t135 * t124 + 0.8e1 * t24 * t4 * t28 + 0.4e1 * t40 * t53 - 0.2e1 * t81 * t53 + 0.16e2 * t216 * t218 - 0.2e1 * t16 * t17;
    t224 = t16 * t60;
    t229 = t77 * t60;
    t237 = ZR * t6;
    t248 = t224 - 0.3e1 * t16 * t5 + 0.3e1 * t77 * t5 - t229 + t98 * t5 + t98 * t60 - t160 * t60 + 0.4e1 * t198 * t42 - 0.12e2 * t3 * t19 + 0.16e2 * t3 * t237 * xc - 0.8e1 * t3 * t6 * xc - 0.2e1 * t198 * t53 + 0.2e1 * t90 * t35;
    t257 = t71 * kn;
    t281 = -0.2e1 * t129 * t35 - 0.8e1 * t61 * t99 * xc - 0.2e1 * t109 * t53 - 0.2e1 * t77 * t257 + 0.8e1 * t3 * t4 * t113 + 0.8e1 * t86 * t57 + 0.4e1 * t93 * t135 * t4 - 0.16e2 * t24 * kx * t48 + 0.2e1 * t229 * t63 + 0.2e1 * t16 * t257 + 0.24e2 * t3 * ZR * t17 * xc + 0.2e1 * t77 * t19;
    t314 = -0.16e2 * t216 * t113 - 0.8e1 * t61 * t22 * ZR - 0.8e1 * t3 * t218 + 0.4e1 * t153 * t42 - 0.2e1 * t153 * t53 + 0.2e1 * t16 * t124 - 0.8e1 * t45 * t28 + 0.4e1 * t22 * t1 * t134 * t4 * t60 + 0.2e1 * t77 * t17 - 0.2e1 * t193 * t35 - 0.2e1 * t224 * t63 - 0.16e2 * t3 * t237 * t8 - 0.8e1 * t45 * t4 * t28;
    num3B = -kx * (t73 + t122 + t164 + t188 + t223 + t248 + t281 + t314);

    t1 = 0.8e1 * xc;
    t2 = ZR * xc;
    t4 = ZR * ZR;
    t5 = t4 * xc;
    t7 = -t1 + 0.16e2 * t2 - 0.8e1 * t5;
    t9 = kn * kn;
    t10 = t9 * kn;
    t11 = t7 * kx * t10;
    t13 = -0.4e1 * ZR + 0.4e1 * t4;
    t17 = kx * xc;
    t18 = sin(t17);
    t20 = kx * kx;
    t23 = -t13;
    t25 = t23 * t20 * kn;
    t27 = cos(t17);
    t32 = exp(kn * (x - 0.2e1 - xc));
    t34 = xc * xc;
    t35 = t34 - xc;
    t42 = 0.8e1 * t35 * t4 - 0.16e2 * t35 * ZR + 0.8e1 * t34 - t1;
    t44 = t9 * t9;
    t52 = t20 * kx;
    t60 = 0.4e1 * xc;
    t67 = sin(kx);
    t71 = exp(kn * (x - 0.3e1));
    t73 = t4 - 0.1e1;
    t74 = 0.2e1 * t73;
    t76 = t9 * t18;
    t79 = kn * t27;
    t80 = t74 * t20 * t79;
    t84 = exp(kn * (x + xc - 0.2e1));
    t88 = -0.2e1 * t4 + 0.4e1 * ZR - 0.2e1;
    t91 = -t88;
    t93 = t91 * t20 * t79;
    t95 = 0.3e1 * xc;
    t98 = exp(kn * (x + t95 - 0.4e1));
    t106 = exp(kn * (x - xc - 0.4e1));
    t113 = exp(kn * (x - t95 - 0.2e1));
    t116 = 0.4e1 * xc - 0.4e1 * t5;
    t123 = 0.2e1 * xc;
    t126 = exp(kn * (x - t123 - 0.1e1));
    t128 = 0.2e1 * ZR;
    t129 = -t4 + t128 - 0.1e1;
    t137 = exp(kn * (x - t60 - 0.1e1));
    t139 = 0.1e1 + t4 + t128;
    t147 = exp(kn * (x - 0.1e1));
    t150 = -0.2e1 * xc + 0.2e1;
    t152 = t150 * t4 - 0.2e1 + t123;
    t155 = -t73;
    t156 = 0.3e1 * t155;
    t166 = exp(kn * (x + t123 - 0.3e1));
    t180 = exp(kn * (x + xc - 0.4e1));
    t184 = -t150 * t4 - t123 + 0.2e1;
    t197 = exp(kn * (x - t123 - 0.3e1));
    num4B = ((t11 + t13 * kx * t9) * t18 + (t7 * t20 * t9 + t25) * t27) * t32 + (t42 * kx * t44 + (-0.24e2 * t2 + 0.12e2 * t5 + 0.12e2 * xc) * kx * t10 + (t42 * t52 - 0.4e1 * kx * ZR) * t9 + (0.8e1 * t2 - 0.4e1 * t5 - t60) * t52 * kn - 0.4e1 * t52 * ZR) * t67 * t71 + (t74 * kx * t76 + t80) * t84 + (t88 * kx * t76 + t93) * t98 + (-t74 * kx * t76 + t80) * t106 + (t91 * kx * t76 + t93) * t113 + (t116 * kx * t10 + t116 * t52 * kn) * t67 * t126 + (t129 * kx * t9 + t129 * t52) * t67 * t137 + (t139 * kx * t9 + t139 * t52) * t67 * t147 + (t152 * kx * t10 + t156 * kx * t9 + t152 * t52 * kn + t73 * t52) * t67 * t166 + ((t11 + t23 * kx * t9) * t18 + (-t7 * t20 * t9 + t25) * t27) * t180 + (t184 * kx * t10 - t156 * kx * t9 + t184 * t52 * kn + t155 * t52) * t67 * t197;


    t4 = exp(-0.2e1 * kn * (xc + 0.1e1));
    t5 = kn * t4;
    t8 = kn * (xc - 0.1e1);
    t10 = exp(0.4e1 * t8);
    t12 = exp(-0.2e1 * kn);
    t15 = ZR * ZR;
    t17 = exp(-0.4e1 * kn);
    t19 = kn * kn;
    t20 = t19 * t12;
    t24 = t15 * xc;
    t27 = xc * xc;
    t37 = kn * xc;
    t39 = exp(-0.2e1 * t37);
    t44 = exp(0.2e1 * t8);
    t45 = kn * t44;
    t51 = exp(0.2e1 * kn * (xc - 0.2e1));
    t55 = 0.1e1 + 0.4e1 * t5 - t10 - 0.8e1 * t12 * ZR + t15 + t17 * t15 + 0.32e2 * t20 * ZR * xc + 0.4e1 * t5 * t24 - 0.32e2 * t20 * ZR * t27 + 0.16e2 * t20 * t15 * t27 + 0.2e1 * ZR - 0.16e2 * t20 * t24 - 0.4e1 * t37 * t39 * t15 - 0.4e1 * t45 * t24 + 0.4e1 * t37 * t51 * t15;
    t74 = exp(-0.4e1 * t37);
    t83 = -0.4e1 * t5 * xc - 0.4e1 * t37 * t51 - t10 * t15 + 0.16e2 * t20 * t27 - 0.16e2 * t20 * xc + 0.4e1 * t37 * t39 + 0.4e1 * t45 * xc + 0.4e1 * t45 * t15 - 0.4e1 * t5 * t15 - t74 - t74 * t15 + 0.2e1 * t74 * ZR + 0.2e1 * t10 * ZR + t17 - 0.4e1 * t45 + 0.2e1 * t17 * ZR;
    denB = 0.2e1 * kn * (t55 + t83);

    _C1B=num1B/denB; _C2B=num2B/denB; _C3B=num3B/denB; _C4B=num4B/denB;
    Z=ZB; _C1=_C1B; _C2=_C2B; _C3=_C3B; _C4=_C4B;
  }
  u1 = -kn * ((_C1 + x * _C3) * exp(-kn * x) + _C2 + x * _C4 + kx * sin(kx * x)) / Z * pow(kx * kx + kn * kn, -0.2e1);

  u2 = (-(_C1 + x * _C3) * kn + _C3) / Z * pow(kx * kx + kn * kn, -0.2e1) * exp(-kn * x) + (_C4 + (_C2 + x * _C4) * kn) / Z * pow(kx * kx + kn * kn, -0.2e1) + kx * kx * cos(kx * x) / Z * pow(kx * kx + kn * kn, -0.2e1);

  u3 = (-0.2e1 * kn * kx * kx * pow(kx * kx + kn * kn, -0.2e1) + kn / (kx * kx + kn * kn)) * cos(kx * x) - 0.2e1 * kn * (-(_C1 + x * _C3) * kn + _C3) * pow(kx * kx + kn * kn, -0.2e1) * exp(-kn * x) - 0.2e1 * kn * (_C4 + (_C2 + x * _C4) * kn) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * _C3 * kn * exp(-kn * x) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * _C4 * kn * pow(kx * kx + kn * kn, -0.2e1);

  u4 = 0.2e1 * kn * (kn * _C1 + kn * x * _C3 - _C3) * exp(-kn * x) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * kn * (kn * _C2 + kn * x * _C4 + _C4) * pow(kx * kx + kn * kn, -0.2e1) + kx * (kn - kx) * (kx + kn) * sin(kx * x) * pow(kx * kx + kn * kn, -0.2e1);

  u5 = -0.2e1 * _C3 * kn * exp(-kn * x) * pow(kx * kx + kn * kn, -0.2e1) - 0.2e1 * _C4 * kn * pow(kx * kx + kn * kn, -0.2e1) - cos(kx * x) * kn / (kx * kx + kn * kn);

  u6 = (0.2e1 * kn * kx * kx * pow(kx * kx + kn * kn, -0.2e1) + kn / (kx * kx + kn * kn)) * cos(kx * x) + 0.2e1 * kn * (-(_C1 + x * _C3) * kn + _C3) * pow(kx * kx + kn * kn, -0.2e1) * exp(-kn * x) + 0.2e1 * kn * (_C4 + (_C2 + x * _C4) * kn) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * _C3 * kn * exp(-kn * x) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * _C4 * kn * pow(kx * kx + kn * kn, -0.2e1);

  sum5 +=u5*cos(kn*z);/* pressure */
  sum6 +=u6*cos(kn*z);/* zz stress */
  u1 *= cos(kn*z); /* x velocity */
  sum1 += u1;
  u2 *= sin(kn*z); /* z velocity */
  sum2 += u2;
  u3 *= cos(kn*z); /* xx stress */
  sum3 += u3;
  u4 *= sin(kn*z); /* zx stress */
  sum4 += u4;
  mag=sqrt(sum1*sum1+sum2*sum2);
  /* Output */
  if( vel != NULL ) {
    vel[0] = sum1;
    vel[1] = sum2;
  }
  if( presssure != NULL ) {
    (*presssure) = sum5;
  }
  if( total_stress != NULL ) {
    total_stress[0] = sum3;
    total_stress[1] = sum6; 
    total_stress[2] = sum4;
  }
  if( strain_rate != NULL ) {
    if( x>xc ) {
      Z = ZB;
    }
    else {
      Z = ZA;
    }
    strain_rate[0] = (sum3+sum5)/(2.0*Z);
    strain_rate[1] = (sum6+sum5)/(2.0*Z);
    strain_rate[2] = (sum4)/(2.0*Z);
  }
}

/* ZB >> ZA */
void _solCx_B(
  const double pos[],
  double _eta_A, double _eta_B, 	/* Input parameters: density, viscosity A, viscosity B */
  double _x_c, int _n, 			/* Input parameters: viscosity jump location, wavenumber in x */
  double vel[], double* presssure, 
  double total_stress[], double strain_rate[] )
{
  double Z,u1,u2,u3,u4,u5,u6,ZA,ZB,ZR;
  double sum1,sum2,sum3,sum4,sum5,sum6,mag,x,z,xc;
  double num1A,num2A,num3A,num4A,num1B,num2B,num3B,num4B,denA,denB;
  double _C1A,_C2A,_C3A,_C4A,_C1B,_C2B,_C3B,_C4B,_C1,_C2,_C3,_C4;
  int n,nx;
  double kx, kn;

  double t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
  double t11,t12,t13,t14,t15,t16,t17,t18,t19,t20;
  double t21,t22,t23,t24,t25,t26,t27,t28,t29,t30;
  double t31,t32,t33,t34,t36,t37,t38,t39,t40,t41;
  double t42,t43,t44,t45,t46,t47,t48,t49,t50,t51;
  double t52,t53,t54,t55,t56,t57,t58,t59,t60,t61;
  double t62,t63,t64,t65,t66,t67,t68,t69,t70,t71;
  double t72,t73,t74,t75,t76,t78,t79,t81,t83,t84;
  double t85,t86,t88,t89,t90,t91,t92,t93,t94,t95;
  double t96,t97,t98,t99,t100,t101,t102,t103,t104,t105;
  double t106,t107,t108,t109,t110,t113,t114,t115,t116,t117;
  double t118,t119,t120,t121,t122,t123,t124,t126,t127,t128;
  double t129,t131,t132,t133,t134,t135,t136,t137,t138,t139;
  double t140,t141,t142,t143,t144,t145,t146,t147,t148,t149;
  double t150,t151,t152,t153,t154,t155,t156,t157,t159,t160;
  double t161,t162,t164,t165,t166,t169,t170,t171,t173,t174;
  double t177,t178,t180,t181,t183,t184,t185,t186,t187,t188;
  double t190,t195,t196,t197,t198,t199,t200,t201,t206,t208;
  double t209,t210,t212,t213,t214,t215,t216,t220,t221,t222;
  double t224,t226,t227,t230,t232,t235,t237,t239,t240,t241;
  double t242,t244,t245,t248,t250,t256,t258,t259,t261,t263;
  double t265,t271,t273,t275,t276,t278,t279,t282,t283,t285;
  double t288,t293,t295,t296,t298,t299,t300,t301,t303,t314;
  double t317,t321,t326,t328,t331,t363,t371,t374,t382,t402;
  double t431,t442,t470;

  nx=1.0;
  kx=nx*M_PI;
  kn=_n*M_PI;
    
  ZA=_eta_A; /* left column viscosity */
  ZB=_eta_B; /* right column viscosity */
  xc = _x_c;
	
  x = pos[0];
  z = pos[1];
	
  sum1=0.0;
  sum2=0.0;
  sum3=0.0;
  sum4=0.0;
  sum5=0.0;
  sum6=0.0;

  ZR=ZA/ZB;
  if (x<xc){
    t1 = kn * kn;
    t4 = exp(kn * (xc - 0.2e1));
    t5 = t1 * t4;
    t6 = kx * xc;
    t7 = cos(t6);
    t9 = xc * xc;
    t10 = kx * t7 * t9;
    t13 = kn * xc;
    t14 = exp(-t13);
    t15 = t1 * t14;
    t16 = ZR * ZR;
    t18 = sin(t6);
    t19 = t16 * xc * t18;
    t21 = t5 * ZR;
    t22 = t6 * t7;
    t27 = exp(kn * (xc - 0.4e1));
    t28 = kn * t27;
    t31 = exp(-0.3e1 * t13);
    t32 = kn * t31;
    t37 = t1 * kn;
    t38 = t37 * t4;
    t39 = ZR * t18;
    t40 = t39 * xc;
    t43 = sin(kx);
    t44 = t16 * t43;
    t45 = 0.2e1 * xc;
    t48 = exp(kn * (t45 - 0.3e1));
    t50 = t48 * t9 * t37;
    t54 = ZR * t9 * t18;
    t60 = exp(-kn * (xc + 0.2e1));
    t61 = t1 * t60;
    t64 = kn * t14;
    t69 = exp(kn * (0.3e1 * xc - 0.4e1));
    t70 = kn * t69;
    t75 = -0.4e1 * t5 * t10 + t15 * t19 - 0.8e1 * t21 * t22 - t28 * t22 + t32 * t16 * t22 + 0.4e1 * t5 * t22 + 0.8e1 * t38 * t40 - 0.2e1 * t44 * t50 - 0.8e1 * t38 * t54 - t32 * t18 - 0.4e1 * t61 * t18 - t64 * t18 + t70 * t18 + t28 * t18 - 0.4e1 * t5 * t18;
    t76 = ZR * t43;
    t81 = t5 * t16;
    t84 = t48 * xc;
    t85 = t84 * t37;
    t92 = exp(-kn * (t45 + 0.1e1));
    t93 = t76 * t92;
    t94 = kx * kx;
    t96 = t94 * xc * kn;
    t102 = t1 * t69;
    t104 = kn * t60;
    t108 = t18 * xc;
    t113 = t9 * t18;
    t118 = kn * t4;
    t119 = t16 * t18;
    t122 = 0.2e1 * t76 * t50 + 0.2e1 * t5 * t19 + 0.4e1 * t81 * t22 + 0.2e1 * t44 * t85 + 0.4e1 * t61 * t22 + 0.2e1 * t93 * t96 - t64 * t16 * t22 + t64 * t22 + t102 * t19 + 0.2e1 * t104 * t16 * t22 - 0.4e1 * t38 * t108 + 0.4e1 * t5 * t39 + 0.4e1 * t38 * t113 + 0.4e1 * t5 * t108 + 0.2e1 * t118 * t119;
    t127 = exp(-0.3e1 * kn);
    t128 = t127 * kn;
    t133 = t127 * t1;
    t137 = t92 * xc;
    t138 = t137 * t37;
    t141 = t61 * t16;
    t147 = t16 * t7 * kx;
    t151 = t92 * t9 * t37;
    t156 = t137 * t1;
    t159 = t37 * t60;
    t162 = t76 * t48;
    t164 = t94 * kn * t9;
    t169 = -0.2e1 * t118 * t39 + 0.2e1 * t44 * t128 + 0.2e1 * t76 * t128 - t44 * t133 - 0.4e1 * t38 * t19 - 0.2e1 * t44 * t138 + 0.4e1 * t141 * t22 - 0.4e1 * t81 * t10 + 0.4e1 * t118 * t147 + 0.2e1 * t44 * t151 - t70 * t16 * t22 + 0.4e1 * t76 * t156 + 0.4e1 * t159 * t19 + 0.2e1 * t162 * t164 + 0.2e1 * t61 * t19;
    t171 = t16 * t9 * t18;
    t185 = t61 * ZR;
    t198 = ZR * t7 * kx;
    t206 = 0.4e1 * t38 * t171 + 0.2e1 * t76 * t138 - 0.4e1 * t44 * t156 - 0.2e1 * t76 * t151 - 0.2e1 * t102 * t40 + 0.2e1 * t70 * ZR * t22 - 0.8e1 * t185 * t22 - 0.8e1 * t159 * t40 - 0.4e1 * t141 * t10 - t70 * t22 + t28 * t16 * t22 - 0.4e1 * t104 * t147 + 0.4e1 * t104 * t198 - 0.4e1 * t118 * t198 - 0.2e1 * t118 * t16 * t22;
    t209 = t1 * t27;
    t213 = t1 * t31;
    t220 = exp(-kn);
    t221 = t220 * kn;
    t224 = t92 * t1;
    t227 = t92 * kn;
    t232 = t92 * t94;
    t235 = t48 * kn;
    t240 = t48 * t94;
    t242 = t209 * t19 - 0.2e1 * t76 * t85 + t213 * t19 - 0.2e1 * t32 * ZR * t22 - 0.4e1 * t159 * t171 - 0.2e1 * t76 * t221 + t44 * t224 - t76 * t224 + 0.2e1 * t44 * t227 - 0.2e1 * t76 * t227 + t44 * t232 - t76 * t232 - 0.2e1 * t44 * t235 + 0.2e1 * t76 * t235 + t44 * t240;
    t244 = t48 * t1;
    t259 = t84 * t1;
    t271 = -t76 * t240 + t44 * t244 - t76 * t244 + 0.4e1 * t159 * t108 + 0.4e1 * t61 * t39 - 0.4e1 * t159 * t113 + 0.4e1 * t61 * t108 - 0.2e1 * t104 * t119 + 0.2e1 * t104 * t39 - 0.4e1 * t44 * t259 - 0.2e1 * t213 * t40 + t32 * t22 + 0.8e1 * t159 * t54 - 0.6e1 * t61 * t40 - 0.6e1 * t5 * t40;
    t273 = t44 * t92;
    t276 = t44 * t48;
    t298 = t127 * t94;
    t303 = -0.2e1 * t273 * t96 + 0.2e1 * t276 * t96 + 0.4e1 * t76 * t259 + 0.8e1 * t21 * t10 + 0.2e1 * t118 * ZR * t22 + 0.2e1 * t273 * t164 - 0.2e1 * t93 * t164 + 0.8e1 * t185 * t10 - 0.2e1 * t104 * ZR * t22 - 0.2e1 * t162 * t96 - t76 * t133 - t44 * t298 - t76 * t298 + t64 * t119 - t15 * t108;
    t314 = t220 * t1;
    t317 = t220 * t94;
    t326 = t70 * t119 - 0.2e1 * t70 * t39 + t102 * t108 - t28 * t119 - t209 * t108 - t32 * t119 + 0.2e1 * t32 * t39 + t213 * t108 - t44 * t314 - t76 * t314 - t44 * t317 - t76 * t317 - 0.2e1 * t44 * t221 - 0.4e1 * t61 * t10 - 0.2e1 * t276 * t164;
    num1A = -kx * (t75 + t122 + t169 + t206 + t242 + t271 + t303 + t326);

    t3 = ZR * ZR;
    t4 = t3 * xc;
    t5 = xc - 0.2e1 * ZR * xc + t4;
    t7 = kn * kn;
    t8 = t5 * kx * t7;
    t10 = 0.1e1 + t3 - 0.2e1 * ZR;
    t14 = kx * xc;
    t15 = sin(t14);
    t18 = kx * kx;
    t20 = cos(t14);
    t21 = kn * t20;
    t24 = 0.3e1 * xc;
    t27 = exp(kn * (x + t24 - 0.4e1));
    t29 = xc * xc;
    t30 = xc - t29;
    t31 = 0.4e1 * t30;
    t33 = -t30;
    t34 = 0.8e1 * t33;
    t36 = 0.4e1 * xc;
    t37 = 0.4e1 * t29;
    t38 = t31 * t3 + t34 * ZR + t36 - t37;
    t40 = t7 * kn;
    t48 = (0.2e1 * t4 + (0.4e1 - 0.6e1 * xc) * ZR - 0.4e1 + t36) * kx * t7;
    t49 = -t3 + ZR;
    t50 = 0.2e1 * t49;
    t51 = t50 * kx;
    t56 = t38 * t18 * t7;
    t57 = 0.2e1 * xc;
    t58 = t57 - 0.4e1;
    t60 = -t58;
    t70 = exp(kn * (x - xc - 0.2e1));
    t72 = -xc + t4;
    t74 = t72 * kx * t7;
    t75 = -t3 + 0.1e1;
    t85 = exp(kn * (x + xc - 0.4e1));
    t98 = exp(kn * (x - xc));
    t100 = -t3 - ZR;
    t102 = t100 * kx * t7;
    t104 = -0.2e1 * t100;
    t107 = t18 * kx;
    t108 = t100 * t107;
    t110 = sin(kx);
    t114 = exp(kn * (x - 0.3e1));
    t126 = exp(kn * (x - t24));
    t135 = exp(kn * (x - 0.1e1));
    t137 = 0.2e1 * t33;
    t139 = -t137;
    t141 = t137 * t3 + t139 * ZR;
    t144 = -t36 + 0.1e1;
    t150 = (t144 * t3 - t144 * ZR) * kx * t7;
    t153 = -t50 * kx;
    t157 = -t49 * t107;
    t162 = exp(kn * (x - t57 - 0.1e1));
    t184 = exp(kn * (x + xc - 0.2e1));
    t188 = t139 * t3 + t137 * ZR;
    t198 = exp(kn * (x + t57 - 0.3e1));
    num2A = ((t8 + t10 * kx * kn) * t15 - t5 * t18 * t21) * t27 + ((t38 * kx * t40 + t48 + t51 * kn) * t15 + (t56 + (t58 * t3 + t60 * ZR) * t18 * kn) * t20) * t70 + ((t74 + t75 * kx * kn) * t15 + t72 * t18 * t21) * t85 + ((t74 - t75 * kx * kn) * t15 - t72 * t18 * t21) * t98 + (t102 + t104 * kx * kn + t108) * t110 * t114 + ((t8 - t10 * kx * kn) * t15 + t5 * t18 * t21) * t126 + (t102 - t104 * kx * kn + t108) * t110 * t135 + (t141 * kx * t40 + t150 + (t141 * t107 + t153) * kn + t157) * t110 * t162 + (((-t31 * t3 - t34 * ZR - t36 + t37) * kx * t40 + t48 + t153 * kn) * t15 + (t56 + (t60 * t3 + t58 * ZR) * t18 * kn) * t20) * t184 + (t188 * kx * t40 + t150 + (t188 * t107 + t51) * kn + t157) * t110 * t198;

    t1 = ZR * ZR;
    t2 = sin(kx);
    t3 = t1 * t2;
    t4 = 0.2e1 * xc;
    t7 = exp(-kn * (t4 + 0.1e1));
    t9 = kx * kx;
    t11 = t9 * xc * kn;
    t14 = kn * kn;
    t15 = t14 * kn;
    t18 = exp(kn * (xc - 0.2e1));
    t19 = t15 * t18;
    t20 = kx * xc;
    t21 = sin(t20);
    t22 = t1 * t21;
    t23 = t22 * xc;
    t26 = ZR * t2;
    t31 = t7 * xc * t15;
    t36 = exp(kn * (t4 - 0.3e1));
    t38 = t36 * xc * t15;
    t46 = exp(-kn * (xc + 0.2e1));
    t47 = t14 * t46;
    t49 = cos(t20);
    t50 = ZR * kx * t49;
    t58 = kx * t49;
    t59 = t58 * xc;
    t63 = t7 * t9;
    t64 = t63 * kn;
    t67 = t14 * t18;
    t69 = t1 * kx * t49;
    t72 = ZR * t21;
    t73 = t72 * xc;
    t81 = kn * t46;
    t88 = kn * xc;
    t89 = exp(-t88);
    t90 = kn * t89;
    t97 = exp(kn * (xc - 0.4e1));
    t98 = kn * t97;
    t100 = 0.2e1 * t3 * t64 + 0.4e1 * t67 * t69 - 0.8e1 * t19 * t73 - 0.2e1 * t26 * t64 + 0.4e1 * t47 * t1 * t59 + 0.2e1 * t81 * t69 - 0.2e1 * t81 * t50 - 0.8e1 * t67 * t50 - t90 * t69 + 0.8e1 * t67 * ZR * t59 - t98 * t69;
    t104 = t15 * t46;
    t113 = t21 * xc;
    t121 = exp(kn * (0.3e1 * xc - 0.4e1));
    t122 = kn * t121;
    t132 = exp(-0.3e1 * kn);
    t133 = t132 * t14;
    t135 = t132 * t9;
    t138 = t36 * t9;
    t139 = t138 * kn;
    t142 = exp(-kn);
    t143 = t142 * t9;
    t146 = t142 * t14;
    t152 = 0.2e1 * t67 * t22 + t3 * t133 + t3 * t135 + t26 * t133 + 0.2e1 * t139 * t3 - t26 * t143 - t3 * t143 - t26 * t146 - t3 * t146 + t26 * t135 + 0.2e1 * t122 * t50;
    t155 = t36 * t14;
    t164 = t7 * t14;
    t174 = t36 * t15;
    t177 = t7 * t15;
    t183 = exp(-0.3e1 * t88);
    t184 = t14 * t183;
    t190 = kn * t183;
    t196 = t14 * t121;
    t199 = 0.2e1 * t3 * t174 - 0.2e1 * t26 * t177 + 0.2e1 * t3 * t177 + 0.2e1 * t184 * t72 - t184 * t22 - 0.4e1 * t19 * t22 - t190 * t58 + 0.4e1 * t67 * t58 + 0.8e1 * t19 * t72 + t196 * t22 + t90 * t58;
    t201 = t14 * t89;
    t209 = t14 * t97;
    t235 = kn * t18;
    t240 = -0.4e1 * t19 * t21 - t201 * t21 + t196 * t21 + t209 * t21 - 0.4e1 * t67 * t59 - t190 * t69 + 0.2e1 * t190 * t50 - 0.4e1 * t67 * t1 * t59 + 0.2e1 * t26 * t31 + 0.2e1 * t235 * t69 - 0.2e1 * t235 * t50;
    num3A = -kx * (-t3 * t63 - 0.2e1 * t196 * t72 + 0.4e1 * t19 * t23 + t199 - 0.8e1 * t47 * ZR * t59 - 0.2e1 * t3 * t7 * t11 + 0.2e1 * t26 * t36 * t11 - 0.2e1 * t3 * t36 * t11 + 0.2e1 * t26 * t7 * t11 - 0.3e1 * t26 * t164 - t122 * t58 + 0.2e1 * t26 * t38 + 0.3e1 * t26 * t155 + t240 - 0.4e1 * t104 * t21 - t209 * t22 + t100 + 0.3e1 * t3 * t164 + t3 * t138 + t152 - 0.4e1 * t104 * t22 - t26 * t138 + t98 * t58 - 0.2e1 * t26 * t139 + 0.8e1 * t104 * t72 - 0.2e1 * t26 * t174 - 0.3e1 * t3 * t155 - t184 * t21 + 0.2e1 * t47 * t72 - 0.2e1 * t67 * t72 + 0.8e1 * t47 * t50 + 0.4e1 * t19 * t113 - 0.8e1 * t104 * t73 + 0.4e1 * t47 * t59 - t122 * t69 + 0.4e1 * t104 * t23 - 0.2e1 * t47 * t22 - 0.4e1 * t47 * t58 - 0.2e1 * t3 * t38 - 0.4e1 * t47 * t69 - 0.2e1 * t31 * t3 + 0.4e1 * t104 * t113 + t201 * t22 + t26 * t63);

    t1 = ZR * ZR;
    t3 = -t1 - 0.1e1 + 0.2e1 * ZR;
    t5 = kn * kn;
    t6 = kx * xc;
    t7 = sin(t6);
    t8 = t5 * t7;
    t10 = -t3;
    t11 = kx * kx;
    t13 = cos(t6);
    t14 = kn * t13;
    t15 = t10 * t11 * t14;
    t17 = 0.3e1 * xc;
    t20 = exp(kn * (x + t17 - 0.4e1));
    t22 = 0.1e1 - xc;
    t23 = 0.4e1 * t22;
    t25 = -t22;
    t26 = 0.8e1 * t25;
    t28 = 0.4e1 * xc;
    t29 = t23 * t1 + t26 * ZR + 0.4e1 - t28;
    t31 = t5 * kn;
    t32 = t29 * kx * t31;
    t33 = t1 - ZR;
    t34 = 0.2e1 * t33;
    t41 = -t34;
    t43 = t41 * t11 * kn;
    t49 = exp(kn * (x - xc - 0.2e1));
    t51 = t1 - 0.1e1;
    t55 = t51 * t11 * t14;
    t59 = exp(kn * (x + xc - 0.4e1));
    t67 = exp(kn * (x - xc));
    t69 = -t1 - ZR;
    t72 = t11 * kx;
    t75 = sin(kx);
    t79 = exp(kn * (x - 0.3e1));
    t86 = exp(kn * (x - t17));
    t88 = -t69;
    t96 = exp(kn * (x - 0.1e1));
    t98 = 0.2e1 * t25;
    t102 = t98 * t1 - t98 * ZR;
    t104 = t102 * kx * t31;
    t105 = -t33;
    t106 = 0.3e1 * t105;
    t110 = t102 * t72 * kn;
    t114 = 0.2e1 * xc;
    t117 = exp(kn * (x - t114 - 0.1e1));
    t135 = exp(kn * (x + xc - 0.2e1));
    t145 = exp(kn * (x + t114 - 0.3e1));
    num4A = (t3 * kx * t8 + t15) * t20 + ((t32 + t34 * kx * t5) * t7 + (t29 * t11 * t5 + t43) * t13) * t49 + (t51 * kx * t8 + t55) * t59 + (-t51 * kx * t8 + t55) * t67 + (t69 * kx * t5 + t69 * t72) * t75 * t79 + (t10 * kx * t8 + t15) * t86 + (t88 * kx * t5 + t88 * t72) * t75 * t96 + (t104 + t106 * kx * t5 + t110 + t33 * t72) * t75 * t117 + ((t32 + t41 * kx * t5) * t7 + ((-t23 * t1 - t26 * ZR - 0.4e1 + t28) * t11 * t5 + t43) * t13) * t135 + (t104 - t106 * kx * t5 + t110 + t105 * t72) * t75 * t145;


    t1 = kn * xc;
    t3 = exp(-0.4e1 * t1);
    t6 = ZR * ZR;
    t7 = kn * kn;
    t9 = exp(-0.2e1 * kn);
    t10 = t7 * t9;
    t11 = xc * xc;
    t19 = exp(-0.2e1 * t1);
    t26 = exp(-0.2e1 * kn * (xc + 0.1e1));
    t27 = kn * t26;
    t28 = t6 * xc;
    t38 = kn * (-0.1e1 + xc);
    t40 = exp(0.2e1 * t38);
    t41 = kn * t40;
    t45 = exp(-0.4e1 * kn);
    t50 = exp(0.2e1 * kn * (xc - 0.2e1));
    t55 = exp(0.4e1 * t38);
    t58 = 0.1e1 + 0.2e1 * t3 * ZR + t6 - 0.32e2 * t10 * ZR * t11 + 0.32e2 * t10 * ZR * xc + 0.4e1 * t1 * t19 * t6 - 0.4e1 * t27 * t28 - 0.4e1 * t27 + 0.16e2 * t10 * t6 * t11 - 0.16e2 * t10 * t28 + 0.4e1 * t41 * t28 + t45 * t6 - 0.4e1 * t1 * t50 * t6 - t55 * t6 + 0.2e1 * ZR;
    t83 = 0.4e1 * t41 - t3 * t6 + 0.2e1 * t45 * ZR - 0.8e1 * t9 * ZR + 0.2e1 * t55 * ZR + 0.16e2 * t10 * t11 - 0.16e2 * t10 * xc - 0.4e1 * t41 * t6 - 0.4e1 * t41 * xc + 0.4e1 * t1 * t50 + 0.4e1 * t27 * t6 + 0.4e1 * t27 * xc - 0.4e1 * t1 * t19 - t3 + t45 - t55;
    denA = kn * (t58 + t83);

    _C1A=num1A/denA; _C2A=num2A/denA; _C3A=num3A/denA; _C4A=num4A/denA;
    Z=ZA; _C1=_C1A; _C2=_C2A; _C3=_C3A; _C4=_C4A;
  }else{
    t1 = sin(kx);
    t2 = exp(-kn);
    t3 = t1 * t2;
    t4 = kn * kn;
    t5 = t4 * kn;
    t6 = ZR * t5;
    t7 = xc * xc;
    t13 = exp(kn * (xc - 0.2e1));
    t14 = kn * t13;
    t15 = kx * xc;
    t16 = cos(t15);
    t17 = t15 * t16;
    t20 = kx * kx;
    t22 = t20 * t4 * t7;
    t26 = t20 * xc * kn;
    t29 = t4 * t13;
    t30 = kx * t16;
    t31 = t30 * t7;
    t39 = exp(kn * (-0.2e1 + 0.3e1 * xc));
    t40 = t4 * t39;
    t41 = ZR * xc;
    t42 = sin(t15);
    t43 = t41 * t42;
    t46 = kn * xc;
    t47 = exp(-t46);
    t48 = t4 * t47;
    t49 = ZR * ZR;
    t53 = t49 * t1;
    t54 = 0.2e1 * xc;
    t57 = exp(-kn * (t54 + 0.1e1));
    t58 = t57 * xc;
    t64 = exp(kn * (-0.1e1 + t54));
    t65 = t1 * t64;
    t66 = t49 * xc;
    t67 = t66 * t5;
    t70 = kn * t47;
    t73 = t66 * t42;
    t76 = kn * t39;
    t78 = ZR * t16 * kx;
    t81 = t4 * t4;
    t86 = t66 * t4;
    t89 = t5 * t47;
    t90 = t49 * t7;
    t91 = t90 * t42;
    t95 = exp(-0.3e1 * t46);
    t96 = t4 * t95;
    t99 = ZR * t7;
    t100 = t99 * t42;
    t103 = kn * t95;
    t107 = 0.32e2 * t3 * t6 * t7 + 0.4e1 * t14 * t17 - 0.8e1 * t3 * t22 + 0.4e1 * t3 * t26 + 0.8e1 * t29 * t31 - 0.16e2 * t29 * t17 + 0.4e1 * t40 * t43 + 0.8e1 * t48 * t49 * t31 + 0.4e1 * t53 * t58 * t4 - 0.2e1 * t65 * t67 - 0.4e1 * t70 * t17 - 0.8e1 * t48 * t73 + 0.8e1 * t76 * t78 - 0.8e1 * t3 * t49 * t81 * t7 - 0.4e1 * t65 * t86 + 0.8e1 * t89 * t91 - 0.2e1 * t96 * t73 - 0.16e2 * t89 * t100 + 0.4e1 * t103 * ZR * t17;
    t116 = exp(-0.3e1 * kn);
    t117 = t1 * t116;
    t123 = exp(-kn * (xc + 0.2e1));
    t124 = kn * t123;
    t129 = t7 * xc;
    t136 = t20 * kn * t7;
    t149 = t53 * t57;
    t152 = t3 * t49;
    t160 = t29 * t49;
    t166 = t49 * kn;
    t169 = -0.16e2 * t48 * ZR * t31 + 0.4e1 * t96 * t43 + 0.12e2 * t48 * t43 + t117 * t4 - 0.4e1 * t14 * t42 - 0.4e1 * t124 * t30 - 0.2e1 * t103 * t17 + 0.8e1 * t3 * t49 * t129 * t81 + 0.2e1 * t65 * t49 * t136 + 0.4e1 * t70 * ZR * t17 - 0.4e1 * t76 * ZR * t17 + 0.8e1 * t3 * t86 + 0.2e1 * t76 * t17 + 0.2e1 * t149 * t26 - 0.8e1 * t152 * t22 + 0.4e1 * t3 * t67 + 0.8e1 * t3 * t81 * t129 - 0.16e2 * t160 * t17 + 0.2e1 * t53 * t58 * t5 + 0.2e1 * t65 * t166;
    t173 = t1 * t57;
    t178 = t90 * t5;
    t181 = t49 * t20;
    t183 = t5 * xc;
    t186 = t5 * t13;
    t195 = t4 * xc;
    t208 = exp(kn * (-0.3e1 + 0.4e1 * xc));
    t209 = t1 * t208;
    t215 = t4 * t123;
    t220 = 0.8e1 * t160 * t31 - 0.2e1 * t173 * t26 - 0.8e1 * t14 * t78 + 0.2e1 * t65 * t178 + t65 * t181 + 0.2e1 * t65 * t183 + 0.16e2 * t186 * t73 - 0.8e1 * t186 * t91 + 0.2e1 * t173 * t136 - 0.2e1 * t149 * t136 + 0.4e1 * t65 * t195 + 0.12e2 * t29 * t43 - 0.2e1 * t124 * t42 + t117 * t20 - 0.2e1 * t117 * kn - t209 * t20 + 0.2e1 * t209 * kn + 0.2e1 * t103 * t42 + 0.4e1 * t215 * t42 + 0.4e1 * t70 * t42;
    t227 = t5 * t7;
    t230 = t49 * t42;
    t241 = t4 * t49;
    t250 = t4 * ZR;
    t258 = exp(kn * (t54 - 0.3e1));
    t259 = t258 * xc;
    t263 = t29 * ZR;
    t271 = t42 * xc;
    t275 = 0.8e1 * t29 * t42 - 0.16e2 * t3 * ZR * t81 * t129 - 0.2e1 * t65 * t227 - 0.4e1 * t215 * t230 - 0.8e1 * t29 * t73 + 0.8e1 * t14 * t30 - 0.2e1 * t53 * t57 * t7 * t5 - t209 * t241 + 0.2e1 * t209 * t166 - 0.32e2 * t186 * t43 - t209 * t181 + 0.2e1 * t65 * t26 + 0.2e1 * t209 * t250 - 0.4e1 * t209 * kn * ZR + 0.8e1 * t53 * t259 * t4 + 0.32e2 * t263 * t17 - 0.16e2 * t3 * t178 + 0.2e1 * t209 * ZR * t20 + 0.16e2 * t186 * t271 + t65 * t241;
    t278 = exp(t46);
    t279 = kn * t278;
    t285 = t7 * t42;
    t288 = ZR * t42;
    t293 = t4 * t129 * t20;
    t296 = t116 * t20;
    t298 = ZR * t1;
    t299 = t116 * t4;
    t303 = t116 * kn;
    t321 = t4 * t278;
    t331 = 0.2e1 * t279 * t49 * t17 - 0.4e1 * t29 * t271 - 0.8e1 * t186 * t285 - 0.8e1 * t29 * t288 - 0.16e2 * t3 * ZR * t293 + t53 * t296 + 0.2e1 * t298 * t299 + t53 * t299 - 0.4e1 * t298 * t303 - 0.2e1 * t53 * t303 + 0.4e1 * t14 * t288 - 0.2e1 * t65 * t136 + 0.2e1 * t124 * t17 - 0.4e1 * t70 * t288 - 0.8e1 * t3 * t81 * t7 + 0.16e2 * t186 * t100 + 0.2e1 * t321 * t73 - 0.4e1 * t53 * t258 * t26 + 0.8e1 * t152 * t293 - 0.2e1 * t96 * t271;
    t363 = t3 * t20;
    t371 = t66 * kn;
    t374 = t1 * t258;
    t382 = -0.4e1 * t103 * t288 + 0.2e1 * t103 * t230 - 0.16e2 * t263 * t31 - 0.4e1 * t14 * ZR * t17 - 0.4e1 * t48 * t271 + 0.2e1 * t298 * t296 + 0.4e1 * t298 * t2 * t20 + 0.4e1 * t298 * t2 * t4 - 0.2e1 * t215 * t271 - t53 * t57 * t20 - 0.2e1 * t53 * t57 * kn - t53 * t57 * t4 + 0.8e1 * t298 * t2 * kn + 0.16e2 * t363 * t250 * t7 - 0.2e1 * t279 * t17 + 0.2e1 * t124 * t230 + 0.4e1 * t363 * t371 - 0.8e1 * t195 * t374 - 0.2e1 * t103 * t49 * t17 + 0.8e1 * t89 * t285;
    t402 = t49 * t16 * kx;
    t431 = 0.8e1 * t3 * t293 - 0.4e1 * t53 * t259 * t5 + 0.4e1 * t183 * t374 - 0.8e1 * t3 * t6 * xc - 0.16e2 * t3 * t227 + 0.8e1 * t3 * t195 - 0.16e2 * t3 * t41 * t4 + 0.4e1 * t124 * t402 - 0.2e1 * t321 * t271 - 0.2e1 * t279 * t230 + 0.4e1 * t3 * t183 - 0.2e1 * t124 * t49 * t17 + 0.4e1 * t40 * t230 - 0.2e1 * t76 * t230 - 0.8e1 * t363 * t41 * kn - 0.2e1 * t40 * t73 + 0.4e1 * t76 * t288 + 0.8e1 * t48 * t31 + 0.2e1 * t173 * t227 - 0.4e1 * t173 * t195;
    t442 = t65 * t20;
    t470 = -0.2e1 * t173 * t183 - 0.4e1 * t76 * t30 - 0.2e1 * t40 * t271 - 0.8e1 * t40 * t288 + 0.2e1 * t215 * t73 - 0.2e1 * t442 * t371 + 0.16e2 * t3 * t99 * t81 + 0.4e1 * t46 * t374 * t20 - 0.4e1 * t76 * t402 + 0.2e1 * t279 * t42 + 0.4e1 * t40 * t42 - 0.2e1 * t76 * t42 + t173 * t4 + 0.2e1 * t173 * kn + t173 * t20 - t209 * t4 - t65 * t4 - 0.2e1 * t65 * kn - t442 + 0.2e1 * t76 * t49 * t17;
    num1B = kx * (t107 + t169 + t220 + t275 + t331 + t382 + t431 + t470);

    t1 = 0.2e1 * xc;
    t2 = ZR * ZR;
    t3 = t2 * xc;
    t5 = ZR * xc;
    t7 = t1 + 0.2e1 * t3 - 0.4e1 * t5;
    t9 = kn * kn;
    t11 = 0.4e1 * ZR;
    t12 = 0.2e1 * t2;
    t13 = 0.2e1 - t11 + t12;
    t15 = t13 * kx * kn;
    t17 = kx * xc;
    t18 = sin(t17);
    t21 = kx * kx;
    t23 = cos(t17);
    t24 = kn * t23;
    t27 = 0.3e1 * xc;
    t30 = exp(kn * (x + t27 - 0.4e1));
    t32 = -t3 + xc;
    t33 = 0.2e1 * t32;
    t36 = -t2 + 0.1e1;
    t37 = 0.2e1 * t36;
    t38 = t37 * kx;
    t48 = exp(kn * (x - xc - 0.4e1));
    t50 = t1 - 0.4e1;
    t52 = 0.4e1 * xc;
    t55 = t50 * t2 + (0.8e1 - t52) * ZR + t1 - 0.4e1;
    t68 = exp(kn * (x - t27 - 0.2e1));
    t70 = 0.16e2 * xc;
    t71 = xc * xc;
    t72 = 0.8e1 * t71;
    t76 = 0.16e2 * t71;
    t79 = (t70 - t72) * t2 + (-0.32e2 * xc + t76) * ZR + t70 - t72;
    t81 = t9 * kn;
    t83 = 0.8e1 * t3;
    t91 = 0.4e1 * ZR - 0.4e1;
    t108 = exp(kn * (x - xc - 0.2e1));
    t114 = t72 - 0.16e2 * ZR * t71 + 0.8e1 * t2 * t71;
    t138 = exp(kn * (x + xc - 0.4e1));
    t140 = t71 * xc;
    t141 = t71 - t140;
    t148 = 0.8e1 * t141 * t2 - 0.16e2 * t141 * ZR + t72 - 0.8e1 * t140;
    t150 = t9 * t9;
    t155 = 0.8e1 * xc;
    t161 = t21 * kx;
    t180 = sin(kx);
    t184 = exp(kn * (x - 0.3e1));
    t186 = 0.2e1 * ZR;
    t187 = -0.1e1 - t186 - t2;
    t198 = exp(kn * (x - 0.1e1));
    t200 = 0.4e1 * t32;
    t212 = exp(kn * (x - t1 - 0.1e1));
    t214 = 0.1e1 + t2 - t186;
    t222 = exp(kn * (x - t52 - 0.1e1));
    t226 = -t50 * t2 + t1 - 0.4e1;
    t230 = -t37 * kx;
    t239 = exp(kn * (x + xc - 0.2e1));
    t242 = 0.2e1 * t71 - 0.2e1 * xc;
    t244 = 0.2e1 * t71;
    t245 = t242 * t2 - t244 + t1;
    t248 = -0.1e1 + t52;
    t261 = exp(kn * (x - t1 - 0.3e1));
    t265 = -t242 * t2 + t244 - t1;
    t282 = exp(kn * (x + t1 - 0.3e1));
    num2B = ((t7 * kx * t9 + t15) * t18 - t7 * t21 * t24) * t30 + ((t33 * kx * t9 + t38 * kn) * t18 - t33 * t21 * t24) * t48 + ((t55 * kx * t9 - t13 * kx * kn) * t18 + t55 * t21 * t24) * t68 + ((t79 * kx * t81 + (t83 + (0.8e1 - 0.12e2 * xc) * ZR - 0.8e1 + t52) * kx * t9 + t91 * kx * kn) * t18 + (t79 * t21 * t9 + ((-0.8e1 - t52) * ZR + 0.8e1 + t52) * t21 * kn) * t23) * t108 + ((t114 * kx * t81 + (-0.12e2 * t5 + t83 + t52) * kx * t9 - t91 * kx * kn) * t18 + (-t114 * t21 * t9 + (0.4e1 * t5 - 0.4e1 * xc) * t21 * kn) * t23) * t138 + (t148 * kx * t150 + ((-t76 + t52) * t2 + (0.32e2 * t71 - t155) * ZR - t76 + t52) * kx * t81 + (t148 * t161 + (-t83 + (-0.4e1 + t70) * ZR - t155) * kx) * t9 + ((-0.8e1 * t5 + 0.4e1 * t3 + t52) * t161 + 0.8e1 * kx * ZR) * kn - 0.4e1 * t161 * ZR) * t180 * t184 + (t187 * kx * t9 + (-t11 - 0.2e1 - t12) * kx * kn + t187 * t161) * t180 * t198 + (t200 * kx * t81 + 0.8e1 * t32 * kx * t9 + t200 * t161 * kn) * t180 * t212 + (t214 * kx * t9 + t15 + t214 * t161) * t180 * t222 + ((t226 * kx * t9 + t230 * kn) * t18 + t226 * t21 * t24) * t239 + (t245 * kx * t81 + (t248 * t2 - t52 + 0.1e1) * kx * t9 + (t245 * t161 + t230) * kn + t36 * t161) * t180 * t261 + (t265 * kx * t81 + (-t248 * t2 - 0.1e1 + t52) * kx * t9 + (t265 * t161 + t38) * kn - t36 * t161) * t180 * t282;

    t1 = kn * kn;
    t4 = exp(kn * (xc - 0.2e1));
    t5 = t1 * t4;
    t7 = kx * xc;
    t8 = cos(t7);
    t9 = t7 * t8;
    t12 = kn * xc;
    t13 = exp(-t12);
    t14 = t1 * t13;
    t21 = exp(kn * (-0.2e1 + 0.3e1 * xc));
    t22 = kn * t21;
    t24 = ZR * t8 * kx;
    t27 = sin(kx);
    t28 = exp(-kn);
    t29 = t27 * t28;
    t30 = kx * kx;
    t31 = t29 * t30;
    t32 = ZR * xc;
    t37 = exp(-0.3e1 * t12);
    t38 = t1 * t37;
    t39 = sin(t7);
    t42 = 0.2e1 * xc;
    t45 = exp(-kn * (t42 + 0.1e1));
    t46 = t27 * t45;
    t47 = t1 * kn;
    t52 = exp(kn * (-0.1e1 + t42));
    t53 = t27 * t52;
    t56 = exp(t12);
    t57 = t1 * t56;
    t61 = exp(-0.3e1 * kn);
    t62 = t27 * t61;
    t64 = t1 * t21;
    t70 = 0.16e2 * t5 * ZR * t9 - 0.16e2 * t14 * ZR * t9 + 0.4e1 * t22 * t24 - 0.8e1 * t31 * t32 * kn - 0.2e1 * t38 * t39 - 0.2e1 * t46 * t47 + 0.2e1 * t53 * t47 - 0.2e1 * t57 * t39 + t62 * t1 + 0.2e1 * t64 * t39 - 0.3e1 * t46 * t1 + t46 * t30;
    t74 = exp(kn * (-0.3e1 + 0.4e1 * xc));
    t75 = t27 * t74;
    t79 = t53 * t30;
    t84 = exp(-kn * (xc + 0.2e1));
    t85 = t1 * t84;
    t90 = ZR * ZR;
    t91 = t90 * t39;
    t94 = t90 * t27;
    t106 = t30 * kn;
    t109 = t1 * t1;
    t113 = -t75 * t1 + 0.3e1 * t53 * t1 - t79 + t62 * t30 - t75 * t30 + 0.2e1 * t85 * t39 + 0.4e1 * t5 * t39 - 0.2e1 * t85 * t91 - 0.2e1 * t94 * t45 * xc * t47 + 0.24e2 * t29 * ZR * t47 * xc - 0.2e1 * t53 * t90 * t47 - 0.2e1 * t46 * t106 - 0.8e1 * t29 * xc * t109;
    t115 = xc * xc;
    t119 = ZR * t27;
    t120 = t61 * t30;
    t132 = t45 * t30;
    t139 = ZR * t39;
    t142 = kn * t37;
    t143 = t8 * kx;
    t146 = kn * t13;
    t149 = t47 * xc;
    t152 = exp(kn * (t42 - 0.3e1));
    t153 = t27 * t152;
    t156 = 0.8e1 * t29 * t109 * t115 + 0.2e1 * t119 * t120 - 0.4e1 * t119 * t28 * t1 - 0.4e1 * t119 * t28 * t30 + 0.3e1 * t94 * t45 * t1 - t94 * t132 + 0.2e1 * t94 * t45 * t47 - 0.2e1 * t38 * t91 + 0.4e1 * t38 * t139 - 0.2e1 * t142 * t143 + 0.4e1 * t146 * t143 + 0.4e1 * t149 * t153;
    t162 = t30 * xc * kn;
    t165 = t47 * t4;
    t166 = t91 * xc;
    t170 = t90 * t8 * kx;
    t184 = t29 * t90;
    t186 = t1 * t115 * t30;
    t196 = t90 * xc;
    t197 = t196 * t47;
    t200 = -0.12e2 * t29 * t149 + 0.2e1 * t64 * t91 - 0.2e1 * t53 * t162 + 0.8e1 * t165 * t166 - 0.2e1 * t142 * t170 + 0.4e1 * t142 * t24 - 0.16e2 * t29 * ZR * t115 * t109 - 0.8e1 * t5 * t9 + 0.16e2 * t31 * t32 * t1 + 0.8e1 * t184 * t186 - 0.4e1 * t94 * t152 * t162 - 0.4e1 * t94 * t152 * xc * t47 - 0.12e2 * t29 * t197;
    t209 = t47 * t13;
    t210 = t139 * xc;
    t216 = t30 * t1 * xc;
    t221 = t90 * t109;
    t237 = -0.2e1 * t22 * t170 + 0.16e2 * t29 * ZR * t109 * xc - 0.16e2 * t209 * t210 + 0.8e1 * t209 * t166 - 0.8e1 * t29 * t216 + 0.4e1 * t29 * t162 + 0.8e1 * t29 * t221 * t115 + 0.4e1 * t12 * t153 * t30 + 0.8e1 * t14 * t90 * t9 + 0.8e1 * t14 * t9 - 0.4e1 * t64 * t139 - 0.2e1 * t22 * t143;
    t245 = kn * t4;
    t248 = t90 * t30;
    t250 = ZR * t1;
    t256 = kn * t56;
    t259 = kn * t84;
    t265 = t196 * kn;
    t273 = 0.2e1 * t46 * t149 - 0.16e2 * t165 * t210 - 0.8e1 * t5 * t90 * t9 - 0.4e1 * t245 * t24 + t53 * t248 - 0.16e2 * t31 * t250 * t115 - 0.8e1 * t184 * t216 + 0.2e1 * t256 * t170 + 0.2e1 * t259 * t170 - 0.2e1 * t53 * t248 * kn + 0.4e1 * t31 * t265 - 0.8e1 * t29 * t221 * xc + 0.2e1 * t79 * t265;
    t283 = t1 * t90;
    t295 = t39 * xc;
    t300 = -0.4e1 * t14 * t39 + 0.4e1 * t245 * t143 - 0.2e1 * t259 * t143 - 0.2e1 * t53 * t149 - t75 * t283 - 0.3e1 * t53 * t283 + 0.2e1 * t75 * ZR * t30 + 0.2e1 * t75 * t250 - t75 * t248 + 0.2e1 * t53 * t197 + 0.8e1 * t165 * t295 - 0.4e1 * t5 * t139;
    t301 = t61 * t1;
    t328 = 0.2e1 * t119 * t301 + t94 * t301 + 0.2e1 * t46 * t162 + t94 * t120 + 0.4e1 * t14 * t139 - 0.4e1 * t146 * t24 - 0.2e1 * t256 * t143 + 0.8e1 * t209 * t295 + 0.2e1 * t57 * t91 - 0.2e1 * t94 * t45 * t162 + 0.2e1 * t53 * t106 + 0.2e1 * t94 * t132 * kn + 0.8e1 * t29 * t186;
    num3B = -kx * (t70 + t113 + t156 + t200 + t237 + t273 + t300 + t328);

    t1 = ZR * ZR;
    t4 = -0.2e1 * t1 + 0.4e1 * ZR - 0.2e1;
    t6 = kn * kn;
    t7 = kx * xc;
    t8 = sin(t7);
    t9 = t6 * t8;
    t11 = -t4;
    t12 = kx * kx;
    t14 = cos(t7);
    t15 = kn * t14;
    t16 = t11 * t12 * t15;
    t18 = 0.3e1 * xc;
    t21 = exp(kn * (x + t18 - 0.4e1));
    t23 = t1 - 0.1e1;
    t24 = 0.2e1 * t23;
    t27 = -t24;
    t29 = t27 * t12 * t15;
    t33 = exp(kn * (x - xc - 0.4e1));
    t40 = exp(kn * (x - t18 - 0.2e1));
    t42 = t1 * xc;
    t44 = 0.8e1 * xc;
    t45 = ZR * xc;
    t47 = -0.8e1 * t42 - t44 + 0.16e2 * t45;
    t49 = t6 * kn;
    t50 = t47 * kx * t49;
    t52 = 0.4e1 - 0.4e1 * ZR;
    t59 = -t52;
    t61 = t59 * t12 * kn;
    t67 = exp(kn * (x - xc - 0.2e1));
    t81 = exp(kn * (x + xc - 0.4e1));
    t83 = xc * xc;
    t84 = -xc + t83;
    t91 = 0.8e1 * t1 * t84 - 0.16e2 * t84 * ZR - t44 + 0.8e1 * t83;
    t93 = t6 * t6;
    t101 = t12 * kx;
    t108 = 0.4e1 * xc;
    t116 = sin(kx);
    t120 = exp(kn * (x - 0.3e1));
    t122 = 0.2e1 * ZR;
    t123 = t122 + t1 + 0.1e1;
    t131 = exp(kn * (x - 0.1e1));
    t134 = -0.4e1 * xc + 0.4e1 * t42;
    t141 = 0.2e1 * xc;
    t144 = exp(kn * (x - t141 - 0.1e1));
    t146 = -t1 - 0.1e1 + t122;
    t154 = exp(kn * (x - t108 - 0.1e1));
    t161 = exp(kn * (x + xc - 0.2e1));
    t164 = -0.2e1 * xc + 0.2e1;
    t166 = t164 * t1 + t141 - 0.2e1;
    t169 = -t23;
    t170 = 0.3e1 * t169;
    t180 = exp(kn * (x - t141 - 0.3e1));
    t184 = -t164 * t1 - t141 + 0.2e1;
    t197 = exp(kn * (x + t141 - 0.3e1));
    num4B = (t4 * kx * t9 + t16) * t21 + (t24 * kx * t9 + t29) * t33 + (t11 * kx * t9 + t16) * t40 + ((t50 + t52 * kx * t6) * t8 + (t47 * t12 * t6 + t61) * t14) * t67 + ((t50 + t59 * kx * t6) * t8 + (-t47 * t12 * t6 + t61) * t14) * t81 + (t91 * kx * t93 + (-0.24e2 * t45 + 0.12e2 * xc + 0.12e2 * t42) * kx * t49 + (t91 * t101 - 0.4e1 * kx * ZR) * t6 + (-0.4e1 * t42 - t108 + 0.8e1 * t45) * t101 * kn - 0.4e1 * t101 * ZR) * t116 * t120 + (t123 * kx * t6 + t123 * t101) * t116 * t131 + (t134 * kx * t49 + t134 * t101 * kn) * t116 * t144 + (t146 * kx * t6 + t146 * t101) * t116 * t154 + (t27 * kx * t9 + t29) * t161 + (t166 * kx * t49 + t170 * kx * t6 + t166 * t101 * kn + t23 * t101) * t116 * t180 + (t184 * kx * t49 - t170 * kx * t6 + t184 * t101 * kn + t169 * t101) * t116 * t197;


    t4 = exp(-0.2e1 * kn * (xc + 0.1e1));
    t5 = kn * t4;
    t8 = exp(-0.2e1 * kn);
    t12 = kn * (-0.1e1 + xc);
    t14 = exp(0.4e1 * t12);
    t17 = kn * xc;
    t19 = exp(-0.4e1 * t17);
    t23 = exp(-0.4e1 * kn);
    t26 = kn * kn;
    t27 = t26 * t8;
    t28 = xc * xc;
    t32 = ZR * ZR;
    t33 = t32 * xc;
    t40 = exp(0.2e1 * t12);
    t41 = kn * t40;
    t50 = 0.1e1 - 0.4e1 * t5 - 0.8e1 * t8 * ZR + 0.2e1 * t14 * ZR + 0.2e1 * t19 * ZR + 0.2e1 * t23 * ZR - 0.32e2 * t27 * ZR * t28 - 0.16e2 * t27 * t33 + 0.16e2 * t27 * t32 * t28 + 0.4e1 * t41 * t33 + 0.4e1 * t41 - t19 * t32 - t14 * t32 + t23 * t32 + 0.16e2 * t27 * t28;
    t52 = exp(-0.2e1 * t17);
    t62 = exp(0.2e1 * kn * (xc - 0.2e1));
    t83 = -0.4e1 * t17 * t52 + 0.4e1 * t5 * xc + 0.4e1 * t5 * t32 + 0.4e1 * t17 * t62 - 0.4e1 * t17 * t62 * t32 - 0.4e1 * t5 * t33 + 0.4e1 * t17 * t52 * t32 + 0.32e2 * t27 * ZR * xc - t19 + t23 - t14 + t32 - 0.16e2 * t27 * xc + 0.2e1 * ZR - 0.4e1 * t41 * xc - 0.4e1 * t41 * t32;
    denB = 0.2e1 * kn * (t50 + t83);

    _C1B=num1B/denB; _C2B=num2B/denB; _C3B=num3B/denB; _C4B=num4B/denB;
    Z=ZB; _C1=_C1B; _C2=_C2B; _C3=_C3B; _C4=_C4B;
  }
  u1 = -kn * ((_C1 + x * _C3) * exp(-kn * x) + _C2 + x * _C4 + kx * sin(kx * x)) / Z * pow(kx * kx + kn * kn, -0.2e1);

  u2 = (-(_C1 + x * _C3) * kn + _C3) / Z * pow(kx * kx + kn * kn, -0.2e1) * exp(-kn * x) + (_C4 + (_C2 + x * _C4) * kn) / Z * pow(kx * kx + kn * kn, -0.2e1) + kx * kx * cos(kx * x) / Z * pow(kx * kx + kn * kn, -0.2e1);

  u3 = (-0.2e1 * kn * kx * kx * pow(kx * kx + kn * kn, -0.2e1) + kn / (kx * kx + kn * kn)) * cos(kx * x) - 0.2e1 * kn * (-(_C1 + x * _C3) * kn + _C3) * pow(kx * kx + kn * kn, -0.2e1) * exp(-kn * x) - 0.2e1 * kn * (_C4 + (_C2 + x * _C4) * kn) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * _C3 * kn * exp(-kn * x) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * _C4 * kn * pow(kx * kx + kn * kn, -0.2e1);

  u4 = 0.2e1 * kn * (kn * _C1 + kn * x * _C3 - _C3) * exp(-kn * x) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * kn * (kn * _C2 + kn * x * _C4 + _C4) * pow(kx * kx + kn * kn, -0.2e1) + kx * (kn - kx) * (kx + kn) * sin(kx * x) * pow(kx * kx + kn * kn, -0.2e1);

  u5 = -0.2e1 * _C3 * kn * exp(-kn * x) * pow(kx * kx + kn * kn, -0.2e1) - 0.2e1 * _C4 * kn * pow(kx * kx + kn * kn, -0.2e1) - cos(kx * x) * kn / (kx * kx + kn * kn);

  u6 = (0.2e1 * kn * kx * kx * pow(kx * kx + kn * kn, -0.2e1) + kn / (kx * kx + kn * kn)) * cos(kx * x) + 0.2e1 * kn * (-(_C1 + x * _C3) * kn + _C3) * pow(kx * kx + kn * kn, -0.2e1) * exp(-kn * x) + 0.2e1 * kn * (_C4 + (_C2 + x * _C4) * kn) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * _C3 * kn * exp(-kn * x) * pow(kx * kx + kn * kn, -0.2e1) + 0.2e1 * _C4 * kn * pow(kx * kx + kn * kn, -0.2e1);

  sum5 +=u5*cos(kn*z);/* pressure */
  sum6 +=u6*cos(kn*z);/* zz stress */
  u1 *= cos(kn*z); /* x velocity */
  sum1 += u1;
  u2 *= sin(kn*z); /* z velocity */
  sum2 += u2;
  u3 *= cos(kn*z); /* xx stress */
  sum3 += u3;
  u4 *= sin(kn*z); /* zx stress */
  sum4 += u4;
  mag=sqrt(sum1*sum1+sum2*sum2);
  /* Output */
  if( vel != NULL ) {
    vel[0] = sum1;
    vel[1] = sum2;
  }
  if( presssure != NULL ) {
    (*presssure) = sum5;
  }
  if( total_stress != NULL ) {
    total_stress[0] = sum3;
    total_stress[1] = sum6; 
    total_stress[2] = sum4;
  }
  if( strain_rate != NULL ) {
    if( x>xc ) {
      Z = ZB;
    }
    else {
      Z = ZA;
    }
    strain_rate[0] = (sum3+sum5)/(2.0*Z);
    strain_rate[1] = (sum6+sum5)/(2.0*Z);
    strain_rate[2] = (sum4)/(2.0*Z);
  }
}
