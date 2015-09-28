/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "types.h"
#include "ColourMap.h"
#include "X11Colours.h"
#include <ctype.h>
#include <string.h>

/* Reads hex or colour from X11 Colour Chart */
/* Defaults to black if anything else */
void lucColour_FromX11ColourName( lucColour* self, Name x11ColourName )
{
   int rgb[3];

   if (strncmp(x11ColourName,"#",1) == 0)
   {
      lucColour_FromHexName( self, x11ColourName );
      return;
   }
   else if (strncasecmp(x11ColourName,"snow",4) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 250;
      rgb[2] = 250;
   }
   else if (strncasecmp(x11ColourName,"GhostWhite",10) == 0)
   {
      rgb[0] = 248;
      rgb[1] = 248;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"WhiteSmoke",10) == 0)
   {
      rgb[0] = 245;
      rgb[1] = 245;
      rgb[2] = 245;
   }
   else if (strncasecmp(x11ColourName,"gainsboro",9) == 0)
   {
      rgb[0] = 220;
      rgb[1] = 220;
      rgb[2] = 220;
   }
   else if (strncasecmp(x11ColourName,"FloralWhite",11) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 250;
      rgb[2] = 240;
   }
   else if (strncasecmp(x11ColourName,"OldLace",7) == 0)
   {
      rgb[0] = 253;
      rgb[1] = 245;
      rgb[2] = 230;
   }
   else if (strncasecmp(x11ColourName,"linen",5) == 0)
   {
      rgb[0] = 250;
      rgb[1] = 240;
      rgb[2] = 230;
   }
   else if (strncasecmp(x11ColourName,"AntiqueWhite",12) == 0)
   {
      rgb[0] = 250;
      rgb[1] = 235;
      rgb[2] = 215;
   }
   else if (strncasecmp(x11ColourName,"PapayaWhip",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 239;
      rgb[2] = 213;
   }
   else if (strncasecmp(x11ColourName,"BlanchedAlmond",14) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 235;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"bisque",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 228;
      rgb[2] = 196;
   }
   else if (strncasecmp(x11ColourName,"PeachPuff",9) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 218;
      rgb[2] = 185;
   }
   else if (strncasecmp(x11ColourName,"NavajoWhite",11) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 222;
      rgb[2] = 173;
   }
   else if (strncasecmp(x11ColourName,"moccasin",8) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 228;
      rgb[2] = 181;
   }
   else if (strncasecmp(x11ColourName,"cornsilk",8) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 248;
      rgb[2] = 220;
   }
   else if (strncasecmp(x11ColourName,"ivory",5) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 240;
   }
   else if (strncasecmp(x11ColourName,"LemonChiffon",12) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 250;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"seashell",8) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 245;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"honeydew",8) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 255;
      rgb[2] = 240;
   }
   else if (strncasecmp(x11ColourName,"MintCream",9) == 0)
   {
      rgb[0] = 245;
      rgb[1] = 255;
      rgb[2] = 250;
   }
   else if (strncasecmp(x11ColourName,"azure",5) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"AliceBlue",9) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 248;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"lavender",8) == 0)
   {
      rgb[0] = 230;
      rgb[1] = 230;
      rgb[2] = 250;
   }
   else if (strncasecmp(x11ColourName,"LavenderBlush",13) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 240;
      rgb[2] = 245;
   }
   else if (strncasecmp(x11ColourName,"MistyRose",9) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 228;
      rgb[2] = 225;
   }
   else if (strncasecmp(x11ColourName,"white",5) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"black",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"DarkSlateGray",13) == 0)
   {
      rgb[0] = 47;
      rgb[1] = 79;
      rgb[2] = 79;
   }
   else if (strncasecmp(x11ColourName,"DarkSlateGrey",13) == 0)
   {
      rgb[0] = 47;
      rgb[1] = 79;
      rgb[2] = 79;
   }
   else if (strncasecmp(x11ColourName,"DimGray",7) == 0)
   {
      rgb[0] = 105;
      rgb[1] = 105;
      rgb[2] = 105;
   }
   else if (strncasecmp(x11ColourName,"DimGrey",7) == 0)
   {
      rgb[0] = 105;
      rgb[1] = 105;
      rgb[2] = 105;
   }
   else if (strncasecmp(x11ColourName,"SlateGray",9) == 0)
   {
      rgb[0] = 112;
      rgb[1] = 128;
      rgb[2] = 144;
   }
   else if (strncasecmp(x11ColourName,"SlateGrey",9) == 0)
   {
      rgb[0] = 112;
      rgb[1] = 128;
      rgb[2] = 144;
   }
   else if (strncasecmp(x11ColourName,"LightSlateGray",14) == 0)
   {
      rgb[0] = 119;
      rgb[1] = 136;
      rgb[2] = 153;
   }
   else if (strncasecmp(x11ColourName,"LightSlateGrey",14) == 0)
   {
      rgb[0] = 119;
      rgb[1] = 136;
      rgb[2] = 153;
   }
   else if (strncasecmp(x11ColourName,"gray",4) == 0)
   {
      rgb[0] = 190;
      rgb[1] = 190;
      rgb[2] = 190;
   }
   else if (strncasecmp(x11ColourName,"grey",4) == 0)
   {
      rgb[0] = 190;
      rgb[1] = 190;
      rgb[2] = 190;
   }
   else if (strncasecmp(x11ColourName,"LightGrey",9) == 0)
   {
      rgb[0] = 211;
      rgb[1] = 211;
      rgb[2] = 211;
   }
   else if (strncasecmp(x11ColourName,"LightGray",9) == 0)
   {
      rgb[0] = 211;
      rgb[1] = 211;
      rgb[2] = 211;
   }
   else if (strncasecmp(x11ColourName,"MidnightBlue",12) == 0)
   {
      rgb[0] = 25;
      rgb[1] = 25;
      rgb[2] = 112;
   }
   else if (strncasecmp(x11ColourName,"navy",4) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 128;
   }
   else if (strncasecmp(x11ColourName,"NavyBlue",8) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 128;
   }
   else if (strncasecmp(x11ColourName,"CornflowerBlue",14) == 0)
   {
      rgb[0] = 100;
      rgb[1] = 149;
      rgb[2] = 237;
   }
   else if (strncasecmp(x11ColourName,"DarkSlateBlue",13) == 0)
   {
      rgb[0] = 72;
      rgb[1] = 61;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"SlateBlue",9) == 0)
   {
      rgb[0] = 106;
      rgb[1] = 90;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"MediumSlateBlue",15) == 0)
   {
      rgb[0] = 123;
      rgb[1] = 104;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"LightSlateBlue",14) == 0)
   {
      rgb[0] = 132;
      rgb[1] = 112;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"MediumBlue",10) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"RoyalBlue",9) == 0)
   {
      rgb[0] = 65;
      rgb[1] = 105;
      rgb[2] = 225;
   }
   else if (strncasecmp(x11ColourName,"blue",4) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"DodgerBlue",10) == 0)
   {
      rgb[0] = 30;
      rgb[1] = 144;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"DeepSkyBlue",11) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 191;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"SkyBlue",7) == 0)
   {
      rgb[0] = 135;
      rgb[1] = 206;
      rgb[2] = 235;
   }
   else if (strncasecmp(x11ColourName,"LightSkyBlue",12) == 0)
   {
      rgb[0] = 135;
      rgb[1] = 206;
      rgb[2] = 250;
   }
   else if (strncasecmp(x11ColourName,"SteelBlue",9) == 0)
   {
      rgb[0] = 70;
      rgb[1] = 130;
      rgb[2] = 180;
   }
   else if (strncasecmp(x11ColourName,"LightSteelBlue",14) == 0)
   {
      rgb[0] = 176;
      rgb[1] = 196;
      rgb[2] = 222;
   }
   else if (strncasecmp(x11ColourName,"LightBlue",9) == 0)
   {
      rgb[0] = 173;
      rgb[1] = 216;
      rgb[2] = 230;
   }
   else if (strncasecmp(x11ColourName,"PowderBlue",10) == 0)
   {
      rgb[0] = 176;
      rgb[1] = 224;
      rgb[2] = 230;
   }
   else if (strncasecmp(x11ColourName,"PaleTurquoise",13) == 0)
   {
      rgb[0] = 175;
      rgb[1] = 238;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"DarkTurquoise",13) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 206;
      rgb[2] = 209;
   }
   else if (strncasecmp(x11ColourName,"MediumTurquoise",15) == 0)
   {
      rgb[0] = 72;
      rgb[1] = 209;
      rgb[2] = 204;
   }
   else if (strncasecmp(x11ColourName,"turquoise",9) == 0)
   {
      rgb[0] = 64;
      rgb[1] = 224;
      rgb[2] = 208;
   }
   else if (strncasecmp(x11ColourName,"cyan",4) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"LightCyan",9) == 0)
   {
      rgb[0] = 224;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"CadetBlue",9) == 0)
   {
      rgb[0] = 95;
      rgb[1] = 158;
      rgb[2] = 160;
   }
   else if (strncasecmp(x11ColourName,"MediumAquamarine",16) == 0)
   {
      rgb[0] = 102;
      rgb[1] = 205;
      rgb[2] = 170;
   }
   else if (strncasecmp(x11ColourName,"aquamarine",10) == 0)
   {
      rgb[0] = 127;
      rgb[1] = 255;
      rgb[2] = 212;
   }
   else if (strncasecmp(x11ColourName,"DarkGreen",9) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 100;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"DarkOliveGreen",14) == 0)
   {
      rgb[0] = 85;
      rgb[1] = 107;
      rgb[2] = 47;
   }
   else if (strncasecmp(x11ColourName,"DarkSeaGreen",12) == 0)
   {
      rgb[0] = 143;
      rgb[1] = 188;
      rgb[2] = 143;
   }
   else if (strncasecmp(x11ColourName,"SeaGreen",8) == 0)
   {
      rgb[0] = 46;
      rgb[1] = 139;
      rgb[2] = 87;
   }
   else if (strncasecmp(x11ColourName,"MediumSeaGreen",14) == 0)
   {
      rgb[0] = 60;
      rgb[1] = 179;
      rgb[2] = 113;
   }
   else if (strncasecmp(x11ColourName,"LightSeaGreen",13) == 0)
   {
      rgb[0] = 32;
      rgb[1] = 178;
      rgb[2] = 170;
   }
   else if (strncasecmp(x11ColourName,"PaleGreen",9) == 0)
   {
      rgb[0] = 152;
      rgb[1] = 251;
      rgb[2] = 152;
   }
   else if (strncasecmp(x11ColourName,"SpringGreen",11) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 255;
      rgb[2] = 127;
   }
   else if (strncasecmp(x11ColourName,"LawnGreen",9) == 0)
   {
      rgb[0] = 124;
      rgb[1] = 252;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"green",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"chartreuse",10) == 0)
   {
      rgb[0] = 127;
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"MediumSpringGreen",17) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 250;
      rgb[2] = 154;
   }
   else if (strncasecmp(x11ColourName,"GreenYellow",11) == 0)
   {
      rgb[0] = 173;
      rgb[1] = 255;
      rgb[2] = 47;
   }
   else if (strncasecmp(x11ColourName,"LimeGreen",9) == 0)
   {
      rgb[0] = 50;
      rgb[1] = 205;
      rgb[2] = 50;
   }
   else if (strncasecmp(x11ColourName,"YellowGreen",11) == 0)
   {
      rgb[0] = 154;
      rgb[1] = 205;
      rgb[2] = 50;
   }
   else if (strncasecmp(x11ColourName,"ForestGreen",11) == 0)
   {
      rgb[0] = 34;
      rgb[1] = 139;
      rgb[2] = 34;
   }
   else if (strncasecmp(x11ColourName,"OliveDrab",9) == 0)
   {
      rgb[0] = 107;
      rgb[1] = 142;
      rgb[2] = 35;
   }
   else if (strncasecmp(x11ColourName,"DarkKhaki",9) == 0)
   {
      rgb[0] = 189;
      rgb[1] = 183;
      rgb[2] = 107;
   }
   else if (strncasecmp(x11ColourName,"khaki",5) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 230;
      rgb[2] = 140;
   }
   else if (strncasecmp(x11ColourName,"PaleGoldenrod",13) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 232;
      rgb[2] = 170;
   }
   else if (strncasecmp(x11ColourName,"LightGoldenrodYellow",20) == 0)
   {
      rgb[0] = 250;
      rgb[1] = 250;
      rgb[2] = 210;
   }
   else if (strncasecmp(x11ColourName,"LightYellow",11) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 224;
   }
   else if (strncasecmp(x11ColourName,"yellow",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"gold",4) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 215;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"LightGoldenrod",14) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 221;
      rgb[2] = 130;
   }
   else if (strncasecmp(x11ColourName,"goldenrod",9) == 0)
   {
      rgb[0] = 218;
      rgb[1] = 165;
      rgb[2] = 32;
   }
   else if (strncasecmp(x11ColourName,"DarkGoldenrod",13) == 0)
   {
      rgb[0] = 184;
      rgb[1] = 134;
      rgb[2] = 11;
   }
   else if (strncasecmp(x11ColourName,"RosyBrown",9) == 0)
   {
      rgb[0] = 188;
      rgb[1] = 143;
      rgb[2] = 143;
   }
   else if (strncasecmp(x11ColourName,"IndianRed",9) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 92;
      rgb[2] = 92;
   }
   else if (strncasecmp(x11ColourName,"SaddleBrown",11) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 69;
      rgb[2] = 19;
   }
   else if (strncasecmp(x11ColourName,"sienna",6) == 0)
   {
      rgb[0] = 160;
      rgb[1] = 82;
      rgb[2] = 45;
   }
   else if (strncasecmp(x11ColourName,"peru",4) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 133;
      rgb[2] = 63;
   }
   else if (strncasecmp(x11ColourName,"burlywood",9) == 0)
   {
      rgb[0] = 222;
      rgb[1] = 184;
      rgb[2] = 135;
   }
   else if (strncasecmp(x11ColourName,"beige",5) == 0)
   {
      rgb[0] = 245;
      rgb[1] = 245;
      rgb[2] = 220;
   }
   else if (strncasecmp(x11ColourName,"wheat",5) == 0)
   {
      rgb[0] = 245;
      rgb[1] = 222;
      rgb[2] = 179;
   }
   else if (strncasecmp(x11ColourName,"SandyBrown",10) == 0)
   {
      rgb[0] = 244;
      rgb[1] = 164;
      rgb[2] = 96;
   }
   else if (strncasecmp(x11ColourName,"tan",3) == 0)
   {
      rgb[0] = 210;
      rgb[1] = 180;
      rgb[2] = 140;
   }
   else if (strncasecmp(x11ColourName,"chocolate",9) == 0)
   {
      rgb[0] = 210;
      rgb[1] = 105;
      rgb[2] = 30;
   }
   else if (strncasecmp(x11ColourName,"firebrick",9) == 0)
   {
      rgb[0] = 178;
      rgb[1] = 34;
      rgb[2] = 34;
   }
   else if (strncasecmp(x11ColourName,"brown",5) == 0)
   {
      rgb[0] = 165;
      rgb[1] = 42;
      rgb[2] = 42;
   }
   else if (strncasecmp(x11ColourName,"DarkSalmon",10) == 0)
   {
      rgb[0] = 233;
      rgb[1] = 150;
      rgb[2] = 122;
   }
   else if (strncasecmp(x11ColourName,"salmon",6) == 0)
   {
      rgb[0] = 250;
      rgb[1] = 128;
      rgb[2] = 114;
   }
   else if (strncasecmp(x11ColourName,"LightSalmon",11) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 160;
      rgb[2] = 122;
   }
   else if (strncasecmp(x11ColourName,"orange",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 165;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"DarkOrange",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 140;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"coral",5) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 127;
      rgb[2] = 80;
   }
   else if (strncasecmp(x11ColourName,"LightCoral",10) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 128;
      rgb[2] = 128;
   }
   else if (strncasecmp(x11ColourName,"tomato",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 99;
      rgb[2] = 71;
   }
   else if (strncasecmp(x11ColourName,"OrangeRed",9) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 69;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"red",3) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"HotPink",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 105;
      rgb[2] = 180;
   }
   else if (strncasecmp(x11ColourName,"DeepPink",8) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 20;
      rgb[2] = 147;
   }
   else if (strncasecmp(x11ColourName,"pink",4) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 192;
      rgb[2] = 203;
   }
   else if (strncasecmp(x11ColourName,"LightPink",9) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 182;
      rgb[2] = 193;
   }
   else if (strncasecmp(x11ColourName,"PaleVioletRed",13) == 0)
   {
      rgb[0] = 219;
      rgb[1] = 112;
      rgb[2] = 147;
   }
   else if (strncasecmp(x11ColourName,"maroon",6) == 0)
   {
      rgb[0] = 176;
      rgb[1] = 48;
      rgb[2] = 96;
   }
   else if (strncasecmp(x11ColourName,"MediumVioletRed",15) == 0)
   {
      rgb[0] = 199;
      rgb[1] = 21;
      rgb[2] = 133;
   }
   else if (strncasecmp(x11ColourName,"VioletRed",9) == 0)
   {
      rgb[0] = 208;
      rgb[1] = 32;
      rgb[2] = 144;
   }
   else if (strncasecmp(x11ColourName,"magenta",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 0;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"violet",6) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 130;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"plum",4) == 0)
   {
      rgb[0] = 221;
      rgb[1] = 160;
      rgb[2] = 221;
   }
   else if (strncasecmp(x11ColourName,"orchid",6) == 0)
   {
      rgb[0] = 218;
      rgb[1] = 112;
      rgb[2] = 214;
   }
   else if (strncasecmp(x11ColourName,"MediumOrchid",12) == 0)
   {
      rgb[0] = 186;
      rgb[1] = 85;
      rgb[2] = 211;
   }
   else if (strncasecmp(x11ColourName,"DarkOrchid",10) == 0)
   {
      rgb[0] = 153;
      rgb[1] = 50;
      rgb[2] = 204;
   }
   else if (strncasecmp(x11ColourName,"DarkViolet",10) == 0)
   {
      rgb[0] = 148;
      rgb[1] = 0;
      rgb[2] = 211;
   }
   else if (strncasecmp(x11ColourName,"BlueViolet",10) == 0)
   {
      rgb[0] = 138;
      rgb[1] = 43;
      rgb[2] = 226;
   }
   else if (strncasecmp(x11ColourName,"purple",6) == 0)
   {
      rgb[0] = 160;
      rgb[1] = 32;
      rgb[2] = 240;
   }
   else if (strncasecmp(x11ColourName,"MediumPurple",12) == 0)
   {
      rgb[0] = 147;
      rgb[1] = 112;
      rgb[2] = 219;
   }
   else if (strncasecmp(x11ColourName,"thistle",7) == 0)
   {
      rgb[0] = 216;
      rgb[1] = 191;
      rgb[2] = 216;
   }
   else if (strncasecmp(x11ColourName,"snow1",5) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 250;
      rgb[2] = 250;
   }
   else if (strncasecmp(x11ColourName,"snow2",5) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 233;
      rgb[2] = 233;
   }
   else if (strncasecmp(x11ColourName,"snow3",5) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 201;
      rgb[2] = 201;
   }
   else if (strncasecmp(x11ColourName,"snow4",5) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 137;
      rgb[2] = 137;
   }
   else if (strncasecmp(x11ColourName,"seashell1",9) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 245;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"seashell2",9) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 229;
      rgb[2] = 222;
   }
   else if (strncasecmp(x11ColourName,"seashell3",9) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 197;
      rgb[2] = 191;
   }
   else if (strncasecmp(x11ColourName,"seashell4",9) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 134;
      rgb[2] = 130;
   }
   else if (strncasecmp(x11ColourName,"AntiqueWhite1",13) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 239;
      rgb[2] = 219;
   }
   else if (strncasecmp(x11ColourName,"AntiqueWhite2",13) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 223;
      rgb[2] = 204;
   }
   else if (strncasecmp(x11ColourName,"AntiqueWhite3",13) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 192;
      rgb[2] = 176;
   }
   else if (strncasecmp(x11ColourName,"AntiqueWhite4",13) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 131;
      rgb[2] = 120;
   }
   else if (strncasecmp(x11ColourName,"bisque1",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 228;
      rgb[2] = 196;
   }
   else if (strncasecmp(x11ColourName,"bisque2",7) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 213;
      rgb[2] = 183;
   }
   else if (strncasecmp(x11ColourName,"bisque3",7) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 183;
      rgb[2] = 158;
   }
   else if (strncasecmp(x11ColourName,"bisque4",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 125;
      rgb[2] = 107;
   }
   else if (strncasecmp(x11ColourName,"PeachPuff1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 218;
      rgb[2] = 185;
   }
   else if (strncasecmp(x11ColourName,"PeachPuff2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 203;
      rgb[2] = 173;
   }
   else if (strncasecmp(x11ColourName,"PeachPuff3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 175;
      rgb[2] = 149;
   }
   else if (strncasecmp(x11ColourName,"PeachPuff4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 119;
      rgb[2] = 101;
   }
   else if (strncasecmp(x11ColourName,"NavajoWhite1",12) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 222;
      rgb[2] = 173;
   }
   else if (strncasecmp(x11ColourName,"NavajoWhite2",12) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 207;
      rgb[2] = 161;
   }
   else if (strncasecmp(x11ColourName,"NavajoWhite3",12) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 179;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"NavajoWhite4",12) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 121;
      rgb[2] = 94;
   }
   else if (strncasecmp(x11ColourName,"LemonChiffon1",13) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 250;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"LemonChiffon2",13) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 233;
      rgb[2] = 191;
   }
   else if (strncasecmp(x11ColourName,"LemonChiffon3",13) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 201;
      rgb[2] = 165;
   }
   else if (strncasecmp(x11ColourName,"LemonChiffon4",13) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 137;
      rgb[2] = 112;
   }
   else if (strncasecmp(x11ColourName,"cornsilk1",9) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 248;
      rgb[2] = 220;
   }
   else if (strncasecmp(x11ColourName,"cornsilk2",9) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 232;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"cornsilk3",9) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 200;
      rgb[2] = 177;
   }
   else if (strncasecmp(x11ColourName,"cornsilk4",9) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 136;
      rgb[2] = 120;
   }
   else if (strncasecmp(x11ColourName,"ivory1",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 240;
   }
   else if (strncasecmp(x11ColourName,"ivory2",6) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 238;
      rgb[2] = 224;
   }
   else if (strncasecmp(x11ColourName,"ivory3",6) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 205;
      rgb[2] = 193;
   }
   else if (strncasecmp(x11ColourName,"ivory4",6) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 139;
      rgb[2] = 131;
   }
   else if (strncasecmp(x11ColourName,"honeydew1",9) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 255;
      rgb[2] = 240;
   }
   else if (strncasecmp(x11ColourName,"honeydew2",9) == 0)
   {
      rgb[0] = 224;
      rgb[1] = 238;
      rgb[2] = 224;
   }
   else if (strncasecmp(x11ColourName,"honeydew3",9) == 0)
   {
      rgb[0] = 193;
      rgb[1] = 205;
      rgb[2] = 193;
   }
   else if (strncasecmp(x11ColourName,"honeydew4",9) == 0)
   {
      rgb[0] = 131;
      rgb[1] = 139;
      rgb[2] = 131;
   }
   else if (strncasecmp(x11ColourName,"LavenderBlush1",14) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 240;
      rgb[2] = 245;
   }
   else if (strncasecmp(x11ColourName,"LavenderBlush2",14) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 224;
      rgb[2] = 229;
   }
   else if (strncasecmp(x11ColourName,"LavenderBlush3",14) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 193;
      rgb[2] = 197;
   }
   else if (strncasecmp(x11ColourName,"LavenderBlush4",14) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 131;
      rgb[2] = 134;
   }
   else if (strncasecmp(x11ColourName,"MistyRose1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 228;
      rgb[2] = 225;
   }
   else if (strncasecmp(x11ColourName,"MistyRose2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 213;
      rgb[2] = 210;
   }
   else if (strncasecmp(x11ColourName,"MistyRose3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 183;
      rgb[2] = 181;
   }
   else if (strncasecmp(x11ColourName,"MistyRose4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 125;
      rgb[2] = 123;
   }
   else if (strncasecmp(x11ColourName,"azure1",6) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"azure2",6) == 0)
   {
      rgb[0] = 224;
      rgb[1] = 238;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"azure3",6) == 0)
   {
      rgb[0] = 193;
      rgb[1] = 205;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"azure4",6) == 0)
   {
      rgb[0] = 131;
      rgb[1] = 139;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"SlateBlue1",10) == 0)
   {
      rgb[0] = 131;
      rgb[1] = 111;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"SlateBlue2",10) == 0)
   {
      rgb[0] = 122;
      rgb[1] = 103;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"SlateBlue3",10) == 0)
   {
      rgb[0] = 105;
      rgb[1] = 89;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"SlateBlue4",10) == 0)
   {
      rgb[0] = 71;
      rgb[1] = 60;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"RoyalBlue1",10) == 0)
   {
      rgb[0] = 72;
      rgb[1] = 118;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"RoyalBlue2",10) == 0)
   {
      rgb[0] = 67;
      rgb[1] = 110;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"RoyalBlue3",10) == 0)
   {
      rgb[0] = 58;
      rgb[1] = 95;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"RoyalBlue4",10) == 0)
   {
      rgb[0] = 39;
      rgb[1] = 64;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"blue1",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"blue2",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"blue3",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"blue4",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"DodgerBlue1",11) == 0)
   {
      rgb[0] = 30;
      rgb[1] = 144;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"DodgerBlue2",11) == 0)
   {
      rgb[0] = 28;
      rgb[1] = 134;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"DodgerBlue3",11) == 0)
   {
      rgb[0] = 24;
      rgb[1] = 116;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"DodgerBlue4",11) == 0)
   {
      rgb[0] = 16;
      rgb[1] = 78;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"SteelBlue1",10) == 0)
   {
      rgb[0] = 99;
      rgb[1] = 184;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"SteelBlue2",10) == 0)
   {
      rgb[0] = 92;
      rgb[1] = 172;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"SteelBlue3",10) == 0)
   {
      rgb[0] = 79;
      rgb[1] = 148;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"SteelBlue4",10) == 0)
   {
      rgb[0] = 54;
      rgb[1] = 100;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"DeepSkyBlue1",12) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 191;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"DeepSkyBlue2",12) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 178;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"DeepSkyBlue3",12) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 154;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"DeepSkyBlue4",12) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 104;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"SkyBlue1",8) == 0)
   {
      rgb[0] = 135;
      rgb[1] = 206;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"SkyBlue2",8) == 0)
   {
      rgb[0] = 126;
      rgb[1] = 192;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"SkyBlue3",8) == 0)
   {
      rgb[0] = 108;
      rgb[1] = 166;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"SkyBlue4",8) == 0)
   {
      rgb[0] = 74;
      rgb[1] = 112;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"LightSkyBlue1",13) == 0)
   {
      rgb[0] = 176;
      rgb[1] = 226;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"LightSkyBlue2",13) == 0)
   {
      rgb[0] = 164;
      rgb[1] = 211;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"LightSkyBlue3",13) == 0)
   {
      rgb[0] = 141;
      rgb[1] = 182;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"LightSkyBlue4",13) == 0)
   {
      rgb[0] = 96;
      rgb[1] = 123;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"SlateGray1",10) == 0)
   {
      rgb[0] = 198;
      rgb[1] = 226;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"SlateGray2",10) == 0)
   {
      rgb[0] = 185;
      rgb[1] = 211;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"SlateGray3",10) == 0)
   {
      rgb[0] = 159;
      rgb[1] = 182;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"SlateGray4",10) == 0)
   {
      rgb[0] = 108;
      rgb[1] = 123;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"LightSteelBlue1",15) == 0)
   {
      rgb[0] = 202;
      rgb[1] = 225;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"LightSteelBlue2",15) == 0)
   {
      rgb[0] = 188;
      rgb[1] = 210;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"LightSteelBlue3",15) == 0)
   {
      rgb[0] = 162;
      rgb[1] = 181;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"LightSteelBlue4",15) == 0)
   {
      rgb[0] = 110;
      rgb[1] = 123;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"LightBlue1",10) == 0)
   {
      rgb[0] = 191;
      rgb[1] = 239;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"LightBlue2",10) == 0)
   {
      rgb[0] = 178;
      rgb[1] = 223;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"LightBlue3",10) == 0)
   {
      rgb[0] = 154;
      rgb[1] = 192;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"LightBlue4",10) == 0)
   {
      rgb[0] = 104;
      rgb[1] = 131;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"LightCyan1",10) == 0)
   {
      rgb[0] = 224;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"LightCyan2",10) == 0)
   {
      rgb[0] = 209;
      rgb[1] = 238;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"LightCyan3",10) == 0)
   {
      rgb[0] = 180;
      rgb[1] = 205;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"LightCyan4",10) == 0)
   {
      rgb[0] = 122;
      rgb[1] = 139;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"PaleTurquoise1",14) == 0)
   {
      rgb[0] = 187;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"PaleTurquoise2",14) == 0)
   {
      rgb[0] = 174;
      rgb[1] = 238;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"PaleTurquoise3",14) == 0)
   {
      rgb[0] = 150;
      rgb[1] = 205;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"PaleTurquoise4",14) == 0)
   {
      rgb[0] = 102;
      rgb[1] = 139;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"CadetBlue1",10) == 0)
   {
      rgb[0] = 152;
      rgb[1] = 245;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"CadetBlue2",10) == 0)
   {
      rgb[0] = 142;
      rgb[1] = 229;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"CadetBlue3",10) == 0)
   {
      rgb[0] = 122;
      rgb[1] = 197;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"CadetBlue4",10) == 0)
   {
      rgb[0] = 83;
      rgb[1] = 134;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"turquoise1",10) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 245;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"turquoise2",10) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 229;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"turquoise3",10) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 197;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"turquoise4",10) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 134;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"cyan1",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"cyan2",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 238;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"cyan3",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 205;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"cyan4",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 139;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"DarkSlateGray1",14) == 0)
   {
      rgb[0] = 151;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"DarkSlateGray2",14) == 0)
   {
      rgb[0] = 141;
      rgb[1] = 238;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"DarkSlateGray3",14) == 0)
   {
      rgb[0] = 121;
      rgb[1] = 205;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"DarkSlateGray4",14) == 0)
   {
      rgb[0] = 82;
      rgb[1] = 139;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"aquamarine1",11) == 0)
   {
      rgb[0] = 127;
      rgb[1] = 255;
      rgb[2] = 212;
   }
   else if (strncasecmp(x11ColourName,"aquamarine2",11) == 0)
   {
      rgb[0] = 118;
      rgb[1] = 238;
      rgb[2] = 198;
   }
   else if (strncasecmp(x11ColourName,"aquamarine3",11) == 0)
   {
      rgb[0] = 102;
      rgb[1] = 205;
      rgb[2] = 170;
   }
   else if (strncasecmp(x11ColourName,"aquamarine4",11) == 0)
   {
      rgb[0] = 69;
      rgb[1] = 139;
      rgb[2] = 116;
   }
   else if (strncasecmp(x11ColourName,"DarkSeaGreen1",13) == 0)
   {
      rgb[0] = 193;
      rgb[1] = 255;
      rgb[2] = 193;
   }
   else if (strncasecmp(x11ColourName,"DarkSeaGreen2",13) == 0)
   {
      rgb[0] = 180;
      rgb[1] = 238;
      rgb[2] = 180;
   }
   else if (strncasecmp(x11ColourName,"DarkSeaGreen3",13) == 0)
   {
      rgb[0] = 155;
      rgb[1] = 205;
      rgb[2] = 155;
   }
   else if (strncasecmp(x11ColourName,"DarkSeaGreen4",13) == 0)
   {
      rgb[0] = 105;
      rgb[1] = 139;
      rgb[2] = 105;
   }
   else if (strncasecmp(x11ColourName,"SeaGreen1",9) == 0)
   {
      rgb[0] = 84;
      rgb[1] = 255;
      rgb[2] = 159;
   }
   else if (strncasecmp(x11ColourName,"SeaGreen2",9) == 0)
   {
      rgb[0] = 78;
      rgb[1] = 238;
      rgb[2] = 148;
   }
   else if (strncasecmp(x11ColourName,"SeaGreen3",9) == 0)
   {
      rgb[0] = 67;
      rgb[1] = 205;
      rgb[2] = 128;
   }
   else if (strncasecmp(x11ColourName,"SeaGreen4",9) == 0)
   {
      rgb[0] = 46;
      rgb[1] = 139;
      rgb[2] = 87;
   }
   else if (strncasecmp(x11ColourName,"PaleGreen1",10) == 0)
   {
      rgb[0] = 154;
      rgb[1] = 255;
      rgb[2] = 154;
   }
   else if (strncasecmp(x11ColourName,"PaleGreen2",10) == 0)
   {
      rgb[0] = 144;
      rgb[1] = 238;
      rgb[2] = 144;
   }
   else if (strncasecmp(x11ColourName,"PaleGreen3",10) == 0)
   {
      rgb[0] = 124;
      rgb[1] = 205;
      rgb[2] = 124;
   }
   else if (strncasecmp(x11ColourName,"PaleGreen4",10) == 0)
   {
      rgb[0] = 84;
      rgb[1] = 139;
      rgb[2] = 84;
   }
   else if (strncasecmp(x11ColourName,"SpringGreen1",12) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 255;
      rgb[2] = 127;
   }
   else if (strncasecmp(x11ColourName,"SpringGreen2",12) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 238;
      rgb[2] = 118;
   }
   else if (strncasecmp(x11ColourName,"SpringGreen3",12) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 205;
      rgb[2] = 102;
   }
   else if (strncasecmp(x11ColourName,"SpringGreen4",12) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 139;
      rgb[2] = 69;
   }
   else if (strncasecmp(x11ColourName,"green1",6) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"green2",6) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 238;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"green3",6) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 205;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"green4",6) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 139;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"chartreuse1",11) == 0)
   {
      rgb[0] = 127;
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"chartreuse2",11) == 0)
   {
      rgb[0] = 118;
      rgb[1] = 238;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"chartreuse3",11) == 0)
   {
      rgb[0] = 102;
      rgb[1] = 205;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"chartreuse4",11) == 0)
   {
      rgb[0] = 69;
      rgb[1] = 139;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"OliveDrab1",10) == 0)
   {
      rgb[0] = 192;
      rgb[1] = 255;
      rgb[2] = 62;
   }
   else if (strncasecmp(x11ColourName,"OliveDrab2",10) == 0)
   {
      rgb[0] = 179;
      rgb[1] = 238;
      rgb[2] = 58;
   }
   else if (strncasecmp(x11ColourName,"OliveDrab3",10) == 0)
   {
      rgb[0] = 154;
      rgb[1] = 205;
      rgb[2] = 50;
   }
   else if (strncasecmp(x11ColourName,"OliveDrab4",10) == 0)
   {
      rgb[0] = 105;
      rgb[1] = 139;
      rgb[2] = 34;
   }
   else if (strncasecmp(x11ColourName,"DarkOliveGreen1",15) == 0)
   {
      rgb[0] = 202;
      rgb[1] = 255;
      rgb[2] = 112;
   }
   else if (strncasecmp(x11ColourName,"DarkOliveGreen2",15) == 0)
   {
      rgb[0] = 188;
      rgb[1] = 238;
      rgb[2] = 104;
   }
   else if (strncasecmp(x11ColourName,"DarkOliveGreen3",15) == 0)
   {
      rgb[0] = 162;
      rgb[1] = 205;
      rgb[2] = 90;
   }
   else if (strncasecmp(x11ColourName,"DarkOliveGreen4",15) == 0)
   {
      rgb[0] = 110;
      rgb[1] = 139;
      rgb[2] = 61;
   }
   else if (strncasecmp(x11ColourName,"khaki1",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 246;
      rgb[2] = 143;
   }
   else if (strncasecmp(x11ColourName,"khaki2",6) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 230;
      rgb[2] = 133;
   }
   else if (strncasecmp(x11ColourName,"khaki3",6) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 198;
      rgb[2] = 115;
   }
   else if (strncasecmp(x11ColourName,"khaki4",6) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 134;
      rgb[2] = 78;
   }
   else if (strncasecmp(x11ColourName,"LightGoldenrod1",15) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 236;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"LightGoldenrod2",15) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 220;
      rgb[2] = 130;
   }
   else if (strncasecmp(x11ColourName,"LightGoldenrod3",15) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 190;
      rgb[2] = 112;
   }
   else if (strncasecmp(x11ColourName,"LightGoldenrod4",15) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 129;
      rgb[2] = 76;
   }
   else if (strncasecmp(x11ColourName,"LightYellow1",12) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 224;
   }
   else if (strncasecmp(x11ColourName,"LightYellow2",12) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 238;
      rgb[2] = 209;
   }
   else if (strncasecmp(x11ColourName,"LightYellow3",12) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 205;
      rgb[2] = 180;
   }
   else if (strncasecmp(x11ColourName,"LightYellow4",12) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 139;
      rgb[2] = 122;
   }
   else if (strncasecmp(x11ColourName,"yellow1",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"yellow2",7) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 238;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"yellow3",7) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 205;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"yellow4",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 139;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"gold1",5) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 215;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"gold2",5) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 201;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"gold3",5) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 173;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"gold4",5) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 117;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"goldenrod1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 193;
      rgb[2] = 37;
   }
   else if (strncasecmp(x11ColourName,"goldenrod2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 180;
      rgb[2] = 34;
   }
   else if (strncasecmp(x11ColourName,"goldenrod3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 155;
      rgb[2] = 29;
   }
   else if (strncasecmp(x11ColourName,"goldenrod4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 105;
      rgb[2] = 20;
   }
   else if (strncasecmp(x11ColourName,"DarkGoldenrod1",14) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 185;
      rgb[2] = 15;
   }
   else if (strncasecmp(x11ColourName,"DarkGoldenrod2",14) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 173;
      rgb[2] = 14;
   }
   else if (strncasecmp(x11ColourName,"DarkGoldenrod3",14) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 149;
      rgb[2] = 12;
   }
   else if (strncasecmp(x11ColourName,"DarkGoldenrod4",14) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 101;
      rgb[2] = 8;
   }
   else if (strncasecmp(x11ColourName,"RosyBrown1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 193;
      rgb[2] = 193;
   }
   else if (strncasecmp(x11ColourName,"RosyBrown2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 180;
      rgb[2] = 180;
   }
   else if (strncasecmp(x11ColourName,"RosyBrown3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 155;
      rgb[2] = 155;
   }
   else if (strncasecmp(x11ColourName,"RosyBrown4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 105;
      rgb[2] = 105;
   }
   else if (strncasecmp(x11ColourName,"IndianRed1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 106;
      rgb[2] = 106;
   }
   else if (strncasecmp(x11ColourName,"IndianRed2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 99;
      rgb[2] = 99;
   }
   else if (strncasecmp(x11ColourName,"IndianRed3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 85;
      rgb[2] = 85;
   }
   else if (strncasecmp(x11ColourName,"IndianRed4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 58;
      rgb[2] = 58;
   }
   else if (strncasecmp(x11ColourName,"sienna1",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 130;
      rgb[2] = 71;
   }
   else if (strncasecmp(x11ColourName,"sienna2",7) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 121;
      rgb[2] = 66;
   }
   else if (strncasecmp(x11ColourName,"sienna3",7) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 104;
      rgb[2] = 57;
   }
   else if (strncasecmp(x11ColourName,"sienna4",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 71;
      rgb[2] = 38;
   }
   else if (strncasecmp(x11ColourName,"burlywood1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 211;
      rgb[2] = 155;
   }
   else if (strncasecmp(x11ColourName,"burlywood2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 197;
      rgb[2] = 145;
   }
   else if (strncasecmp(x11ColourName,"burlywood3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 170;
      rgb[2] = 125;
   }
   else if (strncasecmp(x11ColourName,"burlywood4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 115;
      rgb[2] = 85;
   }
   else if (strncasecmp(x11ColourName,"wheat1",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 231;
      rgb[2] = 186;
   }
   else if (strncasecmp(x11ColourName,"wheat2",6) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 216;
      rgb[2] = 174;
   }
   else if (strncasecmp(x11ColourName,"wheat3",6) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 186;
      rgb[2] = 150;
   }
   else if (strncasecmp(x11ColourName,"wheat4",6) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 126;
      rgb[2] = 102;
   }
   else if (strncasecmp(x11ColourName,"tan1",4) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 165;
      rgb[2] = 79;
   }
   else if (strncasecmp(x11ColourName,"tan2",4) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 154;
      rgb[2] = 73;
   }
   else if (strncasecmp(x11ColourName,"tan3",4) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 133;
      rgb[2] = 63;
   }
   else if (strncasecmp(x11ColourName,"tan4",4) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 90;
      rgb[2] = 43;
   }
   else if (strncasecmp(x11ColourName,"chocolate1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 127;
      rgb[2] = 36;
   }
   else if (strncasecmp(x11ColourName,"chocolate2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 118;
      rgb[2] = 33;
   }
   else if (strncasecmp(x11ColourName,"chocolate3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 102;
      rgb[2] = 29;
   }
   else if (strncasecmp(x11ColourName,"chocolate4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 69;
      rgb[2] = 19;
   }
   else if (strncasecmp(x11ColourName,"firebrick1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 48;
      rgb[2] = 48;
   }
   else if (strncasecmp(x11ColourName,"firebrick2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 44;
      rgb[2] = 44;
   }
   else if (strncasecmp(x11ColourName,"firebrick3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 38;
      rgb[2] = 38;
   }
   else if (strncasecmp(x11ColourName,"firebrick4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 26;
      rgb[2] = 26;
   }
   else if (strncasecmp(x11ColourName,"brown1",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 64;
      rgb[2] = 64;
   }
   else if (strncasecmp(x11ColourName,"brown2",6) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 59;
      rgb[2] = 59;
   }
   else if (strncasecmp(x11ColourName,"brown3",6) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 51;
      rgb[2] = 51;
   }
   else if (strncasecmp(x11ColourName,"brown4",6) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 35;
      rgb[2] = 35;
   }
   else if (strncasecmp(x11ColourName,"salmon1",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 140;
      rgb[2] = 105;
   }
   else if (strncasecmp(x11ColourName,"salmon2",7) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 130;
      rgb[2] = 98;
   }
   else if (strncasecmp(x11ColourName,"salmon3",7) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 112;
      rgb[2] = 84;
   }
   else if (strncasecmp(x11ColourName,"salmon4",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 76;
      rgb[2] = 57;
   }
   else if (strncasecmp(x11ColourName,"LightSalmon1",12) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 160;
      rgb[2] = 122;
   }
   else if (strncasecmp(x11ColourName,"LightSalmon2",12) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 149;
      rgb[2] = 114;
   }
   else if (strncasecmp(x11ColourName,"LightSalmon3",12) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 129;
      rgb[2] = 98;
   }
   else if (strncasecmp(x11ColourName,"LightSalmon4",12) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 87;
      rgb[2] = 66;
   }
   else if (strncasecmp(x11ColourName,"orange1",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 165;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"orange2",7) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 154;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"orange3",7) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 133;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"orange4",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 90;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"DarkOrange1",11) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 127;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"DarkOrange2",11) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 118;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"DarkOrange3",11) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 102;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"DarkOrange4",11) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 69;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"coral1",6) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 114;
      rgb[2] = 86;
   }
   else if (strncasecmp(x11ColourName,"coral2",6) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 106;
      rgb[2] = 80;
   }
   else if (strncasecmp(x11ColourName,"coral3",6) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 91;
      rgb[2] = 69;
   }
   else if (strncasecmp(x11ColourName,"coral4",6) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 62;
      rgb[2] = 47;
   }
   else if (strncasecmp(x11ColourName,"tomato1",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 99;
      rgb[2] = 71;
   }
   else if (strncasecmp(x11ColourName,"tomato2",7) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 92;
      rgb[2] = 66;
   }
   else if (strncasecmp(x11ColourName,"tomato3",7) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 79;
      rgb[2] = 57;
   }
   else if (strncasecmp(x11ColourName,"tomato4",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 54;
      rgb[2] = 38;
   }
   else if (strncasecmp(x11ColourName,"OrangeRed1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 69;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"OrangeRed2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 64;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"OrangeRed3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 55;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"OrangeRed4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 37;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"red1",4) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"red2",4) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"red3",4) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"red4",4) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"DeepPink1",9) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 20;
      rgb[2] = 147;
   }
   else if (strncasecmp(x11ColourName,"DeepPink2",9) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 18;
      rgb[2] = 137;
   }
   else if (strncasecmp(x11ColourName,"DeepPink3",9) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 16;
      rgb[2] = 118;
   }
   else if (strncasecmp(x11ColourName,"DeepPink4",9) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 10;
      rgb[2] = 80;
   }
   else if (strncasecmp(x11ColourName,"HotPink1",8) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 110;
      rgb[2] = 180;
   }
   else if (strncasecmp(x11ColourName,"HotPink2",8) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 106;
      rgb[2] = 167;
   }
   else if (strncasecmp(x11ColourName,"HotPink3",8) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 96;
      rgb[2] = 144;
   }
   else if (strncasecmp(x11ColourName,"HotPink4",8) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 58;
      rgb[2] = 98;
   }
   else if (strncasecmp(x11ColourName,"pink1",5) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 181;
      rgb[2] = 197;
   }
   else if (strncasecmp(x11ColourName,"pink2",5) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 169;
      rgb[2] = 184;
   }
   else if (strncasecmp(x11ColourName,"pink3",5) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 145;
      rgb[2] = 158;
   }
   else if (strncasecmp(x11ColourName,"pink4",5) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 99;
      rgb[2] = 108;
   }
   else if (strncasecmp(x11ColourName,"LightPink1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 174;
      rgb[2] = 185;
   }
   else if (strncasecmp(x11ColourName,"LightPink2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 162;
      rgb[2] = 173;
   }
   else if (strncasecmp(x11ColourName,"LightPink3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 140;
      rgb[2] = 149;
   }
   else if (strncasecmp(x11ColourName,"LightPink4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 95;
      rgb[2] = 101;
   }
   else if (strncasecmp(x11ColourName,"PaleVioletRed1",14) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 130;
      rgb[2] = 171;
   }
   else if (strncasecmp(x11ColourName,"PaleVioletRed2",14) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 121;
      rgb[2] = 159;
   }
   else if (strncasecmp(x11ColourName,"PaleVioletRed3",14) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 104;
      rgb[2] = 137;
   }
   else if (strncasecmp(x11ColourName,"PaleVioletRed4",14) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 71;
      rgb[2] = 93;
   }
   else if (strncasecmp(x11ColourName,"maroon1",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 52;
      rgb[2] = 179;
   }
   else if (strncasecmp(x11ColourName,"maroon2",7) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 48;
      rgb[2] = 167;
   }
   else if (strncasecmp(x11ColourName,"maroon3",7) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 41;
      rgb[2] = 144;
   }
   else if (strncasecmp(x11ColourName,"maroon4",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 28;
      rgb[2] = 98;
   }
   else if (strncasecmp(x11ColourName,"VioletRed1",10) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 62;
      rgb[2] = 150;
   }
   else if (strncasecmp(x11ColourName,"VioletRed2",10) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 58;
      rgb[2] = 140;
   }
   else if (strncasecmp(x11ColourName,"VioletRed3",10) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 50;
      rgb[2] = 120;
   }
   else if (strncasecmp(x11ColourName,"VioletRed4",10) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 34;
      rgb[2] = 82;
   }
   else if (strncasecmp(x11ColourName,"magenta1",8) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 0;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"magenta2",8) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 0;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"magenta3",8) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 0;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"magenta4",8) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 0;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"orchid1",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 131;
      rgb[2] = 250;
   }
   else if (strncasecmp(x11ColourName,"orchid2",7) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 122;
      rgb[2] = 233;
   }
   else if (strncasecmp(x11ColourName,"orchid3",7) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 105;
      rgb[2] = 201;
   }
   else if (strncasecmp(x11ColourName,"orchid4",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 71;
      rgb[2] = 137;
   }
   else if (strncasecmp(x11ColourName,"plum1",5) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 187;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"plum2",5) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 174;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"plum3",5) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 150;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"plum4",5) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 102;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"MediumOrchid1",13) == 0)
   {
      rgb[0] = 224;
      rgb[1] = 102;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"MediumOrchid2",13) == 0)
   {
      rgb[0] = 209;
      rgb[1] = 95;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"MediumOrchid3",13) == 0)
   {
      rgb[0] = 180;
      rgb[1] = 82;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"MediumOrchid4",13) == 0)
   {
      rgb[0] = 122;
      rgb[1] = 55;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"DarkOrchid1",11) == 0)
   {
      rgb[0] = 191;
      rgb[1] = 62;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"DarkOrchid2",11) == 0)
   {
      rgb[0] = 178;
      rgb[1] = 58;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"DarkOrchid3",11) == 0)
   {
      rgb[0] = 154;
      rgb[1] = 50;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"DarkOrchid4",11) == 0)
   {
      rgb[0] = 104;
      rgb[1] = 34;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"purple1",7) == 0)
   {
      rgb[0] = 155;
      rgb[1] = 48;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"purple2",7) == 0)
   {
      rgb[0] = 145;
      rgb[1] = 44;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"purple3",7) == 0)
   {
      rgb[0] = 125;
      rgb[1] = 38;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"purple4",7) == 0)
   {
      rgb[0] = 85;
      rgb[1] = 26;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"MediumPurple1",13) == 0)
   {
      rgb[0] = 171;
      rgb[1] = 130;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"MediumPurple2",13) == 0)
   {
      rgb[0] = 159;
      rgb[1] = 121;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"MediumPurple3",13) == 0)
   {
      rgb[0] = 137;
      rgb[1] = 104;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"MediumPurple4",13) == 0)
   {
      rgb[0] = 93;
      rgb[1] = 71;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"thistle1",8) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 225;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"thistle2",8) == 0)
   {
      rgb[0] = 238;
      rgb[1] = 210;
      rgb[2] = 238;
   }
   else if (strncasecmp(x11ColourName,"thistle3",8) == 0)
   {
      rgb[0] = 205;
      rgb[1] = 181;
      rgb[2] = 205;
   }
   else if (strncasecmp(x11ColourName,"thistle4",8) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 123;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"gray0",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"grey0",5) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"gray1",5) == 0)
   {
      rgb[0] = 3;
      rgb[1] = 3;
      rgb[2] = 3;
   }
   else if (strncasecmp(x11ColourName,"grey1",5) == 0)
   {
      rgb[0] = 3;
      rgb[1] = 3;
      rgb[2] = 3;
   }
   else if (strncasecmp(x11ColourName,"gray2",5) == 0)
   {
      rgb[0] = 5;
      rgb[1] = 5;
      rgb[2] = 5;
   }
   else if (strncasecmp(x11ColourName,"grey2",5) == 0)
   {
      rgb[0] = 5;
      rgb[1] = 5;
      rgb[2] = 5;
   }
   else if (strncasecmp(x11ColourName,"gray3",5) == 0)
   {
      rgb[0] = 8;
      rgb[1] = 8;
      rgb[2] = 8;
   }
   else if (strncasecmp(x11ColourName,"grey3",5) == 0)
   {
      rgb[0] = 8;
      rgb[1] = 8;
      rgb[2] = 8;
   }
   else if (strncasecmp(x11ColourName,"gray4",5) == 0)
   {
      rgb[0] = 10;
      rgb[1] = 10;
      rgb[2] = 10;
   }
   else if (strncasecmp(x11ColourName,"grey4",5) == 0)
   {
      rgb[0] = 10;
      rgb[1] = 10;
      rgb[2] = 10;
   }
   else if (strncasecmp(x11ColourName,"gray5",5) == 0)
   {
      rgb[0] = 13;
      rgb[1] = 13;
      rgb[2] = 13;
   }
   else if (strncasecmp(x11ColourName,"grey5",5) == 0)
   {
      rgb[0] = 13;
      rgb[1] = 13;
      rgb[2] = 13;
   }
   else if (strncasecmp(x11ColourName,"gray6",5) == 0)
   {
      rgb[0] = 15;
      rgb[1] = 15;
      rgb[2] = 15;
   }
   else if (strncasecmp(x11ColourName,"grey6",5) == 0)
   {
      rgb[0] = 15;
      rgb[1] = 15;
      rgb[2] = 15;
   }
   else if (strncasecmp(x11ColourName,"gray7",5) == 0)
   {
      rgb[0] = 18;
      rgb[1] = 18;
      rgb[2] = 18;
   }
   else if (strncasecmp(x11ColourName,"grey7",5) == 0)
   {
      rgb[0] = 18;
      rgb[1] = 18;
      rgb[2] = 18;
   }
   else if (strncasecmp(x11ColourName,"gray8",5) == 0)
   {
      rgb[0] = 20;
      rgb[1] = 20;
      rgb[2] = 20;
   }
   else if (strncasecmp(x11ColourName,"grey8",5) == 0)
   {
      rgb[0] = 20;
      rgb[1] = 20;
      rgb[2] = 20;
   }
   else if (strncasecmp(x11ColourName,"gray9",5) == 0)
   {
      rgb[0] = 23;
      rgb[1] = 23;
      rgb[2] = 23;
   }
   else if (strncasecmp(x11ColourName,"grey9",5) == 0)
   {
      rgb[0] = 23;
      rgb[1] = 23;
      rgb[2] = 23;
   }
   else if (strncasecmp(x11ColourName,"gray10",6) == 0)
   {
      rgb[0] = 26;
      rgb[1] = 26;
      rgb[2] = 26;
   }
   else if (strncasecmp(x11ColourName,"grey10",6) == 0)
   {
      rgb[0] = 26;
      rgb[1] = 26;
      rgb[2] = 26;
   }
   else if (strncasecmp(x11ColourName,"gray11",6) == 0)
   {
      rgb[0] = 28;
      rgb[1] = 28;
      rgb[2] = 28;
   }
   else if (strncasecmp(x11ColourName,"grey11",6) == 0)
   {
      rgb[0] = 28;
      rgb[1] = 28;
      rgb[2] = 28;
   }
   else if (strncasecmp(x11ColourName,"gray12",6) == 0)
   {
      rgb[0] = 31;
      rgb[1] = 31;
      rgb[2] = 31;
   }
   else if (strncasecmp(x11ColourName,"grey12",6) == 0)
   {
      rgb[0] = 31;
      rgb[1] = 31;
      rgb[2] = 31;
   }
   else if (strncasecmp(x11ColourName,"gray13",6) == 0)
   {
      rgb[0] = 33;
      rgb[1] = 33;
      rgb[2] = 33;
   }
   else if (strncasecmp(x11ColourName,"grey13",6) == 0)
   {
      rgb[0] = 33;
      rgb[1] = 33;
      rgb[2] = 33;
   }
   else if (strncasecmp(x11ColourName,"gray14",6) == 0)
   {
      rgb[0] = 36;
      rgb[1] = 36;
      rgb[2] = 36;
   }
   else if (strncasecmp(x11ColourName,"grey14",6) == 0)
   {
      rgb[0] = 36;
      rgb[1] = 36;
      rgb[2] = 36;
   }
   else if (strncasecmp(x11ColourName,"gray15",6) == 0)
   {
      rgb[0] = 38;
      rgb[1] = 38;
      rgb[2] = 38;
   }
   else if (strncasecmp(x11ColourName,"grey15",6) == 0)
   {
      rgb[0] = 38;
      rgb[1] = 38;
      rgb[2] = 38;
   }
   else if (strncasecmp(x11ColourName,"gray16",6) == 0)
   {
      rgb[0] = 41;
      rgb[1] = 41;
      rgb[2] = 41;
   }
   else if (strncasecmp(x11ColourName,"grey16",6) == 0)
   {
      rgb[0] = 41;
      rgb[1] = 41;
      rgb[2] = 41;
   }
   else if (strncasecmp(x11ColourName,"gray17",6) == 0)
   {
      rgb[0] = 43;
      rgb[1] = 43;
      rgb[2] = 43;
   }
   else if (strncasecmp(x11ColourName,"grey17",6) == 0)
   {
      rgb[0] = 43;
      rgb[1] = 43;
      rgb[2] = 43;
   }
   else if (strncasecmp(x11ColourName,"gray18",6) == 0)
   {
      rgb[0] = 46;
      rgb[1] = 46;
      rgb[2] = 46;
   }
   else if (strncasecmp(x11ColourName,"grey18",6) == 0)
   {
      rgb[0] = 46;
      rgb[1] = 46;
      rgb[2] = 46;
   }
   else if (strncasecmp(x11ColourName,"gray19",6) == 0)
   {
      rgb[0] = 48;
      rgb[1] = 48;
      rgb[2] = 48;
   }
   else if (strncasecmp(x11ColourName,"grey19",6) == 0)
   {
      rgb[0] = 48;
      rgb[1] = 48;
      rgb[2] = 48;
   }
   else if (strncasecmp(x11ColourName,"gray20",6) == 0)
   {
      rgb[0] = 51;
      rgb[1] = 51;
      rgb[2] = 51;
   }
   else if (strncasecmp(x11ColourName,"grey20",6) == 0)
   {
      rgb[0] = 51;
      rgb[1] = 51;
      rgb[2] = 51;
   }
   else if (strncasecmp(x11ColourName,"gray21",6) == 0)
   {
      rgb[0] = 54;
      rgb[1] = 54;
      rgb[2] = 54;
   }
   else if (strncasecmp(x11ColourName,"grey21",6) == 0)
   {
      rgb[0] = 54;
      rgb[1] = 54;
      rgb[2] = 54;
   }
   else if (strncasecmp(x11ColourName,"gray22",6) == 0)
   {
      rgb[0] = 56;
      rgb[1] = 56;
      rgb[2] = 56;
   }
   else if (strncasecmp(x11ColourName,"grey22",6) == 0)
   {
      rgb[0] = 56;
      rgb[1] = 56;
      rgb[2] = 56;
   }
   else if (strncasecmp(x11ColourName,"gray23",6) == 0)
   {
      rgb[0] = 59;
      rgb[1] = 59;
      rgb[2] = 59;
   }
   else if (strncasecmp(x11ColourName,"grey23",6) == 0)
   {
      rgb[0] = 59;
      rgb[1] = 59;
      rgb[2] = 59;
   }
   else if (strncasecmp(x11ColourName,"gray24",6) == 0)
   {
      rgb[0] = 61;
      rgb[1] = 61;
      rgb[2] = 61;
   }
   else if (strncasecmp(x11ColourName,"grey24",6) == 0)
   {
      rgb[0] = 61;
      rgb[1] = 61;
      rgb[2] = 61;
   }
   else if (strncasecmp(x11ColourName,"gray25",6) == 0)
   {
      rgb[0] = 64;
      rgb[1] = 64;
      rgb[2] = 64;
   }
   else if (strncasecmp(x11ColourName,"grey25",6) == 0)
   {
      rgb[0] = 64;
      rgb[1] = 64;
      rgb[2] = 64;
   }
   else if (strncasecmp(x11ColourName,"gray26",6) == 0)
   {
      rgb[0] = 66;
      rgb[1] = 66;
      rgb[2] = 66;
   }
   else if (strncasecmp(x11ColourName,"grey26",6) == 0)
   {
      rgb[0] = 66;
      rgb[1] = 66;
      rgb[2] = 66;
   }
   else if (strncasecmp(x11ColourName,"gray27",6) == 0)
   {
      rgb[0] = 69;
      rgb[1] = 69;
      rgb[2] = 69;
   }
   else if (strncasecmp(x11ColourName,"grey27",6) == 0)
   {
      rgb[0] = 69;
      rgb[1] = 69;
      rgb[2] = 69;
   }
   else if (strncasecmp(x11ColourName,"gray28",6) == 0)
   {
      rgb[0] = 71;
      rgb[1] = 71;
      rgb[2] = 71;
   }
   else if (strncasecmp(x11ColourName,"grey28",6) == 0)
   {
      rgb[0] = 71;
      rgb[1] = 71;
      rgb[2] = 71;
   }
   else if (strncasecmp(x11ColourName,"gray29",6) == 0)
   {
      rgb[0] = 74;
      rgb[1] = 74;
      rgb[2] = 74;
   }
   else if (strncasecmp(x11ColourName,"grey29",6) == 0)
   {
      rgb[0] = 74;
      rgb[1] = 74;
      rgb[2] = 74;
   }
   else if (strncasecmp(x11ColourName,"gray30",6) == 0)
   {
      rgb[0] = 77;
      rgb[1] = 77;
      rgb[2] = 77;
   }
   else if (strncasecmp(x11ColourName,"grey30",6) == 0)
   {
      rgb[0] = 77;
      rgb[1] = 77;
      rgb[2] = 77;
   }
   else if (strncasecmp(x11ColourName,"gray31",6) == 0)
   {
      rgb[0] = 79;
      rgb[1] = 79;
      rgb[2] = 79;
   }
   else if (strncasecmp(x11ColourName,"grey31",6) == 0)
   {
      rgb[0] = 79;
      rgb[1] = 79;
      rgb[2] = 79;
   }
   else if (strncasecmp(x11ColourName,"gray32",6) == 0)
   {
      rgb[0] = 82;
      rgb[1] = 82;
      rgb[2] = 82;
   }
   else if (strncasecmp(x11ColourName,"grey32",6) == 0)
   {
      rgb[0] = 82;
      rgb[1] = 82;
      rgb[2] = 82;
   }
   else if (strncasecmp(x11ColourName,"gray33",6) == 0)
   {
      rgb[0] = 84;
      rgb[1] = 84;
      rgb[2] = 84;
   }
   else if (strncasecmp(x11ColourName,"grey33",6) == 0)
   {
      rgb[0] = 84;
      rgb[1] = 84;
      rgb[2] = 84;
   }
   else if (strncasecmp(x11ColourName,"gray34",6) == 0)
   {
      rgb[0] = 87;
      rgb[1] = 87;
      rgb[2] = 87;
   }
   else if (strncasecmp(x11ColourName,"grey34",6) == 0)
   {
      rgb[0] = 87;
      rgb[1] = 87;
      rgb[2] = 87;
   }
   else if (strncasecmp(x11ColourName,"gray35",6) == 0)
   {
      rgb[0] = 89;
      rgb[1] = 89;
      rgb[2] = 89;
   }
   else if (strncasecmp(x11ColourName,"grey35",6) == 0)
   {
      rgb[0] = 89;
      rgb[1] = 89;
      rgb[2] = 89;
   }
   else if (strncasecmp(x11ColourName,"gray36",6) == 0)
   {
      rgb[0] = 92;
      rgb[1] = 92;
      rgb[2] = 92;
   }
   else if (strncasecmp(x11ColourName,"grey36",6) == 0)
   {
      rgb[0] = 92;
      rgb[1] = 92;
      rgb[2] = 92;
   }
   else if (strncasecmp(x11ColourName,"gray37",6) == 0)
   {
      rgb[0] = 94;
      rgb[1] = 94;
      rgb[2] = 94;
   }
   else if (strncasecmp(x11ColourName,"grey37",6) == 0)
   {
      rgb[0] = 94;
      rgb[1] = 94;
      rgb[2] = 94;
   }
   else if (strncasecmp(x11ColourName,"gray38",6) == 0)
   {
      rgb[0] = 97;
      rgb[1] = 97;
      rgb[2] = 97;
   }
   else if (strncasecmp(x11ColourName,"grey38",6) == 0)
   {
      rgb[0] = 97;
      rgb[1] = 97;
      rgb[2] = 97;
   }
   else if (strncasecmp(x11ColourName,"gray39",6) == 0)
   {
      rgb[0] = 99;
      rgb[1] = 99;
      rgb[2] = 99;
   }
   else if (strncasecmp(x11ColourName,"grey39",6) == 0)
   {
      rgb[0] = 99;
      rgb[1] = 99;
      rgb[2] = 99;
   }
   else if (strncasecmp(x11ColourName,"gray40",6) == 0)
   {
      rgb[0] = 102;
      rgb[1] = 102;
      rgb[2] = 102;
   }
   else if (strncasecmp(x11ColourName,"grey40",6) == 0)
   {
      rgb[0] = 102;
      rgb[1] = 102;
      rgb[2] = 102;
   }
   else if (strncasecmp(x11ColourName,"gray41",6) == 0)
   {
      rgb[0] = 105;
      rgb[1] = 105;
      rgb[2] = 105;
   }
   else if (strncasecmp(x11ColourName,"grey41",6) == 0)
   {
      rgb[0] = 105;
      rgb[1] = 105;
      rgb[2] = 105;
   }
   else if (strncasecmp(x11ColourName,"gray42",6) == 0)
   {
      rgb[0] = 107;
      rgb[1] = 107;
      rgb[2] = 107;
   }
   else if (strncasecmp(x11ColourName,"grey42",6) == 0)
   {
      rgb[0] = 107;
      rgb[1] = 107;
      rgb[2] = 107;
   }
   else if (strncasecmp(x11ColourName,"gray43",6) == 0)
   {
      rgb[0] = 110;
      rgb[1] = 110;
      rgb[2] = 110;
   }
   else if (strncasecmp(x11ColourName,"grey43",6) == 0)
   {
      rgb[0] = 110;
      rgb[1] = 110;
      rgb[2] = 110;
   }
   else if (strncasecmp(x11ColourName,"gray44",6) == 0)
   {
      rgb[0] = 112;
      rgb[1] = 112;
      rgb[2] = 112;
   }
   else if (strncasecmp(x11ColourName,"grey44",6) == 0)
   {
      rgb[0] = 112;
      rgb[1] = 112;
      rgb[2] = 112;
   }
   else if (strncasecmp(x11ColourName,"gray45",6) == 0)
   {
      rgb[0] = 115;
      rgb[1] = 115;
      rgb[2] = 115;
   }
   else if (strncasecmp(x11ColourName,"grey45",6) == 0)
   {
      rgb[0] = 115;
      rgb[1] = 115;
      rgb[2] = 115;
   }
   else if (strncasecmp(x11ColourName,"gray46",6) == 0)
   {
      rgb[0] = 117;
      rgb[1] = 117;
      rgb[2] = 117;
   }
   else if (strncasecmp(x11ColourName,"grey46",6) == 0)
   {
      rgb[0] = 117;
      rgb[1] = 117;
      rgb[2] = 117;
   }
   else if (strncasecmp(x11ColourName,"gray47",6) == 0)
   {
      rgb[0] = 120;
      rgb[1] = 120;
      rgb[2] = 120;
   }
   else if (strncasecmp(x11ColourName,"grey47",6) == 0)
   {
      rgb[0] = 120;
      rgb[1] = 120;
      rgb[2] = 120;
   }
   else if (strncasecmp(x11ColourName,"gray48",6) == 0)
   {
      rgb[0] = 122;
      rgb[1] = 122;
      rgb[2] = 122;
   }
   else if (strncasecmp(x11ColourName,"grey48",6) == 0)
   {
      rgb[0] = 122;
      rgb[1] = 122;
      rgb[2] = 122;
   }
   else if (strncasecmp(x11ColourName,"gray49",6) == 0)
   {
      rgb[0] = 125;
      rgb[1] = 125;
      rgb[2] = 125;
   }
   else if (strncasecmp(x11ColourName,"grey49",6) == 0)
   {
      rgb[0] = 125;
      rgb[1] = 125;
      rgb[2] = 125;
   }
   else if (strncasecmp(x11ColourName,"gray50",6) == 0)
   {
      rgb[0] = 127;
      rgb[1] = 127;
      rgb[2] = 127;
   }
   else if (strncasecmp(x11ColourName,"grey50",6) == 0)
   {
      rgb[0] = 127;
      rgb[1] = 127;
      rgb[2] = 127;
   }
   else if (strncasecmp(x11ColourName,"gray51",6) == 0)
   {
      rgb[0] = 130;
      rgb[1] = 130;
      rgb[2] = 130;
   }
   else if (strncasecmp(x11ColourName,"grey51",6) == 0)
   {
      rgb[0] = 130;
      rgb[1] = 130;
      rgb[2] = 130;
   }
   else if (strncasecmp(x11ColourName,"gray52",6) == 0)
   {
      rgb[0] = 133;
      rgb[1] = 133;
      rgb[2] = 133;
   }
   else if (strncasecmp(x11ColourName,"grey52",6) == 0)
   {
      rgb[0] = 133;
      rgb[1] = 133;
      rgb[2] = 133;
   }
   else if (strncasecmp(x11ColourName,"gray53",6) == 0)
   {
      rgb[0] = 135;
      rgb[1] = 135;
      rgb[2] = 135;
   }
   else if (strncasecmp(x11ColourName,"grey53",6) == 0)
   {
      rgb[0] = 135;
      rgb[1] = 135;
      rgb[2] = 135;
   }
   else if (strncasecmp(x11ColourName,"gray54",6) == 0)
   {
      rgb[0] = 138;
      rgb[1] = 138;
      rgb[2] = 138;
   }
   else if (strncasecmp(x11ColourName,"grey54",6) == 0)
   {
      rgb[0] = 138;
      rgb[1] = 138;
      rgb[2] = 138;
   }
   else if (strncasecmp(x11ColourName,"gray55",6) == 0)
   {
      rgb[0] = 140;
      rgb[1] = 140;
      rgb[2] = 140;
   }
   else if (strncasecmp(x11ColourName,"grey55",6) == 0)
   {
      rgb[0] = 140;
      rgb[1] = 140;
      rgb[2] = 140;
   }
   else if (strncasecmp(x11ColourName,"gray56",6) == 0)
   {
      rgb[0] = 143;
      rgb[1] = 143;
      rgb[2] = 143;
   }
   else if (strncasecmp(x11ColourName,"grey56",6) == 0)
   {
      rgb[0] = 143;
      rgb[1] = 143;
      rgb[2] = 143;
   }
   else if (strncasecmp(x11ColourName,"gray57",6) == 0)
   {
      rgb[0] = 145;
      rgb[1] = 145;
      rgb[2] = 145;
   }
   else if (strncasecmp(x11ColourName,"grey57",6) == 0)
   {
      rgb[0] = 145;
      rgb[1] = 145;
      rgb[2] = 145;
   }
   else if (strncasecmp(x11ColourName,"gray58",6) == 0)
   {
      rgb[0] = 148;
      rgb[1] = 148;
      rgb[2] = 148;
   }
   else if (strncasecmp(x11ColourName,"grey58",6) == 0)
   {
      rgb[0] = 148;
      rgb[1] = 148;
      rgb[2] = 148;
   }
   else if (strncasecmp(x11ColourName,"gray59",6) == 0)
   {
      rgb[0] = 150;
      rgb[1] = 150;
      rgb[2] = 150;
   }
   else if (strncasecmp(x11ColourName,"grey59",6) == 0)
   {
      rgb[0] = 150;
      rgb[1] = 150;
      rgb[2] = 150;
   }
   else if (strncasecmp(x11ColourName,"gray60",6) == 0)
   {
      rgb[0] = 153;
      rgb[1] = 153;
      rgb[2] = 153;
   }
   else if (strncasecmp(x11ColourName,"grey60",6) == 0)
   {
      rgb[0] = 153;
      rgb[1] = 153;
      rgb[2] = 153;
   }
   else if (strncasecmp(x11ColourName,"gray61",6) == 0)
   {
      rgb[0] = 156;
      rgb[1] = 156;
      rgb[2] = 156;
   }
   else if (strncasecmp(x11ColourName,"grey61",6) == 0)
   {
      rgb[0] = 156;
      rgb[1] = 156;
      rgb[2] = 156;
   }
   else if (strncasecmp(x11ColourName,"gray62",6) == 0)
   {
      rgb[0] = 158;
      rgb[1] = 158;
      rgb[2] = 158;
   }
   else if (strncasecmp(x11ColourName,"grey62",6) == 0)
   {
      rgb[0] = 158;
      rgb[1] = 158;
      rgb[2] = 158;
   }
   else if (strncasecmp(x11ColourName,"gray63",6) == 0)
   {
      rgb[0] = 161;
      rgb[1] = 161;
      rgb[2] = 161;
   }
   else if (strncasecmp(x11ColourName,"grey63",6) == 0)
   {
      rgb[0] = 161;
      rgb[1] = 161;
      rgb[2] = 161;
   }
   else if (strncasecmp(x11ColourName,"gray64",6) == 0)
   {
      rgb[0] = 163;
      rgb[1] = 163;
      rgb[2] = 163;
   }
   else if (strncasecmp(x11ColourName,"grey64",6) == 0)
   {
      rgb[0] = 163;
      rgb[1] = 163;
      rgb[2] = 163;
   }
   else if (strncasecmp(x11ColourName,"gray65",6) == 0)
   {
      rgb[0] = 166;
      rgb[1] = 166;
      rgb[2] = 166;
   }
   else if (strncasecmp(x11ColourName,"grey65",6) == 0)
   {
      rgb[0] = 166;
      rgb[1] = 166;
      rgb[2] = 166;
   }
   else if (strncasecmp(x11ColourName,"gray66",6) == 0)
   {
      rgb[0] = 168;
      rgb[1] = 168;
      rgb[2] = 168;
   }
   else if (strncasecmp(x11ColourName,"grey66",6) == 0)
   {
      rgb[0] = 168;
      rgb[1] = 168;
      rgb[2] = 168;
   }
   else if (strncasecmp(x11ColourName,"gray67",6) == 0)
   {
      rgb[0] = 171;
      rgb[1] = 171;
      rgb[2] = 171;
   }
   else if (strncasecmp(x11ColourName,"grey67",6) == 0)
   {
      rgb[0] = 171;
      rgb[1] = 171;
      rgb[2] = 171;
   }
   else if (strncasecmp(x11ColourName,"gray68",6) == 0)
   {
      rgb[0] = 173;
      rgb[1] = 173;
      rgb[2] = 173;
   }
   else if (strncasecmp(x11ColourName,"grey68",6) == 0)
   {
      rgb[0] = 173;
      rgb[1] = 173;
      rgb[2] = 173;
   }
   else if (strncasecmp(x11ColourName,"gray69",6) == 0)
   {
      rgb[0] = 176;
      rgb[1] = 176;
      rgb[2] = 176;
   }
   else if (strncasecmp(x11ColourName,"grey69",6) == 0)
   {
      rgb[0] = 176;
      rgb[1] = 176;
      rgb[2] = 176;
   }
   else if (strncasecmp(x11ColourName,"gray70",6) == 0)
   {
      rgb[0] = 179;
      rgb[1] = 179;
      rgb[2] = 179;
   }
   else if (strncasecmp(x11ColourName,"grey70",6) == 0)
   {
      rgb[0] = 179;
      rgb[1] = 179;
      rgb[2] = 179;
   }
   else if (strncasecmp(x11ColourName,"gray71",6) == 0)
   {
      rgb[0] = 181;
      rgb[1] = 181;
      rgb[2] = 181;
   }
   else if (strncasecmp(x11ColourName,"grey71",6) == 0)
   {
      rgb[0] = 181;
      rgb[1] = 181;
      rgb[2] = 181;
   }
   else if (strncasecmp(x11ColourName,"gray72",6) == 0)
   {
      rgb[0] = 184;
      rgb[1] = 184;
      rgb[2] = 184;
   }
   else if (strncasecmp(x11ColourName,"grey72",6) == 0)
   {
      rgb[0] = 184;
      rgb[1] = 184;
      rgb[2] = 184;
   }
   else if (strncasecmp(x11ColourName,"gray73",6) == 0)
   {
      rgb[0] = 186;
      rgb[1] = 186;
      rgb[2] = 186;
   }
   else if (strncasecmp(x11ColourName,"grey73",6) == 0)
   {
      rgb[0] = 186;
      rgb[1] = 186;
      rgb[2] = 186;
   }
   else if (strncasecmp(x11ColourName,"gray74",6) == 0)
   {
      rgb[0] = 189;
      rgb[1] = 189;
      rgb[2] = 189;
   }
   else if (strncasecmp(x11ColourName,"grey74",6) == 0)
   {
      rgb[0] = 189;
      rgb[1] = 189;
      rgb[2] = 189;
   }
   else if (strncasecmp(x11ColourName,"gray75",6) == 0)
   {
      rgb[0] = 191;
      rgb[1] = 191;
      rgb[2] = 191;
   }
   else if (strncasecmp(x11ColourName,"grey75",6) == 0)
   {
      rgb[0] = 191;
      rgb[1] = 191;
      rgb[2] = 191;
   }
   else if (strncasecmp(x11ColourName,"gray76",6) == 0)
   {
      rgb[0] = 194;
      rgb[1] = 194;
      rgb[2] = 194;
   }
   else if (strncasecmp(x11ColourName,"grey76",6) == 0)
   {
      rgb[0] = 194;
      rgb[1] = 194;
      rgb[2] = 194;
   }
   else if (strncasecmp(x11ColourName,"gray77",6) == 0)
   {
      rgb[0] = 196;
      rgb[1] = 196;
      rgb[2] = 196;
   }
   else if (strncasecmp(x11ColourName,"grey77",6) == 0)
   {
      rgb[0] = 196;
      rgb[1] = 196;
      rgb[2] = 196;
   }
   else if (strncasecmp(x11ColourName,"gray78",6) == 0)
   {
      rgb[0] = 199;
      rgb[1] = 199;
      rgb[2] = 199;
   }
   else if (strncasecmp(x11ColourName,"grey78",6) == 0)
   {
      rgb[0] = 199;
      rgb[1] = 199;
      rgb[2] = 199;
   }
   else if (strncasecmp(x11ColourName,"gray79",6) == 0)
   {
      rgb[0] = 201;
      rgb[1] = 201;
      rgb[2] = 201;
   }
   else if (strncasecmp(x11ColourName,"grey79",6) == 0)
   {
      rgb[0] = 201;
      rgb[1] = 201;
      rgb[2] = 201;
   }
   else if (strncasecmp(x11ColourName,"gray80",6) == 0)
   {
      rgb[0] = 204;
      rgb[1] = 204;
      rgb[2] = 204;
   }
   else if (strncasecmp(x11ColourName,"grey80",6) == 0)
   {
      rgb[0] = 204;
      rgb[1] = 204;
      rgb[2] = 204;
   }
   else if (strncasecmp(x11ColourName,"gray81",6) == 0)
   {
      rgb[0] = 207;
      rgb[1] = 207;
      rgb[2] = 207;
   }
   else if (strncasecmp(x11ColourName,"grey81",6) == 0)
   {
      rgb[0] = 207;
      rgb[1] = 207;
      rgb[2] = 207;
   }
   else if (strncasecmp(x11ColourName,"gray82",6) == 0)
   {
      rgb[0] = 209;
      rgb[1] = 209;
      rgb[2] = 209;
   }
   else if (strncasecmp(x11ColourName,"grey82",6) == 0)
   {
      rgb[0] = 209;
      rgb[1] = 209;
      rgb[2] = 209;
   }
   else if (strncasecmp(x11ColourName,"gray83",6) == 0)
   {
      rgb[0] = 212;
      rgb[1] = 212;
      rgb[2] = 212;
   }
   else if (strncasecmp(x11ColourName,"grey83",6) == 0)
   {
      rgb[0] = 212;
      rgb[1] = 212;
      rgb[2] = 212;
   }
   else if (strncasecmp(x11ColourName,"gray84",6) == 0)
   {
      rgb[0] = 214;
      rgb[1] = 214;
      rgb[2] = 214;
   }
   else if (strncasecmp(x11ColourName,"grey84",6) == 0)
   {
      rgb[0] = 214;
      rgb[1] = 214;
      rgb[2] = 214;
   }
   else if (strncasecmp(x11ColourName,"gray85",6) == 0)
   {
      rgb[0] = 217;
      rgb[1] = 217;
      rgb[2] = 217;
   }
   else if (strncasecmp(x11ColourName,"grey85",6) == 0)
   {
      rgb[0] = 217;
      rgb[1] = 217;
      rgb[2] = 217;
   }
   else if (strncasecmp(x11ColourName,"gray86",6) == 0)
   {
      rgb[0] = 219;
      rgb[1] = 219;
      rgb[2] = 219;
   }
   else if (strncasecmp(x11ColourName,"grey86",6) == 0)
   {
      rgb[0] = 219;
      rgb[1] = 219;
      rgb[2] = 219;
   }
   else if (strncasecmp(x11ColourName,"gray87",6) == 0)
   {
      rgb[0] = 222;
      rgb[1] = 222;
      rgb[2] = 222;
   }
   else if (strncasecmp(x11ColourName,"grey87",6) == 0)
   {
      rgb[0] = 222;
      rgb[1] = 222;
      rgb[2] = 222;
   }
   else if (strncasecmp(x11ColourName,"gray88",6) == 0)
   {
      rgb[0] = 224;
      rgb[1] = 224;
      rgb[2] = 224;
   }
   else if (strncasecmp(x11ColourName,"grey88",6) == 0)
   {
      rgb[0] = 224;
      rgb[1] = 224;
      rgb[2] = 224;
   }
   else if (strncasecmp(x11ColourName,"gray89",6) == 0)
   {
      rgb[0] = 227;
      rgb[1] = 227;
      rgb[2] = 227;
   }
   else if (strncasecmp(x11ColourName,"grey89",6) == 0)
   {
      rgb[0] = 227;
      rgb[1] = 227;
      rgb[2] = 227;
   }
   else if (strncasecmp(x11ColourName,"gray90",6) == 0)
   {
      rgb[0] = 229;
      rgb[1] = 229;
      rgb[2] = 229;
   }
   else if (strncasecmp(x11ColourName,"grey90",6) == 0)
   {
      rgb[0] = 229;
      rgb[1] = 229;
      rgb[2] = 229;
   }
   else if (strncasecmp(x11ColourName,"gray91",6) == 0)
   {
      rgb[0] = 232;
      rgb[1] = 232;
      rgb[2] = 232;
   }
   else if (strncasecmp(x11ColourName,"grey91",6) == 0)
   {
      rgb[0] = 232;
      rgb[1] = 232;
      rgb[2] = 232;
   }
   else if (strncasecmp(x11ColourName,"gray92",6) == 0)
   {
      rgb[0] = 235;
      rgb[1] = 235;
      rgb[2] = 235;
   }
   else if (strncasecmp(x11ColourName,"grey92",6) == 0)
   {
      rgb[0] = 235;
      rgb[1] = 235;
      rgb[2] = 235;
   }
   else if (strncasecmp(x11ColourName,"gray93",6) == 0)
   {
      rgb[0] = 237;
      rgb[1] = 237;
      rgb[2] = 237;
   }
   else if (strncasecmp(x11ColourName,"grey93",6) == 0)
   {
      rgb[0] = 237;
      rgb[1] = 237;
      rgb[2] = 237;
   }
   else if (strncasecmp(x11ColourName,"gray94",6) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 240;
      rgb[2] = 240;
   }
   else if (strncasecmp(x11ColourName,"grey94",6) == 0)
   {
      rgb[0] = 240;
      rgb[1] = 240;
      rgb[2] = 240;
   }
   else if (strncasecmp(x11ColourName,"gray95",6) == 0)
   {
      rgb[0] = 242;
      rgb[1] = 242;
      rgb[2] = 242;
   }
   else if (strncasecmp(x11ColourName,"grey95",6) == 0)
   {
      rgb[0] = 242;
      rgb[1] = 242;
      rgb[2] = 242;
   }
   else if (strncasecmp(x11ColourName,"gray96",6) == 0)
   {
      rgb[0] = 245;
      rgb[1] = 245;
      rgb[2] = 245;
   }
   else if (strncasecmp(x11ColourName,"grey96",6) == 0)
   {
      rgb[0] = 245;
      rgb[1] = 245;
      rgb[2] = 245;
   }
   else if (strncasecmp(x11ColourName,"gray97",6) == 0)
   {
      rgb[0] = 247;
      rgb[1] = 247;
      rgb[2] = 247;
   }
   else if (strncasecmp(x11ColourName,"grey97",6) == 0)
   {
      rgb[0] = 247;
      rgb[1] = 247;
      rgb[2] = 247;
   }
   else if (strncasecmp(x11ColourName,"gray98",6) == 0)
   {
      rgb[0] = 250;
      rgb[1] = 250;
      rgb[2] = 250;
   }
   else if (strncasecmp(x11ColourName,"grey98",6) == 0)
   {
      rgb[0] = 250;
      rgb[1] = 250;
      rgb[2] = 250;
   }
   else if (strncasecmp(x11ColourName,"gray99",6) == 0)
   {
      rgb[0] = 252;
      rgb[1] = 252;
      rgb[2] = 252;
   }
   else if (strncasecmp(x11ColourName,"grey99",6) == 0)
   {
      rgb[0] = 252;
      rgb[1] = 252;
      rgb[2] = 252;
   }
   else if (strncasecmp(x11ColourName,"gray100",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"grey100",7) == 0)
   {
      rgb[0] = 255;
      rgb[1] = 255;
      rgb[2] = 255;
   }
   else if (strncasecmp(x11ColourName,"DarkGrey",8) == 0)
   {
      rgb[0] = 169;
      rgb[1] = 169;
      rgb[2] = 169;
   }
   else if (strncasecmp(x11ColourName,"DarkGray",8) == 0)
   {
      rgb[0] = 169;
      rgb[1] = 169;
      rgb[2] = 169;
   }
   else if (strncasecmp(x11ColourName,"DarkBlue",8) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"DarkCyan",8) == 0)
   {
      rgb[0] = 0;
      rgb[1] = 139;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"DarkMagenta",11) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 0;
      rgb[2] = 139;
   }
   else if (strncasecmp(x11ColourName,"DarkRed",7) == 0)
   {
      rgb[0] = 139;
      rgb[1] = 0;
      rgb[2] = 0;
   }
   else if (strncasecmp(x11ColourName,"LightGreen",10) == 0)
   {
      rgb[0] = 144;
      rgb[1] = 238;
      rgb[2] = 144;
   }
   else  		/* Default is Black */
   {
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0;
   }

   /* Convert from int to float */
   self->red   = (float) rgb[0]/255.0;
   self->green = (float) rgb[1]/255.0;
   self->blue  = (float) rgb[2]/255.0;
   self->opacity = 1.0;
}

void lucColour_FromHexName( lucColour* self, Name hexName )
{
   char* red;
   char* green;
   char* blue;
   int i;
   int rgb[3];

   /* Check to make sure colour is valid */
   if (hexName[0] != '#')
   {
      lucColour_FromX11ColourName( self, "black" );
      printf( "Cannot recognise hex colour %s.\n", hexName);
      return;
   }
   for (i = 1 ; i <= 6 ; i++)
   {
      if (isxdigit(hexName[i]) == 0)
      {
         lucColour_FromX11ColourName( self, "black" );
         printf( "Cannot recognise hex colour %s.\n", hexName);
         return;
      }
   }

   /* Seperate colours */
   red = StG_Strdup(&hexName[1]);
   red[2] = '\0';
   green = StG_Strdup(&hexName[3]);
   green[2] = '\0';
   blue = StG_Strdup(&hexName[5]);
   blue[2] = '\0';

   /* Read colours */
   sscanf(red,   "%x", &rgb[0]);
   sscanf(green, "%x", &rgb[1]);
   sscanf(blue,  "%x", &rgb[2]);

   Memory_Free( blue );
   Memory_Free( green );
   Memory_Free( red );

   self->red   = (float) rgb[0]/255.0;
   self->green = (float) rgb[1]/255.0;
   self->blue  = (float) rgb[2]/255.0;
   self->opacity = 1.0;
}


