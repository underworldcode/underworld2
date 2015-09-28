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

#ifndef GraphicsUtil__
#define GraphicsUtil__
#include "Include.h"

#define INBUF_SIZE 65535

#define GL_Error_Check \
  { \
    GLenum error = GL_NO_ERROR; \
    while ((error = glGetError()) != GL_NO_ERROR) { \
      printf("OpenGL error [ %s : %d ] \"%s\".\n",  \
            __FILE__, __LINE__, glErrorString(error)); \
    } \
  }

#define STRINGIFY(A) #A

#define BLEND_NORMAL 0
#define BLEND_PNG 1
#define BLEND_ADD 2

#define FONT_FIXED    0
#define FONT_SMALL    1
#define FONT_NORMAL   2
#define FONT_SERIF    3

#define FONT_DEFAULT FONT_NORMAL

#ifndef min
 #define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
 #define max(a,b) ((a)>(b)?(a):(b))
#endif

#define EPSILON 0.000001
#ifndef M_PI
#define M_PI 3.1415926536
#endif

#define DEG2RAD (M_PI/180.0)
#define RAD2DEG (180.0/M_PI)

#define crossProduct(a,b,c) \
   (a)[0] = (b)[1] * (c)[2] - (c)[1] * (b)[2]; \
   (a)[1] = (b)[2] * (c)[0] - (c)[2] * (b)[0]; \
   (a)[2] = (b)[0] * (c)[1] - (c)[0] * (b)[1];

#define dotProduct(v,q) \
   ((v)[0] * (q)[0] + \
   (v)[1] * (q)[1] + \
   (v)[2] * (q)[2])

#define vectorAdd(a, b, c) \
   (a)[0] = (b)[0] + (c)[0]; \
   (a)[1] = (b)[1] + (c)[1]; \
   (a)[2] = (b)[2] + (c)[2]; \

#define vectorSubtract(a, b, c) \
   (a)[0] = (b)[0] - (c)[0]; \
   (a)[1] = (b)[1] - (c)[1]; \
   (a)[2] = (b)[2] - (c)[2]; \

#define vectorMagnitude(v) sqrt(dotProduct(v,v));

//Get eye pos vector z by multiplying vertex by modelview matrix
#define eyeDistance(M,V) -(M[2] * V[0] + M[6] * V[1] + M[10] * V[2] + M[14]);

#define printVertex(v) printf("%9f,%9f,%9f",v[0],v[1],v[2]);
// Print out a matrix
#ifndef M
#define M(mat,row,col)  mat[col*4+row]
#endif
#define printMatrix(mat) {              \
        int r, p;                       \
        printf("--------- --------- --------- ---------\n"); \
        for (r=0; r<4; r++) {           \
            for (p=0; p<4; p++)         \
                printf("%9f ", M(mat, r,p)); \
            printf("\n");               \
        } printf("--------- --------- --------- ---------\n"); }

#define frand (rand() / (float) RAND_MAX)

// In OpenGL you cannot set the raster position to be outside the viewport - 
// but this is a hack which OpenGL allows (and advertises) - That you can get the raster position to a legal value
// and then move the raster position by calling glBitmap with a NULL bitmap
#define MoveRaster( deltaX, deltaY ) \
   glBitmap( 0,0,0.0,0.0, (float)(deltaX), (float)(deltaY), NULL )

//Some predefined rgba colour constants
#define LUC_BLACK          0xff000000
#define LUC_WHITE          0xffffffff
#define LUC_GREEN          0xff00ff00
#define LUC_BLUE           0xffff0000
#define LUC_CYAN           0xffffff00
#define LUC_YELLOW         0xff00ffff
#define LUC_RED            0xff0000ff
#define LUC_GREY           0xff888888
#define LUC_SILVER         0xffcccccc
#define LUC_MIDBLUE        0xffff3333
#define LUC_STRAW          0xff77ffff
#define LUC_ORANGE         0xff0088ff
#define LUC_DARKGREEN      0xff006400
#define LUC_DARKBLUE       0xff00008B
#define LUC_SEAGREEN       0xff2E8B57
#define LUC_YELLOWGREEN    0xff9ACD32
#define LUC_OLIVE          0xff808000
#define LUC_LIGHTBLUE      0xff1E90FF
#define LUC_DARKTURQUOISE  0xff00CED1
#define LUC_BISQUE         0xffFFE4C4
#define LUC_ORANGERED      0xffFF4500
#define LUC_BRICK          0xffB22222
#define LUC_DARKRED        0xff8B0000

extern float *x_coords_, *y_coords_;  // Saves arrays of x,y points on circle for set segment count
extern FILE* infostream;
void abort_program(const char * s, ...);
void debug_print(const char *fmt, ...);

//Defines a 32bit colour accessible in multiple ways:
// - rgba 4-byte array
// - r,g,b,a component struct
// - integer (hex AABBGGRR little endian)
// - float for storing in a float data block
typedef union {
   GLubyte rgba[4];
   int value;
   float fvalue;
   struct {
      GLubyte r;
      GLubyte g;
      GLubyte b;
      GLubyte a;
   };
} Colour;

Colour parseRGBA(std::string value);

class TextureData  //Texture TGA image data
{
  public:
  GLuint   bpp;      // Image Color Depth In Bits Per Pixel.
  GLuint   width;    // Image Width
  GLuint   height;   // Image Height
  GLuint   depth;    // Image Depth
  GLuint   id;       // Texture ID Used To Select A Texture

  TextureData() : bpp(0), width(0), height(0), depth(0) {glGenTextures(1, &id);}
  ~TextureData() {glDeleteTextures(1, &id);}
};   

void compareCoordMinMax(float* min, float* max, float *coord);
void clearMinMax(float* min, float* max);
void getCoordRange(float* min, float* max, float* dims);

//Class for handling filenames/paths
class FilePath
{
  public:
   std::string full;
   std::string path;
   std::string file;
   std::string base;
   std::string ext;
   std::string type; //Same as ext but always lowercase

   FilePath() {};
   FilePath(std::string fp) {parse(fp);}

   void parse(std::string fp)
   {
      full = fp;
      //From: http://stackoverflow.com/a/8520815/866759
      // Remove directory if present.
      // Do this before extension removal incase directory has a period character.
      const size_t last_slash = full.find_last_of("\\/");
      if (std::string::npos != last_slash)
      {
         path = full.substr(0, last_slash + 1);
         file = full.substr(last_slash + 1);
      }
      else
      {
         file = full;
      }

      // Remove extension if present.
      const size_t period = file.rfind('.');
      if (std::string::npos != period)
      {
         base = file.substr(0, period);
         ext = file.substr(period + 1);
         type = ext;
         std::transform(type.begin(), type.end(), type.begin(), ::tolower);
      }
      else
      {
         base = file;
      }
   }
};

std::string GetBinaryPath(const char* argv0, const char* progname);

//General purpose geometry data store types...
typedef union
{
   float val;
   unsigned int idx;
} floatidx; /* For index values stored in float container */

class FloatValues
{
  public:
   std::vector<float> value;
   int next;
   float minimum;
   float maximum;
   int datasize;
   float dimCoeff;   //For scaling
   std::string units;      //Scaling units
   unsigned int offset;
   bool generated;
   static long membytes;  //Track memory usage
   static long mempeak;

   FloatValues() : next(0), minimum(0), maximum(0), datasize(1), dimCoeff(1.0), offset(0), generated(false) {}
   ~FloatValues() { if (value.size()) clear(); }

   virtual void read(unsigned int n, const void* data)
   {
      int size = next + n;
      int oldsize = value.size();
      if (oldsize < size)
      {
         //Always at least double size for efficiency
         if (size < oldsize*2) size = oldsize*2;
         resize(size);
      }
      memcpy(&value[next], data, n * sizeof(float));
      next += n;
   }

   void setup(float min, float max, float dimFactor, const char* units)
   {
      //if (min < minimum) minimum = min;
      //if (max > maximum) maximum = max;
      minimum = min;
      maximum = max;
      dimCoeff = dimFactor;
      this->units = std::string(units ? units : "");
   }

   inline float operator[] (unsigned i)
   {
      //if (i >= value.size())
      //   abort_program("Out of bounds %d -- %d (max idx %d)\n", i, i, value.size()-1);
      return value[i];
   }

   Colour toColour(unsigned i)
   {
      //Interpret a float value as RGBA colour
      Colour c;
      c.fvalue = value[i];
      return c;
   }

   void resize(unsigned long size)
   {
      int oldsize = value.size();
      if (oldsize < size)
      {
         value.resize(size);
         FloatValues::membytes += 4*(size-oldsize);
         if (FloatValues::membytes > FloatValues::mempeak) FloatValues::mempeak = FloatValues::membytes;
         //printf("============== MEMORY total %.3f mb, added %d ==============\n", FloatValues::membytes/1000000.0f, 4*(size-oldsize));
      }
   }

   int size() { return next; }
   void clear()
   {
      int count = value.size();
      if (count == 0) return;
      value.clear(); offset = 0;
      FloatValues::membytes -= 4*count;
      next = 0;
      //if (bytes) printf("============== MEMORY total %.3f mb, removed %d ==============\n", FloatValues::membytes/1000000.0f, bytes);
   }

   //Update saved position
   void setOffset() { offset = value.size();}

   void erase(int start, int end)
   {
      //erase elements:
      value.erase(value.begin()+start, value.begin()+end);
      if (offset > 0) offset -= start;
      FloatValues::membytes -= 4*(end - start);
      //printf("============== MEMORY total %.3f mb, erased %d ==============\n", FloatValues::membytes/1000000.0f, (end - start));
   }
};

class Coord3DValues : public FloatValues
{
  public:
   Coord3DValues() {datasize = 3;}

   void read(unsigned int n, const void* data)
   {
      FloatValues::read(n * 3, data);
   }

   inline float* operator[] (unsigned i)
   {
      //if (i*3 >= value.size()) 
      //   abort_program("Out of bounds %d -- %d (max idx %d)\n", i, i*3, value.size()-1);
      return &value[i * 3];
   }
};

class Coord2DValues : public FloatValues
{
  public:
   Coord2DValues() {datasize = 2;}

   void read(unsigned int n, const void* data)
   {
      FloatValues::read(n * 2, data);
   }

   inline float* operator[] (unsigned i)
   {
      //if (i*2 >= value.size()) 
      //   abort_program("Out of bounds %d -- %d (max idx %d)\n", i, i*2, value.size()-1);
      return &value[i * 2];
   }
};

//Generic 3d vector
class Vec3d
{
 public:
   float x;
   float y;
   float z;
   float* ref() {return &x;}

   Vec3d() : x(0), y(0), z(0) {}
   Vec3d(const Vec3d& copy) : x(copy.x), y(copy.y), z(copy.z) {}
   Vec3d(Vec3d* copy) : x(copy->x), y(copy->y), z(copy->z) {}
   Vec3d(float val) : x(val), y(val), z(val) {}
   Vec3d(float x, float y, float z) : x(x), y(y), z(z) {}
   Vec3d(float pos[3]) : x(pos[0]), y(pos[1]), z(pos[2]) {}

   float& operator[] (unsigned int i)
   {
      if (i==0) return x;
      if (i==1) return y;
      return z;
   }

   Vec3d operator-() const
   {
      return Vec3d(-x, -y, -z);
   }

   Vec3d operator+(const Vec3d& rhs) const
   {
      return Vec3d(x + rhs.x, y + rhs.y, z + rhs.z);
   }

   Vec3d& operator+=(const Vec3d& rhs)
   {
      x += rhs.x; y += rhs.y; z += rhs.z;
      return *this;
   }

   Vec3d operator-(const Vec3d& rhs) const
   {
      return Vec3d(x - rhs.x, y - rhs.y, z - rhs.z);
   }

   Vec3d& operator-=(const Vec3d& rhs)
   {
      x -= rhs.x; y -= rhs.y; z -= rhs.z;
      return *this;
   }

   Vec3d& operator=(const Vec3d& rhs)
   {
      x = rhs.x; y = rhs.y; z = rhs.z;
      return *this;
   }

   Vec3d operator*(const Vec3d& rhs) const
   {
      return Vec3d(x * rhs.x, y * rhs.y, z * rhs.z);
   }

   Vec3d& operator*=(const Vec3d& rhs)
   {
      x *= rhs.x; y *= rhs.y; z *= rhs.z;
      return *this;
   }

   Vec3d operator*(const float& scalar) const
   {
      return Vec3d(x * scalar, y * scalar, z * scalar);
   }

   Vec3d& operator*=(const float& scalar)
   {
      x *= scalar; y *= scalar; z *= scalar;
      return *this;
   }

   Vec3d& cross(const Vec3d& rhs)
   {
      float tempx = y * rhs.z - rhs.y * z;
      float tempy = z * rhs.x - rhs.z * x;
      z = x * rhs.y - rhs.x * y;
      x = tempx;
      y = tempy;
      return *this;
   }

   float dot(const Vec3d& rhs) const
   {
      return x * rhs.x + y * rhs.y + z * rhs.z;
   }

   float magnitude() const
   {
      return sqrt(dot(*this));
   }

   void normalise()
   {
      float mag = magnitude();
      x /= mag;
      y /= mag;
      z /= mag;
   }

   //Returns angle in radians between this and another vector
   // cosine of angle between vectors = (v1 . v2) / |v1|.|v2|
   float angle(const Vec3d& other)
   {
      float result = dot(other) / (magnitude() * other.magnitude());
      if (result >= -1.0 && result <= 1.0)
         return acos(result);
      else
         return 0;
   }

   bool operator==(const Vec3d &rhs) const
   {
      return equals(rhs, 1e-8f);
   }

   bool equals(const Vec3d &rhs, float epsilon) const
   {
      //Comparison for equality
      return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon && fabs(z - rhs.z) < epsilon;
   }

   bool operator<(const Vec3d &rhs) const
   {
      //Comparison for vertex sort
      if (x != rhs.x) return x < rhs.x;
      if (y != rhs.y) return y < rhs.y;
      return z < rhs.z;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Vec3d& vec);
};

std::ostream & operator<<(std::ostream &os, const Colour& colour);
std::ostream & operator<<(std::ostream &os, const Vec3d& vec);

Vec3d vectorNormalToPlane(float pos0[3], float pos1[3], float pos2[3]);

/* Quaternion utility functions -
 * easily store rotations and apply to vectors
 * will be used for new camera functions
 * and wherever rotations need to be saved
 */
class Quaternion
{
   float matrix[16];
  public:
   float x;
   float y;
   float z;
   float w;

   Quaternion() {identity();}
   Quaternion(const Quaternion& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}
   Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

   float& operator[] (unsigned int i)
   {
      if (i==0) return x;
      if (i==1) return y;
      if (i==2) return z;
      return w;
   }

   void identity() {x = y = z = 0.0f; w = 1.0f;}

   void set(float x, float y, float z, float w)
   {
      this->x = x;
      this->y = y;
      this->z = z;
      this->w = w;
   }

   /* Convert from Axis Angle */
   void fromAxisAngle(Vec3d& v, float angle)
   {
      angle *= 0.5f * DEG2RAD;
      float sinAngle = sin(angle);
      Vec3d vn(v);
      vn.normalise();
      vn *= sinAngle;

      x = vn.x;
      y = vn.y;
      z = vn.z;
      w = cos(angle);
   }

   /* Convert to Axis Angle */
   void getAxisAngle(Vec3d& axis, float& angle)
   {
      float scale = sqrt(x * x + y * y + z * z);
      axis.x = x / scale;
      axis.y = y / scale;
      axis.z = z / scale;
      angle = acos(w) * 2.0f * RAD2DEG;
   }

   /* Multiplying q1 with q2 applies the rotation q2 to q1 */
   Quaternion operator*(const Quaternion& rhs) const
   {
      return Quaternion(
         w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
         w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
         w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
         w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
      );
   }

   /* We need to get the inverse of a quaternion to properly apply a quaternion-rotation to a vector
   * The conjugate of a quaternion is the same as the inverse, as long as the quaternion is unit-length */
   void conjugate()
   {
      x = -x;
      y = -y;
      z = -z;
   }

   /* Multiplying a quaternion q with a vector v applies the q-rotation to v */
   Vec3d operator* (const Vec3d &vec) const
   {
      /*/https://molecularmusings.wordpress.com/2013/05/24/a-faster-quaternion-vector-multiplication/
      Vec3d q = Vec3d(x, y, z);
      Vec3d t = q.cross(vec) * 2.0;
      Vec3d r = vec + (t * w) + q.cross(t);
      return r;*/

      Vec3d vn(vec);
      float mag = vn.magnitude();
      vn.normalise();

      Quaternion vecQuat(vn.x, vn.y, vn.z, 0.0f);
      Quaternion conj(-x, -y, -z, w);
      /* result = q * v * q^ */
      Quaternion result = vecQuat * conj;
      result = *this * result;

      /* Get transformed vector (which is normalised) and restore scale */
      return Vec3d(result.x*mag, result.y*mag, result.z*mag);
      //return Vec3d(result.x, result.y, result.z);
   }

   float magnitude()
   {
      return sqrt(x * x + y * y + z * z + w * w);
   }

   /* Quaternions store scale as well as rotation, normalizing removes scaling */
   void normalise()
   {
      float length = magnitude();
      if (length > 0.0 && length != 1.0 )
      {
         float scale = (1.0f / length);
         x *= scale;
         y *= scale;
         z *= scale;
         w *= scale;
      }
   }

   /* Returns Quaternion to aim the Z-Axis along the vector v */
   void aimZAxis(Vec3d& v)
   {
      Vec3d vn(v);
      vn.normalise();

      set(vn.y, -vn.x, 0, 1.0f + vn.z);

      if (x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f )
      {
         x = 0;
         y = 1.0f;
         w = 0; /* If we can't normalize, just set it */
      }
      else
         normalise();
   }

   /* quaternion to aim vector from --> to */
   void aim(Vec3d& from, Vec3d& to)
   {
      /* get axis of rotation */
      Vec3d axis, cr(from);
      axis = cr.cross(to);
      float dot = from.dot(to);

      /* get scaled cos of angle between vectors and set initial quaternion */
      set(axis.x, axis.y, axis.z, dot);

      /* normalize to get cos theta, sin theta r */
      normalise();

      /* set up for half angle calculation */
      w += 1.0f;

      /* if vectors are opposing */
      if (w <= 0.000001f)
      {
         /* find orthogonal vector
          * take cross product with x axis */
         if (from.z*from.z > from.x*from.z)
            set(0.0f, 0.0f, from.z, -from.y);
         /* or take cross product with z axis */
         else
            set(0.0f, from.y, -from.x, 0.0f);
      }

      /* normalize again to get rotation quaternion */
      normalise();
   }

   // Convert to Matrix
   float* getMatrix()
   {
      // This calculation would be a lot more complicated for non-unit length quaternions
      // Note: expects the matrix in column-major format like expected by OpenGL
      float x2 = x + x;
      float y2 = y + y;
      float z2 = z + z;

      float xx2 = x * x2;
      float xy2 = x * y2;
      float xz2 = x * z2;
      float yy2 = y * y2;
      float yz2 = y * z2;
      float zz2 = z * z2;
      float wx2 = w * x2;
      float wy2 = w * y2;
      float wz2 = w * z2;

      matrix[0] = 1 - (yy2 + zz2);
      matrix[1] = xy2 + wz2;
      matrix[2] = xz2 - wy2;
      matrix[3] = 0;

      matrix[4] = xy2 - wz2;
      matrix[5] = 1 - (xx2 + zz2);
      matrix[6] = yz2 + wx2;
      matrix[7] = 0;

      matrix[8] = xz2 + wy2;
      matrix[9] = yz2 - wx2;
      matrix[10] = 1 - (xx2 + yy2);
      matrix[11] = 0;

      matrix[12] = 0;
      matrix[13] = 0;
      matrix[14] = 0;
      matrix[15] = 1;

      return matrix;
   }

   void apply()
   {
      glMultMatrixf(getMatrix());
   }

   void toEuler(float& bank, float& heading, float& attitude)
   {
      float test = x*y + z*w;
      if (test > 0.499)
      {
         // singularity at north pole
         heading = 2 * atan2(x,w) * RAD2DEG;
         attitude = M_PI/2 * RAD2DEG;
         bank = 0;
         return;
      }
      if (test < -0.499)
      {
         // singularity at south pole
         heading = -2 * atan2(x,w) * RAD2DEG;
         attitude = - M_PI/2 * RAD2DEG;
         bank = 0;
         return;
      }
      float sqx = x*x;
      float sqy = y*y;
      float sqz = z*z;
      heading = atan2(2*y*w-2*x*z , 1 - 2*sqy - 2*sqz) * RAD2DEG;
      attitude = asin(2*test) * RAD2DEG;
      bank = atan2(2*x*w-2*y*z , 1 - 2*sqx - 2*sqz) * RAD2DEG;
   }
}; 

//Property containers now using json
void jsonParseProperties(std::string& properties, json::Object& object);
void jsonParseProperty(std::string& data, json::Object& object);

//Utility class for parsing property,value strings
typedef std::vector<std::string> prop_values;
typedef std::map<std::string, prop_values> prop_value_map;
typedef std::map<std::string, prop_values>::iterator prop_iterator_type;
class PropertyParser
{
   prop_value_map props;
   bool ignoreCase;

 public:
   PropertyParser() : ignoreCase(true) {};

   PropertyParser(std::istream& is, char delim, bool ic=true) : ignoreCase(ic) {parse(is, delim);}
   PropertyParser(std::istream& is, bool ic=true) : ignoreCase(ic) {parse(is);}
   
   void stringify(std::ostream& os, char delim='=')
   {
      for (prop_iterator_type iterator = props.begin(); iterator != props.end(); iterator++)
         os << iterator->first << delim << get(iterator->first) << std::endl;
   } 

   //Parse lines with delimiter, ie: key=value
   void parse(std::istream& is, char delim)
   {
      std::string line;
      while(std::getline(is, line))
      {
         parseLine(line, delim);
      }
   }

   //Parse lines as whitespace separated eg: " key   value"
   void parse(std::istream& is)
   {
      std::string line;
      while(std::getline(is, line))
      {
         parseLine(line);
      }
   }

   //Parse lines with delimiter, ie: key=value
   void parseLine(std::string& line, char delim)
   {
      std::istringstream iss(line);
      std::string temp, key, value;

      std::getline(iss, temp, delim);
      
      std::istringstream isskey(temp);
      isskey >> key;

      //Read values into vector until stream empty
      props[key] = prop_values();
      do
      {
         std::getline(iss, temp, delim);

         std::istringstream issval(temp);
         issval >> value;

         if (ignoreCase)
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);

         props[key].push_back(value);
         //std::cerr << "Key " << key << " == " << value << std::endl;
      } while (iss.good());
      //std::cerr << props[key].size() << " values added to key: " << key << " [0] = " << props[key][0] << std::endl;
   }

   //Parse lines as whitespace separated eg: " key   value"
   void parseLine(std::string& line)
   {
      std::istringstream iss(line);
      std::string key, value;

      iss >> key;
      if (ignoreCase)
         std::transform(key.begin(), key.end(), key.begin(), ::tolower);
      props[key] = prop_values();

      //Read values into vector until stream empty
      while (iss.good())
      {
         iss >> value;
         //Detect quotes, if found read until end quote if any
         if (value.length() > 0 && value.at(0) == '"')
         {
            size_t start = line.find('"');
            size_t end = line.find('"', start+1);
            if (end <= start) continue;
            iss.seekg(end+2, std::ios_base::beg);
            value = line.substr(start+1, end-start-1);
         }
         props[key].push_back(value);
         //std::cerr << key << " => " << value << std::endl;
      }
   }

   std::string get(std::string key, unsigned int idx=0)
   {
      if (ignoreCase)
         std::transform(key.begin(), key.end(), key.begin(), ::tolower);
      if (props.find(key) != props.end()) 
      {
        if (idx < props[key].size())
          return props[key][idx];
      }
      return std::string("");
   }

   bool exists(std::string key)
   {
      if (ignoreCase)
         std::transform(key.begin(), key.end(), key.begin(), ::tolower);
      if (props.find(key) != props.end()) return true;
      return false;
   }

   std::string operator[] (std::string key)
   {
      return get(key);
   }

   int Int(std::string key, int def=0, unsigned int idx=0)
   {
      std::stringstream parsess(get(key, idx));
      int val;
      if (!(parsess >> val))
        return def;
      return val;
   }

   unsigned int Hex(std::string key, unsigned int idx=0)
   {
      std::stringstream parsess(get(key, idx));
      unsigned int val;
      parsess >> std::hex >> val;
      return val;
   }

   unsigned int Colour(std::string key, unsigned int idx=0)
   {
      //Parse colour as RRGGBBAA hex string:
      //reverse order to get (AA)RRGGBB little endian hex, convert to ARGB int
      std::string str = get(key, idx);
      int len = str.length();
      if (len != 6 && len != 8) return LUC_BLACK;
      if (str.length() == 6) str += "ff";

      //Reverse components but preserve order of each 2 letter code
      if (str.length() == 6) //Alpha optional
        str = "ff" + str.substr(4,2) + str.substr(2,2) + str.substr(0,2);
      else
        str = str.substr(6,2) + str.substr(4,2) + str.substr(2,2) + str.substr(0,2);

      std::stringstream parsess(str);
      unsigned int val;
      parsess >> std::hex >> val;

      return val;
   }

   float Float(std::string key, float def=0.0, unsigned int idx=0)
   {
      std::stringstream parsess(get(key, idx));
      float val;
      if (!(parsess >> val))
        return def;
      return val;
   }

   bool Bool(std::string key, bool def=false, unsigned int idx=0)
   {
      std::stringstream parsess(get(key, idx));
      bool val;
      //Bools only parsed as integer, not "true" "false"
      //if (!(parsess >> std::boolalpha >> val)
      if (!(parsess >> val)) return def;
      return val;
   }

   bool has(int& val, std::string key, unsigned int idx=0)
   {
      std::stringstream parsess(get(key, idx));
      parsess >> val;
      return !parsess.fail();
   }

   bool has(float& val, std::string key, unsigned int idx=0)
   {
      std::stringstream parsess(get(key, idx));
      parsess >> val;
      return !parsess.fail();
   }

   bool has(bool& val, std::string key, unsigned int idx=0)
   {
      std::stringstream parsess(get(key, idx));
      parsess >> val;
      return !parsess.fail();
      //return (parsess >> std::boolalpha >> val);
   }


};

const char* glErrorString(GLenum errorCode);
int gluProjectf(float objx, float objy, float objz, float *windowCoordinate);
int gluProjectf(float objx, float objy, float objz, float* modelview, float*projection, int* viewport, float *windowCoordinate);
bool gluInvertMatrixf(const float m[16], float invOut[16]);

void Viewport2d(int width, int height);

//3d fonts
void PrintSetColour(int colour);
void PrintString(const char* str);
void Printf(int x, int y, float scale, const char *fmt, ...);
void Print(int x, int y, float scale, const char *str);
void Print3d(double x, double y, double z, double scale, const char *str);
void Print3dBillboard(double x, double y, double z, double scale, const char *str);
int PrintWidth(const char *string, float scale);
void DeleteFont();

//Bitmap texture fonts
void lucPrintString(const char* str);
void lucPrintf(int x, int y, const char* fmt, ...);
void lucPrint(int x, int y, const char* str);
void lucPrint3d(double x, double y, double z, const char *str);
void lucSetFontCharset(int charset);
int lucPrintWidth(const char *string);
void lucSetupRasterFont();
void lucBuildFont(int glyphsize, int columns, int startidx, int stopidx);
void lucDeleteFont();

void drawCuboid(float pos[3], float width, float height, float depth, bool filled=true, float linewidth=1.0f);
void drawCuboid(float min[3], float max[3], bool filled=true, float linewidth=1.0f);
void drawNormalVector( float pos[3], float vector[3], float scale);
void vectorNormalise(float vector[3]);
void normalToPlane( float normal[3], float pos0[3], float pos1[3], float pos2[3]);
float triAngle(float v0[3], float v1[3], float v2[3]);

void Colour_SetColour(Colour* colour);
void Colour_Invert(Colour& colour);
void Colour_SetXOR(bool switchOn);
Colour Colour_FromJson(json::Object& object, std::string key, GLubyte red=0, GLubyte green=0, GLubyte blue=0, GLubyte alpha=255);
Colour Colour_FromJson(json::Value& value, GLubyte red=0, GLubyte green=0, GLubyte blue=0, GLubyte alpha=255);
json::Value Colour_ToJson(int colourval);
json::Value Colour_ToJson(Colour& colour);
void Colour_ToArray(Colour colour, float* array);
void Colour_SetUniform(GLint uniform, Colour colour);

void calcCircleCoords(int segment_count);
void drawSphere_(float centre[3], float radius, int segment_count, Colour* colour);
void drawEllipsoid_(float centre[3], float radiusX, float radiusY, float radiusZ, int segment_count, Colour* colour);
void drawVector3d_( float pos[3], float vector[3], float scale, float radius, float head_scale, int segment_count, Colour *colour0, Colour *colour1);
void drawTrajectory_(float coord0[3], float coord1[3], float radius, float arrowHeadSize, int segment_count, float scale[3], Colour *colour0, Colour *colour1, float maxLength=HUGE_VAL);

void RawImageFlip(void* image, int width, int height, int bpp);
int LoadTextureTGA(TextureData *texture, const char *filename, bool mipmaps, GLenum mode);
int LoadTexturePPM(TextureData *texture, const char *filename, bool mipmaps, GLenum mode);
int LoadTexturePNG(TextureData *texture, const char *filename, bool mipmaps, GLenum mode);
int LoadTextureJPEG(TextureData *texture, const char *filename, bool mipmaps, GLenum mode);
int BuildTexture(TextureData *texture, GLubyte* imageData , bool mipmaps, GLenum format, GLenum mode);

void writeImage(GLubyte *image, int width, int height, const char* basename, bool transparent);

#ifdef HAVE_LIBPNG
//PNG utils
void write_png(std::ostream& stream, int bpp, int width, int height, void* data);
void* read_png(std::istream& stream, GLuint& bpp, GLuint& width, GLuint& height);
#else
#define read_png(stream, bpp, width, height) NULL
#endif

#endif //GraphicsUtil__
