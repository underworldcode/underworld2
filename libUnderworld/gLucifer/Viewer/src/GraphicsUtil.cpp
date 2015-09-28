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
#if defined HAVE_LIBPNG or defined _WIN32
#include <png.h>
#include <zlib.h>
#endif

#include "GraphicsUtil.h"
#include <string.h>
#include <math.h>

#ifdef USE_FONTS
#include  "font.h"
#include  "FontSans.h"
#endif

#ifdef HAVE_GL2PS
#include <gl2ps.h>
#endif

FILE* infostream = NULL;

Colour fontColour;

long FloatValues::membytes = 0;
long FloatValues::mempeak = 0;

int segments__ = 0;    // Saves segment count for circle based objects
float *x_coords_ = NULL, *y_coords_ = NULL;  // Saves arrays of x,y points on circle for set segment count

unsigned int fontbase = 0, fontcharset = FONT_DEFAULT, fonttexture;

void compareCoordMinMax(float* min, float* max, float *coord)
{
   for (int i=0; i<3; i++)
   {
      if (coord[i] > max[i] && coord[i] < HUGE_VAL) 
      {
         max[i] = coord[i];
         //std::cerr << Vec3d(max) << std::endl;
      }
      if (coord[i] < min[i] && coord[i] > -HUGE_VAL) 
      {
         min[i] = coord[i];
         //std::cerr << Vec3d(min) << std::endl;
      }
   }
}

void clearMinMax(float* min, float* max)
{
   for (int i=0; i<3; i++)
   {
      min[i] = HUGE_VAL;
      max[i] = -HUGE_VAL;
   }
}

void getCoordRange(float* min, float* max, float* dims)
{
   for (int i=0; i<3; i++)
   {
      dims[i] = max[i] - min[i];
   }
}

std::string GetBinaryPath(const char* argv0, const char* progname)
{
   //Try the PATH env var if argv0 contains no path info
   FilePath xpath;
   if (!argv0 || strlen(argv0) == 0 || strcmp(argv0, progname) == 0)
   {
      std::stringstream path(getenv("PATH"));
      std::string line;
      while (std::getline(path, line, ':'))
      {
         std::stringstream pathentry;
         pathentry << line << "/" << argv0;
         std::string pstr = pathentry.str();
         const char* pathstr = pstr.c_str();
#ifdef _WIN32
         if (strstr(pathstr, ".exe"))
#else
         if (access(pathstr, X_OK) == 0)
#endif
         {
            xpath.parse(pathstr);
            break;
         }
      }
   }
   else
   {
      xpath.parse(argv0);
   }
   return xpath.path;
}

//Parse multi-line string
void jsonParseProperties(std::string& properties, json::Object& object)
{
   //Process all lines
   std::stringstream ss(properties);
   std::string line;
   while (std::getline(ss, line))
      jsonParseProperty(line, object);
}

//Property containers now using json
void jsonParseProperty(std::string& data, json::Object& object)
{
   //Parse a key=value property where value is a json object
   std::string key, value;
   std::istringstream iss(data);
   std::getline(iss, key, '=');
   std::getline(iss, value, '=');

   if (value.length() > 0)
   {
      //Ignore case
      std::transform(key.begin(), key.end(), key.begin(), ::tolower);
      //std::cerr << "Key " << key << " == " << value << std::endl;

      try
      {
         if (value.find("[") == std::string::npos && value.find("{") == std::string::npos)
         {
            //This JSON parser only accepts objects or arrays as base element
            std::string nvalue = "[" + value + "]";
            object[key] = json::Deserialize(nvalue).ToArray()[0];
         }
         else
         {
            object[key] = json::Deserialize(value);
         }
      }
      catch (std::exception& e)
      {
         //std::cerr << "[" << key << "] " << data << " : " << e.what();
         //Treat as a string value
         object[key] = json::Value(value);
      }
   }
}

Colour Colour_FromJson(json::Object& object, std::string key, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
   Colour colour = {red, green, blue, alpha};
   if (!object.HasKey(key)) return colour;
   return Colour_FromJson(object[key], red, green, blue, alpha);
}

Colour Colour_FromJson(json::Value& value, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
   Colour colour = {red, green, blue, alpha};
   //Will accept integer colour or [r,g,b,a] array
   if (value.GetType() == json::IntVal)
   {
      colour.value = value.ToInt();
   }
   else if (value.GetType() == json::ArrayVal)
   {
      json::Array array = value.ToArray();
      if (array[0].ToFloat(0) > 1.0)
         colour.r = array[0].ToInt(0);
      else
         colour.r = array[0].ToFloat(0)*255.0;
      if (array[1].ToFloat(0) > 1.0)
         colour.g = array[1].ToInt(0);
      else
         colour.g = array[1].ToFloat(0)*255.0;
      if (array[2].ToFloat(0) > 1.0)
         colour.b = array[2].ToInt(0);
      else
         colour.b = array[2].ToFloat(0)*255.0;

      if (array.size() > 3)
         colour.a = array[3].ToFloat(0)*255.0;
   }

   return colour;
}

json::Value Colour_ToJson(int colourval)
{
   Colour colour;
   colour.value = colourval;
   return Colour_ToJson(colour);
}

json::Value Colour_ToJson(Colour& colour)
{
   json::Array array;
   array.push_back(colour.r/255.0); 
   array.push_back(colour.g/255.0); 
   array.push_back(colour.b/255.0); 
   array.push_back(colour.a/255.0); 
   return array;
}

void Colour_SetUniform(GLint uniform, Colour colour)
{
   float array[4];
   Colour_ToArray(colour, array);
   glUniform4fv(uniform, 1, array);
}

void Colour_ToArray(Colour colour, float* array)
{
   array[0] = colour.r/255.0;
   array[1] = colour.g/255.0;
   array[2] = colour.b/255.0;
   array[3] = colour.a/255.0;
}

void debug_print(const char *fmt, ...)
{
   if (fmt == NULL || infostream == NULL) return;
   va_list args;
   va_start(args, fmt);
   //vprintf(fmt, args);
   vfprintf(infostream, fmt, args);
   //debug_print("\n");
   va_end(args);
}

const char* glErrorString(GLenum errorCode)
{
    switch (errorCode)
    {
      case GL_NO_ERROR:
         return "No error";
      case GL_INVALID_ENUM:
         return "Invalid enumerant";
      case GL_INVALID_VALUE:
         return "Invalid value";
      case GL_INVALID_OPERATION:
         return "Invalid operation";
      case GL_STACK_OVERFLOW:
         return "Stack overflow";
      case GL_STACK_UNDERFLOW:
         return "Stack underflow";
      case GL_OUT_OF_MEMORY:
         return "Out of memory";
   }
   return "Unknown error";
}

int gluProjectf(float objx, float objy, float objz, float *windowCoordinate)
{
   //https://www.opengl.org/wiki/GluProject_and_gluUnProject_code
   int viewport[4];
   float projection[16], modelview[16];
   glGetIntegerv(GL_VIEWPORT, viewport);
   glGetFloatv(GL_PROJECTION_MATRIX, projection);
   glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
   return gluProjectf(objx, objy, objz, modelview, projection, viewport, windowCoordinate);
}

int gluProjectf(float objx, float objy, float objz, float* modelview, float*projection, int* viewport, float *windowCoordinate)
{
   //https://www.opengl.org/wiki/GluProject_and_gluUnProject_code
   //Transformation vectors
   float fTempo[8];
   //Modelview transform
   fTempo[0]=modelview[0]*objx+modelview[4]*objy+modelview[8]*objz+modelview[12];  //w is always 1
   fTempo[1]=modelview[1]*objx+modelview[5]*objy+modelview[9]*objz+modelview[13];
   fTempo[2]=modelview[2]*objx+modelview[6]*objy+modelview[10]*objz+modelview[14];
   fTempo[3]=modelview[3]*objx+modelview[7]*objy+modelview[11]*objz+modelview[15];
   //Projection transform, the final row of projection matrix is always [0 0 -1 0]
   //so we optimize for that.
   fTempo[4]=projection[0]*fTempo[0]+projection[4]*fTempo[1]+projection[8]*fTempo[2]+projection[12]*fTempo[3];
   fTempo[5]=projection[1]*fTempo[0]+projection[5]*fTempo[1]+projection[9]*fTempo[2]+projection[13]*fTempo[3];
   fTempo[6]=projection[2]*fTempo[0]+projection[6]*fTempo[1]+projection[10]*fTempo[2]+projection[14]*fTempo[3];
   fTempo[7]=-fTempo[2];
   //The result normalizes between -1 and 1
   if(fTempo[7]==0.0)	//The w value
      return 0;
   fTempo[7]=1.0/fTempo[7];
   //Perspective division
   fTempo[4]*=fTempo[7];
   fTempo[5]*=fTempo[7];
   fTempo[6]*=fTempo[7];
   //Window coordinates
   //Map x, y to range 0-1
   windowCoordinate[0]=(fTempo[4]*0.5+0.5)*viewport[2]+viewport[0];
   windowCoordinate[1]=(fTempo[5]*0.5+0.5)*viewport[3]+viewport[1];
   //This is only correct when glDepthRange(0.0, 1.0)
   windowCoordinate[2]=(1.0+fTempo[6])*0.5;	//Between 0 and 1
   return 1;
}

/*
** Modified from MESA GLU 9.0.0 src/libutil/project.c
** (SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008))
** License URL as required: http://oss.sgi.com/projects/FreeB/
**
** Invert 4x4 matrix.
** Contributed by David Moore (See Mesa bug #6748)
*/
bool gluInvertMatrixf(const float m[16], float invOut[16])
{
    float inv[16], det;
    int i;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

void Viewport2d(int width, int height)
{
   if (width && height)
   {
      // Set up 2D Viewer the size of the viewport
      glPushAttrib(GL_ENABLE_BIT);
      glDisable( GL_DEPTH_TEST );
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      //Left, right, bottom, top, near, far
      glOrtho(0.0, (GLfloat) width, 0.0, (GLfloat) height, -1.0f, 1.0f);
      
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      
      // Disable lighting
      glDisable(GL_LIGHTING);
      // Disable line smoothing in 2d mode
      glDisable(GL_LINE_SMOOTH);
   }
   else
   {
      // Restore settings
      glPopAttrib();
      //glEnable(GL_LINE_SMOOTH);
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
   }
}

#ifdef USE_FONTS
void PrintSetColour(int colour)
{
   fontColour.value = colour;
}

void PrintString(const char* str)
{
   if (charLists == 0 || !glIsList(charLists+1))   // Load font if not yet done
      GenerateFontCharacters();
   
   fontColour.rgba[3] = 255;
   glColor4ubv(fontColour.rgba);
   glDisable(GL_CULL_FACE);
   glListBase(charLists - 32);      // Set font display list base (space)
   glCallLists(strlen(str),GL_UNSIGNED_BYTE, str);      // Display
}

void Printf(int x, int y, float scale, const char *fmt, ...)
{
   char text[512];
   va_list ap;                 // Pointer to arguments list
   if (fmt == NULL) return;    // No format string
   va_start(ap, fmt);          // Parse format string for variables
   vsprintf(text, fmt, ap);    // Convert symbols
   va_end(ap);

   Print(x, y, scale, text);   // Print result string
}

void Print(int x, int y, float scale, const char *str)
{
   glPushMatrix();
   //glLoadIdentity();
   glTranslated(x, y, 0);
      glScalef(scale, scale, scale);
   PrintString(str);
   glPopMatrix();
}

void Print3d(double x, double y, double z, double scale, const char *str)
{
   glPushMatrix();
   //glLoadIdentity();
   glTranslated(x, y, z);
      glScaled(scale * 0.01, scale * 0.01, scale * 0.01);
   PrintString(str);
   glPopMatrix();
}

void Print3dBillboard(double x, double y, double z, double scale, const char *str)
{
   float modelview[16];
   int i,j;

   // save the current modelview matrix
   glPushMatrix();
   glTranslated(x, y, z);

   // get the current modelview matrix
   glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

   // undo all rotations
   // beware all scaling is lost as well 
   for( i=0; i<3; i++ ) 
      for( j=0; j<3; j++ ) {
         if ( i==j )
            modelview[i*4+j] = 1.0;
         else
            modelview[i*4+j] = 0.0;
      }

   // set the modelview with no rotations and scaling
   glLoadMatrixf(modelview);

   glScaled(scale * 0.01, scale * 0.01, scale * 0.01);
   PrintString(str);

   // restores the modelview matrix
   glPopMatrix();
}

// String width calc 
int PrintWidth(const char *string, float scale)
{
   // Sum character widths in string
   int i, len = 0, slen = strlen(string);
   for (i = 0; i < slen; i++)
      len += font_charwidths[string[i]-32];

   return scale * len;
}

void DeleteFont()
{
   // Delete fonts
    if (charLists > 0) glDeleteLists(charLists, GLYPHS);
    charLists = 0;
}

//Bitmap font stuff
void lucPrintString(const char* str)
{
   if (fontbase == 0)                        /* Load font if not yet done */
      lucSetupRasterFont();

   if (fontcharset > FONT_SERIF || fontcharset < FONT_FIXED)      /* Character set valid? */
      fontcharset = FONT_FIXED;

   /* First save state of enable flags */
   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_LIGHTING);
   glDisable(GL_CULL_FACE);

   glEnable(GL_TEXTURE_2D);                            /* Enable Texture Mapping */
   glBindTexture(GL_TEXTURE_2D, fonttexture);
   glListBase(fontbase - 32 + (96 * fontcharset));      /* Choose the font and charset */
   glCallLists(strlen(str),GL_UNSIGNED_BYTE, str);      /* Display */
   glDisable(GL_TEXTURE_2D);                           /* Disable Texture Mapping */

   glPopAttrib();
}

void lucPrintf(int x, int y, const char *fmt, ...)
{
   char    text[512];
   va_list ap;                 /* Pointer to arguments list */
   if (fmt == NULL) return;    /* No format string */
   va_start(ap, fmt);          /* Parse format string for variables */
   vsprintf(text, fmt, ap);    /* Convert symbols */
   va_end(ap);
   lucPrint(x, y, text);       /* Print result string */
}

void lucPrint(int x, int y, const char *str)
{
#ifdef HAVE_GL2PS
int mode;
glGetIntegerv(GL_RENDER_MODE, &mode);
if (mode == GL_FEEDBACK)
{
   /* call to gl2pText is required for text output using vector formats,
    * as no text is stored in the GL feedback buffer */
   glRasterPos2d(x, y+bmpfont_charheights[fontcharset]);
   switch (fontcharset) 
   {
   case FONT_FIXED:
      gl2psText( str, "Courier", 12);
      break;
   case FONT_SMALL:
      gl2psText( str, "Helvetica", 8);
      break;
   case FONT_NORMAL:
      gl2psText( str, "Helvetica", 14);
      break;
   case FONT_SERIF:
      gl2psText( str, "Times-Roman", 14);
      break;
   }
   return;
}
#endif

   glPushMatrix();
   glLoadIdentity();
   glTranslated(x, y-bmpfont_charheights[fontcharset], 0);
   lucPrintString(str);
   glPopMatrix();
}

void lucPrint3d(double x, double y, double z, const char *str)
{
   /* Calculate projected screen coords in viewport */
   float pos[3];
   GLint viewportArray[4];
   glGetIntegerv(GL_VIEWPORT, viewportArray);
   gluProjectf(x, y, z, pos);

   /* Switch to ortho view with 1 unit = 1 pixel and print using calculated screen coords */
   Viewport2d(viewportArray[2], viewportArray[3]);
   glDepthFunc(GL_ALWAYS);
   glAlphaFunc(GL_GREATER, 0.25);
   glEnable(GL_ALPHA_TEST);

   /* Print at calculated position, compensating for viewport offset */
   int xs, ys;
   xs = (int)(pos[0]) - viewportArray[0];
   ys = (int)(pos[1]) - viewportArray[1]; //(viewportArray[3] - (yPos - viewportArray[1]));
   lucPrint(xs, ys, str);

   /* Restore state */
   Viewport2d(0, 0);
   /* Put back settings */
   glDepthFunc(GL_LESS);
   glDisable(GL_ALPHA_TEST);
}

void lucSetFontCharset(int charset)
{
   fontcharset = charset;
}

/* String width calc */
int lucPrintWidth(const char *string)
{
   /* Sum character widths in string */
   int i, len = 0, slen = strlen(string);
   for (i = 0; i < slen; i++)
      len += bmpfont_charwidths[string[i]-32 + (96 * fontcharset)];

   return len + slen;   /* Additional pixel of spacing for each character */
}

void lucSetupRasterFont()
{
   /* Load font bitmaps and Convert To Textures */
   int i, j;
   unsigned char pixel_data[IMAGE_HEIGHT * IMAGE_WIDTH * IMAGE_BYTES_PER_PIXEL];
   unsigned char fontdata[IMAGE_HEIGHT][IMAGE_WIDTH];   /* font texture data */

   /* Get font pixels from source data - interpret RGB (greyscale) as alpha channel */
   IMAGE_RUN_LENGTH_DECODE(pixel_data, IMAGE_RLE_PIXEL_DATA, IMAGE_WIDTH * IMAGE_HEIGHT, IMAGE_BYTES_PER_PIXEL);
   for (i = 0; i < IMAGE_HEIGHT; i++)
      for (j = 0; j < IMAGE_WIDTH; j++)
         fontdata[ i ][ j ] = 255 - pixel_data[ IMAGE_BYTES_PER_PIXEL * (IMAGE_WIDTH * i + j) ];

   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   /* create and bind texture */
   glGenTextures(1, &fonttexture);
   glBindTexture(GL_TEXTURE_2D, fonttexture);
   glEnable(GL_COLOR_MATERIAL);
   /* use linear filtering */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   /* generate the texture from bitmap alpha data */
   glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_ALPHA, GL_UNSIGNED_BYTE, fontdata);
   fontbase = glGenLists(BMP_GLYPHS);

   /* Build font display lists */
   lucBuildFont(16, 16, 0, 384);      /* 16x16 glyphs, 16 columns - 4 fonts */
}

void lucBuildFont(int glyphsize, int columns, int startidx, int stopidx)
{
   /* Build font display lists */
   int i;
   static float yoffset;
   float divX = IMAGE_WIDTH / (float)glyphsize;
   float divY = IMAGE_HEIGHT / (float)glyphsize;
   float glyphX = 1 / divX;   /* Width & height of a glyph in texture coords */
   float glyphY = 1 / divY;
   GLfloat cx, cy;         /* the character coordinates in our texture */
   if (startidx == 0) yoffset = 0;
   glBindTexture(GL_TEXTURE_2D, fonttexture);
   for (i = 0; i < (stopidx - startidx); i++)
   {
      cx = (float) (i % columns) / divX;
      cy = yoffset + (float) (i / columns) / divY;
      glNewList(fontbase + startidx + i, GL_COMPILE);
      glBegin(GL_QUADS);
      glTexCoord2f(cx, cy + glyphY);
      glVertex2i(0, 0);
      glTexCoord2f(cx + glyphX, cy + glyphY);
      glVertex2i(glyphsize, 0);
      glTexCoord2f(cx + glyphX, cy);
      glVertex2i(glyphsize, glyphsize);
      glTexCoord2f(cx, cy);
      glVertex2i(0, glyphsize);
      glEnd();
      /* Shift right width of character + 1 */
      glTranslated(bmpfont_charwidths[startidx + i]+1, 0, 0);
      glEndList();
   }
   /* Save vertical offset to resume from */
   yoffset = cy + glyphY;
}

void lucDeleteFont()
{
   /* Delete fonts */
   if (fontbase > 0) glDeleteLists(fontbase, BMP_GLYPHS);
   fontbase = 0;
   if (fonttexture) glDeleteTextures(1, &fonttexture);
   fonttexture = 0;
}
#else
void PrintSetColour(int colour) {}
void PrintString(const char* str) {}
void Printf(int x, int y, float scale, const char *fmt, ...) {}
void Print(int x, int y, float scale, const char *str) {}
void Print3d(double x, double y, double z, double scale, const char *str) {}
void Print3dBillboard(double x, double y, double z, double scale, const char *str) {}
int PrintWidth(const char *string, float scale) {return 0;}
void DeleteFont() {}
void lucPrintString(const char* str) {}
void lucPrintf(int x, int y, const char *fmt, ...) {}
void lucPrint(int x, int y, const char *str) {}
void lucPrint3d(double x, double y, double z, const char *str) {}
void lucSetFontCharset(int charset) {}
int lucPrintWidth(const char *string) {return 0;}
void lucSetupRasterFont() {}
void lucBuildFont(int glyphsize, int columns, int startidx, int stopidx) {}
void lucDeleteFont() {}
#endif

//Vector ops

// vectorNormalise calculates the magnitude of a vector
// \hat v = frac{v} / {|v|}
// This function uses function dotProduct to calculate v . v
void vectorNormalise(float vector[3]) {
   float mag;
   mag = sqrt(dotProduct(vector,vector));
   vector[2] = vector[2]/mag;
   vector[1] = vector[1]/mag;
   vector[0] = vector[0]/mag;
}

std::ostream & operator<<(std::ostream &os, const Colour& colour)
{
    return os << "(" << (int)colour.r << "," << (int)colour.g << "," << (int)colour.b << "," << (int)colour.a << ")";
}

std::ostream & operator<<(std::ostream &os, const Vec3d& vec)
{
    return os << "[" << vec.x << "," << vec.y << "," << vec.z << "]";
}

// Given three points which define a plane, returns a vector which is normal to that plane
Vec3d vectorNormalToPlane(float pos0[3], float pos1[3], float pos2[3])
{
   Vec3d vector0 = Vec3d(pos0);
   Vec3d vector1 = Vec3d(pos1);
   Vec3d vector2 = Vec3d(pos2);
   
   vector1 -= vector0;
   vector2 -= vector0;
   
   return vector1.cross(vector2);
}

// Given three points which define a plane, NormalToPlane will give the unit vector which is normal to that plane
// Uses vectorSubtract, crossProduct and VectorNormalise
void normalToPlane( float normal[3], float pos0[3], float pos1[3], float pos2[3])
{
   float vector1[3], vector2[3];

//printf(" PLANE: %f,%f,%f - %f,%f,%f - %f,%f,%f\n", pos0[0], pos0[1], pos0[2], pos1[0], pos1[1], pos1[2], pos2[0], pos2[1], pos2[2]);
   vectorSubtract(vector1, pos1, pos0);
   vectorSubtract(vector2, pos2, pos0);
   
   crossProduct(normal, vector1, vector2);
//printf(" %f,%f,%f x %f,%f,%f == %f,%f,%f\n", vector1[0], vector1[1], vector1[2], vector2[0], vector2[1], vector2[2], normal[0], normal[1], normal[2]);

   //vectorNormalise( normal);
}

// Given 3 x 3d vertices defining a triangle, calculate the inner angle at the first vertex
float triAngle(float v0[3], float v1[3], float v2[3])
{
   //Returns angle at v0 in radians for triangle defined by v0,v1,v2

   //Get lengths of each side of triangle adjacent to this vertex
   float e0[3], e1[3]; //Triangle edge vectors
   vectorSubtract(e0, v1, v0);
   vectorSubtract(e1, v2, v0);

   //Normalise to simplify dot product calc
   vectorNormalise(e0);
   vectorNormalise(e1);
   //Return triangle angle (in radians)
   return acos(dotProduct(e0,e1));
}

// Draw box, optionally filled and coloured
void drawCuboid(float pos[3], float width, float height, float depth, bool filled, float linewidth)
{
   float min[3] = {pos[0] - 0.5f * width, pos[1] - 0.5f * height, pos[2] - 0.5f * depth};
   float max[3] = {min[0] + width, min[1] + height, min[2] + depth};
   drawCuboid(min, max, filled, linewidth);
}
// (To allow view through front faces to back, swap max and min)
void drawCuboid(float min[3], float max[3], bool filled, float linewidth)
{
   if (linewidth <= 0) linewidth = 1;
   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_LINE_SMOOTH);
   if (!filled) 
   {
      glDisable(GL_LIGHTING);
      glDisable(GL_CULL_FACE);
      glLineWidth(linewidth);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) ;
   }
   else 
   {
      glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;
      glEnable(GL_LIGHTING);

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      //Has length & width?
      if (min[0] != max[0] && min[2] != max[2])
      {
         // Bottom
         glNormal3f(0.0, -1.0, 0.0);
         glBegin(GL_QUADS);
            glVertex3f(max[0], min[1], max[2]);
            glVertex3f(min[0], min[1], max[2]);
            glVertex3f(min[0], min[1], min[2]); 
            glVertex3f(max[0], min[1], min[2]);
         glEnd();
         // Top
         glNormal3f(0.0, 1.0, 0.0);
         glBegin(GL_QUADS);
            glVertex3f(max[0], max[1], max[2]);  
            glVertex3f(max[0], max[1], min[2]); 
            glVertex3f(min[0], max[1], min[2]);
            glVertex3f(min[0], max[1], max[2]);
         glEnd();
      }
   }

   //Has width & height?
   if (min[0] != max[0] && min[1] != max[1])
   {
      // Front
      glNormal3f(0.0, 0.0, -1.0);
      glBegin(GL_QUADS);
         glVertex3f(max[0], min[1], min[2]);  
         glVertex3f(min[0], min[1], min[2]);
         glVertex3f(min[0], max[1], min[2]);
         glVertex3f(max[0], max[1], min[2]); 
      glEnd();
      // Back
      glNormal3f(0.0, 0.0, 1.0);
      glBegin(GL_QUADS);
         glVertex3f(max[0], max[1], max[2]);
         glVertex3f(min[0], max[1], max[2]);
         glVertex3f(min[0], min[1], max[2]);
         glVertex3f(max[0], min[1], max[2]);
      glEnd();
   }

   //Has length & height?
   if (min[2] != max[2] && min[1] != max[1])
   {
      // Left
      glNormal3f(-1.0, 0.0, 0.0);
      glBegin(GL_QUADS);
         glVertex3f(min[0], max[1], max[2]);
         glVertex3f(min[0], max[1], min[2]);
         glVertex3f(min[0], min[1], min[2]);
         glVertex3f(min[0], min[1], max[2]);
      glEnd();
      // Right
      glNormal3f(1.0, 0.0, 0.0);
      glBegin(GL_QUADS);
         glVertex3f(max[0], max[1], max[2]);
         glVertex3f(max[0], min[1], max[2]);
         glVertex3f(max[0], min[1], min[2]);
         glVertex3f(max[0], max[1], min[2]);
      glEnd();
   }

   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) ;
   glPopAttrib();
   glLineWidth(1.0f);
}

// Debugging function, draws a small white line representing normal (red at head)
void drawNormalVector( float pos[3], float vector[3], float scale)
{
   // Length of the drawn vector = vector magnitude * scaling factor
   float length = scale * sqrt(dotProduct(vector,vector));
   float vec[3];
   float rangle;

   // Copy vector
   memcpy(vec, vector, sizeof(vec));

   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_LIGHTING);
   glPushMatrix();
   // Vector is centered on pos[x,y,z]
   // Translate to the point of arrow -> position + vector/2
   glTranslatef(pos[0] + scale * 0.5f * vec[0],
                pos[1] + scale * 0.5f * vec[1],
                pos[2] + scale * 0.5f * vec[2]);

   // Rotate to orient the cone
   //...Want to align our z-axis to point along arrow vector:
   // axis of rotation = (z x vec)
   // cosine of angle between vector and z-axis = (z . vec) / |z|.|vec| */
   // Normalise vector first so OSMesa doesn't have a fit when given a tiny vector as a rotation axis
   vectorNormalise(vec); 
   //Angle of rotation = acos(vec . [0,0,1]) = acos(vec[2])
   rangle = RAD2DEG * acos(vec[2]);
   //Axis of rotation = vec x [0,0,1] = -vec[1],vec[0],0
   glRotatef(rangle, -vec[1], vec[0], 0);

   //Translate back from point by half length
   //thus our working coordinate system origin is halfway down vec with z-axis aligned with head
   glTranslatef(0.0f, 0.0f, -length*0.5f);

   // Render vector as two lines, white base, red at tip
   glColor3f(1.0, 0.0, 0.0);
   glBegin(GL_LINES);
      glVertex3d(0, 0, 0);
      glVertex3d(0, 0, length*0.5);
   glEnd();

   glColor3f(1.0, 1.0, 1.0);
   glBegin(GL_LINES);
      glVertex3d(0, 0, 0);
      glVertex3d(0, 0, -length*0.5);
   glEnd();

   glPopMatrix();
   glPopAttrib();
}

// Calculates a set of points on a unit circle for a given number of segments__
// Used to optimised rendering circular objects when segment count isn't changed
void calcCircleCoords(int segment_count)
{
   // Recalc required? Only done first time called and when segment count changes
   GLfloat angle;
   float angle_inc = 2*M_PI / (float)segment_count;
   int idx;
   if (segments__ == segment_count) return;

   // Calculate unit circle points when divided into specified segments__
   // and store in static variable to re-use every time a vector with the
   // same segment count is drawn
   segments__ = segment_count;
   if (x_coords_ != NULL) delete[] x_coords_;
   if (y_coords_ != NULL) delete[] y_coords_;

   x_coords_ = new float[segment_count + 1];
   y_coords_ = new float[segment_count + 1];

   // Loop around in a circle and specify even points along the circle
   // as the vertices for the triangle fan cone, cone base and arrow shaft
   for (idx = 0; idx <= segments__; idx++)
   {
      angle = angle_inc * (float)idx;
      // Calculate x and y position of the next vertex and cylinder normals (unit circle coords)
      x_coords_[idx] = sin(angle);
      y_coords_[idx] = cos(angle);
   }
}

void drawSphere_(float centre[3], float radius, int segment_count, Colour* colour)
{
   //Case of ellipsoid where all 3 radii are equal
   drawEllipsoid_(centre, radius, radius, radius, segment_count, colour);
}

// Create a 3d ellipsoid given centre point, 3 radii and number of triangle segments to use
// Based on algorithm and equations from:
// http://local.wasp.uwa.edu.au/~pbourke/texture_colour/texturemap/index.html
// http://paulbourke.net/geometry/sphere/
void drawEllipsoid_(float centre[3], float radiusX, float radiusY, float radiusZ, int segment_count, Colour* colour)
{
   int i,j;
   float edge[3],pos[3];

   if (colour) Colour_SetColour(colour);
   if (radiusX < 0) radiusX = -radiusX;
   if (radiusY < 0) radiusY = -radiusY;
   if (radiusZ < 0) radiusZ = -radiusZ;
   if (segment_count < 0) segment_count = -segment_count;
   calcCircleCoords(segment_count);   // Get circle coords

   for (j=0;j<segment_count/2;j++)
   {
      glBegin(GL_QUAD_STRIP);
      for (i=0;i<=segment_count;i++)
      {
         // Get index from pre-calculated coords which is back 1/4 circle from j+1 (same as forward 3/4circle)
         int idx = ((int)(1 + j + 0.75 * segment_count) % segment_count);
         edge[0] = y_coords_[idx] * y_coords_[i];
         edge[1] = x_coords_[idx];
         edge[2] = y_coords_[idx] * x_coords_[i];
         pos[0] = centre[0] + radiusX * edge[0];
         pos[1] = centre[1] + radiusY * edge[1];
         pos[2] = centre[2] + radiusZ * edge[2];

         // Flip for normal
         edge[0] = -edge[0];
         edge[1] = -edge[1];
         edge[2] = -edge[2];

         glNormal3fv(edge);
         glTexCoord2f(i/(float)segment_count, 2*(j+1)/(float)segment_count);
         glVertex3fv(pos);

         // Get index from pre-calculated coords which is back 1/4 circle from j (same as forward 3/4circle)
         idx = ((int)(j + 0.75 * segment_count) % segment_count);
         edge[0] = y_coords_[idx] * y_coords_[i];
         edge[1] = x_coords_[idx];
         edge[2] = y_coords_[idx] * x_coords_[i];
         pos[0] = centre[0] + radiusX * edge[0];
         pos[1] = centre[1] + radiusY * edge[1];
         pos[2] = centre[2] + radiusZ * edge[2];

         // Flip for normal
         edge[0] = -edge[0];
         edge[1] = -edge[1];
         edge[2] = -edge[2];

         glNormal3fv(edge);
         glTexCoord2f(i/(float)segment_count, 2*j/(float)segment_count);
         glVertex3fv(pos);
      }
      glEnd();
   }
}

// Draws a trajectory vector between two coordinates,
// uses spheres and cylinder sections.
// coord0: start coord1: end
// radius: radius of cylinder/sphere sections to draw
// arrowHeadSize: if > 0 then finishes with arrowhead in vector direction at coord1
// segment_count: number of primitives to draw circular geometry with, 16 is usally a good default
// scale: scaling factor for each direction
// colour0: colour at coord0, if NULL then current OpenGL colour is not changed
// colour1: colour at coord1, if NULL then current OpenGL colour is not changed
// maxLength: length limit, sections exceeding this will be skipped
void drawTrajectory_(float coord0[3], float coord1[3], float radius, float arrowHeadSize, int segment_count, float scale[3], Colour *colour0, Colour *colour1, float maxLength)
{
   float length = 0;
   float vector[3];
   float pos[3];

   if (coord1 == NULL) return;

   //Scale end coord
   coord1[0] *= scale[0];
   coord1[1] *= scale[1];
   coord1[2] *= scale[2];

   if (coord0 == NULL)
   {
      // Initial position, no vector yet
      arrowHeadSize = 0;
   }
   else
   {
      //Scale start coord
      coord0[0] *= scale[0];
      coord0[1] *= scale[1];
      coord0[2] *= scale[2];

      // Obtain a vector between the two points
      vectorSubtract(vector, coord1, coord0);

      // Get centre position on vector between two coords
      pos[0] = coord0[0] + vector[0] * 0.5;
      pos[1] = coord0[1] + vector[1] * 0.5;
      pos[2] = coord0[2] + vector[2] * 0.5;

      // Get length
      length = sqrt(dotProduct(vector,vector));
   }

   //Exceeds max length? Draw endpoint only
   if (length > maxLength)
   {
      drawSphere_(coord1, radius, segment_count, colour1);
      return;
   }

   // Draw
   if (arrowHeadSize > 0)
   {
      // Draw final section as arrow head
      // Position so centred on end of tube adjusted for arrowhead radius (tube radius * head size)
      // Too small a section to fit arrowhead? expand so length is at least 2*r ...
      if (length < 2.0 * radius * arrowHeadSize)
      {
         // Adjust length
         float length_adj = arrowHeadSize * radius * 2.0 / length;
         vector[0] *= length_adj;
         vector[1] *= length_adj;
         vector[2] *= length_adj;
         // Adjust to centre position
         pos[0] = coord0[0] + vector[0] * 0.5;
         pos[1] = coord0[1] + vector[1] * 0.5;
         pos[2] = coord0[2] + vector[2] * 0.5;
      }
      // Draw the vector arrow
      drawVector3d_(pos, vector, 1.0, radius, arrowHeadSize, segment_count, colour0, colour1);

   }
   else
   {
      // Check segment length large enough to warrant joining points with cylinder section ...
      // Skip any section smaller than 0.3 * radius, draw sphere only for continuity
      if (length > radius * 0.30)
      {
         // Join last set of points with this set
         drawVector3d_(pos, vector, 1.0, radius, 0.0, segment_count, colour0, colour1);
         if (segment_count < 3 || radius < 1.0e-3 ) return; //Too small for spheres
         drawSphere_(pos, radius, segment_count, colour1);
      }
      // Finish with sphere, closes gaps in angled joins
      if (length > radius * 0.10)
         drawSphere_(coord1, radius, segment_count, colour1);
   }
}

// Draws a 3d vector
// pos: centre position at which to draw vector
// scale: scaling factor for entire vector
// radius: radius of cylinder sections to draw,
//         if zero a default value is automatically calculated based on length & scale
// head_scale: scaling factor for head radius compared to shaft, if zero then no arrow head is drawn
// segment_count: number of primitives to draw circular geometry with, 16 is usually a good default
// colour0: colour at tail, if NULL then current OpenGL colour is not changed
// colour1: colour at head, if NULL then current OpenGL colour is not changed
#define RADIUS_DEFAULT_RATIO 0.02   // Default radius as a ratio of length
void drawVector3d_(float pos[3], float vector[3], float scale, float radius, float head_scale, int segment_count, Colour *colour0, Colour *colour1)
{
   Vec3d vec(vector);

   // Negative scale? Flip vector
   if (scale < 0)
   {
      scale = 0 - scale;
      vec = Vec3d() - vec;
   }

   // Previous implementation was head_scale as a ratio of length [0,1], 
   // now uses ratio to radius (> 1), so adjust if < 1
   if (head_scale > 0 && head_scale < 1.0) 
      head_scale = 0.5 * head_scale / RADIUS_DEFAULT_RATIO; // Convert from fraction of length to multiple of radius

   // Get circle coords
   calcCircleCoords(segment_count);

   // Render a 3d arrow, cone with base for head, cylinder for shaft
   //glDisable(GL_CULL_FACE);
   //glEnable(GL_LIGHTING);
   //glEnable(GL_DEPTH_TEST);
   //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   // Length of the drawn vector = vector magnitude * scaling factor
   float length = scale * vec.magnitude();

   // Default shaft radius based on length of vector (2%)
   if (radius == 0) radius = length * RADIUS_DEFAULT_RATIO;
   // Head radius based on shaft radius
   float head_radius = head_scale * radius;

   glPushMatrix();
   // Vector is centered on pos[x,y,z]
   // Translate to the point of arrow -> position + vector/2
   glTranslatef(pos[0] + scale * 0.5f * vec[0],
                pos[1] + scale * 0.5f * vec[1],
                pos[2] + scale * 0.5f * vec[2]);

   // Rotate to orient the cone
   //...Want to align our z-axis to point along arrow vector:
   // axis of rotation = (z x vec)
   // cosine of angle between vector and z-axis = (z . vec) / |z|.|vec| */
   // Normalise vector first so OSMesa doesn't have a fit when given a tiny vector as a rotation axis
   vec.normalise();
   float rangle = RAD2DEG * vec.angle(Vec3d(0.0, 0.0, 1.0));
   //Axis of rotation = vec x [0,0,1] = -vec[1],vec[0],0
   glRotatef(rangle, -vec[1], vec[0], 0);

   // Translate back from point by size of arrowhead
   // thus our working coordinate system origin is at the base of head with z-axis aligned with head
   glTranslatef(0.0f, 0.0f, -head_radius*2);

   //Lines only for tiny vectors or very low quality setting
   if (segment_count < 3 || radius < 1.0e-3 )
   {
      // Draw Line
      float shaft_vertex[3] = {0,0,0};
      glPushAttrib(GL_ENABLE_BIT);
      glDisable(GL_LIGHTING);
      glBegin(GL_LINES);
      if (colour0) Colour_SetColour(colour0);   // Set tail colour if provided 
      glVertex3fv(shaft_vertex);
      shaft_vertex[2] = -length + head_radius*2; // Shaft length to base of head
      if (colour1) Colour_SetColour(colour1);   // Set head colour if provided 
      glVertex3fv(shaft_vertex);
      glEnd();
      glPopAttrib();
   }
   // Render a cylinder quad strip for shaft
   else if (length > head_radius*2)
   {
      int v;
      float shaft_vertex[3];
      glBegin(GL_QUAD_STRIP);
      for (v=0; v <= segments__; v++)
      {
         glNormal3f(x_coords_[v], y_coords_[v], 0);

         // Base of shaft 
         shaft_vertex[0] = radius * x_coords_[v];
         shaft_vertex[1] = radius * y_coords_[v];
         shaft_vertex[2] = -length + head_radius*2; // Shaft length to base of head 
         if (colour0) Colour_SetColour(colour0);   // Set tail colour if provided 
         glVertex3fv(shaft_vertex);

         // Top of shaft 
         shaft_vertex[2] = 0.0;
         if (colour1) Colour_SetColour(colour1);   // Set head colour if provided 
         glVertex3fv(shaft_vertex);
      }
      glEnd();
   }

   // Render the arrowhead cone and base with two triangle fans 
   // Don't bother drawing head for tiny vectors 
   if (segment_count >= 3 && head_scale > 0 && head_radius >= 1.0e-10 )
   {
      int v;
      // Pinnacle vertex is at point of arrow 
      float pinnacle[3] = {0, 0, head_radius * 2};

      // First pair of vertices on circle define a triangle when combined with pinnacle 
      // First normal is between first and last triangle normals 1/|\seg-1 
      float vertex1[3] = {0.0, 0.0, 0.0};
      float vertex0[3] = {head_radius * x_coords_[1], head_radius * y_coords_[1], 0.0};

      if (colour1) Colour_SetColour(colour1);   // Set head colour if provided 

      glBegin(GL_TRIANGLE_FAN);
      // Pinnacle vertex of cone / centre of base circle 
      glNormal3f(0.0f, 0.0f, 1.0f);
      glVertex3fv(pinnacle);

      // Subsequent vertices describe outer edges of cone base 
      for (v=segments__; v >= 0; v--)
      {
         // Calc next vertex from unit circle coords
         vertex1[0] = head_radius * x_coords_[v];
         vertex1[1] = head_radius * y_coords_[v];
         //Calculate normal
         Vec3d normal = vectorNormalToPlane(pinnacle, vertex0, vertex1);
         normal.normalise();
         glNormal3fv(normal.ref());
         // Draw vertex 
         glVertex2fv(vertex1);
         // Save previous vertex 
         memcpy( vertex0, vertex1, sizeof(float) * 3 );
      }
      glEnd();

      // Flatten cone for circle base -> set common point to share z-coord 
      glBegin(GL_TRIANGLE_FAN);
      // Centre of base circle, normal facing back along arrow 
      glNormal3f(0.0f, 0.0f, -1.0f);
      glVertex3f(0.0f, 0.0f, 0.0f);

      // Repeat vertices for outer edges of cone base 
      for (v=0; v<=segments__; v++)
      {
         // Calc next vertex from unit circle coords
         vertex1[0] = head_radius * x_coords_[v];
         vertex1[1] = head_radius * y_coords_[v];
         // Draw vertex 
         glVertex2fv(vertex1);
      }
      glEnd();
   }

   // Restore modelview 
   glPopMatrix();
}

void Colour_SetColour(Colour* colour)
{
   glColor4ubv(colour->rgba);
}

void Colour_Invert(Colour& colour)
{
   int alpha = colour.value & 0xff000000;
   colour.value = (~colour.value & 0x00ffffff) | alpha;
}

void Colour_SetXOR(bool switchOn)
{
   if (switchOn)
   {
      glColor4f(1.0, 1.0, 1.0, 1.0);
      PrintSetColour(0xffffffff);
      glLogicOp(GL_XOR);
      glEnable(GL_COLOR_LOGIC_OP);
   }
   else
      glDisable(GL_COLOR_LOGIC_OP);
} 

Colour parseRGBA(std::string value)
{
   //Parse rgba(r,g,b,a) values
   Colour col;
   int c;
   float alpha;
   try
   {
      std::stringstream ss(value.substr(5));
      for (int i=0; i<3; i++)
      {
         ss >> c;
         col.rgba[i] = c;
         char next = ss.peek();
         if (next == ',' || next == ' ')
            ss.ignore();
      }
      ss >> alpha;
      if (alpha > 1.)
         col.a = alpha;
      else
        col.a = 255 * alpha;
   }
   catch (std::exception& e)
   {
      std::cerr << "Failed to parse rgba colour: " << value << " : " << e.what() << std::endl;
   }
   return col; //rgba(c[0],c[1],c[2],c[3]);
}

void RawImageFlip(void* image, int width, int height, int bpp)
{
   int scanline = bpp * width;
   GLubyte* ptr1 = (GLubyte*)image;
   GLubyte* ptr2 = ptr1 + scanline * (height-1);
   GLubyte* temp = new GLubyte[scanline];
   for (int y=0; y<height/2; y++)
   {
      memcpy(temp, ptr1, scanline);
      memcpy(ptr1, ptr2, scanline);
      memcpy(ptr2, temp, scanline);
      ptr1 += scanline;
      ptr2 -= scanline;
   }
   delete[] temp;
}

// Loads TGA Image
int LoadTextureTGA(TextureData *texture, const char *filename, bool mipmaps, GLenum mode)
{    
   GLubyte      TGAheader0[12]={0,0,2,0,0,0,0,0,0,0,0,0};    // Uncompressed RGB(A) TGA Header
   GLubyte      TGAheader1[12]={0,0,3,0,0,0,0,0,0,0,0,0};    // Uncompressed B&W TGA Header
   GLubyte      TGAcompare[12];         // Used To Compare TGA Header
   GLubyte      header[6];            // First 6 Useful Bytes From The Header
   GLuint      bytesPerPixel;         // Holds Number Of Bytes Per Pixel Used In The TGA File
   GLuint      imageSize;            // Used To Store The Image Size When Setting Aside Ram
   GLubyte *imageData;            // Image Data (Up To 32 Bits)

   FILE *file = fopen(filename, "rb");      // Open The TGA File

   if (file==NULL ||                   // Does File Even Exist?
       fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||   // Are There 12 Bytes To Read?
       (memcmp(TGAheader0,TGAcompare,sizeof(TGAheader0))!=0 &&
       memcmp(TGAheader1,TGAcompare,sizeof(TGAheader1))!=0) ||      // Does The Header Match What We Want?
       fread(header,1,sizeof(header),file)!=sizeof(header))      // If So Read Next 6 Header Bytes
   {
      if (file == NULL)               // Does The File Even Exist? *Added Jim Strong*
         return 0;               // return 0
      else                        // Otherwise
      {
         fclose(file);               // If Anything Failed, Close The File
         return 0;               // return 0
      }
   }

   texture->width  = header[1] * 256 + header[0];   // Determine The TGA Width   (highbyte*256+lowbyte)
   texture->height = header[3] * 256 + header[2];   // Determine The TGA Height   (highbyte*256+lowbyte)
    
   //Check width & height
    if (texture->width   <=0 || texture->height <=0)
   {
      fclose(file);                         // If Anything Failed, Close The File
      return 0;                         // return 0
   }

   texture->bpp   = header[4];               // Grab The TGA's Bits Per Pixel (16, 24 or 32)
   bytesPerPixel   = texture->bpp/8;            // Divide By 8 To Get The Bytes Per Pixel
   imageSize      = texture->width*texture->height*bytesPerPixel;   // Calculate The Memory Required For The TGA Data

   imageData= new GLubyte[imageSize];   // Reserve Memory To Hold The TGA Data
   if (imageData==NULL ||               // Does The Storage Memory Exist?
      fread(imageData, 1, imageSize, file)!=imageSize)   // Does The Image Size Match The Memory Reserved?
   {
      if (imageData!=NULL)            // Was Image Data Loaded
         delete[] imageData;            // If So, Release The Image Data

      fclose(file);                     // Close The File
      return 0;                     // return 0
   }

   fclose (file);                        // Close The File

   return BuildTexture(texture, imageData, mipmaps, 0, mode);
}

// Loads a PPM image
int LoadTexturePPM(TextureData *texture, const char *filename, bool mipmaps, GLenum mode)
{
   bool readTag = false, readWidth = false, readHeight = false, readColourCount = false;
   char stringBuffer[241];
   int ppmType, colourCount;
   GLuint bytesPerPixel, imageSize;
   GLubyte *imageData;

   FILE* imageFile = fopen(filename, "rb");
   if (imageFile == NULL) 
   {
      debug_print("Cannot open '%s'\n", filename);
      return 0;
   }

   while (!readTag || !readWidth || !readHeight || !readColourCount)
   {
      // Read in a new line from file
      char* charPtr = fgets( stringBuffer, 240, imageFile );
      assert ( charPtr );

      for (charPtr = stringBuffer ; charPtr < stringBuffer + 240 ; charPtr++ )
      {
         // Check if we should go to a new line - this will happen for comments, line breaks and terminator characters
         if ( *charPtr == '#' || *charPtr == '\n' || *charPtr == '\0' )
            break;

         // Check if this is a space - if this is the case, then go to next line
         if ( *charPtr == ' ' || *charPtr == '\t' )
            continue;

         if ( !readTag )
         {
            sscanf( charPtr, "P%d", &ppmType );
            readTag = true;
         }
         else if ( !readWidth )
         {
            sscanf( charPtr, "%u", &texture->width );
            readWidth = true;
         }
         else if ( !readHeight )
         {
            sscanf( charPtr, "%u", &texture->height );
            readHeight = true;
         }
         else if ( !readColourCount )
         {
            sscanf( charPtr, "%d", &colourCount );
            readColourCount = true;
         }

         // Go to next white space
         charPtr = strpbrk( charPtr, " \t" );

         // If there are no more characters in line then go to next line
         if ( charPtr == NULL )
            break;
      }
   }

   // Only allow PPM images of type P6 and with 256 colours
   if ( ppmType != 6 || colourCount != 255 ) abort_program("Unable to load PPM Texture file, incorrect format");

   texture->bpp = 24;
   bytesPerPixel = texture->bpp/8; //bits to bytes
   imageSize = texture->width*texture->height*bytesPerPixel;
   imageData = new GLubyte[imageSize];   // Reserve Memory

   //Flip scanlines vertically
   for (int j = texture->height - 1 ; j >= 0 ; j--)
      fread(&imageData[texture->width * j * bytesPerPixel], bytesPerPixel, texture->width, imageFile);
   fclose(imageFile);

   return BuildTexture(texture, imageData, mipmaps, GL_RGB, mode);
}

int LoadTexturePNG(TextureData *texture, const char *filename, bool mipmaps, GLenum mode)
{
   GLubyte *imageData;

   std::ifstream file(filename, std::ios::binary);
   if (!file) 
   {
      debug_print("Cannot open '%s'\n", filename);
      return 0;
   }
   imageData = (GLubyte*)read_png(file, texture->bpp, texture->width, texture->height);
   
   file.close();

   return BuildTexture(texture, imageData, mipmaps, texture->bpp == 24 ? GL_RGB : GL_RGBA, mode);
}

int LoadTextureJPEG(TextureData *texture, const char *filename, bool mipmaps, GLenum mode)
{
   int width, height, bytesPerPixel;
   GLubyte* imageData = (GLubyte*)jpgd::decompress_jpeg_image_from_file(filename, &width, &height, &bytesPerPixel, 3);

   RawImageFlip(imageData, width, height, 3);
   
   texture->width = width;
   texture->height = height;
   texture->bpp = 24;
   
   return BuildTexture(texture, imageData, mipmaps, GL_RGB, mode);
}

int BuildTexture(TextureData *texture, GLubyte* imageData , bool mipmaps, GLenum format, GLenum mode)
{
   // Build A Texture From The Data
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture->id);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);  //GL_MODULATE/BLEND/REPLACE/DECAL

   // use linear filtering
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   if (mipmaps)
   {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);    //set so texImage2d will gen mipmaps
   }
   else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   //Load the texture data based on bits per pixel
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   switch (texture->bpp)
   {
   case 8:
      if (!format) format = GL_ALPHA;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, imageData);
      break;
   case 16:
      if (!format) format = GL_LUMINANCE_ALPHA;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, imageData);
      break;
   case 24:
      if (!format) format = GL_BGR;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, imageData);
      break;
   case 32:
      if (!format) format = GL_BGRA;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, imageData);
      break;
   }

   //Dispose of data
   delete[] imageData;

   return 1;
}


void abort_program(const char * s, ...)
{
   va_list args;
   va_start(args, s);
   vprintf(s, args);
   debug_print("\n");
   va_end(args);
   abort();
}

void writeImage(GLubyte *image, int width, int height, const char* basename, bool transparent)
{
   char path[256];
#ifdef HAVE_LIBPNG
   //Write data to image file
   sprintf(path, "%s.png", basename);
   std::ofstream file(path, std::ios::binary);
   write_png(file, transparent ? 4 : 3, width, height, image);
#else
   //JPEG support with built in encoder
   sprintf(path, "%s.jpg", basename);

   // Fill in the compression parameter structure.
   jpge::params params;
   params.m_quality = 95;
   params.m_subsampling = jpge::H2V1;   //H2V2/H2V1/H1V1-none/0-grayscale

   if (!compress_image_to_jpeg_file(path, width, height, 3, image, params))
      abort_program("[write_jpeg] File %s could not be saved\n", path);
#endif
}

#ifdef HAVE_LIBPNG
//PNG image read/write support
static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   std::istream* stream = (std::istream*)png_get_io_ptr(png_ptr);
   stream->read((char*)data, length);
   if (stream->fail() || stream->eof()) png_error(png_ptr, "Read Error");
}

static void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   std::ostream* stream = (std::ostream*)png_get_io_ptr(png_ptr);
   stream->write((const char*)data, length);
   if (stream->bad()) png_error(png_ptr, "Write Error");
}

static void png_flush(png_structp png_ptr)
{
   std::ostream* stream = (std::ostream*)png_get_io_ptr(png_ptr);
   stream->flush();
}

void* read_png(std::istream& stream, GLuint& bpp, GLuint& width, GLuint& height)
{
   char header[8];   // 8 is the maximum size that can be checked
   unsigned int y;

   png_byte color_type;
   png_byte bit_depth;

   png_structp png_ptr;
   png_infop info_ptr;
   png_bytep * row_pointers;

   // open file and test for it being a png
   stream.read(header, 8);
   if (png_sig_cmp((png_byte*)&header, 0, 8))
      abort_program("[read_png_file] File is not recognized as a PNG file");

   // initialize stuff
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   
   if (!png_ptr)
      abort_program("[read_png_file] png_create_read_struct failed");

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
      abort_program("[read_png_file] png_create_info_struct failed");

   //if (setjmp(png_jmpbuf(png_ptr)))
   //   abort_program("[read_png_file] Error during init_io");

   // initialize png I/O
   png_set_read_fn(png_ptr, (png_voidp)&stream, png_read_data);
   png_set_sig_bytes(png_ptr, 8);

   png_read_info(png_ptr, info_ptr);

    png_uint_32 imgWidth =  png_get_image_width(png_ptr, info_ptr);
    png_uint_32 imgHeight = png_get_image_height(png_ptr, info_ptr);
    //bits per CHANNEL! note: not per pixel!
    png_uint_32 bitdepth   = png_get_bit_depth(png_ptr, info_ptr);
    //Number of channels
    png_uint_32 channels   = png_get_channels(png_ptr, info_ptr);
    width = imgWidth; height = imgHeight; bit_depth = bitdepth;
    //Row bytes
    png_uint_32 rowbytes  = png_get_rowbytes(png_ptr, info_ptr);

      color_type = png_get_color_type(png_ptr, info_ptr);
      bpp = bitdepth * channels;

   debug_print("Reading PNG: %d x %d, colour type %d, depth %d, channels %d\n", width, height, color_type, bit_depth, channels);

   png_set_interlace_handling(png_ptr);
   png_read_update_info(png_ptr, info_ptr);

   // read file
   //if (setjmp(png_jmpbuf(png_ptr)))
   //   abort_program("[read_png_file] Error during read_image");

   row_pointers = new png_bytep[height];
   //for (y=0; y<height; y++)
   //   row_pointers[y] = (png_byte*) malloc(rowbytes);
   png_bytep pixels = new png_byte[width * height * channels];
   for (y=0; y<height; y++)
      row_pointers[y] = (png_bytep)&pixels[rowbytes * y];

   png_read_image(png_ptr, row_pointers);

   png_destroy_info_struct(png_ptr, &info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)0);

   delete[] row_pointers;

   return pixels;
}

void write_png(std::ostream& stream, int bpp, int width, int height, void* data)
{
   int colour_type;
   png_bytep      pixels       = (png_bytep) data;
   int            rowStride;
   png_bytep*     row_pointers = new png_bytep[height];
   png_structp    pngWrite;
   png_infop      pngInfo;
   int            pixel_I;
   int            flip = 1;   //Flip input data vertically (for OpenGL framebuffer data)

   pngWrite = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!pngWrite) {debug_print("[write_png_file] create PNG write struct failed"); return;}

   //Setup for different write modes
   if (bpp > 3)
   {
      colour_type = PNG_COLOR_TYPE_RGB_ALPHA;
      rowStride = width * 4;
   }
   else if (bpp == 3)
   {
      colour_type = PNG_COLOR_TYPE_RGB;
      rowStride = width * 3;  // Don't need to pad lines! pack alignment is set to 1
   }
   else
   {
      colour_type = PNG_COLOR_TYPE_GRAY;
      rowStride = width;
   }

   // Set pointers to start of each scanline
   for ( pixel_I = 0 ; pixel_I < height ; pixel_I++ )
   {
      if (flip)
         row_pointers[pixel_I] = (png_bytep) &pixels[rowStride * (height - pixel_I - 1)];
      else
         row_pointers[pixel_I] = (png_bytep) &pixels[rowStride * pixel_I];
   }

   pngInfo = png_create_info_struct(pngWrite);
   if (!pngInfo) {debug_print("[write_png_file] create PNG info struct failed"); return;}
   //if (setjmp(png_jmpbuf(pngWrite)))
   //   abort_program("[write_png_file] Error during init_io");

   // initialize png I/O
   png_set_write_fn(pngWrite, (png_voidp)&stream, png_write_data, png_flush);
   png_set_compression_level(pngWrite, Z_BEST_COMPRESSION);

   //if (setjmp(png_jmpbuf(pngWrite)))
   //   abort_program("[write_png_file] Error writing header");   

   png_set_IHDR(pngWrite, pngInfo,
                width, height,
                8,
                colour_type,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);

   png_write_info(pngWrite, pngInfo);
   png_write_image(pngWrite, row_pointers);
   png_write_end(pngWrite, pngInfo);

   // Clean Up
   png_destroy_info_struct(pngWrite, &pngInfo);
   png_destroy_write_struct(&pngWrite, NULL);
   delete[] row_pointers;

   debug_print("[write_png_file] File successfully written\n");
}

#endif //HAVE_LIBPNG
