/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
** Copyright (c) 2010, Monash University
** All rights reserved.
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
**       * Redistributions of source code must retain the above copyright notice,
**          this list of conditions and the following disclaimer.
**       * Redistributions in binary form must reproduce the above copyright
**         notice, this list of conditions and the following disclaimer in the
**         documentation and/or other materials provided with the distribution.
**       * Neither the name of the Monash University nor the names of its contributors
**         may be used to endorse or promote products derived from this software
**         without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
** BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
** OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**
** Contact:
*%  Owen Kaluza - Owen.Kaluza(at)monash.edu
*%
*% Development Team :
*%  http://www.underworldproject.org/aboutus.html
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef ColourMap__
#define ColourMap__

#include "GraphicsUtil.h"

#define PALETTE_TEX_SIZE 4096

class ColourVal
{
 public:
   Colour colour;
   float value;
   float position;

   ColourVal() : value(0), position(0) {colour.value = 0xff000000;}
   ColourVal(Colour& colour, float pvalue) : colour(colour), value(pvalue), position(0) {}
   ColourVal(Colour& colour) : colour(colour), value(HUGE_VAL), position(0) {}
};

//ColourMap class
class ColourMap
{
   static const int SAMPLE_COUNT = 512;
   Colour precalc[SAMPLE_COUNT];

public:
   std::vector<ColourVal> colours;
   Colour background;
   unsigned int id;
   std::string name;
   float minimum;
   float maximum;
   float range;
   bool log, discrete;
   float centre;
   bool calibrated;
   bool noValues; //Use position data only
   TextureData* texture;

   float dimCoeff;
   std::string units;

   ColourMap(unsigned int id=0, const char* name="", bool log=false, bool discrete=false, float centre=HUGE_VAL, float min=0, float max=1)
       : id(id), minimum(min), maximum(max), log(log), discrete(discrete), centre(centre), calibrated(false), noValues(false), dimCoeff(1.0), units("")
   {
      if (id == 0)
         this->id = ++ColourMap::lastid;
      else if (id > ColourMap::lastid)
         ColourMap::lastid = id;
      this->name = std::string(name);
      texture = NULL;
      background.value = 0xff000000;
   }

   ~ColourMap() {if (texture) delete texture;}

   void add(unsigned int colour);
   void add(unsigned int* colours, int count);
   void add(unsigned int colour, float pvalue);
   void add(float *components, float pvalue);
   void calc();
   void calibrate(float min, float max);
   void calibrate(FloatValues& dataValues);
   void calibrate();
   Colour getfast(float value);
   Colour get(float value);
   float scaleValue(float value);
   Colour getFromScaled(float scaledValue);
   void draw(json::Object& propertiess, int startx, int starty, int length, int height, Colour& printColour);
   void setComponent(int component_index);
   void loadTexture(bool repeat=false);
   void loadPalette(std::string data);
   void print();

   static bool lock;
   static int logscales;
   static unsigned int lastid;
};

#endif //ColourMap__
