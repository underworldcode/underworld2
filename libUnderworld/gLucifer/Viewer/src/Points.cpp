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

#include "GraphicsUtil.h"
#include "Geometry.h"

unsigned int Points::subSample = 1;
Shader* Points::prog = NULL;
int Points::pointType = 0;
GLuint Points::indexvbo = 0;
GLuint Points::vbo = 0;

Points::Points() : Geometry()
{
   type = lucPointType;
   scale = 1.0f;
   attenuate = true;
   pidx = NULL;
}

Points::~Points()
{
   close();
}

void Points::close()
{
   if (vbo)
      glDeleteBuffers(1, &vbo);
   if (indexvbo)
      glDeleteBuffers(1, &indexvbo);
   if (pidx)
      delete[] pidx;

   vbo = 0;
   indexvbo = 0;
   pidx = NULL;

   Geometry::close();
}

void Points::init()
{
   Geometry::init();
}

void Points::update()
{
   Geometry::update();

   // Update particles..
   clock_t t1,t2;
   static unsigned int last_total = 0;

   //if (geom.size() == 0) return;
   if (total == 0) 
      return; 

   //Ensure vbo recreated if total changed
   if (elements < 0 || total != last_total)
   //if (true)
   {
      loadVertices();
   }

   hiddencache.resize(geom.size());
   for (unsigned int i = 0; i < geom.size(); i++) 
      hiddencache[i] = !drawable(i); //Save flags

   last_total = total;

   //Initial depth sort & render
   render();
}

void Points::loadVertices()
{
   debug_print("Reloading %d particles...\n", total);
   // Update points...
   clock_t t1,t2;

   //Reset data structures
   close();

   //Create sorting array
   pidx = new PIndex[total];
   if (pidx == NULL) abort_program("Memory allocation error (failed to allocate %d bytes)", sizeof(PIndex) * total);
   if (geom.size() == 0) return;

   //Calculates normals, deletes duplicate verts, adds triangles to sorting array

   // VBO - copy normals/colours/positions to buffer object for quick display 
   int datasize = sizeof(float) * 5 + sizeof(Colour);   //Vertex(3), two flags and 32-bit colour
   if (!vbo && prog && prog->program)  //Only use vbo if shaders available
   { 
      //glDeleteBuffers(1, &vbo);
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      //Initialise point buffer
      if (glIsBuffer(vbo))
      {
         glBindBuffer(GL_ARRAY_BUFFER, vbo);
         glBufferData(GL_ARRAY_BUFFER, total * datasize, NULL, GL_STREAM_DRAW);
         debug_print("  %d byte VBO created, for %d vertices\n", (int)(total * datasize), total);
      }
      else 
         debug_print("  VBO creation failed!\n");
   }
   else
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
   GL_Error_Check;

   unsigned char *p = NULL, *ptr = NULL;
   if (glIsBuffer(vbo))
   {
      ptr = p = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
      GL_Error_Check;
      if (!p) abort_program("glMapBuffer failed");
   }
   else ptr = NULL;
   //else abort_program("vbo fail");

//////////////////////////////////////////////////
   t1 = clock();
   //debug_print("Reloading %d particles...(size %f)\n", (int)ceil(total / (float)subSample), scale);

   //Get eye distances and copy all particles into sorting array
   int index = 0;
   for (unsigned int s = 0; s < geom.size(); s++) 
   {
      debug_print("Swarm %d, points %d hidden? %s\n", s, geom[s]->count, (hidden[s] ? "yes" : "no"));

      //Calibrate colourMap
      geom[s]->colourCalibrate();

      //Cache values if possible, getColour() is slow!
      GeomData* geo = geom[s];
      ColourMap* cmap = NULL;
      float psize0 = geo->draw->properties["pointsize"].ToFloat(1.0) * geo->draw->properties["scaling"].ToFloat(1.0);
      //TODO: this duplicates code in getColour, 
      // try to optimise getColour better instead
      if (geo->draw->colourMaps[lucColourValueData] && geo->colourValue.size() > 0)
         cmap = geo->draw->colourMaps[lucColourValueData];
      //Set opacity to drawing object/geometry override level if set
      float alpha = geo->draw->properties["opacity"].ToFloat(0.0);
      if (GeomData::opacity > 0.0 && GeomData::opacity < 1.0) alpha = GeomData::opacity;
      float ptype = geo->draw->properties["pointtype"].ToInt(-1);
      bool smooth = geo->draw->properties["pointsmooth"].ToBool(true);

      for (int i = 0; i < geom[s]->count; i ++) 
      {
         pidx[index].id = i;
         pidx[index].index = index;
         pidx[index].geomid = s;
         pidx[index].distance = 0;

         //Copy data to VBO entry
         Colour c;
         if (ptr)
         {
            assert((int)(ptr-p) < total * datasize);
            //Copies vertex bytes
            memcpy(ptr, geo->vertices[i], sizeof(float) * 3);
            ptr += sizeof(float) * 3;
            //Copies colour bytes
            if (cmap)
            {
               c = cmap->getfast(geo->colourValue[i]);
               if (alpha > 0.0) c.a *= alpha;
            }
            else
               geo->getColour(c, i);
            memcpy(ptr, &c, sizeof(Colour));
            ptr += sizeof(Colour);
            //Copies settings (size + smooth)
            float psize = psize0;
            if (geo->sizes.size() > 0) psize *= geo->sizes[i];
            if (!smooth) psize = -psize;
            memcpy(ptr, &psize, sizeof(float));
            ptr += sizeof(float);
            memcpy(ptr, &ptype, sizeof(float));
            ptr += sizeof(float);
         }

         index++;
      }

   }
   t2 = clock(); debug_print("  %.4lf seconds to update particles into sort array and vbo\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();

   if (ptr) glUnmapBuffer(GL_ARRAY_BUFFER);
   GL_Error_Check;
}


//Depth sort the particles before drawing, called whenever the viewing angle has changed
void Points::depthSort()
{
   clock_t t1,t2;
   t1 = clock();
   if (total == 0) return;

   //Sort is much faster without allocate, so keep buffer until size changes
   static long last_size = 0;
   static PIndex* swap = NULL;
   int size = total*sizeof(PIndex);
   if (size != last_size)
   {
      if (swap) delete[] swap;
      swap = new PIndex[total];
      if (swap == NULL) abort_program("Memory allocation error (failed to allocate %d bytes)\n", size);
   }
   last_size = size;

   //Calculate min/max distances from view plane
   float maxdist, mindist; 
   view->getMinMaxDistance(&mindist, &maxdist);

   //Update eye distances, clamping int distance to integer between 0 and SORT_DIST_MAX
   float multiplier = (float)SORT_DIST_MAX / (maxdist - mindist);
   float fdistance;
   for (unsigned int i = 0; i < total; i++)
   {
      //Distance from viewing plane is -eyeZ
      fdistance = eyeDistance(view->modelView, geom[pidx[i].geomid]->vertices[pidx[i].id]);
      pidx[i].distance = (unsigned short)(multiplier * (fdistance - mindist));
   }
   t2 = clock(); debug_print("  %.4lf seconds to calculate distances\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();

   //Depth sort using 2-byte key radix sort, 10 times faster than equivalent quicksort
   radix_sort<PIndex>(pidx, swap, total, 2);
   //radix_sort(pidx, total, sizeof(PIndex), 2);
   //qsort(pidx, geom.size(), sizeof(PIndex), compare);
   t2 = clock(); debug_print("  %.4lf seconds to sort\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();
      GL_Error_Check;
}

//Reloads points into display list or VBO, required after data update and depth sort
static uint32_t SEED_VAL = 123456789;
#define SHR3 (SEED_VAL^=(SEED_VAL<<13), SEED_VAL^=(SEED_VAL>>17), SEED_VAL^=(SEED_VAL<<5))
void Points::render()
{
   clock_t t1,t2,tt;
   if (total == 0) return;

   //First, depth sort the particles
   if (view->is3d)
   {
      debug_print("Depth sorting %d particles...\n", total);
      depthSort();
   }

   tt = t1 = clock();

   // Index buffer object for quick display 
   if (!indexvbo && prog && prog->program)   //Only use vbo if shaders available
   { 
      //glDeleteBuffers(1, &vbo);
      glGenBuffers(1, &indexvbo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexvbo);
      GL_Error_Check;
      //Initialise particle buffer
      if (glIsBuffer(indexvbo))
      {
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, total * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
         debug_print("  %d byte IBO created for %d indices\n", total * sizeof(GLuint), total);
      }
      else 
         debug_print("  IBO creation failed!\n");
   }
   else
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexvbo);
   GL_Error_Check;

   //Re-map vertex indices in sorted order
   if (glIsBuffer(indexvbo))
   {
      t1 = clock();
      GLuint *ptr = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
      if (!ptr) abort_program("glMapBuffer failed");
      //Reverse order farthest to nearest 
      elements = 0;
      for(int i=total-1; i>=0; i--) 
      {  
         if (hiddencache[pidx[i].geomid]) continue;
         // If subSampling, use a pseudo random distribution to select which particles to draw
         // If we just draw every n'th particle, we end up with a whole bunch in one region / proc
         if (subSample > 1 && SHR3 % subSample > 0) continue;
         ptr[elements] = pidx[i].index;
         elements++;
            //printf("%d distance %d idx %d swarm %d vertex ", i, pidx[i].distance, pidx[i].id, pidx[i].geomid);
            //printVertex(geom[pidx[i].geomid]->vertices[pidx[i].id]);
            //printf("\n");
      }
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
      t2 = clock(); debug_print("  %.4lf seconds to upload indices (Sub-sampled %d)\n", (t2-t1)/(double)CLOCKS_PER_SEC, subSample); t1 = clock();
   }
   GL_Error_Check;
   t2 = clock(); debug_print("  Total %.4lf seconds.\n", (t2-tt)/(double)CLOCKS_PER_SEC);
}

void Points::draw()
{
   //Draw, update
   Geometry::draw();
   if (drawcount == 0) return;
   GL_Error_Check;

   setState(0, prog); //Set global draw state (using first object)

   //Re-render the particles if view has rotated
   if (view->sort) render();

   glDepthFunc(GL_LEQUAL); //Ensure points at same depth both get drawn
   glEnable(GL_POINT_SPRITE);
   GL_Error_Check;

   //Gen TexCoords for point sprites (for GLSL < 1.3 where gl_PointCoord not available)
   glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
   GL_Error_Check;


   glDepthFunc(GL_LEQUAL); //Ensure points at same depth both get drawn
   if (!view->is3d) glDisable(GL_DEPTH_TEST);

   if (prog && prog->program)
   {
      prog->use();
      GL_Error_Check;
      //Point size distance attenuation (disabled for 2d models)
      if (view->is3d && attenuate) //Adjust scaling by model size when using distance size attenuation 
         prog->setUniform("uPointScale", scale * view->model_size);
      else
         prog->setUniform("uPointScale", scale);
      prog->setUniform("uPointType", pointType);
      prog->setUniform("uOpacity", GeomData::opacity);
      prog->setUniform("uPointDist", (view->is3d && attenuate ? 1 : 0));

      glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
      GL_Error_Check;

      // Draw using vertex buffer object
      int stride = 5 * sizeof(float) + sizeof(Colour);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexvbo);
      if (elements > 0 && glIsBuffer(vbo) && glIsBuffer(indexvbo))
      {
         //Built in attributes gl_Vertex & gl_Color (Note: for OpenGL 3.0 onwards, should define our own generic attributes)
         glVertexPointer(3, GL_FLOAT, stride, (GLvoid*)0); // Load vertex x,y,z only
         glColorPointer(4, GL_UNSIGNED_BYTE, stride, (GLvoid*)(3*sizeof(float)));   // Load rgba, offset 3 float
         glEnableClientState(GL_VERTEX_ARRAY);
         glEnableClientState(GL_COLOR_ARRAY);

         //Generic vertex attributes, "aSize", "aPointType"
         GLint aSize = 0, aPointType = 0;
         if (prog && prog->program)
         {
            aSize = prog->attribs["aSize"];
            if (aSize >= 0)
            {
               glEnableVertexAttribArray(aSize);
               glVertexAttribPointer(aSize, 1, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3*sizeof(float)+sizeof(Colour)));
            }
            aPointType = prog->attribs["aPointType"];
            if (aPointType >= 0)
            {
               glEnableVertexAttribArray(aPointType);
               glVertexAttribPointer(aPointType, 1, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(4*sizeof(float)+sizeof(Colour)));
            }
         }
          
         //Draw the points
         glDrawElements(GL_POINTS, elements, GL_UNSIGNED_INT, (GLvoid*)0);

         if (prog && prog->program)
         {
            if (aSize >= 0) glDisableVertexAttribArray(aSize);
            if (aPointType >= 0) glDisableVertexAttribArray(aPointType);
         }
         glDisableClientState(GL_VERTEX_ARRAY);
         glDisableClientState(GL_COLOR_ARRAY);
      }
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
   }

   //Restore state
      glBindTexture(GL_TEXTURE_2D, 0);
   glDisable(GL_POINT_SPRITE);
   glDisable(GL_ALPHA_TEST);
   glDisable(GL_TEXTURE_2D);
   glDepthFunc(GL_LESS);
      glEnable(GL_DEPTH_TEST);
   GL_Error_Check;
   labels();
}

void Points::jsonWrite(unsigned int id, std::ostream* osp)
{
   std::ostream& os = *osp;
   bool first = true;
   for (unsigned int i = 0; i < geom.size(); i++) 
   {
      if (geom[i]->draw->id == id && drawable(i))
      {
         std::cerr << "Collected " << geom[i]->count << " vertices/values (" << i << ")" << std::endl;
         //Only supports dump of vertex, size and colourValue at present
         //if (subSample && rand() % subSample > 0) continue; //Not possible to subsample with this method...
         std::string vertices = base64_encode(reinterpret_cast<const unsigned char*>(&geom[i]->vertices.value[0]), geom[i]->vertices.size() * sizeof(float));
         if (!first) os << "," << std::endl;
         first = false;
         os << "        {" << std::endl;
         os << "          \"vertices\" : \n          {" << std::endl;
         os << "            \"size\" : 3," << std::endl;
         os << "            \"data\" : \"" << vertices << "\"" << std::endl;
         os << "          }," << std::endl;
         std::string normals = base64_encode(reinterpret_cast<const unsigned char*>(&geom[i]->normals.value[0]), geom[i]->normals.size() * sizeof(float));
         os << "          \"normals\" : \n          {" << std::endl;
         os << "            \"size\" : 3," << std::endl;
         os << "            \"data\" : \"" << normals << "\"" << std::endl;
         
         if (geom[i]->sizes.size() == geom[i]->count)
         {
            std::string sizes = base64_encode(reinterpret_cast<const unsigned char*>(&geom[i]->sizes.value[0]), geom[i]->sizes.size() * sizeof(float));
            os << "          }," << std::endl;
            os << "          \"sizes\" : \n          {" << std::endl;
            os << "            \"size\" : 1," << std::endl;
            os << "            \"data\" : \"" << sizes << "\"" << std::endl;
         }

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

