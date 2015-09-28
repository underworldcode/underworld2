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

Shader* TriSurfaces::prog = NULL;

TriSurfaces::TriSurfaces(bool flat2Dflag) : Geometry()
{
   type = lucTriangleType;
   tricount = 0;
   vbo = 0;
   indexvbo = 0;
   tidx = NULL;
   flat2d = flat2Dflag;
}

TriSurfaces::~TriSurfaces()
{
   close();
}

void TriSurfaces::close()
{
   if (vbo)
      glDeleteBuffers(1, &vbo);
   if (indexvbo)
      glDeleteBuffers(1, &indexvbo);
   if (tidx)
      delete[] tidx;

   vbo = 0;
   indexvbo = 0;
   tidx = NULL;

   Geometry::close();
}

void TriSurfaces::update()
{
   Geometry::update();

   // Update triangles...
   clock_t t1,t2,tt;
   tt=clock();
   if (geom.size() == 0) return;

   //Get triangle count
   total = 0;
   tricount = 0;
   hiddencache.resize(geom.size());
   for (unsigned int t = 0; t < geom.size(); t++) 
   {
      tricount += geom[t]->indices.size() / 3;
      int tris;
      if (geom[t]->indices.size() > 0)
         tris = geom[t]->indices.size() / 3;
      else if (geom[t]->width > 0 && geom[t]->height > 0)
         tris = 2 * (geom[t]->width-1) * (geom[t]->height-1);
      else
         tris = geom[t]->count / 3;
      total += tris;
      hiddencache[t] = !drawable(t); //Save flags
      debug_print("Surface %d, triangles %d hidden? %s\n", t, tris, (hiddencache[t] ? "yes" : "no"));

      //TODO: hack for now to get wireframe working, should detect/switch opacity
      if (GeomData::wireframe || geom[t]->draw->properties["wireframe"].ToBool(false))
        geom[t]->opaque = true;
   }
   if (total == 0) return;

   //Only reload the vbo data when required
   //Not needed when objects hidden/shown but required if colours changed
   //To force, use Geometry->reset() which sets elements to -1 
   if (elements < 0 || !tidx || tricount != total)
   {
      //Clear buffers
      close();
      //Load & optimise the mesh data
      loadMesh();
      //Send the data to the GPU via VBO
      loadBuffers();
   }

   //Depth sort & render
   render();
}

void TriSurfaces::loadMesh()
{
   // Load & optimise triangle meshes...
   // If indices & normals provided, this simply adds triangles to sorting array and calcs centroid
   // If not, also optimises the mesh, removes duplicate vertices, calcs vertex normals and colours
   clock_t t1,t2,tt;
   tt=clock();

   debug_print("Loading %d triangles...\n", total);

   //Create sorting array
   if (tidx) delete[] tidx;
   tidx = new TIndex[total];
   if (tidx == NULL) abort_program("Memory allocation error (failed to allocate %d bytes)", sizeof(TIndex) * total);

   //Calculate normals, delete duplicate verts, add triangles to sorting array

   //Index data for all vertices
   GLuint unique = 0;
   tricount = 0;
   elements = 0;
   for (unsigned int index = 0; index < geom.size(); index++) 
   {
      //Save initial offset
      GLuint voffset = unique;

      //Has index data, simply load the triangles
      if (geom[index]->indices.size() > 0)
      {
         floatidx i1, i2, i3;
         for (int j=0; j < geom[index]->indices.size(); j += 3)
         {
            i1.val = geom[index]->indices[j];
            i2.val = geom[index]->indices[j+1];
            i3.val = geom[index]->indices[j+2];
            
            TriSurfaces::setTriangle(index,
                                    geom[index]->vertices[i1.idx],
                                    geom[index]->vertices[i2.idx],
                                    geom[index]->vertices[i3.idx],
                                    i1.idx + voffset, i2.idx + voffset, i3.idx + voffset
                                    );
        }

        //Increment by vertex count (all vertices are unique as mesh is pre-optimised)
        elements += geom[index]->indices.size();
        unique += geom[index]->vertices.size() / 3;
        continue;
      }

      bool grid = (geom[index]->width > 0 && geom[index]->height > 0);

      //Add vertices to a vector with indices
      //Sort vertices vector with std::sort & custom compare sort( vec.begin(), vec.end() );
      //Iterate, for duplicates replace indices with index of first
      //Remove duplicate vertices Triangles stored as list of indices
      int hasColours = geom[index]->colourCount();
      bool vertColour = hasColours && (hasColours == geom[index]->count);
      t1=tt=clock();

      //Add vertices to vector
      std::vector<Vertex> verts(geom[index]->count);
      std::vector<Vec3d> normals(geom[index]->count);
      std::vector<GLuint> indices;
      for (int j=0; j < geom[index]->count; j++)
      {
         verts[j].id = verts[j].ref = j;
         verts[j].vert = geom[index]->vertices[j];
      }
      t2 = clock(); debug_print("  %.4lf seconds to add to sort vector\n", (t2-t1)/(double)CLOCKS_PER_SEC);

      int triverts = 0;
      if (grid)
      {
         //Structured mesh grid, 2 triangles per element, 3 indices per tri 
         int els = (geom[index]->width-1) * (geom[index]->height-1);
         triverts = els * 6;
         indices.resize(triverts);
         calcGridNormals(index, normals);
         calcGridIndices(index, indices);
         elements += triverts;
      }
      else
      {
         //Unstructured mesh, 1 index per vertex 
         triverts = geom[index]->count;
         indices.resize(triverts);
         calcTriangleNormals(index, verts, normals);
         elements += triverts;
      }

      //Now have list of vertices sorted by vertex pos with summed normals and references of duplicates replaced
      t1 = clock();

      //Calibrate colour maps on range for this surface
      geom[index]->colourCalibrate();

      if (grid) 
      {
         //Replace normals
         geom[index]->normals = Coord3DValues();
         read(geom[index], normals.size(), lucNormalData, &normals[0]);
      }
      else
      {
         //Switch out the optimised vertices and normals with the old data stores
         std::vector<Vec3d> vertices;
         geom[index]->vertices = Coord3DValues();
         geom[index]->normals = Coord3DValues();
         geom[index]->indices = FloatValues();
         int i = 0;
         geom[index]->count = 0;
         for (unsigned int v=0; v<verts.size(); v++)
         {
            //Re-write optimised data with unique vertices only
            if (verts[v].id == verts[v].ref)
            {
               //Reference id == self, not a duplicate
               //Normalise final vector
               normals[verts[v].id].normalise();

               //Average final colour
               if (vertColour && verts[v].vcount > 1)
                  geom[index]->colourValue.value[verts[v].id] /= verts[v].vcount;

               int id = verts[v].id;

               //Replace verts & normals
               vertices.push_back(Vec3d(verts[v].vert));
               read(geom[index], 1, lucNormalData, normals[verts[v].id].ref());

               //Save an index lookup entry (Grid indices loaded in previous step)
               indices[verts[v].id] = i++;
               unique++;
            }
            else
            {
               //Duplicate vertex, use index reference
               indices[verts[v].id] = indices[verts[v].ref];
            }
         }
         //Read replacement vertices
         read(geom[index], i, lucVertexData, &vertices[0]);
      }

      t2 = clock(); debug_print("  %.4lf seconds to normalise (and re-buffer)\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();

      t1 = clock();
      //Loop from previous tricount to current tricount
      int idx=0;
      for (unsigned int t = (tricount - triverts / 3); t < tricount; t++)
      {
         //voffset is offset of the last vertex added to the vbo from the previous object
         tidx[t].index[0] = indices[idx++] + voffset;
         tidx[t].index[1] = indices[idx++] + voffset;
         tidx[t].index[2] = indices[idx++] + voffset;
      }
      t2 = clock(); debug_print("  %.4lf seconds to re-index\n", (t2-t1)/(double)CLOCKS_PER_SEC);

      //Read the indices for later use (json export etc)
      geom[index]->indices.read(indices.size(), &indices[0]);

      t2 = clock(); debug_print("  %.4lf seconds to reload & clean up\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();
      debug_print("  Total %.4lf seconds.\n", (t2-tt)/(double)CLOCKS_PER_SEC);
   }

   //debug_print("  *** There were %d unique vertices out of %d total. Buffer allocated for %d\n", unique, total*3, bsize/datasize);
   t2 = clock(); debug_print("  %.4lf seconds to optimise triangle mesh\n", (t2-tt)/(double)CLOCKS_PER_SEC);
}

void TriSurfaces::loadBuffers()
{
   //Copy data to Vertex Buffer Object
   clock_t t1,t2,tt;
   tt=clock();

   //Update VBO...
   debug_print("Reloading %d triangles...\n", tricount);

   // VBO - copy normals/colours/positions to buffer object 
   unsigned char *p, *ptr;
   ptr = p = NULL;
   int datasize = sizeof(float) * 8 + sizeof(Colour);   //Vertex(3), normal(3), texCoord(2) and 32-bit colour
   int vcount = 0;
   for (unsigned int index = 0; index < geom.size(); index++) 
      vcount += geom[index]->count;
   int bsize = vcount * datasize;

    //Initialise vertex buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (glIsBuffer(vbo))
    {
       glBufferData(GL_ARRAY_BUFFER, bsize, NULL, GL_STATIC_DRAW);
       debug_print("  %d byte VBO created, holds %d vertices\n", bsize, bsize/datasize);
       ptr = p = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
       GL_Error_Check;
    }
    if (!p) abort_program("VBO setup failed");

   //Buffer data for all vertices
   for (unsigned int index = 0; index < geom.size(); index++) 
   {
      t1=tt=clock();

      //Calibrate colour maps on range for this surface
      geom[index]->colourCalibrate();
      int hasColours = geom[index]->colourCount();
      int colrange = hasColours ? geom[index]->count / hasColours : 1;
      //if (hasColours) assert(colrange * hasColours == geom[index]->count);
      //if (hasColours && colrange * hasColours != geom[index]->count)
      //   debug_print("WARNING: Vertex Count %d not divisable by colour count %d\n", geom[index]->count, hasColours);
      bool vertColour = hasColours && colrange > 1;
      debug_print("Using 1 colour per %d vertices (%d : %d)\n", colrange, geom[index]->count, hasColours);

      int i = 0;
      Colour colour;
      bool normals = geom[index]->normals.size() == geom[index]->vertices.size();
      debug_print("Mesh %d/%d has normals? %d (%d == %d)\n", index, geom.size(), normals, geom[index]->normals.size(), geom[index]->vertices.size());
      float zero[3] = {0,0,0};
      for (unsigned int v=0; v < geom[index]->count; v++)
      {
         if (colrange <= 1)
            geom[index]->getColour(colour, v);
         else
         {
            //Have colour values but not enough for per-vertex, spread over range (eg: per triangle)
            int cidx = v / colrange;
            if (cidx * colrange == v)
               geom[index]->getColour(colour, cidx);
         }

          //Write vertex data to vbo
         assert((int)(ptr-p) < bsize);
         //Copies vertex bytes
         memcpy(ptr, &geom[index]->vertices[v][0], sizeof(float) * 3);
         ptr += sizeof(float) * 3;
         //Copies normal bytes
         if (normals)
            memcpy(ptr, &geom[index]->normals[v][0], sizeof(float) * 3);
         else
            memcpy(ptr, zero, sizeof(float) * 3);
         ptr += sizeof(float) * 3;
         //Copies texCoord bytes
         if (geom[index]->texCoords.size() > 0)
            memcpy(ptr, &geom[index]->texCoords[v][0], sizeof(float) * 2);
         ptr += sizeof(float) * 2;
         //Copies colour bytes
         memcpy(ptr, &colour, sizeof(Colour));
         ptr += sizeof(Colour);
      }
      t2 = clock(); debug_print("  %.4lf seconds to reload %d vertices\n", (t2-t1)/(double)CLOCKS_PER_SEC, geom[index]->count); t1 = clock();
   }

   glUnmapBuffer(GL_ARRAY_BUFFER);
   GL_Error_Check;
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   debug_print("  Total %.4lf seconds to update triangle buffers\n", (t2-tt)/(double)CLOCKS_PER_SEC);
}

#define MAX3(a,b,c) ( a>b ? (a>c ? a : c) : (b>c ? b : c) )
void TriSurfaces::setTriangle(int index, float* v1, float* v2, float* v3, int idx1, int idx2, int idx3)
{
   tidx[tricount].distance = 0;
   tidx[tricount].geomid = index;
   tidx[tricount].index[0] = idx1;
   tidx[tricount].index[1] = idx2;
   tidx[tricount].index[2] = idx3;

#ifdef DEBUG
   for (int i=0; i<3; i++)
   {
     assert(!isnan(v1[i]));
     assert(!isnan(v2[i]));
     assert(!isnan(v3[i]));
   }
#endif

   //All opaque triangles at start
   if (geom[index]->opaque)
      tidx[tricount].distance = SORT_DIST_MAX;
   else
   {
      //Triangle centroid for depth sorting
      //Actual centroid
      float centroid[3] = {(v1[0]+v2[0]+v3[0])/3, (v1[1]+v2[1]+v3[1])/3, view->is3d ? (v1[2]+v2[2]+v3[2])/3 : 0.0f};
      //Max values in each axis instead of centroid TODO: allow switching sort vertex calc type
      //float centroid[3] = {MAX3(v1[0], v2[0], v3[0]), MAX3(v1[1], v2[1], v3[1]), MAX3(v1[2], v2[2], v3[2])};
      //printf("%d v1 %f,%f,%f v2 %f,%f,%f v3 %f,%f,%f\n", index, v1[0], v1[1], v1[2], v2[0], v2[1], v2[2], v3[0], v3[1], v3[2]);

      //Limit to defined bounding box
      //Possibly should store calculated bounding box separately for goemetry outside border
      for (int i=0; i<3; i++)
      {
         centroid[i] = max(centroid[i], view->min[i]);
         centroid[i] = min(centroid[i], view->max[i]);
      }

      /*if (centroid[2] < view->min[2] || centroid[2] > view->max[2])
      {
         //abort_program
         printf("Warning: centroid out of bounds %f,%f,%f min %f,%f,%f max %f,%f,%f\n", centroid[0], centroid[1], centroid[2], view->min[0], view->min[1], view->min[2], view->max[0], view->max[1], view->max[2]);
         view->checkPointMinMax(centroid);
      }*/
      //assert(centroid[2] >= view->min[2] && centroid[2] <= view->max[2]);
      memcpy(tidx[tricount].centroid, centroid, sizeof(float)*3);
   }
   tricount++;
}

void TriSurfaces::calcTriangleNormals(int index, std::vector<Vertex> &verts, std::vector<Vec3d> &normals)
{
   clock_t t1,t2;
   t1 = clock();
   debug_print("Calculating normals for triangle surface %d size %d\n", index, geom[index]->vertices.size()/3);
   int hasColours = geom[index]->colourCount();
   bool vertColour = (hasColours && hasColours == geom[index]->vertices.size()/3);
   //Calculate face normals for each triangle and copy to each face vertex
   for (unsigned int v=0; v<verts.size(); v += 3)
   {
      //Copies for each vertex
      normals[v] = vectorNormalToPlane(verts[v].vert, verts[v+1].vert, verts[v+2].vert);
      normals[v+1] = Vec3d(normals[v]);
      normals[v+2] = Vec3d(normals[v]);

      //Add to triangle index list for sorting
      setTriangle(index, verts[v].vert, verts[v+1].vert, verts[v+2].vert);
   }
   t2 = clock(); debug_print("  %.4lf seconds to calc facet normals\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();

   //Sort by vertex
   std::sort(verts.begin(), verts.end());
   t2 = clock(); debug_print("  %.4lf seconds to sort %d verts\n", (t2-t1)/(double)CLOCKS_PER_SEC, verts.size()); t1 = clock();

   //Now have list of vertices sorted by vertex pos
   //Search for duplicates and replace references to normals
   int match = 0;
   for(unsigned int v=1; v<verts.size(); v++)
   {
      if (verts[match] == verts[v])
      {
         // If the angle between a given face normal and the face normal
         // associated with the first triangle in the list of triangles for the
         // current vertex is greater than a specified angle, normal is not added
         // to average normal calculation and the corresponding vertex is given
         // the facet normal. This preserves hard edges, specific angle to
         // use depends on the model, but 90 degrees is usually a good start.

         // cosine of angle between vectors = (v1 . v2) / |v1|.|v2|
         float angle = RAD2DEG * normals[verts[v].id].angle(normals[verts[match].id]);
         //debug_print("angle %f ", angle);
         //Don't include vertices in the sum if angle between normals too sharp
         if (angle < 90)
         {
            //Found a duplicate, replace reference idx (original retained in "id")
            verts[v].ref = verts[match].ref;

            //Add this normal to matched normal
            normals[verts[match].id] += normals[verts[v].id];

            //Colour value, add to matched
            if (vertColour)
               geom[index]->colourValue.value[verts[match].id] += geom[index]->colourValue.value[verts[v].id];

            verts[match].vcount++;
            verts[v].vcount = 0;
         }
      }
      else
      {
         //First occurrence, following comparisons will be against this vertex...
         match = v;
      }
   }
   t2 = clock(); debug_print("  %.4lf seconds to replace duplicates\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();
}

void TriSurfaces::calcGridNormals(int i, std::vector<Vec3d> &normals)
{
   //Normals: calculate from surface geometry
   clock_t t1,t2;
   t1=clock();
   debug_print("Calculating normals for grid surface %d... ", i);

   // Calc pre-vertex normals for irregular meshes by averaging four surrounding triangle facet normals
   int n = 0;
   for (int j = 0 ; j < geom[i]->height; j++ )
   {
      for (int k = 0 ; k < geom[i]->width; k++ )
      {
         // Get sum of normal vectors
         if (j > 0)
         {
            if (k > 0)
            {
               // Look back
               normals[n] += vectorNormalToPlane(geom[i]->vertices[geom[i]->width * j + k], 
                             geom[i]->vertices[geom[i]->width * (j-1) + k], geom[i]->vertices[geom[i]->width * j + k-1]);
            }

            if (k < geom[i]->width - 1)
            {
               // Look back in x, forward in y
               normals[n] += vectorNormalToPlane(geom[i]->vertices[geom[i]->width * j + k], 
                             geom[i]->vertices[geom[i]->width * j + k+1], geom[i]->vertices[geom[i]->width * (j-1) + k]);
            }
         }

         if (j <  geom[i]->height - 1)
         {
            if (k > 0)
            {
               // Look forward in x, back in y
               normals[n] += vectorNormalToPlane(geom[i]->vertices[geom[i]->width * j + k], 
                             geom[i]->vertices[geom[i]->width * j + k-1], geom[i]->vertices[geom[i]->width * (j+1) + k]);
            }

            if (k < geom[i]->width - 1)
            {
               // Look forward
               normals[n] += vectorNormalToPlane(geom[i]->vertices[geom[i]->width * j + k], 
                             geom[i]->vertices[geom[i]->width * (j+1) + k], geom[i]->vertices[geom[i]->width * j + k+1]);
            }
         }

         //Normalise to average
         normals[n].normalise();
         //Copy directly into normal block
         //memcpy(geom[i]->normals[j * geom[i]->width + k], normal.ref(), sizeof(float) * 3);
         n++;
      }
   }
   t2 = clock(); debug_print("  %.4lf seconds\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();
}

void TriSurfaces::calcGridIndices(int i, std::vector<GLuint> &indices)
{
   //Normals: calculate from surface geometry
   clock_t t1,t2;
   t1=clock();
   debug_print("Calculating indices for grid tri surface %d... ", i);

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
         assert(o <= indices.size()-6);
         //Tri 1
         setTriangle(i, geom[i]->vertices[offset0], geom[i]->vertices[offset1], geom[i]->vertices[offset2]);
         indices[o++] = offset0;
         indices[o++] = offset1;
         indices[o++] = offset2;
         //Tri 2
         setTriangle(i, geom[i]->vertices[offset1], geom[i]->vertices[offset3], geom[i]->vertices[offset2]);
         indices[o++] = offset1;
         indices[o++] = offset3;
         indices[o++] = offset2;
      }
   }
   t2 = clock(); debug_print("  %.4lf seconds\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();
}

//Depth sort the triangles before drawing, called whenever the viewing angle has changed
void TriSurfaces::depthSort()
{
   clock_t t1,t2;
   t1 = clock();
   assert(tidx);

   //Sort is much faster without allocate, so keep buffer until size changes
   static long last_size = 0;
   static TIndex* swap = NULL;
   int size = tricount*sizeof(TIndex);
   if (size != last_size)
   {
      if (swap) delete[] swap;
      swap = new TIndex[tricount];
      if (swap == NULL) abort_program("Memory allocation error (failed to allocate %d bytes)", size);
   }
   last_size = size;

   //Calculate min/max distances from view plane
   float maxdist, mindist; 
   view->getMinMaxDistance(&mindist, &maxdist);
   //printMatrix(modelView);
   //printf("MINDIST %f MAXDIST %f\n", mindist, maxdist);
   int shift = view->properties["shift"].ToInt(0);

   //Update eye distances, clamping int distance to integer between 1 and 65534
   float multiplier = (SORT_DIST_MAX-1.0) / (maxdist - mindist);
   if (tricount == 0) return;
   int opaqueCount = 0;
   float fdistance;
   for (unsigned int i = 0; i < tricount; i++)
   {
      //Distance from viewing plane is -eyeZ
      //Max dist 65535 reserved for opaque triangles
      if (tidx[i].distance < SORT_DIST_MAX) 
      //if (tidx[i].distance > 0) 
      {
         //if (i%6==0)printf("! %d centroid %f,%f,%f\n", i, tidx[i].centroid[0], tidx[i].centroid[1], tidx[i].centroid[2]);
         fdistance = eyeDistance(view->modelView, tidx[i].centroid);
         tidx[i].distance = (unsigned short)(multiplier * (fdistance - mindist));
         assert(tidx[i].distance >= 0 && tidx[i].distance <= SORT_DIST_MAX);
         //Shift by id hack
         if (shift) tidx[i].distance += tidx[i].geomid * shift;
         //Reverse as radix sort is ascending and we want to draw by distance descending
         //tidx[i].distance = SORT_DIST_MAX - (unsigned short)(multiplier * (fdistance - mindist));
         //assert(tidx[i].distance >= 1 && tidx[i].distance <= SORT_DIST_MAX);
      }
      else
        opaqueCount++;
   }
   t2 = clock(); debug_print("  %.4lf seconds to calculate distances\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();

   //Skip sort if all opaque
   if (opaqueCount == tricount) return;

   //Depth sort using 2-byte key radix sort, 10 times faster than equivalent quicksort
   radix_sort<TIndex>(tidx, swap, tricount, 2);
   t2 = clock(); debug_print("  %.4lf seconds to sort\n", (t2-t1)/(double)CLOCKS_PER_SEC); t1 = clock();
}

//Reloads triangle indices, required after data update and depth sort
void TriSurfaces::render()
{
   clock_t t1,t2;
   if (tricount == 0) return;
   assert(tidx);

   //First, depth sort the triangles
   if (view->is3d)
   {
      debug_print("Depth sorting %d triangles...\n", tricount);
      depthSort();
   }

   t1 = clock();

   //Prepare the Index buffer
   if (!indexvbo)
   {
      assert(elements);
      glGenBuffers(1, &indexvbo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexvbo);
      GL_Error_Check;
      if (glIsBuffer(indexvbo))
      {
         //DYNAMIC_DRAW is really really slow on Quadro K5000s in CAVE2, nVidie 340 drivers
         //glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements * sizeof(GLuint), NULL, GL_STATIC_DRAW);
         debug_print("  %d byte IBO created for %d indices\n", elements * sizeof(GLuint), elements);
      }
      else 
         debug_print("  IBO creation failed\n");
      GL_Error_Check;
   }

   //Re-map vertex indices in sorted order
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexvbo);
   if (glIsBuffer(indexvbo))
   {
      unsigned char *p, *ptr;
      t1 = clock();
      ptr = p = (unsigned char*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
      GL_Error_Check;
      if (!p) abort_program("glMapBuffer failed");
      //Reverse order farthest to nearest 
      elements = 0;
      for(int i=tricount-1; i>=0; i--) 
      //for(int i=0; i<tricount; i++) 
      {
         if (hiddencache[tidx[i].geomid]) continue;
         elements += 3;
         assert((int)(ptr-p) < 3 * tricount * sizeof(GLuint));
         //Copies index bytes
         memcpy(ptr, tidx[i].index, sizeof(GLuint) * 3);
         ptr += sizeof(GLuint) * 3;
      }
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
      GL_Error_Check;
      t2 = clock(); debug_print("  %.4lf seconds to upload %d indices (%d tris)\n", (t2-t1)/(double)CLOCKS_PER_SEC, elements, tricount); t1 = clock();
   }
}

void TriSurfaces::draw()
{
   //Draw, calls update when required
   Geometry::draw();
   GL_Error_Check;
   if (drawcount == 0) return;

   //Re-render the triangles if view has rotated
   if (view->sort) render();

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

      unsigned int start = 0;
      //Reverse order of objects to match index array layout (opaque objects last)
      int tridx = 0;
      for (int index = geom.size()-1; index >= 0; index--) 
      {
         if (hiddencache[index]) continue;
         if (geom[index]->opaque)
         {
            setState(index, prog); //Set draw state settings for this object
            //fprintf(stderr, "(%d) DRAWING OPAQUE TRIANGLES: %d (%d to %d)\n", index, geom[index]->indices.size()/3, start/3, (start+geom[index]->indices.size())/3);
            glDrawRangeElements(GL_TRIANGLES, 0, elements, geom[index]->indices.size(), GL_UNSIGNED_INT, (GLvoid*)(start*sizeof(GLuint)));
            start += geom[index]->indices.size();
         }
         else
           tridx = index;
      }

      time = ((clock()-t1)/(double)CLOCKS_PER_SEC);
      if (time > 0.005) debug_print("  %.4lf seconds to draw opaque triangles\n", time);
      t1 = clock();

      //Set draw state settings for first non-opaque object
      //NOTE: per-object textures do not work with transparency!
      setState(tridx, prog);

      //Draw remaining elements (transparent, depth sorted)
      //fprintf(stderr, "(*) DRAWING TRANSPARENT TRIANGLES: %d\n", (elements-start)/3);
      if (start < elements)
      {
         if (start > 0)
         {
            //fprintf(stderr, "(*) DRAWING TRANSPARENT TRIANGLES: %d\n", elements-start);
            glDrawRangeElements(GL_TRIANGLES, 0, elements, elements-start, GL_UNSIGNED_INT, (GLvoid*)(start*sizeof(GLuint)));
         }
         else
         {
            //Render all triangles - elements is the number of indices. 3 indices needed to make a single triangle
            //(If there is no separate opaque/transparent geometry)
            glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, (GLvoid*)0);
         }
      }

      time = ((clock()-t1)/(double)CLOCKS_PER_SEC);
      if (time > 0.005) debug_print("  %.4lf seconds to draw transparent triangles\n", time);

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
   }
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   //Restore state
   //glEnable(GL_LIGHTING);
   //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   //glDisable(GL_CULL_FACE);
   glBindTexture(GL_TEXTURE_2D, 0);

   time = ((clock()-t0)/(double)CLOCKS_PER_SEC);
   if (time > 0.05)
     debug_print("  %.4lf seconds to draw triangles\n", time);
   GL_Error_Check;
}

void TriSurfaces::jsonWrite(unsigned int id, std::ostream* osp)
{
   bool first = true;
   for (unsigned int index = 0; index < geom.size(); index++) 
   {
      //Save json data for export
      if (geom[index]->draw->id == id && drawable(index))
      {
         std::ostream& os = *osp;
         std::string indices_str = base64_encode(reinterpret_cast<const unsigned char*>(&geom[index]->indices.value[0]), geom[index]->indices.size() * sizeof(GLuint));
         std::string vertices_str = base64_encode(reinterpret_cast<const unsigned char*>(&geom[index]->vertices.value[0]), geom[index]->vertices.size() * sizeof(float) * 3);
         std::string normals_str = base64_encode(reinterpret_cast<const unsigned char*>(&geom[index]->normals.value[0]), geom[index]->normals.size() * sizeof(float) * 3);

         std::cerr << "Collected " << geom[index]->count << " vertices/values (" << index << ")" << std::endl;
         //Only supports dump of vertex, normal and colourValue at present
         if (!first) os << "," << std::endl;
         first = false;
         os << "        {" << std::endl;
         os << "          \"indices\" : \n          {" << std::endl;
         os << "            \"size\" : 1," << std::endl;
         os << "            \"count\" : " << geom[index]->indices.size() << "," << std::endl;
         os << "            \"data\" : \"" << indices_str << "\"" << std::endl;
         os << "          }," << std::endl;
         os << "          \"vertices\" : \n          {" << std::endl;
         os << "            \"size\" : 3," << std::endl;
         os << "            \"count\" : " << geom[index]->vertices.size() << "," << std::endl;
         os << "            \"data\" : \"" << vertices_str << "\"" << std::endl;
         os << "          }," << std::endl;
         os << "          \"normals\" : \n          {" << std::endl;
         os << "            \"size\" : 3," << std::endl;
         os << "            \"count\" : " << geom[index]->normals.size() << "," << std::endl;
         os << "            \"data\" : \"" << normals_str << "\"" << std::endl;
         
         if (geom[index]->colourValue.size())
         {
            std::string values_str = base64_encode(reinterpret_cast<const unsigned char*>(&geom[index]->colourValue.value[0]), geom[index]->colourValue.size() * sizeof(float));
            os << "          }," << std::endl;
            os << "          \"values\" : \n          {" << std::endl;
            os << "            \"size\" : 1," << std::endl;
            os << "            \"count\" : " << geom[index]->colourValue.size() << "," << std::endl;
            os << "            \"minimum\" : " << geom[index]->colourValue.minimum << "," << std::endl;
            os << "            \"maximum\" : " << geom[index]->colourValue.maximum << "," << std::endl;
            os << "            \"data\" : \"" << values_str << "\"" << std::endl;
         }
         os << "          }" << std::endl;
         os << "        }";
      }
   }
}
