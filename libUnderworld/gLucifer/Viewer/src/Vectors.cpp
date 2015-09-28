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

Vectors::Vectors() : Geometry()
{
   type = lucVectorType;
   //Create sub-renderers
   lines = new Lines(true); //Only used for 2d lines
   tris = new TriSurfaces();
}

Vectors::~Vectors()
{
   delete lines;
   delete tris;
}

void Vectors::close() 
{
   lines->close();
   tris->close();
}

void Vectors::update()
{
   //Convert vectors to triangles
   clock_t t1,t2,tt;
   tt=clock();
   lines->clear();
   lines->setView(view);
   tris->clear();
   tris->setView(view);
   int tot = 0;
   for (unsigned int i=0; i<geom.size(); i++) 
   {
      //Create new data stores for output geometry
      tris->add(geom[i]->draw);
      lines->add(geom[i]->draw);

      tot += geom[i]->count;

      float arrowHead = geom[i]->draw->properties["arrowhead"].ToFloat(2.0);

      //Dynamic range?
      float scaling = scale * geom[i]->draw->properties["scaling"].ToFloat(1.0);

      if (geom[i]->vectors.maximum > 0) 
      {
         debug_print("[Adjusted vector scaling from %.2e by %.2e to %.2e ]\n", 
               scaling, 1/geom[i]->vectors.maximum, scaling/geom[i]->vectors.maximum);
         scaling *= 1.0/geom[i]->vectors.maximum;
      }

      //Load scaling factors from properties
      int quality = glyphSegments(geom[i]->draw->properties["glyphs"].ToInt(2));
      scaling *= geom[i]->draw->properties["length"].ToFloat(1.0);
      //debug_print("Scaling %f arrowhead %f quality %d %d\n", scaling, arrowHead, glyphs);

      //Default (0) = automatically calculated radius
      float radius = scale * geom[i]->draw->properties["radius"].ToFloat(0.0);

      if (scaling <= 0) scaling = 1.0;

      float minL = view->model_size * 0.01; //Minimum length for visibility

      Colour colour;
      geom[i]->colourCalibrate();

      for (int v=0; v < geom[i]->count; v++) 
      {
         //Scale position & vector manually (as global scaling is disabled to avoid distorting glyphs)
         Vec3d pos(geom[i]->vertices[v]);
         Vec3d vec(geom[i]->vectors[v]);
         if (view->scale[0] != 1.0 || view->scale[1] != 1.0 || view->scale[2] != 1.0)
         {
            Vec3d scale(view->scale);
            pos *= scale;
            vec *= scale;
         }

         geom[i]->getColour(colour, v);

         if (vec.magnitude() * scaling >= minL)
         {
            tris->drawVector(geom[i]->draw, pos.ref(), vec.ref(), scaling, radius, radius, arrowHead, quality);
            //Per arrow colours (can do this as long as sub-renderer always outputs same tri count)
            tris->read(geom[i]->draw, 1, lucRGBAData, &colour.value);
         }
         //Always draw the lines so when zoomed out shaft visible (prevents visible boundary between 2d/3d renders)
         lines->drawVector(geom[i]->draw, pos.ref(), vec.ref(), scaling, radius, radius, arrowHead, 0);
         //Per arrow colours (can do this as long as sub-renderer always outputs same tri count)
         lines->read(geom[i]->draw, 1, lucRGBAData, &colour.value);

      }

      //Adjust bounding box
      tris->compareMinMax(geom[i]->min, geom[i]->max);
      lines->compareMinMax(geom[i]->min, geom[i]->max);
   }
   t1 = clock(); debug_print("Plotted %d vector arrows in %.4lf seconds\n", tot, (t1-tt)/(double)CLOCKS_PER_SEC);

   tris->update();
   lines->update();
}

void Vectors::draw()
{
   Geometry::draw();
   if (drawcount == 0) return;

   // Undo any scaling factor for arrow drawing...
   glPushMatrix();
   if (view->scale[0] != 1.0 || view->scale[1] != 1.0 || view->scale[2] != 1.0)
      glScalef(1.0/view->scale[0], 1.0/view->scale[1], 1.0/view->scale[2]);

   tris->draw();

   // Re-Apply scaling factors
   glPopMatrix();

   lines->draw();
}
