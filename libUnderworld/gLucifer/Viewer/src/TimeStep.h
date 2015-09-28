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
//TimeStep - timestep data class
#ifndef TimeStep__
#define TimeStep__

#include "Geometry.h"

class TimeStep
{
  public:
   static std::vector<TimeStep*> timesteps; //Active model timesteps
   static int gap;
   int step;
   float time;
   //Scaling for timesteps
   float dimCoeff;
   std::string units;

   TimeStep(int step, float time, float dimCoeff, std::string units) : step(step), time(time), dimCoeff(dimCoeff), units(units) {}
   TimeStep(int step, float time) : step(step), time(time), dimCoeff(1.0), units("") {}
   TimeStep() : step(0), time(0), dimCoeff(1.0), units("") {}

   ~TimeStep()
   {
      //Free cached geometry
      for (unsigned int i=0; i < cache.size(); i++)
         delete cache[i];
   }

   //Cached data
   static int cachesize;
   std::vector<Geometry*> cache;

   void write(std::vector<Geometry*> &data)
   {
      //for (unsigned int i=0; i < data.size(); i++)
      //   //Release any graphics memory
      //   data[i]->close();

      cache = data;
   }

   void read(std::vector<Geometry*> &data)
   {
      //Preserve global properties when switching
      for (unsigned int i=0; i < data.size(); i++)
      {
         cache[i]->scale = data[i]->scale;
         //Release any graphics memory
         data[i]->close();
      }

      data = cache;
   }
};

#endif
