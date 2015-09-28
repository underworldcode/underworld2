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

Shapes::Shapes() : Geometry()
{
   type = lucShapeType;
   //Create sub-renderers
   tris = new TriSurfaces();
}

Shapes::~Shapes()
{
   delete tris;
}

void Shapes::close() 
{
   tris->close();
}

void Shapes::update()
{
   //Convert shapes to triangles
   tris->clear();
   tris->setView(view);
   for (unsigned int i=0; i<geom.size(); i++) 
   {
      //Create a new data store for output geometry
      tris->add(geom[i]->draw);

      float scaling = geom[i]->draw->properties["scaling"].ToFloat(1.0);

      //Load constant scaling factors from properties
      float dims[3];
      dims[0] = geom[i]->draw->properties["width"].ToFloat(FLT_MIN);
      dims[1] = geom[i]->draw->properties["height"].ToFloat(FLT_MIN);
      dims[2] = geom[i]->draw->properties["length"].ToFloat(FLT_MIN);
      int shape = geom[i]->draw->properties["shape"].ToInt(0);
      int quality = glyphSegments(geom[i]->draw->properties["glyphs"].ToInt(6));
      //Points drawn as shapes?
      if (!geom[i]->draw->properties.HasKey("shape"))
      {
         dims[0] = dims[1] = dims[2] = geom[i]->draw->properties["pointsize"].ToFloat(1.0) / 8.0;
         quality = glyphSegments(geom[i]->draw->properties["glyphs"].ToInt(4));
      }

      if (scaling <= 0) scaling = 1.0;

      Colour colour;
      geom[i]->colourCalibrate();

      for (int v=0; v < geom[i]->count; v++) 
      {
         //Scale the dimensions by variables (dynamic range options? by setting max/min?)
         float sdims[3] = {dims[0], dims[1], dims[2]};
         if (geom[i]->xWidths.size() > 0) sdims[0] = geom[i]->xWidths[v];
         if (geom[i]->yHeights.size() > 0) sdims[1] = geom[i]->yHeights[v]; else sdims[1] = sdims[0];
         if (geom[i]->zLengths.size() > 0) sdims[2] = geom[i]->zLengths[v]; else sdims[2] = sdims[1];

         //Multiply by constant scaling factors if present
         for (int c=0; c<3; c++)
         {
            if (dims[c] != FLT_MIN) sdims[c] *= dims[c];
            //Apply scaling, also inverse of model scaling to avoid distorting glyphs
            sdims[c] *= scaling * scale / view->scale[c];
         }

         //Setup orientation using alignment vector
         Quaternion qrot;
         if (geom[i]->vectors.size() > 0)
         {
            Vec3d vec(geom[i]->vectors[v]);
            //vec *= Vec3d(view->scale); //Scale

            // Rotate to orient the shape
            //...Want to align our z-axis to point along arrow vector:
            // axis of rotation = (z x vec)
            // cosine of angle between vector and z-axis = (z . vec) / |z|.|vec| *
            vec.normalise();
            float rangle = RAD2DEG * vec.angle(Vec3d(0.0, 0.0, 1.0));
            //Axis of rotation = vec x [0,0,1] = -vec[1],vec[0],0
            Vec3d rvec = Vec3d(-vec.y, vec.x, 0);
            qrot.fromAxisAngle(rvec, rangle);
         }

         //Create shape
         Vec3d posv = Vec3d(geom[i]->vertices[v]);
         Vec3d radii = Vec3d(sdims);
         if (shape == 1)
            tris->drawCuboid(geom[i]->draw, geom[i]->vertices[v], sdims[0], sdims[1], sdims[2], qrot);
         else
            tris->drawEllipsoid(geom[i]->draw, posv, radii, qrot, quality);
         //Per shape colours (can do this as long as sub-renderer always outputs same tri count)
         geom[i]->getColour(colour, v);
         tris->read(geom[i]->draw, 1, lucRGBAData, &colour.value);
      }
      //printf("%d Shapes: %d Vertices: %d Indices: %d\n", i, geom[i]->positions.size()/3, geom[i]->count, geom[i]->indices.size());

      //Adjust bounding box
      tris->compareMinMax(geom[i]->min, geom[i]->max);
   }

   tris->update();
}

void Shapes::draw()
{
   GL_Error_Check;
   Geometry::draw();
   if (drawcount == 0) return;

   GL_Error_Check;
   // Undo any scaling factor for arrow drawing...
   glPushMatrix();
   if (view->scale[0] != 1.0 || view->scale[1] != 1.0 || view->scale[2] != 1.0)
      glScalef(1.0/view->scale[0], 1.0/view->scale[1], 1.0/view->scale[2]);

   tris->draw();

   // Re-Apply scaling factors
   glPopMatrix();
   GL_Error_Check;
}
