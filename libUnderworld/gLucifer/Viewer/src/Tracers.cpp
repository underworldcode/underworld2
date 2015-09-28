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

#include "Geometry.h"
#include "TimeStep.h"

Tracers::Tracers() : Geometry()
{
   type = lucTracerType;
   scaling = true;
   steps = 0;
   timestep = 0;
   //Create sub-renderers
   lines = new Lines(true); //Only used for 2d lines
   tris = new TriSurfaces();
}

Tracers::~Tracers()
{
   delete lines;
   delete tris;
}

void Tracers::close() 
{
   lines->close();
   tris->close();
}

void Tracers::update()
{
   //Convert tracers to triangles
   clock_t t1,t2,tt;
   tt=clock();
   //All tracers stored as single vertex/value block
   //Contains vertex/value for every tracer particle at each timestep
   //Number of particles is number of entries divided by number of timesteps
   lines->clear();
   lines->setView(view);
   tris->clear();
   tris->setView(view);
   for (unsigned int i=0; i<geom.size(); i++) 
   {
      //Create a new data stores for output geometry
      tris->add(geom[i]->draw);
      lines->add(geom[i]->draw);

      //Calculate particle count using data count / data steps
      unsigned int particles = geom[i]->width;
      int count = geom[i]->count;
      int datasteps = particles > 0 ? count / particles : count;
      int timesteps = (datasteps-1) * TimeStep::gap + 1; //Multiply by gap between recorded steps

      //Swarm limit
      int drawSteps = geom[i]->draw->properties["steps"].ToInt(0);
      if (drawSteps > 0 && timesteps > drawSteps)
         timesteps = drawSteps;
      //Global limit
      if (steps > 0 && steps < timesteps)
         timesteps = steps;

      //Get start and end indices
      int max = timesteps;
      //Skipped steps? Use closest available step
      if (TimeStep::gap > 1) max = ceil(timesteps/(float)(TimeStep::gap-1));
      int end = datasteps-1;
      int start = end - max + 1;
      if (start < 0) start = 0;
      debug_print("Tracing %d positions from step indices %d to %d (timesteps %d datasteps %d max %d)\n", particles, start, end, timesteps, datasteps, max);

      //Calibrate colourMap
      geom[i]->colourCalibrate();
      //Calibrate colour maps on timestep if no value data
      bool timecolour = false;
                                              //Force until supplied colour values supported
      if (geom[i]->draw->colourMaps[lucColourValueData])// && geom[i]->colourValue.size() == 0)
         timecolour = true;

      //Get properties
      int quality = glyphSegments(geom[i]->draw->properties["glyphs"].ToInt(2));
      float size0 = geom[i]->draw->properties["scaling"].ToFloat(1.0) * 0.001;
      float limit = geom[i]->draw->properties["limit"].ToFloat(view->model_size * 0.3);
      float factor = geom[i]->draw->properties["scaling"].ToFloat(1.0) * scale * TimeStep::gap * 0.0005;
      float arrowSize = geom[i]->draw->properties["arrowhead"].ToFloat(2.0);
      //Iterate individual tracers
      for (unsigned int p=0; p < particles; p++) 
      {
         float size = size0; 
         float* oldpos = NULL;
         float time, oldtime = TimeStep::timesteps[start]->time;
         float radius, oldRadius = 0;
         for (int step=start; step <= end; step++) 
         {
            // Scale up line towards head of trajectory
            if (scaling && step > start)
            {
               //float factor = geom[i]->draw->properties["scaling"].ToFloat(1.0) * scale * TimeStep::gap * 0.0005;
               if (p==0) debug_print("Scaling tracers from %f by %f to %f\n", size, factor, size+factor);
               size += factor;
            }

            //Lookup by provided particle index?
            int pidx = p;
            if (geom[i]->indices.size() > 0)
            {
               floatidx fidx;
               for (unsigned int x=0; x<particles; x++)
               {
                  fidx.val = geom[i]->indices[step * particles + x];
                  if (fidx.idx == p)
                  {
                     pidx = x;
                     break;
                  }
               }
            }

            float* pos = geom[i]->vertices[step * particles + pidx];
            //printf("p %d step %d POS = %f,%f,%f\n", p, step, pos[0], pos[1], pos[2]);

            //Get colour value either from previous colour values or time step
            //if (timecolour)
            time = TimeStep::timesteps[step]->time;
            //geom[i]->getColour(colour, TimeStep::gap * step * particles + pidx);

            radius = scale * size;

            // Draw section
            if (step > start)
            {
               if (geom[i]->draw->properties["flat"].ToBool(false) || quality < 1)
               {
                  lines->read(geom[i]->draw, 1, lucVertexData, oldpos);
                  lines->read(geom[i]->draw, 1, lucVertexData, pos);
                  lines->read(geom[i]->draw, 1, lucColourValueData, &oldtime);
                  lines->read(geom[i]->draw, 1, lucColourValueData, &time);
               }
               else
               {
                  //Coord scaling passed to drawTrajectory (as global scaling disabled to avoid distorting glyphs)
                  float arrowHead = -1;
                  if (step == end) arrowHead = arrowSize; //geom[i]->draw->properties["arrowhead"].ToFloat(2.0);
                  int diff = tris->getCount(geom[i]->draw);
                  tris->drawTrajectory(geom[i]->draw, oldpos, pos, oldRadius, radius, arrowHead, view->scale, limit, quality);
                  diff = tris->getCount(geom[i]->draw) - diff;
                  //Per vertex colours
                  for (int c=0; c<diff; c++) 
                  {
                     float t = oldtime;
                     //Top of shaft and arrowhead use current colour value, others (base) use previous
                     //(Every second vertex is at top of shaft, first quality*2 are shaft verts)
                     if (c%2==1 || c > quality*2) t = time;
                     tris->read(geom[i]->draw, 1, lucColourValueData, &t);
                  }
               }
            }

            oldtime = time;
            oldpos = pos;
            oldRadius = radius;
         }
      }
      
      if (timecolour)
      {
         float mintime = TimeStep::timesteps[start]->time;
         float maxtime = TimeStep::timesteps[end]->time;
         //Setup colour range on lines/tris data
         lines->setup(geom[i]->draw, lucColourValueData, mintime, maxtime);
         tris->setup(geom[i]->draw, lucColourValueData, mintime, maxtime);
         timecolour = true;
      }

      //Adjust bounding box
      tris->compareMinMax(geom[i]->min, geom[i]->max);
      lines->compareMinMax(geom[i]->min, geom[i]->max);
   }
   GL_Error_Check;
   
   tris->update();
   lines->update();
}

void Tracers::draw()
{
   Geometry::draw();
   if (drawcount == 0) return;

   // Undo any scaling factor for tracer drawing...
   glPushMatrix();
   if (view->scale[0] != 1.0 || view->scale[1] != 1.0 || view->scale[2] != 1.0)
      glScalef(1.0/view->scale[0], 1.0/view->scale[1], 1.0/view->scale[2]);

   tris->draw();

   // Re-Apply scaling factors
   glPopMatrix();

   lines->draw();
}
