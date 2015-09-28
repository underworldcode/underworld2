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

//Init static data
json::Object Geometry::properties;
float Geometry::min[3] = {HUGE_VALF, HUGE_VALF, HUGE_VALF};
float Geometry::max[3] = {-HUGE_VALF, -HUGE_VALF, -HUGE_VALF};
float Geometry::dims[3];
std::string GeomData::names[lucMaxType] = {"Labels", "Points", "Grid", "Triangles", "Vectors", "Tracers", "Lines", "Shapes", "Volume"};
float GeomData::opacity = 0;
int GeomData::glyphs = -1; //Glyph quality (-1 = use default per type)
bool GeomData::wireframe = false;
bool GeomData::cullface = false;
bool GeomData::lit = true;
bool Lines::tubes = false;

//Track min/max coords
void GeomData::checkPointMinMax(float *coord)
{
   //std::cerr << coord[0] << "," << coord[1] << "," << coord[2] << std::endl;
   compareCoordMinMax(min, max, coord);
}

void GeomData::label(std::string& labeltext)
{
   //Adds a vertex label
   labels.push_back(labeltext);
}

const char* GeomData::getLabels()
{
   if (labelptr) free(labelptr);
   labelptr = NULL;
   if (labels.size())
   {
      //Get total length
      int length = 1;
      for (unsigned int i=0; i < labels.size(); i++)
         length += labels[i].size() + 1;
      labelptr = (char*)malloc(sizeof(char) * length);
      labelptr[0] = '\0';
      //Copy labels
      for (unsigned int i=0; i < labels.size(); i++)
         sprintf(labelptr, "%s%s\n", labelptr, labels[i].c_str());
   }
   return (const char*)labelptr;
}

//Utility functions, calibrate colourmaps and get colours
void GeomData::colourCalibrate()
{
   //Calibrate colour maps on ranges for related data
   if (draw->colourMaps[lucColourValueData])
      draw->colourMaps[lucColourValueData]->calibrate(colourValue);
   if (draw->colourMaps[lucOpacityValueData])
      draw->colourMaps[lucOpacityValueData]->calibrate(opacityValue);
   if (draw->colourMaps[lucRedValueData])
      draw->colourMaps[lucRedValueData]->calibrate(redValue);
   if (draw->colourMaps[lucGreenValueData])
      draw->colourMaps[lucGreenValueData]->calibrate(greenValue);
   if (draw->colourMaps[lucBlueValueData])
      draw->colourMaps[lucBlueValueData]->calibrate(blueValue);
   //Cache colour lookups
   draw->cache();
}

//Get colour using specified colourValue
void GeomData::mapToColour(Colour& colour, float value)
{
   colour = draw->colourMaps[lucColourValueData]->getfast(value);

   //Set opacity to drawing object override level if set
   float opacity = draw->properties["opacity"].ToFloat(1.0);
   if (opacity > 0.0 && opacity < 1.0)
      colour.a = opacity * 255;
}

struct ValCache
{
   //Cached values for faster lookup
   float opacity;
   Colour colour;
   DrawingObject* draw;
} valcache;

int GeomData::colourCount()
{
   //Return number of colour values or RGBA colours
   int hasColours = colourValue.size();
   if (hasColours == 0) hasColours = colours.size();
   return hasColours;
}

//Sets the colour for specified vertex index, looks up all provided colourmaps
void GeomData::getColour(Colour& colour, int idx)
{
   //Lookup using base colourmap, then RGBA colours, use colour property if no map
   if (draw->colourMaps[lucColourValueData] && colourValue.size() > 0)
   {
      if (colourValue.size() == 1) idx = 0;  //Single colour value only provided
      //assert(idx < colourValue.size());
      if (idx >= colourValue.size()) idx = colourValue.size() - 1;
      colour = draw->colourMaps[lucColourValueData]->getfast(colourValue[idx]);
   }
   else if (colours.size() > 0)
   {
      if (colours.size() == 1) idx = 0;  //Single colour only provided
      if (idx >= colours.size()) idx = colours.size() - 1;
      //assert(idx < colours.size());
      colour = colours.toColour(idx);
   }
   else
   {
      colour = draw->colour;
   }

   //Set components using component colourmaps...
   Colour cc;
   if (draw->colourMaps[lucRedValueData] && redValue.size() > 0)
   {
      cc = draw->colourMaps[lucRedValueData]->getfast(redValue[idx]);
      colour.r = cc.r;
   }
   if (draw->colourMaps[lucGreenValueData] && greenValue.size() > 0)
   {
      cc = draw->colourMaps[lucGreenValueData]->getfast(greenValue[idx]);
      colour.g = cc.g;
   }
   if (draw->colourMaps[lucBlueValueData] && blueValue.size() > 0)
   {
      cc = draw->colourMaps[lucBlueValueData]->getfast(blueValue[idx]);
      colour.b = cc.b;
   }
   if (draw->colourMaps[lucOpacityValueData] && opacityValue.size() > 0)
   {
      cc = draw->colourMaps[lucOpacityValueData]->getfast(opacityValue[idx]);
      colour.a = cc.a;
   }

   //Set opacity to drawing object override level if set
   if (draw->opacity > 0.0 && draw->opacity < 1.0)
      colour.a = draw->opacity * 255;
}

Geometry::Geometry() : view(NULL), elements(-1), allhidden(false), type(lucMinType), total(0), scale(1.0f), redraw(true)
{
}

Geometry::~Geometry()
{
   clear(true);
   close();
}

//Virtuals to implement
void Geometry::close() //Called on quit or gl context destroy
{
   elements = -1;
}

void Geometry::clear(bool all)
{
   total = 0;
   elements = -1;
   redraw = true;
   //iterate geom and delete all GeomData entries
   for (int i = geom.size()-1; i>=0; i--) 
   {
      unsigned int idx = i;
      if (all || !geom[i]->draw->properties["static"].ToBool(false))
      {
         delete geom[idx]; 
         if (!all) geom.erase(geom.begin()+idx);
      }
   }
   if (all) geom.clear();
}

void Geometry::reset()
{
   elements = -1;
   redraw = true;
}

void Geometry::compareMinMax(float* min, float* max)
{
   //Compare passed min/max with min/max of all geometry
   //(Used by parent to get bounds of sub-renderer objects)
   for (unsigned int i = 0; i < geom.size(); i++) 
   {
      compareCoordMinMax(min, max, geom[i]->min);
      compareCoordMinMax(min, max, geom[i]->max);
      //Also update global min/max
      compareCoordMinMax(Geometry::min, Geometry::max, geom[i]->min);
      compareCoordMinMax(Geometry::min, Geometry::max, geom[i]->max);
   }
   //Update global bounding box size
   getCoordRange(Geometry::min, Geometry::max, Geometry::dims);
}

void Geometry::dumpById(std::ostream& csv, unsigned int id)
{
   for (unsigned int i = 0; i < geom.size(); i++) 
   {
      if (geom[i]->draw->id == id)
      {
         if (type == lucVolumeType)
         {
            //Dump colourValue data only
            std::cout << "Collected " << geom[i]->colourValue.size() << " values (" << i << ")" << std::endl;
            for (int c=0; c < geom[i]->colourValue.size(); c++)
            {
               csv << geom[i]->colourValue[c] << std::endl;
            }
 
         }
         else
         {
            std::cout << "Collected " << geom[i]->count << " vertices/values (" << i << ")" << std::endl;
            //Only supports dump of vertex and colourValue at present
            for (int v=0; v < geom[i]->count; v++)
            {
               csv << geom[i]->vertices[v][0] << ',' <<  geom[i]->vertices[v][1] << ',' << geom[i]->vertices[v][2];

               if (geom[i]->colourValue.size() == geom[i]->count)
                  csv << ',' << geom[i]->colourValue[v];

               csv << std::endl;
            }
         }
      }
   }
}

void Geometry::jsonWrite(unsigned int id, std::ostream* osp)
{
   //Placeholder virtual
}

bool Geometry::hide(unsigned int idx)
{
   if (idx >= geom.size()) return false;
   if (hidden[idx]) return false;
   hidden[idx] = true;
   redraw = true;
   return true;
}

void Geometry::hideAll()
{
   if (hidden.size() == 0) return;
   for (unsigned int i=0; i<hidden.size(); i++)
   {
      hidden[i] = true;
      geom[i]->draw->visible = false;
   }
   allhidden = true;
   redraw = true;
}

bool Geometry::show(unsigned int idx)
{
   if (idx >= geom.size()) return false;
   if (!hidden[idx]) return false;
   hidden[idx] = false;
   redraw = true;
   return true;
}

void Geometry::showAll()
{
   if (hidden.size() == 0) return;
   for (unsigned int i=0; i<hidden.size(); i++)
   {
      hidden[i] = false;
      geom[i]->draw->visible = true;
   }
   allhidden = false;
   redraw = true;
}

void Geometry::showById(unsigned int id, bool state)
{
   for (unsigned int i = 0; i < geom.size(); i++)
   {
      //std::cerr << i << " owned by object " << geom[i]->draw->id << std::endl;
      if (geom[i]->draw->id == id)
      {
         hidden[i] = !state;
         geom[i]->draw->visible = state;
      }
   }
   redraw = true;
}

void Geometry::localiseColourValues()
{
   for (unsigned int i = 0; i < geom.size(); i++) 
   {
      //Get local min and max for each element from colourValues
      if (geom[i]->draw->colourMaps[lucColourValueData])
      {
         geom[i]->colourValue.minimum = HUGE_VAL;
         geom[i]->colourValue.maximum = -HUGE_VAL;
         for (int v=0; v < geom[i]->colourValue.size(); v++)
         {
            // Check min/max against each value
            if (geom[i]->colourValue[v] > geom[i]->colourValue.maximum) geom[i]->colourValue.maximum = geom[i]->colourValue[v];
            if (geom[i]->colourValue[v] < geom[i]->colourValue.minimum) geom[i]->colourValue.minimum = geom[i]->colourValue[v];
         }
      }
   }
}

void Geometry::redrawObject(unsigned int id)
{
   for (unsigned int i = 0; i < geom.size(); i++)
   {
      if (geom[i]->draw->id == id)
      {
         elements = -1; //Force reload data
         redraw = true;
         return;
      }
   }
}

void Geometry::init() //Called on GL init
{
   redraw = true;
}

void Geometry::setState(int index, Shader* prog)
{
   GL_Error_Check;
   if (geom.size() <= index) return;
   DrawingObject* draw = geom[index]->draw;
   int texunit = -1;
   bool lighting = GeomData::lit;
   lighting = lighting && draw->properties["lit"].ToBool(true);

   //Global/Local draw state
   if (GeomData::cullface || draw->properties["cullface"].ToBool(false))
      glEnable(GL_CULL_FACE);
   else
      glDisable(GL_CULL_FACE);

   //Surface specific options
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   if (type == lucTriangleType || type == lucGridType || TriangleBased(type))
   {
      //Don't light surfaces in 2d models
      if (!view->is3d && flat2d) lighting = false;
      //Disable lighting and polygon faces in wireframe mode
      if (GeomData::wireframe || draw->properties["wireframe"].ToBool(false))
      {
         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
         lighting = false;
         glDisable(GL_CULL_FACE);
      }

      if (draw->properties["flat"].ToBool(false))
         glShadeModel(GL_FLAT);
      else
         glShadeModel(GL_SMOOTH);
   }
   else
   {
      //Flat disables lighting for non surface types
      if (draw->properties["flat"].ToBool(false)) lighting = false;
   }

   if (draw->properties["depthtest"].ToBool(true))
      glEnable(GL_DEPTH_TEST);
   else
      glDisable(GL_DEPTH_TEST);

   if (!lighting)
      glDisable(GL_LIGHTING);
   else
      glEnable(GL_LIGHTING);

   float lineWidth = draw->properties["linewidth"].ToFloat(1.0) * scale;
   if (lineWidth <= 0) lineWidth = scale;
   glLineWidth(lineWidth);
   //Textured?
   texunit = draw->useTexture(geom[index]->texture);
   GL_Error_Check;

   //Uniforms for shader programs
   if (prog && prog->program > 0)
   {
      prog->use();
      prog->setUniform("uOpacity", GeomData::opacity);
      prog->setUniform("uLighting", lighting);
      prog->setUniform("uBrightness", Geometry::properties["brightness"].ToFloat(0.0));
      prog->setUniform("uContrast", Geometry::properties["contrast"].ToFloat(1.0));
      prog->setUniform("uSaturation", Geometry::properties["saturation"].ToFloat(1.0));
      prog->setUniform("uAmbient", Geometry::properties["ambient"].ToFloat(0.4));
      prog->setUniform("uDiffuse", Geometry::properties["diffuse"].ToFloat(0.8));
      prog->setUniform("uSpecular", Geometry::properties["specular"].ToFloat(0.0));
      prog->setUniform("uTextured", texunit >= 0);

      if (texunit >= 0)
         prog->setUniform("uTexture", texunit);

      if (geom[index]->normals.size() == 0 && (type == lucTriangleType || TriangleBased(type)))
         prog->setUniform("uCalcNormal", 1);
      else
         prog->setUniform("uCalcNormal", 0);

      if (prog->uniforms["uClipMin"])
      {
         //TODO: Also enable clip for lines (will require line shader)
         float clipMin[3] = {Geometry::properties["xmin"].ToFloat(0.0) * Geometry::dims[0] + Geometry::min[0],
                             Geometry::properties["ymin"].ToFloat(0.0) * Geometry::dims[1] + Geometry::min[1],
                             Geometry::properties["zmin"].ToFloat(0.0) * Geometry::dims[2] + Geometry::min[2]};
         float clipMax[3] = {Geometry::properties["xmax"].ToFloat(1.0) * Geometry::dims[0] + Geometry::min[0],
                             Geometry::properties["ymax"].ToFloat(1.0) * Geometry::dims[1] + Geometry::min[1],
                             Geometry::properties["zmax"].ToFloat(1.0) * Geometry::dims[2] + Geometry::min[2]};
         
         //std::cout << "CLIP MIN " << Vec3d(clipMin) << " CLIP MAX " << Vec3d(clipMax) << std::endl;
         glUniform3fv(prog->uniforms["uClipMin"], 1, clipMin);
         glUniform3fv(prog->uniforms["uClipMax"], 1, clipMax);
      }
   }
   GL_Error_Check;
}

void Geometry::update()
{
}

void Geometry::draw()  //Display saved geometry
{
   GL_Error_Check;

   //Default to no shaders
   if (glUseProgram) glUseProgram(0);

   //Anything to draw?
   drawcount = 0;
   for (unsigned int i=0; i < geom.size(); i++)
   {
      if (drawable(i))
      {
         drawcount++;
         break;
      }
   }

   GL_Error_Check;
   //Have something to update?
   if (drawcount)
   {
      if (redraw)
         update();
   GL_Error_Check;
      labels();
   GL_Error_Check;
   }

   redraw = false;
   GL_Error_Check;

}

void Geometry::labels()
{
   //Print labels
   lucSetFontCharset(FONT_SMALL); //Bitmap fonts
   for (unsigned int i=0; i < geom.size(); i++)
   {
      Colour colour;
      if (drawable(i) && geom[i]->labels.size() > 0) 
      {
         for (unsigned int j=0; j < geom[i]->labels.size(); j++)
         {
            float* p = geom[i]->vertices[j];
            geom[i]->getColour(colour, j);
            //Multiply opacity by global override level if set
            if (GeomData::opacity > 0.0)
               colour.a *= GeomData::opacity;
            glColor4ubv(colour.rgba);
            if (geom[i]->labels[j].size() > 0)
            {
#ifdef USE_OMEGALIB
               Print3dBillboard(p[0], p[1], p[2], 1, geom[i]->labels[j].c_str());
#else
               lucPrint3d(p[0], p[1], p[2], geom[i]->labels[j].c_str());
#endif
            }
         }
      }
   }
}

//Returns true if passed geometry element index is drawable
//ie: has data, in range, not hidden and in viewport object list 
bool Geometry::drawable(unsigned int idx)
{
   if (!geom[idx]->draw->visible) return false;
   //Within bounds and not hidden
   if (idx < geom.size() && geom[idx]->count > 0 && !hidden[idx])
   {
      //Not filtered by viewport?
      if (!view->filtered) return true;

      //Search viewport object set
      if (view->hasObject(geom[idx]->draw))
         return true;

   }
   return false;
}

std::vector<GeomData*> Geometry::getAllObjects(int id)
{
   //Get passed object's data store
   std::vector<GeomData*> geomlist;
   for (int i=0; i<geom.size(); i++)
      if (geom[i]->draw->id == id)
         geomlist.push_back(geom[i]);
   return geomlist;
}

GeomData* Geometry::getObjectStore(DrawingObject* draw)
{
   //Get passed object's most recently added data store
   GeomData* geomdata = NULL;
   for (int i=geom.size()-1; i>=0; i--)
   {
      if (geom[i]->draw == draw)
      {
         geomdata = geom[i];
         break;
      }
   }
   return geomdata;
}

GeomData* Geometry::add(DrawingObject* draw)
{
   GeomData* geomdata = new GeomData(draw);
   geom.push_back(geomdata);
   if (hidden.size() < geom.size()) hidden.push_back(allhidden);
   if (allhidden) draw->visible = false;
   //debug_print("NEW DATA STORE CREATED FOR %s size %d\n", draw->name.c_str(), geom.size());
   return geomdata;
}

void Geometry::setView(View* vp, float* min, float* max)
{
   view = vp;

   if (!min || !max) return;

   //Iterate the selected viewport's drawing objects
   //Apply geometry bounds from all object data within this viewport
   for (unsigned int o=0; o<view->objects.size(); o++)
   {
      //Skip invisible
      if (!view->objects[o] || !view->objects[o]->visible) continue;
      for (int g=0; g<geom.size(); g++)
      {
         if (geom[g]->draw == view->objects[o])
         {
            for (unsigned int j=0; j<3; j++)
            {
               if (geom[g]->min[j] < min[j]) min[j] = geom[g]->min[j];
               if (geom[g]->max[j] > max[j]) max[j] = geom[g]->max[j];
            }
            //printf("Applied bounding dims from object %d...%f,%f,%f - %f,%f,%f\n", geom[g]->draw->id, geom[g]->min[0], geom[g]->min[1], geom[g]->min[2], geom[g]->max[0], geom[g]->max[1], geom[g]->max[2]);
         }
      }
   }
}

//Read geometry data from storage
GeomData* Geometry::read(DrawingObject* draw, int n, lucGeometryDataType dtype, const void* data, int width, int height, int depth)
{
   draw->skip = false;  //Enable object (has data now)
   GeomData* geomdata;
   //Get passed object's most recently added data store
   geomdata = getObjectStore(draw);

   //Objects with a specified width & height: detect new data store when required (full)
   if (!geomdata || (dtype == lucVertexData && 
       geomdata->width > 0 && geomdata->height > 0 && 
       geomdata->width * geomdata->height * geomdata->depth == geomdata->count))
   {
      //No store yet or loading vertices and already have required amount, new object required...
      //Create new data store, save in drawing object and Geometry list
      geomdata = add(draw);
   }

   read(geomdata, n, dtype, data, width, height, depth);

   return geomdata; //Return data store pointer
}

void Geometry::read(GeomData* geomdata, int n, lucGeometryDataType dtype, const void* data, int width, int height, int depth)
{
   //Set width & height if provided
   if (width) geomdata->width = width;
   if (height) geomdata->height = height;
   geomdata->depth = depth;

   //Read the data
   if (n > 0) geomdata->data[dtype]->read(n, data);

   if (dtype == lucVertexData)
   {
      geomdata->count += n;
      total += n;

      //Update bounds on single vertex reads
      if (n == 1) geomdata->checkPointMinMax((float*)data);
   }
}

void Geometry::setup(DrawingObject* draw, lucGeometryDataType dtype, float minimum, float maximum, float dimFactor, const char* units)
{
   //Get passed object's most recently added data store and setup draw data
   GeomData* geomdata = getObjectStore(draw);
   if (!geomdata) return;
   geomdata->data[dtype]->setup(minimum, maximum, dimFactor, units);
}

void Geometry::label(DrawingObject* draw, const char* labels)
{
   //Get passed object's most recently added data store and add vertex labels (newline separated)
   GeomData* geomdata = getObjectStore(draw);
   if (!geomdata) return;

   //Split by newlines
   std::istringstream iss(labels);
   std::string line;
   while(std::getline(iss, line))
      geomdata->label(line);
}

void Geometry::print()
{
   for (unsigned int i = 0; i < geom.size(); i++) 
   {
      switch (type)
      {
      case lucLabelType:
         std::cout << "Labels ";
         break;
      case lucPointType:
         std::cout << "Point swarm ";
         break;
      case lucVectorType:
         std::cout << "Vectors";
         break;
      case lucTracerType:
         std::cout << "Tracer swarm";
         break;
      case lucGridType:
         std::cout << "Surface";
         break;
      case lucTriangleType:
         std::cout << "Isosurface";
         break;
      case lucLineType:
         std::cout << "Lines";
         break;
      case lucShapeType:
         std::cout << "Shapes";
         break;
      default:
         std::cout << "UNKNOWN";
      }

      std::cout << i << " - " << std::endl;
      //std::cout << i << " - " << (drawable(i) ? "shown" : "hidden") << std::endl;
   }
}

//Dumps colourmapped data to image
void Geometry::toImage(unsigned int idx)
{
   geom[idx]->colourCalibrate();
   int width = geom[idx]->width;
   if (width == 0) width = 256;
   int height = geom[idx]->height;
   if (height == 0) height = geom[idx]->colourValue.size() / width;
   char path[256];
   int pixel = 3;
   GLubyte *image = new GLubyte[width * height * pixel];
   // Read the pixels
   for (int y=0; y<height; y++)
   {
     for (int x=0; x<width; x++)
     {
       printf("%f\n", geom[idx]->colourValue[y * width + x]);
       Colour c;
       geom[idx]->getColour(c, y * width + x);
       image[y * width*pixel + x*pixel + 0] = c.r;
       image[y * width*pixel + x*pixel + 1] = c.g;
       image[y * width*pixel + x*pixel + 2] = c.b;
     }
   }
   //Write data to image file
   sprintf(path, "%s.%05d", geom[idx]->draw->name.c_str(), idx);
   writeImage(image, width, height, path, false);
   delete[] image;
}

void Geometry::setTexture(DrawingObject* draw, TextureData* texture)
{
   GeomData* geomdata = getObjectStore(draw);
   geomdata->texture = texture;
   //Must be opaque to draw with own texture
   geomdata->opaque = true;
}

/////////////////////////////////////////////////////////////////////////////////
// Sorting utility functions
/////////////////////////////////////////////////////////////////////////////////
// Comparison for X,Y,Z vertex sort
int compareXYZ(const void *a, const void *b)
{
   float *s1 = (float*)a;
   float *s2 = (float*)b;
   
   if (s1[0] != s2[0]) return s1[0] < s2[0];
   if (s1[1] != s2[1]) return s1[1] < s2[1];
   return s1[2] < s2[2];
}

int compareParticle(const void *a, const void *b)
{
    PIndex *p1 = (PIndex*)a;
    PIndex *p2 = (PIndex*)b;
    if (p1->distance == p2->distance) return 0;
    return p1->distance < p2->distance ? -1 : 1;
}

//Generic radix sorter - template free version
void radix_sort_byte(int byte, long N, unsigned char *source, unsigned char *dest, int size)
{
   // Radix counting sort of 1 byte, 8 bits = 256 bins
   long count[256], index[256];
   int i;
   memset(count, 0, sizeof(count)); //Clear counts

   //Create histogram, count occurences of each possible byte value 0-255
   for (i=0; i<N; i++)
      count[source[i*size+byte]]++;

   //Calculate number of elements less than each value (running total through array)
   //This becomes the offset index into the sorted array
   //(eg: there are 5 smaller values so place me in 6th position = 5)
   index[0]=0;
   for (i=1; i<256; i++) index[i] = index[i-1] + count[i-1];

   //Finally, re-arrange data by index positions
   for (i=0; i<N; i++)
   {
      int val = source[i*size + byte];  //Get value
      memcpy(&dest[index[val]*size], &source[i*size], size);
      index[val]++; //Increment index to push next element with same value forward one
   }
}


//////////////////////////////////
// Draws a 3d vector
// pos: centre position at which to draw vector
// scale: scaling factor for entire vector
// radius: radius of cylinder sections to draw,
//         if zero a default value is automatically calculated based on length & scale
// head_scale: scaling factor for head radius compared to shaft, if zero then no arrow head is drawn
// segment_count: number of primitives to draw circular geometry with, 16 is usually a good default
#define RADIUS_DEFAULT_RATIO 0.02   // Default radius as a ratio of length
void Geometry::drawVector(DrawingObject *draw, float pos[3], float vector[3], float scale, float radius0, float radius1, float head_scale, int segment_count)
{
   std::vector<unsigned int> indices;
   Vec3d vec(vector);

     //Setup orientation using alignment vector
     Quaternion rot;
     // Rotate to orient the shape
     //...Want to align our z-axis to point along arrow vector:
     // axis of rotation = (z x vec)
     // cosine of angle between vector and z-axis = (z . vec) / |z|.|vec| *
     Vec3d rvector(vec);
     rvector.normalise();
     float rangle = RAD2DEG * rvector.angle(Vec3d(0.0, 0.0, 1.0));
     //Axis of rotation = vec x [0,0,1] = -vec[1],vec[0],0
     Vec3d rvec = Vec3d(-rvector.y, rvector.x, 0);
     rot.fromAxisAngle(rvec, rangle);

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

   // Length of the drawn vector = vector magnitude * scaling factor
   float length = scale * vec.magnitude();

   // Default shaft radius based on length of vector (2%)
   if (radius0 == 0) radius0 = length * RADIUS_DEFAULT_RATIO;
   if (radius1 == 0) radius1 = radius0;
   // Head radius based on shaft radius
   float head_radius = head_scale * radius1;

   // Vector is centered on pos[x,y,z]
   // Translate to the point of arrow -> position + vector/2
   Vec3d translate = Vec3d(pos[0] + scale * 0.5f * vec[0],
                           pos[1] + scale * 0.5f * vec[1],
                           pos[2] + scale * 0.5f * vec[2]);

   float headD = head_radius*2;
   //Output is lines only if using very low quality setting
   if (segment_count < 4)
   {
      // Draw Line
      Vec3d vertex0 = Vec3d(0,0,-length);
      Vec3d vertex = translate + rot * vertex0;
      read(draw, 1, lucVertexData, vertex.ref());
      vertex0.z = -headD;
      vertex = translate + rot * vertex0;
      read(draw, 1, lucVertexData, vertex.ref());
      return;
   }
   else if (length > headD)
   {
      int v;
      float shaft_vertex[3];
      for (v=0; v <= segment_count; v++)
      {
         int vertex_index = getCount(draw);

         // Base of shaft 
         Vec3d vertex0 = Vec3d(radius0 * x_coords_[v], radius0 * y_coords_[v], -length); // z = Shaft length to base of head 
         Vec3d vertex = translate + rot * vertex0;

         //Read triangle vertex, normal
         read(draw, 1, lucVertexData, vertex.ref());
         Vec3d normal = rot * Vec3d(x_coords_[v], y_coords_[v], 0);
         //normal.normalise();
         read(draw, 1, lucNormalData, normal.ref());

         // Top of shaft 
         Vec3d vertex1 = Vec3d(radius1 * x_coords_[v], radius1 * y_coords_[v], -headD);
         vertex = translate + rot * vertex1;

         //Read triangle vertex, normal
         read(draw, 1, lucVertexData, vertex.ref());
         read(draw, 1, lucNormalData, normal.ref());

         //Triangle strip indices
         if (v > 0)
         {
            //First tri
            indices.push_back(vertex_index-2);
            indices.push_back(vertex_index-1);
            indices.push_back(vertex_index);
            //Second tri
            indices.push_back(vertex_index-1);
            indices.push_back(vertex_index+1);
            indices.push_back(vertex_index);
         }
      }
   }
   else
   {
      headD = length; //Limit max arrow head diameter
      head_radius = length * 0.5;
   }

   // Render the arrowhead cone and base with two triangle fans 
   // Don't bother drawing head very low quality settings 
   if (segment_count >= 3 && head_scale > 0 && head_radius > 1.0e-7 )
   {
      int v;
      // Pinnacle vertex is at point of arrow 
      int pt = getCount(draw);
      Vec3d pinnacle = Vec3d(0, 0, 0);

      // First pair of vertices on circle define a triangle when combined with pinnacle 
      // First normal is between first and last triangle normals 1/|\seg-1 

      //Read triangle vertex, normal
      Vec3d vertex = translate + pinnacle;;
      read(draw, 1, lucVertexData, vertex.ref());
      
      Vec3d normal = rot * Vec3d(0.0f, 0.0f, 1.0f);
      normal.normalise();
      read(draw, 1, lucNormalData, normal.ref());

      // Subsequent vertices describe outer edges of cone base 
      Vec3d vertex0 = rot * Vec3d(head_radius * x_coords_[1], head_radius * y_coords_[1], -headD);
      for (v=segment_count; v >= 0; v--)
      {
         int vertex_index = getCount(draw);

         // Calc next vertex from unit circle coords
         Vec3d vertex1 = rot * Vec3d(head_radius * x_coords_[v], head_radius * y_coords_[v], -headD);

         //Calculate normal
         Vec3d normal = vectorNormalToPlane(pinnacle.ref(), vertex0.ref(), vertex1.ref());
         vertex0 = vertex1;
         normal.normalise();

         vertex1 = translate + vertex1;

         //Read triangle vertex, normal
         read(draw, 1, lucVertexData, vertex1.ref());
         read(draw, 1, lucNormalData, normal.ref());

         //Triangle fan indices
         indices.push_back(pt);
         indices.push_back(vertex_index-1);
         indices.push_back(vertex_index);
      }

      // Flatten cone for circle base -> set common point to share z-coord 
      // Centre of base circle, normal facing back along arrow 
      pt = getCount(draw);
      pinnacle = rot * Vec3d(0,0,-headD);
      vertex = translate + pinnacle;
      normal = rot * Vec3d(0.0f, 0.0f, -1.0f);
      //Read triangle vertex, normal
      read(draw, 1, lucVertexData, vertex.ref());
      read(draw, 1, lucNormalData, normal.ref());

      // Repeat vertices for outer edges of cone base 
      for (v=0; v<=segment_count; v++)
      {
         int vertex_index = getCount(draw);
         // Calc next vertex from unit circle coords
         Vec3d vertex1 = rot * Vec3d(head_radius * x_coords_[v], head_radius * y_coords_[v], -headD);

         vertex1 = translate + vertex1;

         //Read triangle vertex, normal
         read(draw, 1, lucVertexData, vertex1.ref());
         read(draw, 1, lucNormalData, normal.ref());

         //Triangle fan indices
         indices.push_back(pt);
         indices.push_back(vertex_index-1);
         indices.push_back(vertex_index);
      }
   }

   //Read the triangle indices
   read(draw, indices.size(), lucIndexData, &indices[0]);
}

// Draws a trajectory vector between two coordinates,
// uses spheres and cylinder sections.
// coord0: start coord1: end
// radius: radius of cylinder/sphere sections to draw
// arrowHeadSize: if > 0 then finishes with arrowhead in vector direction at coord1
// segment_count: number of primitives to draw circular geometry with, 16 is usally a good default
// scale: scaling factor for each direction
// maxLength: length limit, sections exceeding this will be skipped
void Geometry::drawTrajectory(DrawingObject *draw, float coord0[3], float coord1[3], float radius0, float radius1, float arrowHeadSize, float scale[3], float maxLength, int segment_count)
{
   float length = 0;
   float vector[3];
   float pos[3];

   assert(coord0 && coord1);

   //Scale end coord
   coord1[0] *= scale[0];
   coord1[1] *= scale[1];
   coord1[2] *= scale[2];

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

   //Exceeds max length? Draw endpoint only
   if (length > maxLength)
   {
      Vec3d centre(coord1);
      drawSphere(draw, centre, radius0, segment_count);
      return;
   }

   // Draw
   if (arrowHeadSize > 0)
   {
      // Draw final section as arrow head
      // Position so centred on end of tube adjusted for arrowhead radius (tube radius * head size)
      // Too small a section to fit arrowhead? expand so length is at least 2*r ...
      if (length < 2.0 * radius1 * arrowHeadSize)
      {
         // Adjust length
         float length_adj = arrowHeadSize * radius1 * 2.0 / length;
         vector[0] *= length_adj;
         vector[1] *= length_adj;
         vector[2] *= length_adj;
         // Adjust to centre position
         pos[0] = coord0[0] + vector[0] * 0.5;
         pos[1] = coord0[1] + vector[1] * 0.5;
         pos[2] = coord0[2] + vector[2] * 0.5;
      }
      // Draw the vector arrow
      drawVector(draw, pos, vector, 1.0, radius0, radius1, arrowHeadSize, segment_count);

   }
   else
   {
      // Check segment length large enough to warrant joining points with cylinder section ...
      // Skip any section smaller than 0.3 * radius, draw sphere only for continuity
      //if (length > radius1 * 0.30)
      {
         // Join last set of points with this set
         drawVector(draw, pos, vector, 1.0, radius0, radius1, 0.0, segment_count);
//         if (segment_count < 3 || radius1 < 1.0e-3 ) return; //Too small for spheres
//          Vec3d centre(pos);
//         drawSphere(geom, centre, radius, segment_count);
      }
      // Finish with sphere, closes gaps in angled joins
//          Vec3d centre(coord1);
//      if (length > radius * 0.10)
//         drawSphere(geom, centre, radius, segment_count);
   }

}

void Geometry::drawCuboid(DrawingObject *draw, float pos[3], float width, float height, float depth, Quaternion& rot)
{
   float min[3] = {-0.5f * width, -0.5f * height, -0.5f * depth};
   float max[3] = {min[0] + width, min[1] + height, min[2] + depth};

   //Corner vertices
   Vec3d verts[8] = 
   {
      Vec3d(min[0], min[1], max[2]),
      Vec3d(max[0], min[1], max[2]),
      Vec3d(max[0], max[1], max[2]),
      Vec3d(min[0], max[1], max[2]),
      Vec3d(min[0], min[1], min[2]),
      Vec3d(max[0], min[1], min[2]),
      Vec3d(max[0], max[1], min[2]),
      Vec3d(min[0], max[1], min[2])
   };

   for (int i=0; i<8; i++)
   {
      /* Multiplying a quaternion q with a vector v applies the q-rotation to v */
      verts[i] = rot * verts[i];
      verts[i] += Vec3d(pos);
      //geom->checkPointMinMax(verts[i].ref());
   }

   //Triangle indices
   unsigned vertex_index = (unsigned)getCount(draw);
   unsigned int indices[36] = {
				0+vertex_index, 1+vertex_index, 2+vertex_index, 2+vertex_index, 3+vertex_index, 0+vertex_index, 
				3+vertex_index, 2+vertex_index, 6+vertex_index, 6+vertex_index, 7+vertex_index, 3+vertex_index, 
				7+vertex_index, 6+vertex_index, 5+vertex_index, 5+vertex_index, 4+vertex_index, 7+vertex_index, 
				4+vertex_index, 0+vertex_index, 3+vertex_index, 3+vertex_index, 7+vertex_index, 4+vertex_index, 
				0+vertex_index, 1+vertex_index, 5+vertex_index, 5+vertex_index, 4+vertex_index, 0+vertex_index,
				1+vertex_index, 5+vertex_index, 6+vertex_index, 6+vertex_index, 2+vertex_index, 1+vertex_index 
			};

   read(draw, 8, lucVertexData, verts[0].ref());
   read(draw, 36, lucIndexData, indices);
}

void Geometry::drawSphere(DrawingObject *draw, Vec3d& centre, float radius, int segment_count)
{
   //Case of ellipsoid where all 3 radii are equal
   Vec3d radii = Vec3d(radius, radius, radius);
   Quaternion qrot;
   drawEllipsoid(draw, centre, radii, qrot, segment_count);
}

// Create a 3d ellipsoid given centre point, 3 radii and number of triangle segments to use
// Based on algorithm and equations from:
// http://local.wasp.uwa.edu.au/~pbourke/texture_colour/texturemap/index.html
// http://paulbourke.net/geometry/sphere/
void Geometry::drawEllipsoid(DrawingObject *draw, Vec3d& centre, Vec3d& radii, Quaternion& rot, int segment_count)
{
   int i,j;
   Vec3d edge, pos;
   float tex[2];

   if (radii.x < 0) radii.x = -radii.x;
   if (radii.y < 0) radii.y = -radii.y;
   if (radii.z < 0) radii.z = -radii.z;
   if (segment_count < 0) segment_count = -segment_count;
   calcCircleCoords(segment_count);

   std::vector<unsigned int> indices;
   for (j=0; j<=segment_count/2; j++)
   {
      //Triangle strip vertices
      for (i=0; i<=segment_count; i++)
      {
         int vertex_index = getCount(draw);
         // Get index from pre-calculated coords which is back 1/4 circle from j+1 (same as forward 3/4circle)
         int circ_index = ((int)(1 + j + 0.75 * segment_count) % segment_count);
         edge = Vec3d(y_coords_[circ_index] * y_coords_[i], x_coords_[circ_index], y_coords_[circ_index] * x_coords_[i]);
         pos = rot * (centre + radii * edge);

         // Flip for normal
         edge = -edge;

         tex[0] = i/(float)segment_count;
         tex[1] = 2*(j+1)/(float)segment_count;
      
         //Read triangle vertex, normal, texcoord
         read(draw, 1, lucVertexData, pos.ref());
         read(draw, 1, lucNormalData, edge.ref());
         read(draw, 1, lucTexCoordData, tex);

         // Get index from pre-calculated coords which is back 1/4 circle from j (same as forward 3/4circle)
         circ_index = ((int)(j + 0.75 * segment_count) % segment_count);
         edge = Vec3d(y_coords_[circ_index] * y_coords_[i], x_coords_[circ_index], y_coords_[circ_index] * x_coords_[i]);
         pos = rot * (centre + radii * edge);

         // Flip for normal
         edge = -edge;

         tex[0] = i/(float)segment_count;
         tex[1] = 2*j/(float)segment_count;

         //Read triangle vertex, normal, texcoord
         read(draw, 1, lucVertexData, pos.ref());
         read(draw, 1, lucNormalData, edge.ref());
         read(draw, 1, lucTexCoordData, tex);

         //Triangle strip indices
         if (i > 0)
         {
            //First tri
            indices.push_back(vertex_index-2);
            indices.push_back(vertex_index-1);
            indices.push_back(vertex_index);
            //Second tri
            indices.push_back(vertex_index-1);
            indices.push_back(vertex_index+1);
            indices.push_back(vertex_index);
         }
      }
   }

   //Read the triangle indices
   read(draw, indices.size(), lucIndexData, &indices[0]);
}

int Geometry::glyphSegments(int def)
{
   //Use global if set, otherwise use passed default
   int quality = GeomData::glyphs;
   if (quality < 0) quality = def;
   //Segment count = quality * 4
   return quality*4;
}
