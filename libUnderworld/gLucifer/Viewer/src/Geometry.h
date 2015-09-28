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
#include "DrawingObject.h"
#include "ColourMap.h"
#include "View.h"
#include "ViewerTypes.h"
#include "Shaders.h"
#include "TimeStep.h"
#include "base64.h"

#ifndef Geometry__
#define Geometry__

#define SORT_DIST_MAX 65535

//Types based on triangle renderer
#define TriangleBased(type) (type == lucShapeType || type == lucVectorType || type == lucTracerType)

// Point indices + distance for sorting
typedef struct 
{
   unsigned short distance;
   GLuint index; //global index
   int id; //id in geom element
   unsigned short geomid; //WARNING: Limits max elements to 65535
} PIndex;

typedef struct 
{
   unsigned short distance;
   GLuint index[3]; //global indices
   float centroid[3];
   unsigned short geomid; //WARNING: Limits max elements to 65535
} TIndex;

//Geometry object data store
class GeomData
{
  public:
   static std::string names[lucMaxType];
   static float opacity;  //Global opacity
   static int glyphs;
   static bool wireframe, cullface, lit;
   DrawingObject* draw; //Parent drawing object
   int count;  //Number of vertices
   int width;
   int height;
   int depth;
   char* labelptr;
   bool opaque;   //Flag for opaque geometry, render first, don't depth sort
   TextureData* texture;

   float distance;

   //Bounding box of content
   float min[3];
   float max[3];

   std::vector<std::string> labels;      //Optional vertex labels

   //Geometry data
   Coord3DValues vertices;
   Coord3DValues positions;
   Coord3DValues vectors;
   Coord3DValues normals;
   FloatValues colourValue;
   FloatValues opacityValue;
   FloatValues redValue;
   FloatValues greenValue;
   FloatValues blueValue;
   FloatValues indices;
   FloatValues xWidths;
   FloatValues yHeights;
   FloatValues zLengths;
   FloatValues colours;
   Coord2DValues texCoords;
   FloatValues sizes;

   std::vector<FloatValues*> data;

   GeomData(DrawingObject* draw) : draw(draw), count(0), width(0), height(0), labelptr(NULL), opaque(false) 
   {
      //opaque = false; //true; //Always true for now (need to check colourmap, opacity and global opacity)
      data.resize(lucMaxDataType+1);
      data[lucVertexData] = &vertices;
      data[lucVectorData] = &vectors;
      data[lucNormalData] = &normals;
      data[lucColourValueData] = &colourValue;
      data[lucOpacityValueData] = &opacityValue;
      data[lucRedValueData] = &redValue;
      data[lucGreenValueData] = &greenValue;
      data[lucBlueValueData] = &blueValue;
      data[lucIndexData] = &indices;
      data[lucXWidthData] = &xWidths;
      data[lucYHeightData] = &yHeights;
      data[lucZLengthData] = &zLengths;
      data[lucRGBAData] = &colours;
      data[lucTexCoordData] = &texCoords;
      data[lucSizeData] = &sizes;

      texture = NULL;

      for (int i=0; i<3; i++)
      {
         min[i] = HUGE_VAL;
         max[i] = -HUGE_VAL;
      }
   }

   ~GeomData()
   {
      if (labelptr) free(labelptr);
      labelptr = NULL;
      if (texture && texture != draw->defaultTexture) delete texture;
   }

   void checkPointMinMax(float *coord);

   void label(std::string& labeltext);
   const char* getLabels();
   void colourCalibrate();
   void mapToColour(Colour& colour, float value);
   int colourCount();
   void getColour(Colour& colour, int idx);
};


class Distance 
{
  public:
   int id;
   float distance;
   Distance(int i, float d) : id(i), distance(d) {}
   bool operator<(const Distance& rhs) const
   {
      return distance < rhs.distance;
   }
};

//Class for surface vertices, sorting, collating & averaging normals
class Vertex 
{
  public:
   Vertex() : vert(NULL), ref(-1), vcount(1) {}

   float* vert;
   int ref;
   int id;
   int vcount;

   bool operator<(const Vertex &rhs) const
   {
      //Comparison for vertex sort
      if (vert[0] != rhs.vert[0]) return vert[0] < rhs.vert[0];
      if (vert[1] != rhs.vert[1]) return vert[1] < rhs.vert[1];
      return vert[2] < rhs.vert[2];
   }

   bool operator==(const Vertex &rhs) const
   {
      //Comparison for equality
      return fabs(vert[0] - rhs.vert[0]) < 0.001 && fabs(vert[1] - rhs.vert[1]) < 0.001 && fabs(vert[2] - rhs.vert[2]) < 0.001;
   }
};

//Comparison for sort by id
struct vertexIdSort
{
   bool operator()(const Vertex &a, const Vertex &b)
   {
      return (a.id < b.id); 
   }
};

//Container class for a list of geometry objects
class Geometry
{
  protected:
   View* view;
   std::vector<GeomData*> geom;
   std::vector<bool> hidden;
   //Cached hidden states (including object/viewport setting)
   std::vector<bool> hiddencache;
   int elements;
   int drawcount;
   bool flat2d; //Flag for flat surfaces in 2d

  public:
   //TODO: Move globals from GeomData and elsewhere to this properties object...
   static json::Object properties; //Global properties
   //Store the actual maximum bounding box
   static float min[3], max[3], dims[3];
   bool allhidden;
   lucGeometryType type;   //Holds the object type
   unsigned int total;     //Total entries of all objects in container
   float scale;   //Scaling factor
   bool redraw;    //Redraw from scratch flag

   Geometry();
   virtual ~Geometry();

   void clear(bool all=false); //Called before new data loaded 
   void reset(); //Called before new data loaded when caching previous data
   virtual void close(); //Called on quit & before gl context recreated 

   void compareMinMax(float* min, float* max);
   void dumpById(std::ostream& csv, unsigned int id);
   virtual void jsonWrite(unsigned int id, std::ostream* osp);
   bool hide(unsigned int idx);
   void hideAll();
   bool show(unsigned int idx);
   void showAll();
   void showById(unsigned int id, bool state);
   void redrawObject(unsigned int id);
   void localiseColourValues();
   bool drawable(unsigned int idx);
   virtual void init(); //Called on GL init
   void setState(int index, Shader* prog=NULL);
   virtual void update();  //Implementation should create geometry here...
   virtual void draw();  //Display saved geometry
   void labels();  //Draw labels
   std::vector<GeomData*> getAllObjects(int id);
   GeomData* getObjectStore(DrawingObject* draw);
   GeomData* add(DrawingObject* draw);
   GeomData* read(DrawingObject* draw, int n, lucGeometryDataType dtype, const void* data, int width=0, int height=0, int depth=1);
   void read(GeomData* geomdata, int n, lucGeometryDataType dtype, const void* data, int width=0, int height=0, int depth=1);
   void setup(DrawingObject* draw, lucGeometryDataType dtype, float minimum, float maximum, float dimFactor=1.0, const char* units="");
   void label(DrawingObject* draw, const char* labels);
   void print();
   int size() {return geom.size();}
   void setView(View* vp, float* min=NULL, float* max=NULL);
   void move(Geometry* other);
   void toImage(unsigned int idx);
   void setTexture(DrawingObject* draw, TextureData* texture);

   void drawVector(DrawingObject *draw, float pos[3], float vector[3], float scale, float radius0, float radius1, float head_scale, int segment_count=24);
   void drawTrajectory(DrawingObject *draw, float coord0[3], float coord1[3], float radius0, float radius1, float arrowHeadSize, float scale[3], float maxLength, int segment_count=24);
   void drawCuboid(DrawingObject *draw, float pos[3], float width, float height, float depth, Quaternion& rot);
   void drawSphere(DrawingObject *draw, Vec3d& centre, float radius, int segment_count=24);
   void drawEllipsoid(DrawingObject *draw, Vec3d& centre, Vec3d& radii, Quaternion& rot, int segment_count=24);
   int glyphSegments(int def=2);

   int getCount(DrawingObject* draw)
   {
      GeomData* geom = getObjectStore(draw);
      if (geom) return geom->count;
      return 0;
   }

};

class TriSurfaces : public Geometry
{
   TIndex *tidx;
   int tricount;
  protected:
   std::vector<Distance> surf_sort;
  public:
   static Shader* prog;
   GLuint indexvbo, vbo;

   TriSurfaces(bool flat2Dflag=false);
   ~TriSurfaces();
   virtual void close();
   virtual void update();
   void loadMesh();
   void loadBuffers();
   void setTriangle(int index, float* v1, float* v2, float* v3, int idx1=0, int idx2=0, int idx3=0);
   void calcTriangleNormals(int index, std::vector<Vertex> &verts, std::vector<Vec3d> &normals);
   void calcGridNormals(int i, std::vector<Vec3d> &normals);
   void calcGridIndices(int i, std::vector<GLuint> &indices);
   void depthSort();
   virtual void render();
   virtual void draw();
   virtual void jsonWrite(unsigned int id, std::ostream* osp);
};

class Lines : public Geometry
{
   TriSurfaces* tris;
   GLuint vbo;
   int linetotal;
   bool all2d;
  public:
   static bool tubes;
   Lines(bool all2Dflag=false);
   ~Lines();
   virtual void close();
   virtual void update();
   virtual void draw();
};

class Vectors : public Geometry
{
   Lines* lines;
   TriSurfaces* tris;
  public:
   Vectors();
   ~Vectors();
   virtual void close();
   virtual void update();
   virtual void draw();
};

class Tracers : public Geometry
{
   Lines* lines;
   TriSurfaces* tris;
  public:
   //Tracer specific data
   int steps;
   bool scaling;
   int timestep;

   Tracers();
   ~Tracers();
   virtual void close();
   virtual void update();
   virtual void draw();
};

class QuadSurfaces : public TriSurfaces
{
  public:
   bool triangles;

   QuadSurfaces(bool flat2Dflag=false);
   ~QuadSurfaces();
   virtual void update();
   virtual void render();
   void calcGridIndices(int i, std::vector<GLuint> &indices, unsigned int vertoffset);
   virtual void draw();
   virtual void jsonWrite(unsigned int id, std::ostream* osp);
};

class Shapes : public Geometry
{
   TriSurfaces* tris;
  public:
   Shapes();
   ~Shapes();
   virtual void close();
   virtual void update();
   virtual void draw();
};

class Points : public Geometry
{
   PIndex *pidx;
  public:
   static Shader* prog;
   static unsigned int subSample;
   static int pointType;
   bool attenuate;
   static GLuint indexvbo, vbo;

   Points();
   ~Points();
   virtual void init();
   virtual void close();
   virtual void update();
   void loadVertices();
   void depthSort();
   void render();
   virtual void draw();
   virtual void jsonWrite(unsigned int id, std::ostream* osp);

   void dumpJSON();
};

class Volumes : public Geometry
{
  public:
   static Shader* prog;
   GLuint colourTexture;
   std::map<int, int> slices;

   Volumes();
   ~Volumes();
   virtual void close();
   virtual void update();
   virtual void draw();
   void render(int i);
   GLubyte* getTiledImage(unsigned int id, int& iw, int& ih, bool flip, int xtiles=16);
   void pngWrite(unsigned int id, int xtiles=16);
   virtual void jsonWrite(unsigned int id, std::ostream* osp);
};

//Sorting util functions
int compareXYZ(const void *a, const void *b);
int comparePoint(const void *a, const void *b);
//void radix(int byte, long N, PIndex *source, PIndex *dest);
//void radixsort2(PIndex *source, long N);
//void radix_sort_byte(int byte, long N, unsigned char *source, unsigned char *dest, int size);
//void radix_sort(void *source, long N, int size, int bytes);

template <typename T>
void radix(char byte, long N, T *source, T *dest)
//void radix(char byte, char size, long N, unsigned char *src, unsigned char *dst)
{
   // Radix counting sort of 1 byte, 8 bits = 256 bins
   int size = sizeof(T);
   long count[256], index[256];
   int i;
   memset(count, 0, sizeof(count)); //Clear counts
   unsigned char* src = (unsigned char*)source;
   //unsigned char* dst = (unsigned char*)dest;

   //Create histogram, count occurences of each possible byte value 0-255
   for (i=0; i<N; i++)
      count[src[i*size+byte]]++;

   //Calculate number of elements less than each value (running total through array)
   //This becomes the offset index into the sorted array
   //(eg: there are 5 smaller values so place me in 6th position = 5)
   index[0]=0;
   for (i=1; i<256; i++) index[i] = index[i-1] + count[i-1];

   //Finally, re-arrange data by index positions
   for (i=0; i<N; i++ )
   {
       int val = src[i*size + byte];  //Get value
       memcpy(&dest[index[val]], &source[i], size);
       //memcpy(&dst[index[val]*size], &src[i*size], size);
       index[val]++; //Increment index to push next element with same value forward one
   }
}


template <typename T>
void radix_sort(T *source, T* swap, long N, char bytes)
{
   assert(bytes % 2 == 0);
   //debug_print("Radix X sort: %d items %d bytes. Byte: ", N, size);
   // Sort bytes from least to most significant 
   for (char x = 0; x < bytes; x += 2) 
   {
      radix<T>(x, N, source, swap);
      radix<T>(x+1, N, swap, source);
      //radix_sort_byte(x, N, (unsigned char*)source, (unsigned char*)temp, size);
      //radix_sort_byte(x+1, N, (unsigned char*)temp, (unsigned char*)source, size);
   }
}


template <typename T>
void radix_sort(T *source, long N, char bytes)
{
   T* temp = (T*)malloc(N*sizeof(T));
   radix_sort(source, temp, N, bytes);
   free(temp);
}

#endif //Geometry__
