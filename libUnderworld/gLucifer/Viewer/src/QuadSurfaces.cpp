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

QuadSurfaces::QuadSurfaces(bool flat2Dflag) : TriSurfaces()
{
   type = lucGridType;
   triangles = true;
   flat2d = flat2Dflag;
}

QuadSurfaces::~QuadSurfaces()
{
}

void QuadSurfaces::update()
{
   clock_t t1,t2,tt;
   t1 = tt = clock();
   // Update and depth sort surfaces..
   //Calculate distances from view plane
   float maxdist, mindist; 
   view->getMinMaxDistance(&mindist, &maxdist);

   Geometry::update();

   tt=clock();
   if (geom.size() == 0) return;

   //Get element/quad count
   debug_print("Reloading and sorting %d quad surfaces...\n", geom.size());
   total = 0;
   hiddencache.resize(geom.size());
   surf_sort.clear();
   int quadverts = 0;
   for (unsigned int i=0; i<geom.size(); i++) 
   {
      int quads = (geom[i]->width-1) * (geom[i]->height-1);
      quadverts += quads * 4;
      total += geom[i]->count; //Actual vertices

      hiddencache[i] = !drawable(i); //Save flags
      debug_print("Surface %d, quads %d hidden? %s\n", i, quadverts/4, (hiddencache[i] ? "yes" : "no"));

      //Get corners of strip
      float* posmin = geom[i]->vertices[0];
      float* posmax = geom[i]->vertices[geom[i]->count - 1];
      float pos[3] = {posmin[0] + (posmax[0] - posmin[0]) * 0.5f,
                      posmin[1] + (posmax[1] - posmin[1]) * 0.5f,
                      posmin[2] + (posmax[2] - posmin[2]) * 0.5f};

      //Calculate distance from viewing plane
      geom[i]->distance = eyeDistance(view->modelView, pos); 
      if (geom[i]->distance < mindist) mindist = geom[i]->distance;
      if (geom[i]->distance > maxdist) maxdist = geom[i]->distance;
      //printf("%d)  %f %f %f distance = %f\n", i, pos[0], pos[1], pos[2], geom[i]->distance);
      surf_sort.push_back(Distance(i, geom[i]->distance));

      //Disable triangle sorting for these surfaces
      geom[i]->opaque = true;
   }
   if (total == 0) return;
   t2 = clock(); debug_print("  %.4lf seconds to calculate distances\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();

   //Sort
   std::sort(surf_sort.begin(), surf_sort.end());
   t2 = clock(); debug_print("  %.4lf seconds to sort\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();

   //Only reload the vbo data when required
   //Not needed when objects hidden/shown but required if colours changed
   //To force, use Geometry->reset() which sets elements to -1 
   if (elements < 0 || elements != quadverts)
   {
      //Clear buffers
      close();
      elements = quadverts;
      //Load & optimise the mesh data
      render();
      //Send the data to the GPU via VBO
      loadBuffers();
   }
}

void QuadSurfaces::render()
{
   //Update quad index buffer
   clock_t t1,t2;

   //Prepare the Index buffer
   if (!indexvbo)
   {
      assert(elements);
      glGenBuffers(1, &indexvbo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexvbo);
      GL_Error_Check;
      if (glIsBuffer(indexvbo))
      {
         //glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * tricount * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
         //debug_print("  %d byte IBO created for %d indices\n", 3 * tricount * sizeof(GLuint), tricount * 3);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
         debug_print("  %d byte IBO created for %d indices\n", elements * sizeof(GLuint), elements);
      }
      else 
         debug_print("  IBO creation failed\n");
      GL_Error_Check;
   }

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexvbo);
   if (!glIsBuffer(indexvbo)) return;

   elements = 0;
   int offset = 0;
   int voffset = 0;
   for (unsigned int index = 0; index < geom.size(); index++) 
   {
      t1=clock();

      std::vector<Vec3d> normals(geom[index]->count);
      std::vector<GLuint> indices;

      //Quad indices
      int quads = (geom[index]->width-1) * (geom[index]->height-1);
      indices.resize(quads*4);
      debug_print("%d x %d grid, quads %d, offset %d\n", geom[index]->width, geom[index]->height, quads, elements);
      calcGridNormals(index, normals);
      calcGridIndices(index, indices, voffset);
      //Vertex index offset
      voffset += geom[index]->count;
      //Index offset
      elements += quads*4;
      //Read new data and continue
      //geom[index]->indices.clear();
      geom[index]->normals.clear();
      //geom[index]->indices.read(indices.size(), &indices[0]);
      geom[index]->normals.read(normals.size(), normals[0].ref());

      t1 = clock();
      int bytes = indices.size()*sizeof(GLuint);
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, bytes, &indices[0]);
      t2 = clock(); debug_print("  %.4lf seconds to upload %d quad indices (%d - %d)\n", (t2-t1)/(double)CLOCKS_PER_SEC, indices.size(), offset, bytes); t1 = clock();
      offset += bytes;
      GL_Error_Check;
   }
}

void QuadSurfaces::calcGridIndices(int i, std::vector<GLuint> &indices, unsigned int vertoffset)
{
   //Normals: calculate from surface geometry
   clock_t t1,t2;
   t1=clock();
   debug_print("Calculating indices for grid quad surface %d... ", i);

   // Calc pre-vertex normals for irregular meshes by averaging four surrounding triangle facet normals
   int o = 0;
   for (int j = 0 ; j < geom[i]->height-1; j++ )
   {
      for (int k = 0 ; k < geom[i]->width-1; k++ )
      {
         //Add indices for two triangles per grid element
         int offset0 = j * geom[i]->width + k;
         int offset1 = (j+1) * geom[i]->width + k;
         int offset2 = j * geom[i]->width + k + 1;
         int offset3 = (j+1) * geom[i]->width + k + 1;

         assert(offset2 + vertoffset < total);
         assert(o <= indices.size()-4);

         //Quads...
         indices[o++] = offset0 + vertoffset;
         indices[o++] = offset1 + vertoffset;
         indices[o++] = offset3 + vertoffset;
         indices[o++] = offset2 + vertoffset;
      }
   }
   t2 = clock(); debug_print("  %.4lf seconds\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();
}

void QuadSurfaces::draw()
{
   GL_Error_Check;
   if (view->sort) redraw = true; //Recalc cross section order

   //Draw, calls update when required
   Geometry::draw();
   if (drawcount == 0) return;

   GL_Error_Check;
   // Draw using vertex buffer object
   clock_t t0 = clock();
   clock_t t1 = clock();
   double time;
   int stride = 8 * sizeof(float) + sizeof(Colour);   //3+3+2 vertices, normals, texCoord + 32-bit colour
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexvbo);
   if (geom.size() > 0 && elements > 0 && glIsBuffer(vbo) && glIsBuffer(indexvbo))
   {
      glVertexPointer(3, GL_FLOAT, stride, (GLvoid*)0); // Load vertex x,y,z only
      glNormalPointer(GL_FLOAT, stride, (GLvoid*)(3*sizeof(float))); // Load normal x,y,z, offset 3 float
      glTexCoordPointer(2, GL_FLOAT, stride, (GLvoid*)(6*sizeof(float))); // Load texcoord x,y
      glColorPointer(4, GL_UNSIGNED_BYTE, stride, (GLvoid*)(8*sizeof(float)));   // Load rgba, offset 6 float
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);

      //Render in reverse sorted order
      for (int i=geom.size()-1; i>=0; i--)
      {
         int id = surf_sort[i].id;
         //if (!drawable(id)) continue;
         if (hiddencache[id]) continue;

         //Get the offset
         unsigned int start = 0;
         for (int g=0; g<geom.size(); g++)
         {
            if (g == id) break;
            start += 4 * (geom[g]->width-1) * (geom[g]->height-1); //geom[g]->indices.size();
         }

         //int id = i; //Sorting disabled
         setState(id, prog); //Set draw state settings for this object
         //fprintf(stderr, "(%d) DRAWING QUADS: %d (%d to %d) elements: %d\n", i, geom[i]->indices.size()/4, start/4, (start+geom[i]->indices.size())/4, elements);
         glDrawRangeElements(GL_QUADS, 0, elements, 4 * (geom[id]->width-1) * (geom[id]->height-1), GL_UNSIGNED_INT, (GLvoid*)(start*sizeof(GLuint)));
         //printf("%d) rendered, distance = %f (%f)\n", id, geom[id]->distance, surf_sort[i].distance);
      }
      //fprintf(stderr, "DRAWING ALL QUADS: %d\n", elements);
      //glDrawElements(GL_QUADS, elements, GL_UNSIGNED_INT, (GLvoid*)(0));

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
   }
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   GL_Error_Check;

   //Restore state
   //glEnable(GL_LIGHTING);
   //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   //glDisable(GL_CULL_FACE);
   glBindTexture(GL_TEXTURE_2D, 0);

   time = ((clock()-t0)/(double)CLOCKS_PER_SEC);
   if (time > 0.05)
     debug_print("  %.4lf seconds to draw quads\n", time);
   GL_Error_Check;
}

void QuadSurfaces::jsonWrite(unsigned int id, std::ostream* osp)
{
   std::ostream& os = *osp;
   bool first = true;
   
   for (unsigned int i = 0; i < geom.size(); i++) 
   {
      if (geom[i]->draw->id == id && drawable(i))
      {
         bool vertNormals = false;
         //if (geom[i]->normals.size() == 0) calcNormals(i);
         if (geom[i]->normals.size() == geom[i]->vertices.size()) vertNormals = true;

         std::cerr << "Collected " << geom[i]->count << " vertices/values (" << i << ")" << std::endl;
         //Only supports dump of vertex, normal and colourValue at present
         if (!first) os << "," << std::endl;
         first = false;
         os << "        {" << std::endl;
         os << "          \"width\" : " << geom[i]->width << "," << std::endl;
         os << "          \"height\" : " << geom[i]->height << "," << std::endl;
         std::string vertices = base64_encode(reinterpret_cast<const unsigned char*>(&geom[i]->vertices.value[0]), geom[i]->vertices.size() * sizeof(float));
         os << "          \"vertices\" : \n          {" << std::endl;
         os << "            \"size\" : 3," << std::endl;
         os << "            \"data\" : \"" << vertices << "\"" << std::endl;
         os << "          }," << std::endl;
         std::string normals = base64_encode(reinterpret_cast<const unsigned char*>(&geom[i]->normals.value[0]), geom[i]->normals.size() * sizeof(float));
         os << "          \"normals\" : \n          {" << std::endl;
         os << "            \"size\" : 3," << std::endl;
         os << "            \"data\" : \"" << normals << "\"" << std::endl;
         
         if (geom[i]->colourValue.size() == geom[i]->count)
         {
            std::string values = base64_encode(reinterpret_cast<const unsigned char*>(&geom[i]->colourValue.value[0]), geom[i]->colourValue.size() * sizeof(float));
            geom[i]->colourCalibrate(); //Calibrate on data range
            os << "          }," << std::endl;
            os << "          \"values\" : \n          {" << std::endl;
            os << "            \"size\" : 1," << std::endl;
            os << "            \"minimum\" : " << geom[i]->colourValue.minimum << "," << std::endl;
            os << "            \"maximum\" : " << geom[i]->colourValue.maximum << "," << std::endl;
            os << "            \"data\" : \"" << values << "\"" << std::endl;
         }
         os << "          }" << std::endl;
         os << "        }";
      }
   }
}
