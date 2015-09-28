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

//Model class
#include "Model.h"

std::vector<TimeStep*> TimeStep::timesteps; //Active model timesteps
int TimeStep::gap = 0;  //Here for now, probably should be in separate TimeStep.cpp
int TimeStep::cachesize = 0;
bool Model::noload = false;
bool Model::pointspheres = false;
int Model::now = -1;

//Static geometry containers, shared by all models for fast switching/drawing
std::vector<Geometry*> Model::geometry;
Geometry* Model::labels = NULL;
Points* Model::points = NULL;
Vectors* Model::vectors = NULL;
Tracers* Model::tracers = NULL;
QuadSurfaces* Model::quadSurfaces = NULL;
TriSurfaces* Model::triSurfaces = NULL;
Lines* Model::lines = NULL;
Shapes* Model::shapes = NULL;
Volumes* Model::volumes = NULL;

Model::Model(FilePath& fn, bool hideall) : readonly(true), file(fn), attached(0), db(NULL)
{
   prefix[0] = '\0';
   
   //Create new geometry containers
   init();
   
   for (unsigned int i=0; i < geometry.size(); i++)
   {
      if (hideall)
         geometry[i]->hideAll();
      //Reset static data
      geometry[i]->close();
   }
}

void Model::init()
{
   //Create new geometry containers
   geometry.resize(lucMaxType);
   geometry[lucLabelType] = labels = new Geometry();
   geometry[lucPointType] = points = new Points();
   geometry[lucVectorType] = vectors = new Vectors();
   geometry[lucTracerType] = tracers = new Tracers();
   geometry[lucGridType] = quadSurfaces = new QuadSurfaces(true);
   geometry[lucVolumeType] = volumes = new Volumes();
   geometry[lucTriangleType] = triSurfaces = new TriSurfaces(true);
   geometry[lucLineType] = lines = new Lines();
   geometry[lucShapeType] = shapes = new Shapes();
   debug_print("Created %d new geometry containers\n", geometry.size());
}

Model::~Model()
{
   clearTimeSteps();

   //Clear drawing objects
   for(unsigned int i=0; i<objects.size(); i++)
      if (objects[i]) delete objects[i]; 

   //Clear views
   for(unsigned int i=0; i<views.size(); i++)
      if (views[i]) delete views[i]; 

   //Clear windows
   for(unsigned int i=0; i<windows.size(); i++)
      if (windows[i]) delete windows[i]; 

   //Clear colourmaps
   for(unsigned int i=0; i<colourMaps.size(); i++)
      if (colourMaps[i]) delete colourMaps[i]; 

   if (db) sqlite3_close(db);
}

bool Model::open(bool write)
{
   //Single file database
   char path[256];
   strcpy(path, file.full.c_str());
   if (sqlite3_open_v2(path, &db, write ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY, NULL))
   {
      //Try 0th timestep of multi-file split database
      sprintf(path, "%s%05d.%s", file.base.c_str(), 0, file.ext.c_str());
      if (sqlite3_open_v2(path, &db, write ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY, NULL))
      {
         // failed
         debug_print("Can't open database %s: %s\n", path, sqlite3_errmsg(db));
         return false;
      }
   }
   // success
   debug_print("Open database %s successful, SQLite version %s\n", path, sqlite3_libversion());
   //rc = sqlite3_busy_handler(db, busy_handler, void*);
   sqlite3_busy_timeout(db, 10000); //10 seconds


   if (write) readonly = false;

   return true;
}

void Model::reopen(bool write)
{
   if (!readonly) return;
   if (db) sqlite3_close(db);
   open(write);

   //Re-attach any attached db file
   if (attached)
   {
      char SQL[1024];
      sprintf(SQL, "attach database '%s' as t%d", apath.c_str(), attached);
      if (issue(SQL))
         debug_print("Model %s found and re-attached\n", apath.c_str());
   }
}

void Model::attach(int timestep)
{
   //Detach any attached db file
   char SQL[1024];
   if (attached && attached != timestep)
   {
      sprintf(SQL, "detach database 't%d'", attached);
      if (issue(SQL))
      {
         debug_print("Model t%d detached\n", attached);
         attached = 0;
         prefix[0] = '\0';
      }
      else
         debug_print("Model t%d detach failed!\n", attached);
   }

   //Attach n'th timestep database if available
   if (timestep > 0 && !attached)
   {
      char path[1024];
      FILE* fp;
      sprintf(path, "%s%s%05d.%s", file.path.c_str(), file.base.c_str(), timestep, file.ext.c_str());
      if (fp = fopen(path, "r"))
      {
         fclose(fp);
         sprintf(SQL, "attach database '%s' as t%d", path, timestep);
         if (issue(SQL))
         {
            sprintf(prefix, "t%d.", timestep);
            debug_print("Model %s found and attached\n", path);
            attached = timestep;
            apath = path;
         }
         else
         {
            debug_print("Model %s found but attach failed!\n", path);
         }
      }
      //else
      //   debug_print("Model %s not found, loading from current db\n", path);
   }
}

void Model::close()
{
   for (unsigned int i=0; i < geometry.size(); i++)
      geometry[i]->close();
}

void Model::clearObjects(bool all)
{
   if (FloatValues::membytes > 0 && geometry.size() > 0)
      debug_print("Clearing geometry, geom memory usage before clear %.3f mb\n", FloatValues::membytes/1000000.0f);
   
   //Clear containers...
   for (unsigned int i=0; i < geometry.size(); i++)
   {
      geometry[i]->redraw = true;
      geometry[i]->clear(all);
   }
}

void Model::reset()
{
   //Flag redraw and clear element count...
   for (unsigned int i=0; i < geometry.size(); i++)
      geometry[i]->reset();
}

void Model::loadWindows()
{
   //Load windows list from database and insert into models
   sqlite3_stmt* statement = select("SELECT id,name,width,height,colour,minX,minY,minZ,maxX,maxY,maxZ from window");
   //sqlite3_stmt* statement = model->select("SELECT id,name,width,height,colour,minX,minY,minZ,maxX,maxY,maxZ,properties from window");
   //window (id, name, width, height, colour, minX, minY, minZ, maxX, maxY, maxZ, properties)
   while ( sqlite3_step(statement) == SQLITE_ROW)
   {
      int id = sqlite3_column_int(statement, 0);
      std::string wtitle = std::string((char*)sqlite3_column_text(statement, 1));
      int width = sqlite3_column_int(statement, 2);
      int height = sqlite3_column_int(statement, 3);
      int bg = sqlite3_column_int(statement, 4);
      float min[3], max[3];
      for (int i=0; i<3; i++)
      {
         min[i] = (float)sqlite3_column_double(statement, 5+i);
         max[i] = (float)sqlite3_column_double(statement, 8+i);
      }

      Win* win = new Win(id, wtitle, width, height, bg, min, max);
      windows.push_back(win);

      //Link the window viewports, objects & colourmaps
      loadLinks(win);
   }
   sqlite3_finalize(statement);
}

//Load model viewports
void Model::loadViewports()
{
   sqlite3_stmt* statement;
   statement = select("SELECT id,title,x,y,near,far FROM viewport ORDER BY y,x", true);

   //viewport:
   //(id, title, x, y, near, far, translateX,Y,Z, rotateX,Y,Z, scaleX,Y,Z, properties
   while (sqlite3_step(statement) == SQLITE_ROW)
   {
      unsigned int viewport_id = (unsigned int)sqlite3_column_int(statement, 0);
      std::string vtitle = std::string((char*)sqlite3_column_text(statement, 1));
      float x = (float)sqlite3_column_double(statement, 2);
      float y = (float)sqlite3_column_double(statement, 3);
      float nearc = (float)sqlite3_column_double(statement, 4);
      float farc = (float)sqlite3_column_double(statement, 5);

      //Create the view object
      View* v = new View(vtitle, false, x, y, nearc, farc);
      //Add to list
      if (views.size() < viewport_id) views.resize(viewport_id);
      views[viewport_id-1] = v;
      debug_print("Loaded viewport \"%s\" at %f,%f\n", vtitle.c_str(), x, y);
   }
   sqlite3_finalize(statement);
}

//Load and apply viewport camera settings
void Model::loadViewCamera(int viewport_id)
{
   int adj=0;
   //Load specified viewport and apply camera settings
   char SQL[1024];
   sprintf(SQL, "SELECT aperture,orientation,focalPointX,focalPointY,focalPointZ,translateX,translateY,translateZ,rotateX,rotateY,rotateZ,scaleX,scaleY,scaleZ,properties FROM viewport WHERE id=%d;", viewport_id);
   sqlite3_stmt* statement = select(SQL);
   if (statement == NULL)
   {
      //Old db
      adj = -5;
      sprintf(SQL, "SELECT translateX,translateY,translateZ,rotateX,rotateY,rotateZ,scaleX,scaleY,scaleZ FROM viewport WHERE id=%d;", viewport_id);
      statement = select(SQL);
   }

   //viewport:
   //(aperture, orientation, focalPointX,Y,Z, translateX,Y,Z, rotateX,Y,Z, scaleX,Y,Z, properties
   if (sqlite3_step(statement) == SQLITE_ROW)
   {
      View* v = views[viewport_id-1];
      float aperture = 45.0;
      int orientation = RIGHT_HANDED;
      if (adj == 0)
      {
         aperture = (float)sqlite3_column_double(statement, 0);
         orientation = sqlite3_column_int(statement, 1);
      }
      float focus[3] = {0,0,0}, rotate[3], translate[3], scale[3] = {1,1,1};
      for (int i=0; i<3; i++)
      {
         //camera
         if (adj == 0)
         {
            if (sqlite3_column_type(statement, 2+i) != SQLITE_NULL)
               focus[i] = (float)sqlite3_column_double(statement, 2+i);
            else
               focus[i] = FLT_MIN;
         }
         translate[i] = (float)sqlite3_column_double(statement, 5+i+adj);
         rotate[i] = (float)sqlite3_column_double(statement, 8+i+adj);
         scale[i] = (float)sqlite3_column_double(statement, 11+i+adj);
      }

      const char *vprops = adj == 0 ? (char*)sqlite3_column_text(statement, 14) : "";

      if (adj == 0) v->focus(focus[0], focus[1], focus[2], aperture, true);
      v->translate(translate[0], translate[1], translate[2]);
      v->rotate(rotate[0], rotate[1], rotate[2]);
      v->setScale(scale[0], scale[1], scale[2]);
      v->setCoordSystem(orientation);
      v->setProperties(std::string(vprops));
      //debug_print("Loaded \"%s\" at %f,%f\n");
   }
   sqlite3_finalize(statement);
}

//Load model objects
void Model::loadObjects()
{
   sqlite3_stmt* statement;
   statement = select("SELECT id, name, colour, opacity, properties FROM object", true);
   if (statement == NULL)
      statement = select("SELECT id, name, colour, opacity FROM object");

   //object (id, name, colourmap_id, colour, opacity, properties)
   while (sqlite3_step(statement) == SQLITE_ROW)
   {
      int object_id = sqlite3_column_int(statement, 0);
      const char *otitle = (char*)sqlite3_column_text(statement, 1);
      int colour = sqlite3_column_int(statement, 2);
      float opacity = (float)sqlite3_column_double(statement, 3);

      //Create drawing object and add to master list
      if (sqlite3_column_type(statement, 4) != SQLITE_NULL)
      {
         std::string props = std::string((char*)sqlite3_column_text(statement, 4));
         addObject(new DrawingObject(otitle, colour, NULL, opacity, props, object_id));
      }
      else
         addObject(new DrawingObject(otitle, colour, NULL, opacity, "", object_id));
   }
   sqlite3_finalize(statement);
}

//Load viewports in current window, objects in each viewport, colourmaps for each object
void Model::loadLinks(Win* win)
{
   //Select statment to get all viewports in window and all objects in viewports
   char SQL[1024];
   //sprintf(SQL, "SELECT id,title,x,y,near,far,aperture,orientation,focalPointX,focalPointY,focalPointZ,translateX,translateY,translateZ,rotateX,rotateY,rotateZ,scaleX,scaleY,scaleZ,properties FROM viewport WHERE id=%d;", win->id);
   sprintf(SQL, "SELECT viewport.id,object.id,object.colourmap_id,object_colourmap.colourmap_id,object_colourmap.data_type FROM window_viewport,viewport,viewport_object,object LEFT OUTER JOIN object_colourmap ON object_colourmap.object_id=object.id WHERE window_viewport.window_id=%d AND viewport.id=window_viewport.viewport_id AND viewport_object.viewport_id=viewport.id AND object.id=viewport_object.object_id", win->id);
   sqlite3_stmt* statement = select(SQL, true); //Don't report errors as these tables are allowed to not exist

   int last_viewport = 0, last_object = 0;
   DrawingObject* draw = NULL;
   View* view = NULL;
   while ( sqlite3_step(statement) == SQLITE_ROW)
   {
      int viewport_id = sqlite3_column_int(statement, 0);
      int object_id = sqlite3_column_int(statement, 1);
      int colourmap_id = sqlite3_column_int(statement, 3); //Linked colourmap id

      //Fields from object_colourmap
      lucGeometryDataType colourmap_datatype = (lucGeometryDataType)sqlite3_column_int(statement, 4);
      if (!colourmap_id)
      {
         //Backwards compatibility with old db files
         colourmap_id = sqlite3_column_int(statement, 2);
         colourmap_datatype = lucColourValueData;
      }

      //Get viewport
      view = views[viewport_id-1];
      if (last_viewport != viewport_id)
      {
         win->addView(view);
         loadViewCamera(viewport_id);
         last_viewport = viewport_id;
         last_object = 0;  //Reset required, in case of single object which is shared between viewports
      }

      //Get drawing object
      if (!objects[object_id-1]) continue; //No geometry
      draw = objects[object_id-1];
      if (last_object != object_id)
      {
         view->addObject(draw);
         last_object = object_id;
      }

      //Add colour maps to drawing objects...
      if (colourmap_id)
      {
         //Find colourmap by id
         ColourMap* cmap = colourMaps[colourmap_id-1];
         //Add colourmap to drawing object
         draw->addColourMap(cmap, colourmap_datatype);
      }
   }
   sqlite3_finalize(statement);
}

//Load colourmaps for each object only
void Model::loadLinks(DrawingObject* draw)
{
   //Select statment to get all viewports in window and all objects in viewports
   char SQL[1024];
   //sprintf(SQL, "SELECT id,title,x,y,near,far,aperture,orientation,focalPointX,focalPointY,focalPointZ,translateX,translateY,translateZ,rotateX,rotateY,rotateZ,scaleX,scaleY,scaleZ,properties FROM viewport WHERE id=%d;", win->id);
   sprintf(SQL, "SELECT object.id,object.colourmap_id,object_colourmap.colourmap_id,object_colourmap.data_type FROM object LEFT OUTER JOIN object_colourmap ON object_colourmap.object_id=object.id WHERE object.id=%d", draw->id);
   sqlite3_stmt* statement = select(SQL);

   while ( sqlite3_step(statement) == SQLITE_ROW)
   {
      int object_id = sqlite3_column_int(statement, 0);
      int colourmap_id = sqlite3_column_int(statement, 2); //Linked colourmap id

      //Fields from object_colourmap
      lucGeometryDataType colourmap_datatype = (lucGeometryDataType)sqlite3_column_int(statement, 3);
      if (!colourmap_id)
      {
         //Backwards compatibility with old db files
         colourmap_id = sqlite3_column_int(statement, 1);
         colourmap_datatype = lucColourValueData;
      }

      //Add colour maps to drawing objects...
      if (colourmap_id > 0)
      {
         if (colourMaps.size() < colourmap_id || !colourMaps[colourmap_id-1])
           abort_program("Invalid colourmap id %d\n", colourmap_id);
         //Find colourmap by id
         ColourMap* cmap = colourMaps[colourmap_id-1];
         //Add colourmap to drawing object
         draw->addColourMap(cmap, colourmap_datatype);
      }
   }
   sqlite3_finalize(statement);
}

void Model::clearTimeSteps()
{
   for (int idx=0; idx < timesteps.size(); idx++)
      delete timesteps[idx];
   timesteps.clear();
}

int Model::loadTimeSteps()
{
   if (!db) return timesteps.size();
   //Don't reload timesteps when data has been cached
   if (TimeStep::cachesize > 0 && timesteps.size() > 0) return timesteps.size();
   clearTimeSteps();
   TimeStep::gap = 0;
   int rows = 0;
   int last_step = 0;
   sqlite3_stmt* statement = select("SELECT * FROM timestep");
   //(id, time, dim_factor, units)
   while ( sqlite3_step(statement) == SQLITE_ROW)
   {
      int step = sqlite3_column_int(statement, 0);
      double time = sqlite3_column_double(statement, 1);
      //Next two fields are not present in pre-release databases
      if (sqlite3_column_type(statement, 2) != SQLITE_NULL)
      {
         double dimCoeff = sqlite3_column_double(statement, 2);
         const char* units = (const char*)sqlite3_column_text(statement, 3);
         timesteps.push_back(new TimeStep(step, time, dimCoeff, std::string(units)));
      }
      else
         timesteps.push_back(new TimeStep(step, time));
      //Save gap
      if (step - last_step > TimeStep::gap) TimeStep::gap = step - last_step;
      last_step = step;
      rows++;
   }
   sqlite3_finalize(statement);
   //Copy to static for use in Tracers etc
   TimeStep::timesteps = timesteps;
   return timesteps.size();
}

void Model::loadColourMaps()
{
   sqlite3_stmt* statement = select("SELECT * FROM colourmap,colourvalue WHERE colourvalue.colourmap_id=colourmap.id");
   //colourmap (id, minimum, maximum, logscale, discrete, centreValue) 
   //colourvalue (id, colourmap_id, colour, position) 
   int map_id = 0;
   double minimum;
   double maximum;
   ColourMap* colourMap = NULL;
   while ( sqlite3_step(statement) == SQLITE_ROW)
   {
      int offset = 0;
      int id = sqlite3_column_int(statement, 0);
      char *cmname = NULL;
      char idname[10];
      sprintf(idname, "%d", id);
      //Name added to schema, support old db versions by checking number of fields (8 colourmap + 4 colourvalue)
      if (sqlite3_column_count(statement) == 12)
      {
        offset = 1;
        cmname = (char*)sqlite3_column_text(statement, 1);
      }

      //New map?
      if (id != map_id)
      {
         map_id = id;
         minimum = sqlite3_column_double(statement, 1+offset);
         maximum = sqlite3_column_double(statement, 2+offset);
         int logscale = sqlite3_column_int(statement, 3+offset);
         int discrete = sqlite3_column_int(statement, 4+offset);
         float centreValue = sqlite3_column_double(statement, 5+offset);
         const char *oprops = (char*)sqlite3_column_text(statement, 6+offset);
         colourMap = new ColourMap(id, cmname ? cmname : idname, logscale, discrete, centreValue, minimum, maximum);
         colourMaps.push_back(colourMap);
      }

      //Add colour value
      int colour = sqlite3_column_int(statement, 9+offset);
      //const char *name = sqlite3_column_name(statement, 7);
      if (sqlite3_column_type(statement, 10+offset) != SQLITE_NULL)
      {
         double value = sqlite3_column_double(statement, 10+offset);
         colourMap->add(colour, value);
      }
      else
         colourMap->add(colour);
      //debug_print("ColourMap: %d min %f, max %f Value %d \n", id, minimum, maximum, colour);
   }

   sqlite3_finalize(statement);
   
   //Load default maps and calibrate all
   initColourMaps();
}

void Model::initColourMaps()
{
   //Load default colourmaps
   ColourMap* colourMap = NULL;
   //Colours: hex, abgr
   colourMap = new ColourMap(0, "Greyscale");
   colourMaps.push_back(colourMap);
   unsigned int colours0[] = {0xff000000,0xffffffff};
   colourMap->add(colours0, 2);

   colourMap = new ColourMap(0, "Topology");
   colourMaps.push_back(colourMap);
   unsigned int colours1[] = {0xff33bb66,0xff00ff00,0xffff3333,0xffffff00,0xff77ffff,0xff0088ff,0xff0000ff,0xff000000};
   colourMap->add(colours1, 8);

   colourMap = new ColourMap(0, "Rainbow");
   colourMaps.push_back(colourMap);
   unsigned int colours2[] = {0xfff020a0,0xffff0000,0xff00ff00,0xff00ffff,0xff00a5ff,0xff0000ff,0xff000000};
   colourMap->add(colours2, 7);

   colourMap = new ColourMap(0, "Heat");
   colourMaps.push_back(colourMap);
   unsigned int colours3[] = {0xff000000,0xff0000ff,0xff00ffff,0xffffffff};
   colourMap->add(colours3, 4);

   colourMap = new ColourMap(0, "Bluered");
   colourMaps.push_back(colourMap);
   unsigned int colours4[] = {0xffff0000,0xffff901e,0xffd1ce00,0xffc4e4ff,0xff00a5ff,0xff2222b2};
   colourMap->add(colours4, 6);

   //Initial calibration for all maps
   for (unsigned int i=0; i<colourMaps.size(); i++)
      colourMaps[i]->calibrate();
}

//SQLite3 utility functions
sqlite3_stmt* Model::select(const char* SQL, bool silent)
{
   //debug_print("Issuing select: %s\n", SQL);
   sqlite3_stmt* statement;
   //Prepare statement...
   int rc = sqlite3_prepare_v2(db, SQL, -1, &statement, NULL);
   if (rc != SQLITE_OK)
   {
      if (!silent)
         debug_print("Prepare error (%s) %s\n", SQL, sqlite3_errmsg(db)); 
      return NULL;
   }
   return statement;
} 

bool Model::issue(const char* SQL, sqlite3* odb)
{
   if (!odb) odb = db; //Use existing database
   // Executes a basic SQLite command (ie: without pointer objects and ignoring result sets) and checks for errors
   //debug_print("Issuing: %s\n", SQL);
   char* zErrMsg;
   if (sqlite3_exec(odb, SQL, NULL, 0, &zErrMsg) != SQLITE_OK)
   {
      debug_print("SQLite error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return false;
   }
   return true;
}

void Model::deleteCache()
{
   if (TimeStep::cachesize == 0) return;
   debug_print("~~~ Cache emptied\n");
   //cache.clear();
}

void Model::cacheStep()
{
   //Don't cache if we already loaded from cache or out of range!
   if (TimeStep::cachesize == 0 || now < 0 || now >= timesteps.size()) return;
   if (timesteps[now]->cache.size() > 0) return; //Already cached this step

   debug_print("~~~ Caching geometry @ %d (step %d : %s), geom memory usage: %.3f mb\n", step(), now, file.base.c_str(), FloatValues::membytes/1000000.0f);

   //Copy all elements
   if (FloatValues::membytes > 0)
   {
      timesteps[now]->write(geometry);
      debug_print("~~~ Cached step, at: %d\n", step());
      geometry.clear();
   }
   else
      debug_print("~~~ Nothing to cache\n");

   //TODO: fix support for partial caching?
   /*/Remove if over limit
   if (cache.size() > GeomCache::size)
   {
      GeomCache* cached = cache.front();
      cache.pop_front();
      //Clear containers...
      for (unsigned int i=0; i < cached->store.size(); i++)
      {
         cached->store[i]->clear(true);
         delete cached->store[i];
      }
      debug_print("~~~ Deleted oldest cached step (%d)\n", cached->step);
      delete cached;
   }*/
}

bool Model::restoreStep()
{
   if (now < 0 || TimeStep::cachesize == 0) return false;
   if (timesteps[now]->cache.size() == 0) 
      return false; //Nothing cached this step

   //Load the cache and save loaded timestep
   timesteps[now]->read(geometry);
   debug_print("~~~ Cache hit at ts %d (idx %d), loading! %s\n", step(), now, file.base.c_str());

   //Switch geometry containers
   labels = geometry[lucLabelType];
   points = (Points*)geometry[lucPointType];
   vectors = (Vectors*)geometry[lucVectorType];
   tracers = (Tracers*)geometry[lucTracerType];
   quadSurfaces = (QuadSurfaces*)geometry[lucGridType];
   volumes = (Volumes*)geometry[lucVolumeType];
   triSurfaces = (TriSurfaces*)geometry[lucTriangleType];
   lines = (Lines*)geometry[lucLineType];
   shapes = (Shapes*)geometry[lucShapeType];

   debug_print("~~~ Geom memory usage after load: %.3f mb\n", FloatValues::membytes/1000000.0f);
   reset();  //Force reload
   return true;
}

void Model::printCache()
{
  printf("-----------CACHE %d steps\n", timesteps.size());
   for (int idx=0; idx < timesteps.size(); idx++)
      printf(" %d: has %d records\n", idx, timesteps[idx]->cache.size());
}

std::string Model::timeStamp()
{
   // Timestep (with scaling applied)
   if (timesteps.size() == 0) return std::string("");
   if (now < 0) return std::string("");

   // Use scaling coeff and units to get display time
   TimeStep* ts = timesteps[now];
   char displayString[32];
   sprintf(displayString, "Time %g%s", ts->time * ts->dimCoeff, ts->units.c_str());

   return std::string(displayString);
}

//Set time step if available, otherwise return false and leave unchanged
bool Model::hasTimeStep(int ts)
{
   if (timesteps.size() == 0 && loadTimeSteps() == 0) return false;
   for (int idx=0; idx < timesteps.size(); idx++)
      if (ts == timesteps[idx]->step)
         return true;
   return false;
}

int Model::nearestTimeStep(int requested)
{
   //Find closest matching timestep to requested, returns index
   int idx;
   //if (timesteps.size() == 0 && loadTimeSteps() == 0) return -1;
   if (loadTimeSteps() == 0 || timesteps.size() == 0) return -1;
   //if (timesteps.size() == 1 && now >= 0 && ) return -1;  //Single timestep

   for (idx=0; idx < timesteps.size(); idx++)
      if (timesteps[idx]->step >= requested) break;

   //Reached end of list? 
   if (idx == timesteps.size()) idx--;

   //Unchanged...
   //if (requested >= now && timesteps[idx]->step == now) return 0;

   if (idx < 0) idx = 0;
   if (idx >= timesteps.size()) idx = timesteps.size() - 1;

   return idx;
}

//Load data at specified timestep
int Model::setTimeStep(int stepidx)
{
   clock_t t1 = clock();
   unsigned int idx=0;

   //Default timestep only and no db? Skip load
   if (timesteps.size() == 0 && !db) 
   {
     now = -1;
     return 0;
   }

   if (stepidx < 0) stepidx = 0; //return -1;
   if (stepidx >= timesteps.size())
     stepidx = timesteps.size()-1;

   //Cache currently loaded data
   if (TimeStep::cachesize > 0) cacheStep();

   //Set the new timestep index
   TimeStep::timesteps = timesteps; //Set to current model timestep vector
   now = stepidx;
   debug_print("TimeStep set to: %d\n", step());

   if (restoreStep())
      return 0; //Cache hit successful return value

   //Create new geometry containers if required
   if (geometry.size() == 0) init();

   //Clear any existing geometry
   clearObjects();


   //Attempt to load from cache first
   //if (restoreStep(now)) return 0; //Cache hit successful return value
   if (!db) return 0;

   //Detach any attached db file and attach n'th timestep database if available
   attach(step());

   int rows = 0;
   if (TimeStep::cachesize > 0)
      //Attempt caching all geometry from database at start
      rows += loadGeometry(0, 0, timesteps[timesteps.size()-1]->step, true);
   else
      rows += loadGeometry();

   debug_print("%.4lf seconds to load %d geometry records from database\n", (clock()-t1)/(double)CLOCKS_PER_SEC, rows);
   return rows;
}

int Model::loadGeometry(int obj_id, int time_start, int time_stop, bool recurseTracers)
{
   if (!db)
   {
      std::cerr << "No database loaded!!\n";
      return 0;
   }
   clock_t t1 = clock();

   //Default to current timestep
   if (time_start < 0) time_start = step();
   if (time_stop < 0) time_stop = step();

   //Load geometry
   char SQL[1024];
   char filter[256] = {'\0'};
   char objfilter[32] = {'\0'};

   //Setup filters, object...
   if (obj_id > 0)
   {
      sprintf(objfilter, "WHERE object_id=%d", obj_id);
      //Remove the skip flag now we have explicitly loaded object
      objects[obj_id-1]->skip = false;
   }

   //...timestep...
   if (time_start >= 0 && time_stop >= 0)
   {
      if (strlen(objfilter) > 0)
         sprintf(filter, "%s AND timestep BETWEEN %d AND %d", objfilter, time_start, time_stop);
      else
         sprintf(filter, " WHERE timestep BETWEEN %d AND %d", time_start, time_stop);
   }
   else
      strcpy(filter, objfilter);

   int datacol = 21;
   //object (id, name, colourmap_id, colour, opacity, wireframe, cullface, scaling, lineWidth, arrowHead, flat, steps, time)
   //geometry (id, object_id, timestep, rank, idx, type, data_type, size, count, width, minimum, maximum, dim_factor, units, labels, 
   //minX, minY, minZ, maxX, maxY, maxZ, data)
   sprintf(SQL, "SELECT id,object_id,timestep,rank,idx,type,data_type,size,count,width,minimum,maximum,dim_factor,units,labels,minX,minY,minZ,maxX,maxY,maxZ,data FROM %sgeometry %s ORDER BY timestep,object_id,idx,rank", prefix, filter);
   sqlite3_stmt* statement = select(SQL, true);

   //Old database compatibility
   if (statement == NULL)
   {
      //object (id, name, colourmap_id, colour, opacity, wireframe, cullface, scaling, lineWidth, arrowHead, flat, steps, time)
      //geometry (id, object_id, timestep, rank, idx, type, data_type, size, count, width, minimum, maximum, dim_factor, units, data)
      sprintf(SQL, "SELECT id,object_id,timestep,rank,idx,type,data_type,size,count,width,minimum,maximum,dim_factor,units,labels,data FROM %sgeometry %s ORDER BY timestep,object_id,idx,rank", prefix, filter);
      sqlite3_stmt* statement = select(SQL, true);
      datacol = 15;

      //Fix
#ifdef ALTER_DB
      reopen(true);  //Open writable
      sprintf(SQL, "ALTER TABLE %sgeometry ADD COLUMN minX REAL; ALTER TABLE %sgeometry ADD COLUMN minY REAL; ALTER TABLE %sgeometry ADD COLUMN minZ REAL; "
                   "ALTER TABLE %sgeometry ADD COLUMN maxX REAL; ALTER TABLE %sgeometry ADD COLUMN maxY REAL; ALTER TABLE %sgeometry ADD COLUMN maxZ REAL; ",
                   prefix, prefix, prefix, prefix, prefix, prefix, prefix);
      debug_print("%s\n", SQL);
      issue(SQL);
#endif
   }

   //Very old database compatibility
   if (statement == NULL)
   {
      sprintf(SQL, "SELECT id,object_id,timestep,rank,idx,type,data_type,size,count,width,minimum,maximum,dim_factor,units,data FROM %sgeometry %s ORDER BY timestep,object_id,idx,rank", prefix, filter);
      statement = select(SQL);
      datacol = 14;
   }

   if (!statement) return 0;
   int rows = 0;
   int tbytes = 0;
   int ret;
   Geometry* active = NULL;
   do 
   {  
      ret = sqlite3_step(statement);
      if (ret == SQLITE_ROW)
      {
         rows++;
         int id = sqlite3_column_int(statement, 0);
         int object_id = sqlite3_column_int(statement, 1);
         int timestep = sqlite3_column_int(statement, 2);
         int height = sqlite3_column_int(statement, 3);  //unused - was rank, now height
         int depth = sqlite3_column_int(statement, 4); //unused - was idx, now depth
         lucGeometryType type = (lucGeometryType)sqlite3_column_int(statement, 5);
         lucGeometryDataType data_type = (lucGeometryDataType)sqlite3_column_int(statement, 6);
         int size = sqlite3_column_int(statement, 7);
         int count = sqlite3_column_int(statement, 8);
         int items = count / size;
         int width = sqlite3_column_int(statement, 9);
         if (height == 0) height = width > 0 ? items / width : 0;
         float minimum = (float)sqlite3_column_double(statement, 10);
         float maximum = (float)sqlite3_column_double(statement, 11);
         //New fields for the scaling features, applied when drawing colour bars
         float dimFactor = (float)sqlite3_column_double(statement, 12);
         const char *units = (const char*)sqlite3_column_text(statement, 13);
         const char *labels = datacol < 15 ? "" : (const char*)sqlite3_column_text(statement, 14);

         const void *data = sqlite3_column_blob(statement, datacol);
         unsigned int bytes = sqlite3_column_bytes(statement, datacol);

         DrawingObject* obj = objects[object_id-1];

         //Skip object? (When noload enabled)
         if (obj->skip) continue;

         //Bulk load: switch timestep and cache if timestep changes!
         if (step() != timestep)
         {
            cacheStep();
            now = nearestTimeStep(timestep);
            debug_print("TimeStep set to: %d, rows %d\n", step(), rows);
         }

         //Create new geometry containers if required
         if (geometry.size() == 0) init();

         if (type == lucTracerType) height = 0;

         //Create object and set parameters
         if (type == lucPointType && pointspheres) type = lucShapeType;
         //if (type == lucGridType) type = lucTriangleType;
         active = geometry[type];

         if (recurseTracers && type == lucTracerType)
         {
            //if (datacol == 13) continue;  //Don't bother supporting tracers from old dbs
            //Only load tracer timesteps when on the vertex data object or will repeat for every type found
            if (data_type != lucVertexData) continue;

            //Tracers are loaded with a new select statement across multiple timesteps...
            //objects[object_id]->steps = timestep+1;
            Tracers* tracers = (Tracers*)active;
            tracers->timestep = timestep; //Set current timestep for tracers
            int stepstart = 0; //timestep - objects[object_id]->steps;
            //int stepstart = timestep - tracers->steps;

            loadGeometry(object_id, stepstart, timestep, false);
         }
         else
         {
            unsigned char* buffer = NULL;
            if (bytes != (unsigned int)(count * sizeof(float)))
            {
               //Decompress!
               unsigned long dst_len = (unsigned long)(count * sizeof(float));
               unsigned long uncomp_len = dst_len;
               unsigned long cmp_len = bytes;
               buffer = new unsigned char[dst_len];
               if (!buffer)
                  abort_program("Out of memory!\n");

#ifdef USE_ZLIB
               int res = uncompress(buffer, &uncomp_len, (const unsigned char *)data, cmp_len);
               if (res != Z_OK || dst_len != uncomp_len)
#else
               int res = tinfl_decompress_mem_to_mem(buffer, uncomp_len, (const unsigned char *)data, cmp_len, TINFL_FLAG_PARSE_ZLIB_HEADER);
               if (!res)
#endif
               {
                  abort_program("uncompress() failed! error code %d\n", res);
                  //abort_program("uncompress() failed! error code %d expected size %d actual size %d\n", res, dst_len, uncomp_len);
               }
               data = buffer; //Replace data pointer
               bytes = uncomp_len;
            }

            tbytes += bytes;   //Byte counter

            //Always add a new element for each new vertex geometry record, not suitable if writing db on multiple procs!
            if (data_type == lucVertexData && recurseTracers) active->add(obj);

            //Read data block
            GeomData* g = active->read(obj, items, data_type, data, width, height, depth);
            active->setup(obj, data_type, minimum, maximum, dimFactor, units);
            if (labels) active->label(obj, labels);

            //Where min/max vertex provided, load
            if (data_type == lucVertexData)
            {
               float min[3] = {0,0,0}, max[3] = {0,0,0};
               if (datacol > 15 && sqlite3_column_type(statement, 15) != SQLITE_NULL)
               {
                  for (int i=0; i<3; i++)
                  {
                     min[i] = (float)sqlite3_column_double(statement, 15+i);
                     max[i] = (float)sqlite3_column_double(statement, 18+i);
                  }
               }

               //Detect null dims data due to bugs in dimension output
               if (min[0] != max[0] || min[1] != max[1] || min[2] != max[2])
               {
                  g->checkPointMinMax(min);
                  g->checkPointMinMax(max);
               }
               else
               {
                  //Slow way, detects bounding box by checking each vertex
                  for (int p=0; p < items*3; p += 3)
                     g->checkPointMinMax((float*)data + p);

                  debug_print("No bounding dims provided for object %d, calculated for %d vertices...%f,%f,%f - %f,%f,%f\n", obj->id, items, g->min[0], g->min[1], g->min[2], g->max[0], g->max[1], g->max[2]);

                  //Fix for future loads
#ifdef ALTER_DB
                  reopen(true);  //Open writable
                  sprintf(SQL, "UPDATE %sgeometry SET minX = '%f', minY = '%f', minZ = '%f', maxX = '%f', maxY = '%f', maxZ = '%f' WHERE id==%d;", 
                          prefix, id, obj->min[0], obj->min[1], obj->min[2], obj->max[0], obj->max[1], obj->max[2]);
                  printf("%s\n", SQL);
                  issue(SQL);
#endif
               }
            }



            if (buffer) delete[] buffer;
   #if 0
            char* types[8] = {"", "POINTS", "GRID", "TRIANGLES", "VECTORS", "TRACERS", "LINES", "SHAPES"};
            if (data_type == lucVertexData)
               printf("[object %d time %d] Read %d vertices into %s object (idx %d) %d x %d\n", 
                        object_id, timestep, items, types[type], active->size()-1, width, height);
            else printf("[object %d time %d] Read %d values of dtype %d into %s object (idx %d) min %f max %f\n", 
                        object_id, timestep, items, data_type, types[type], active->size()-1, minimum, maximum);
            if (labels) printf(labels);
   #endif
         }
      }
      else if (ret != SQLITE_DONE)
         printf("DB STEP FAIL %d %d\n", ret, (ret>>8));
   } while (ret == SQLITE_ROW);

   sqlite3_finalize(statement);
   debug_print("... loaded %d rows, %d bytes, %.4lf seconds\n", rows, tbytes, (clock()-t1)/(double)CLOCKS_PER_SEC);

   return rows;
}

void Model::mergeDatabases()
{
   char SQL[512];
   reopen(true);  //Open writable
   for (int i=0; i<=timesteps.size(); i++)
   {
      debug_print("MERGE %d/%d...%d\n", i, timesteps.size(), step());
      setTimeStep(i);
      if (attached == step())
      {
         sprintf(SQL, "insert into geometry select null, object_id, timestep, rank, idx, type, data_type, size, count, width, minimum, maximum, dim_factor, units, labels, properties, data, minX, minY, minZ, maxX, maxY, maxZ from %sgeometry", prefix);
         issue(SQL);
      }
   }
}

int Model::decompressGeometry(int timestep)
{
   reopen(true);  //Open writable
   clock_t t1 = clock();
   //Load geometry
   char SQL[1024];

   sprintf(SQL, "SELECT id,count,data FROM %sgeometry WHERE timestep=%d ORDER BY idx,rank", prefix, timestep);
   sqlite3_stmt* statement = select(SQL, true);

   if (!statement) return 0;
   int rows = 0;
   int tbytes = 0;
   int ret;
   do 
   {  
      ret = sqlite3_step(statement);
      if (ret == SQLITE_ROW)
      {
         rows++;
         int id = sqlite3_column_int(statement, 0);
         int count = sqlite3_column_int(statement, 1);
         const void *data = sqlite3_column_blob(statement, 2);
         unsigned int bytes = sqlite3_column_bytes(statement, 2);

         unsigned char* buffer = NULL;
         if (bytes != (unsigned int)(count * sizeof(float)))
         {
            //Decompress!
            unsigned long dst_len = (unsigned long )(count * sizeof(float));
            unsigned long uncomp_len = dst_len;
            unsigned long cmp_len = bytes;
            buffer = new unsigned char[dst_len];
            if (!buffer)
               abort_program("Out of memory!\n");

#ifdef USE_ZLIB
            int res = uncompress(buffer, &uncomp_len, (const unsigned char *)data, cmp_len);
            if (res != Z_OK || dst_len != uncomp_len)
#else
            int res = tinfl_decompress_mem_to_mem(buffer, uncomp_len, (const unsigned char *)data, cmp_len, TINFL_FLAG_PARSE_ZLIB_HEADER);
            if (!res)
#endif
            {
               abort_program("uncompress() failed! error code %d\n", res);
               //abort_program("uncompress() failed! error code %d expected size %d actual size %d\n", res, dst_len, uncomp_len);
            }
            data = buffer; //Replace data pointer
            bytes = uncomp_len;
#ifdef ALTER_DB
   //UNCOMPRESSED
   sqlite3_stmt* statement2;
   snprintf(SQL, 1024, "UPDATE %sgeometry SET data = ? WHERE id==%d;", prefix, id);
   printf("%s\n", SQL);
   /* Prepare statement... */
   if (sqlite3_prepare_v2(db, SQL, -1, &statement2, NULL) != SQLITE_OK)
   {
      printf("SQL prepare error: (%s) %s\n", SQL, sqlite3_errmsg(db));
      abort(); //Database errors fatal?
      return 0;
   }
   /* Setup blob data for insert */
   if (sqlite3_bind_blob(statement2, 1, buffer, bytes, SQLITE_STATIC) != SQLITE_OK)
   {
      printf("SQL bind error: %s\n", sqlite3_errmsg(db));
      abort(); //Database errors fatal?
   }
   /* Execute statement */
   if (sqlite3_step(statement2) != SQLITE_DONE )
   {
      printf("SQL step error: (%s) %s\n", SQL, sqlite3_errmsg(db));
      abort(); //Database errors fatal?
   }
   sqlite3_finalize(statement2);
#endif
            }

            if (buffer) delete[] buffer;

      }
      else if (ret != SQLITE_DONE)
         printf("DB STEP FAIL %d %d\n", ret, (ret>>8));
   } while (ret == SQLITE_ROW);

   sqlite3_finalize(statement);
   debug_print("... modified %d rows, %d bytes, %.4lf seconds\n", rows, tbytes, (clock()-t1)/(double)CLOCKS_PER_SEC);

   return rows;
}

void Model::writeDatabase(const char* path, unsigned int id, bool compress)
{
   //TODO: implement zlib compression
   //Write objects to a new database
   sqlite3 *outdb;
   if (sqlite3_open_v2(path, &outdb, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL))
   {
      debug_print("Can't open database %s: %s\n", path, sqlite3_errmsg(outdb));
      return;
   }
   
   // Remove existing data?
   issue("drop table IF EXISTS geometry", outdb);
   issue("drop table IF EXISTS timestep", outdb);
   issue("drop table IF EXISTS object_colourmap", outdb);
   issue("drop table IF EXISTS colourmap", outdb);
   issue("drop table IF EXISTS colourvalue", outdb);
   issue("drop table IF EXISTS object", outdb);

   // Create new tables when not present
   issue("create table IF NOT EXISTS geometry (id INTEGER PRIMARY KEY ASC, object_id INTEGER, timestep INTEGER, rank INTEGER, idx INTEGER, type INTEGER, data_type INTEGER, size INTEGER, count INTEGER, width INTEGER, minimum REAL, maximum REAL, dim_factor REAL, units VARCHAR(32), minX REAL, minY REAL, minZ REAL, maxX REAL, maxY REAL, maxZ REAL, labels VARCHAR(2048), properties VARCHAR(2048), data BLOB, FOREIGN KEY (object_id) REFERENCES object (id) ON DELETE CASCADE ON UPDATE CASCADE, FOREIGN KEY (timestep) REFERENCES timestep (id) ON DELETE CASCADE ON UPDATE CASCADE)", outdb);

   issue(
      "create table IF NOT EXISTS timestep (id INTEGER PRIMARY KEY ASC, time REAL, dim_factor REAL, units VARCHAR(32), properties VARCHAR(2048))", outdb);

   issue(
      "create table object_colourmap (id integer primary key asc, object_id integer, colourmap_id integer, data_type integer, foreign key (object_id) references object (id) on delete cascade on update cascade, foreign key (colourmap_id) references colourmap (id) on delete cascade on update cascade)", outdb);

   issue(
      "create table object (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), colourmap_id INTEGER, colour INTEGER, opacity REAL, properties VARCHAR(2048), FOREIGN KEY (colourmap_id) REFERENCES colourmap (id) ON DELETE CASCADE ON UPDATE CASCADE)", outdb); 

   issue(
      "create table colourvalue (id INTEGER PRIMARY KEY ASC, colourmap_id INTEGER, colour INTEGER, value REAL, FOREIGN KEY (colourmap_id) REFERENCES colourmap (id) ON DELETE CASCADE ON UPDATE CASCADE)", outdb); 

   issue(
      "create table colourmap (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), minimum REAL, maximum REAL, logscale INTEGER, discrete INTEGER, centreValue REAL, properties VARCHAR(2048))", outdb); 
   
   issue("BEGIN EXCLUSIVE TRANSACTION", outdb);

   char SQL[1024];

   //Write colour maps
   for (unsigned int i = 0; i < colourMaps.size(); i++)
   {
      ColourMap* cm = colourMaps[i];
      //if (cm->id < 0) continue; //TODO: Hard-coded maps are written and double up
      snprintf(SQL, 1024, "insert into colourmap (id, name, minimum, maximum, logscale, discrete, centreValue) values (%d, '%s', %g, %g, %d, %d, %g)", cm->id, cm->name.c_str(), cm->minimum, cm->maximum, cm->log, cm->discrete, 0.0 );
      //printf("%s\n", SQL);
      if (!issue(SQL, outdb)) return;

      /* Write colours and values */
      for (int c=0; c< cm->colours.size(); c++)
      {
         snprintf(SQL, 1024, "insert into colourvalue (colourmap_id, colour, value) values (%d, %d, %g)", 
                 cm->id, cm->colours[c].colour.value, cm->colours[c].position * (cm->maximum - cm->minimum) + cm->minimum);
         //printf("%s\n", SQL);
         if (!issue(SQL, outdb)) return;
      }
   }

   //Write objects
   for (unsigned int i=0; i < objects.size(); i++)
   {
      if (objects[i] && (id == 0 || objects[i]->id == id))
      {
         std::string props = json::Serialize(objects[i]->properties);
         Colour c = Colour_FromJson(objects[i]->properties, "colour"); //Write as seperate param too?
         int cmap = 0;
         if (objects[i]->colourMaps[lucColourValueData]) cmap = objects[i]->colourMaps[lucColourValueData]->id;
         snprintf(SQL, 1024, "insert into object (id, name, colourmap_id, colour, opacity, properties) values (%d, '%s', '%d', %d, %g, '%s')", objects[i]->id, objects[i]->name.c_str(), cmap, c.value, objects[i]->properties["opacity"].ToFloat(1.0), props.c_str()); 
         //printf("%s\n", SQL);
         if (!issue(SQL, outdb)) return; 

         /* Add colourmap reference for object */
         for (unsigned int c = 0; c < objects[i]->colourMaps.size(); c++)
         {
            if (!objects[i]->colourMaps[c]) continue;
            /* Link object & colour map */
            snprintf(SQL, 1024, "insert into object_colourmap (object_id, colourmap_id, data_type) values (%d, %d, %d)", objects[i]->id, objects[i]->colourMaps[c]->id, c); 
            printf("%s\n", SQL);
            if (!issue(SQL, outdb)) return;
         }
      }
   }

   //Write timesteps...
   if (timesteps.size() == 0) addTimeStep(0);
   for (unsigned int i = 0; i < timesteps.size(); i++)
   {
      snprintf(SQL, 1024, "insert into timestep (id, time, dim_factor, units, properties) values (%d, %g, %g, '%s', '%s')", timesteps[i]->step, timesteps[i]->time, timesteps[i]->dimCoeff, timesteps[i]->units.c_str(), ""); 
      //printf("%s\n", SQL);
      if (!issue(SQL, outdb)) return; 

      //Get data at this timestep
      setTimeStep(i);

      //Write object data
      for (unsigned int i=0; i < objects.size(); i++)
      {
         if (objects[i] && (id == 0 || objects[i]->id == id))
         {
            //Loop through all geometry classes (points/vectors etc)
            for (int type=lucMinType; type<lucMaxType; type++)
            {
               writeGeometry(outdb, (lucGeometryType)type, objects[i]->id, compress);
            }
         }
      }
   }
   
   issue("COMMIT", outdb);
}

void Model::writeGeometry(sqlite3* outdb, lucGeometryType type, int obj_id, bool compressdata)
{
   std::vector<GeomData*> data = geometry[type]->getAllObjects(obj_id);
   //Loop through and write out all object data
   char SQL[1024];
   int i, data_type;
   for (i=0; i<data.size(); i++)
   {
     for (data_type=lucMinDataType; data_type<lucMaxDataType; data_type++)
     {
        if (data[i]->data[data_type]->size() == 0) continue;
        std::cerr << "Writing geometry (" << data[i]->data[data_type]->size() << " : " 
                  << data_type <<  ") for object : " << obj_id << " => " << objects[obj_id-1]->name << std::endl;
        //Get the data block
        FloatValues* block = data[i]->data[data_type];

        sqlite3_stmt* statement;
        unsigned char* buffer = (unsigned char*)&block->value[0];
        unsigned long src_len = block->size() * sizeof(float);
        // Compress the data if enabled and > 1kb
        unsigned long cmp_len = 0;
        if (compressdata &&  src_len > 1000)
        {
           cmp_len = compressBound(src_len);
           buffer = (unsigned char*)malloc((size_t)cmp_len);
           if (buffer == NULL)
              abort_program("Compress database: out of memory!\n");
           if (compress(buffer, &cmp_len, (const unsigned char *)&block->value[0], src_len) != Z_OK) 
              abort_program("Compress database buffer failed!\n");
           if (cmp_len >= src_len)
           {
              free(buffer);
              buffer = (unsigned char*)&block->value[0];
              cmp_len = 0;
           }
           else
              src_len = cmp_len;
        }

        if (block->minimum == HUGE_VAL) block->minimum = 0;
        if (block->maximum == -HUGE_VAL) block->maximum = 0;
        
        float *min = data[i]->min;
        float *max = data[i]->max;

        snprintf(SQL, 1024, "insert into geometry (object_id, timestep, rank, idx, type, data_type, size, count, width, minimum, maximum, dim_factor, units, minX, minY, minZ, maxX, maxY, maxZ, labels, data) values (%d, %d, %d, %d, %d, %d, %d, %d, %d, %g, %g, %g, '%s', %g, %g, %g, %g, %g, %g, ?, ?)", obj_id, step(), data[i]->height, data[i]->depth, type, data_type, block->datasize, block->size(), data[i]->width, block->minimum, block->maximum, 0.0, "", min[0], min[1], min[2], max[0], max[1], max[2]);

        /* Prepare statement... */
        if (sqlite3_prepare_v2(outdb, SQL, -1, &statement, NULL) != SQLITE_OK)
        {
           abort_program("SQL prepare error: (%s) %s\n", SQL, sqlite3_errmsg(outdb));
        }

        /* Setup text data for insert */
        const char* labels = data[i]->getLabels();
        if (labels)
        {
           if (sqlite3_bind_text(statement, 1, labels, strlen(labels), SQLITE_STATIC) != SQLITE_OK)
              abort_program("SQL bind error: %s\n", sqlite3_errmsg(outdb));
        }

        /* Setup blob data for insert */
        if (sqlite3_bind_blob(statement, 2, buffer, src_len, SQLITE_STATIC) != SQLITE_OK)
           abort_program("SQL bind error: %s\n", sqlite3_errmsg(outdb));

        /* Execute statement */
        if (sqlite3_step(statement) != SQLITE_DONE )
           abort_program("SQL step error: (%s) %s\n", SQL, sqlite3_errmsg(outdb));

        sqlite3_finalize(statement);

        // Free compression buffer
        if (cmp_len > 0) free(buffer);
     }
   }
}

void Model::deleteObject(int id)
{
   reopen(true);  //Open writable
   char SQL[256];
   sprintf(SQL, "DELETE FROM object WHERE id==%1$d; DELETE FROM geometry WHERE object_id=%1$d; DELETE FROM viewport_object WHERE object_id=%1$d;", id);
   issue(SQL);
   issue("vacuum");
   for (unsigned int i=0; i<objects.size(); i++)
   {
      if (!objects[i]) continue;
      if (id == objects[i]->id)
      {
         objects[i] = NULL; //Don't erase as objects referenced by id
         break;
      }
   }
}

void Model::backup(sqlite3 *fromDb, sqlite3* toDb)
{
   sqlite3_backup *pBackup;  // Backup object used to copy data
   pBackup = sqlite3_backup_init(toDb, "main", fromDb, "main");
   if (pBackup)
   {
      (void)sqlite3_backup_step(pBackup, -1);
      (void)sqlite3_backup_finish(pBackup);
   }
}
