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

#include "LavaVu.h"
#include "ColourMap.h"

/////////////////////////////////////////////////////////////////////////////////
// Event handling
/////////////////////////////////////////////////////////////////////////////////
bool LavaVu::mousePress(MouseButton btn, bool down, int x, int y) 
{
   //Only process on mouse release
   static bool translated = false;
   bool redraw = false;
   int scroll = 0;
   viewer->notIdle(); //Reset idle timer
   if (down)
   {
      translated = false;
      if (viewPorts) viewSelect(viewFromPixel(x, y));  //Update active viewport
      viewer->button = btn;
      viewer->last_x = x;
      viewer->last_y = y;
   }
   else
   {
      aview->inertia(false); //Clear inertia

      //switch (viewer->button)
      switch (btn)
      {
      case WheelUp:
         scroll = 1;
         redraw = true;
         break;
      case WheelDown:
         scroll = -1;
         redraw = true;
         break;
      case LeftButton:
         if (!viewer->keyState.alt && !viewer->keyState.shift)
            aview->rotated = true;
         redraw = true;
         break;
      case MiddleButton:
         aview->rotated = true;
         break;
      case RightButton:
         if (!viewer->keyState.alt && !viewer->keyState.shift)
            translated = true;
      default:
         break;
      }

      if (scroll) mouseScroll(scroll);

      //Update cam move in history
      if (translated) record(true, aview->translateString());
      if (aview->rotated) record(true, aview->rotateString());

      viewer->button = NoButton;
   }
   return redraw;
}

bool LavaVu::mouseMove(int x, int y)
{
   float adjust;
   int dx = x - viewer->last_x;
   int dy = y - viewer->last_y;
   viewer->last_x = x;
   viewer->last_y = y;
   viewer->notIdle(); //Reset idle timer

   //For mice with no right button, ctrl+left 
   if (viewer->keyState.ctrl && viewer->button == LeftButton)
      viewer->button = RightButton;

   switch (viewer->button) {
   case LeftButton:
      if (viewer->keyState.alt || viewer->keyState.shift)
         //Mac glut scroll-wheel alternative
         record(true, aview->zoom(-dy * 0.01));
      else
      {
         // left = rotate
         aview->rotate(dy / 5.0f, dx / 5.0, 0.0f);
      }
      break;
   case RightButton:
      if (viewer->keyState.alt || viewer->keyState.shift)
         //Mac glut scroll-wheel alternative
         record(true, aview->zoomClip(-dy * 0.001));
      else
      {
         // right = translate
         adjust = aview->model_size / 1000.0;   //1/1000th of size
         aview->translate(dx * adjust, -dy * adjust, 0);
      }
      break;
   case MiddleButton:
      // middle = rotate z (roll)
      aview->rotate(0.0f, 0.0f, dx / 5.0f);
      break;
   default: 
      return false;
   }

   return true;
}

bool LavaVu::mouseScroll(int scroll) 
{
   //Only process on mouse release
   //Process wheel scrolling
   //CTRL+ALT+SHIFT = eye-separation adjust
   if (viewer->keyState.alt && viewer->keyState.shift && viewer->keyState.ctrl)
      record(true, aview->adjustStereo(0, 0, scroll / 500.0));
   //ALT+SHIFT = focal-length adjust
   if (viewer->keyState.alt && viewer->keyState.shift)
      record(true, aview->adjustStereo(0, scroll * aview->model_size / 100.0, 0));
   //CTRL+SHIFT - fine adjust near clip-plane
   if (viewer->keyState.shift && viewer->keyState.ctrl)
      record(true, aview->zoomClip(scroll * 0.001));
   //SHIFT = move near clip-plane
   else if (viewer->keyState.shift)
      record(true, aview->zoomClip(scroll * 0.01));
   //ALT = adjust field of view (aperture)
   else if (viewer->keyState.alt)
      record(true, aview->adjustStereo(scroll, 0, 0));
   else if (viewer->keyState.ctrl)
      //Fast zoom
      record(true, aview->zoom(scroll * 0.1));
   //Default = slow zoom
   else
      record(true, aview->zoom(scroll * 0.01));

   return true;
}

bool LavaVu::keyPress(unsigned char key, int x, int y)
{
   viewer->notIdle(); //Reset idle timer
   if (viewPorts) viewSelect(viewFromPixel(x, y));  //Update active viewport
   return parseChar(key);
}

void LavaVu::record(bool mouse, std::string command)
{
   command.erase(command.find_last_not_of("\n\r")+1);

   if (!recording) return;
   //This is to allow capture of history from stdout
   if (output) std::cout << command << std::endl;
   //std::cerr << command << std::endl;
   history.push_back(command);
   //Add to linehistory only if not a duplicate
   if (!mouse)
   {
      int lend = linehistory.size()-1;
      if (lend < 0 || command != linehistory[lend])
         linehistory.push_back(command);
   }
}

bool LavaVu::parseChar(unsigned char key)
{
   static int historyline = -1;
   int hline = historyline;
   if (hline < 0) hline = linehistory.size();
   bool response = true;
   bool msg = false;
   historyline = -1;

   //ALT commands
   if (viewer->keyState.alt)
   {
      switch(key) 
      {
      case KEY_UP:    return parseCommands("model up");
      case KEY_DOWN:  return parseCommands("model down");
      case '*':       return parseCommands("autozoom");
      case '/':       return parseCommands("stereo");
      case '\\':      return parseCommands("coordsystem");
      case ',':       return parseCommands("pointtype all");
      case '-':       return parseCommands("pointsample down");
      case '+':       return parseCommands("pointsample up");
      case '=':       return parseCommands("pointsample up");
      case '|':       return parseCommands("rulers");
      case 'a':       return parseCommands("axis");
      case 'b':       return parseCommands("background invert");
      case 'c':       return parseCommands("camera");
      case 'd':       return parseCommands("trianglestrips");
      case 'e':       return parseCommands("list elements");
      case 'f':       return parseCommands("border");
      case 'g':       return parseCommands("logscales");
      case 'h':       return parseCommands("history");
      case 'i':       return parseCommands("image");
      case 'j':       return parseCommands("localise");
      case 'k':       return parseCommands("lockscale");
      case 'u':       return parseCommands("cullface");
      case 'w':       return parseCommands("wireframe");
      case 'J':       return parseCommands("json");
      case 'o':       return parseCommands("list objects");
      case 'q':       return parseCommands("quit");
      case 'r':       return parseCommands("reset");
      case 'p':       return parseCommands("scale points up");
      case 's':       return parseCommands("scale shapes up");
      case 't':       return parseCommands("scale tracers up");
      case 'v':       return parseCommands("scale vectors up");
      case 'l':       return parseCommands("scale lines up");
      case 'R':       return parseCommands("reload");
      case 'B':       return parseCommands("background");
      case 'S':       return parseCommands("scale shapes down");
      case 'V':       return parseCommands("scale vectors down");
      case 'T':       return parseCommands("scale tracers down");
      case 'P':       return parseCommands("scale points down");
      case 'L':       return parseCommands("scale lines down");
      case ' ':    
         if (loop)
            return parseCommands("stop");
         else
            return parseCommands("play");
      default:        return false;
      }
   }

   //Direct commands and verbose command entry
   switch (key) 
   {
   case KEY_RIGHT:    return parseCommands("timestep down");
   case KEY_LEFT:     return parseCommands("timestep up");
   case KEY_ESC:      return parseCommands("quit");
   case KEY_ENTER:
      if (entry.length() == 0)
         return parseCommands("repeat");
      if (entry.length() == 1)
      {
         char ck = entry.at(0);
         //Digit? (9 == ascii 57)
         if (ck > 47 && ck < 58)
         {
           response = parseCommands(entry);
           entry = "";
           return response;
         }
         switch (ck)
         {
            case 'p':   //Points
               if (Model::points->allhidden)
                 response = parseCommands("show points");
               else
                 response = parseCommands("hide points");
               break;
            case 'v':   //Vectors
               if (Model::vectors->allhidden)
                 response = parseCommands("show vectors");
               else
                 response = parseCommands("hide vectors");
               break;
            case 't':   //Tracers
               if (Model::tracers->allhidden)
                 response = parseCommands("show tracers");
               else
                 response = parseCommands("hide tracers");
               break;
            case 'u':   //TriSurfaces
               if (Model::triSurfaces->allhidden)
                 response = parseCommands("show triangles");
               else
                 response = parseCommands("hide triangles");
               break;
            case 'q':   //QuadSurfaces
               if (Model::quadSurfaces->allhidden)
                 response = parseCommands("show quads");
               else
                 response = parseCommands("hide quads");
               break;
            case 's':   //Shapes
               if (Model::shapes->allhidden)
                 response = parseCommands("show shapes");
               else
                 response = parseCommands("hide shapes");
               break;
            case 'l':   //Lines
               if (Model::lines->allhidden)
                 response = parseCommands("show lines");
               else
                 response = parseCommands("hide lines");
               break;
            default:
               //Parse as if entered with ALT
               viewer->keyState.alt = true;
               response = parseChar(ck);
         }
      }
      else
      {
        response = parseCommands(entry);
      }
      entry = "";
      break;
   case KEY_DELETE:
   case KEY_BACKSPACE:  //Backspace
      if (entry.size() > 0)
      {
         std::string::iterator it;
         it = entry.end() - 1;
         entry.erase(it);
      }
      msg = true;
      break;
   case KEY_UP:
      historyline = hline-1;
      if (linehistory.size())
      {
         if (historyline < 0) historyline = 0;
         entry = linehistory[historyline];
      }
      msg = true;
      break;
   case KEY_DOWN:
      historyline = hline+1;
      if (historyline >= linehistory.size())
      {
         historyline = -1;
         entry = "";
      }
      else
         entry = linehistory[historyline];
      msg = true;
      break;
   case KEY_PAGEUP:
      //Previous viewport
      if (viewAll) 
      {
         viewAll = false;
         redrawObjects();
      }
      viewSelect(view-1);
      printMessage("Set viewport to %d", view);
      if (!viewPorts) redrawObjects();
      break;
   case KEY_PAGEDOWN:
      //Next viewport
      if (viewAll) 
      {
         viewAll = false;
         redrawObjects();
      }
      viewSelect(view+1);
      printMessage("Set viewport to %d", view);
      if (!viewPorts) redrawObjects();
      break;
   case KEY_HOME:
      //All viewports
      viewAll = !viewAll;
      if (viewAll) viewPorts = false; //Invalid in viewAll mode
      redrawObjects();
      printMessage("View All set to %s", (viewAll ? "ON" : "OFF"));
      break;
   case KEY_END:
      //No viewports (use first view but full window and all objects)
      viewPorts = !viewPorts;
      if (viewPorts) viewAll = false;  //Invalid in viewPorts mode
      redrawObjects();
      printMessage("ViewPorts set to %s", (viewPorts ? "ON" : "OFF"));
      break;
   case '`':    return parseCommands("fullscreen");
   case KEY_F1: return parseCommands("help");
   case KEY_F2: return parseCommands("antialias");
   case KEY_TAB:
      //Tab-completion from history
      for (int l=history.size()-1; l>=0; l--)
      {
         std::cout << entry << " ==? " << history[l].substr(0, entry.length()) << std::endl;
         if (entry == history[l].substr(0, entry.length()))
         {
            entry = history[l];
            msg = true;
            break;
         }
      }
      break;
   default:
      //Only add printable characters
      if (key > 31 && key < 127)
        entry += key;
      msg = true;
      break;
   }

   if (entry.length() > 0 && msg)
   {
     printMessage(": %s", entry.c_str());
     response = false;
     viewer->postdisplay = true;
   }

   return response;
}

Geometry* LavaVu::getGeometryType(std::string what)
{
   if (what == "points")
      return Model::points;
   if (what == "labels")
      return Model::labels;
   if (what == "vectors")
      return Model::vectors;
   if (what == "tracers")
      return Model::tracers;
   if (what == "triangles")
      return Model::triSurfaces;
   if (what == "quads")
      return Model::quadSurfaces;
   if (what == "shapes")
      return Model::shapes;
   if (what == "lines")
      return Model::lines;
   if (what == "volumes")
      return Model::volumes;
   return NULL;
}

DrawingObject* LavaVu::findObject(std::string what, int id, bool nodefault)
{
   //Find by name/ID match in all drawing objects
   std::transform(what.begin(), what.end(), what.begin(), ::tolower);
   for (unsigned int i=0; i<amodel->objects.size(); i++)
   {
      if (!amodel->objects[i]) continue;
      std::string namekey = amodel->objects[i]->name;
      std::transform(namekey.begin(), namekey.end(), namekey.begin(), ::tolower);
      if (namekey == what || id > 0 && id == amodel->objects[i]->id)
      {
         //std::cerr << "Found by " << (namekey == what ? " NAME : " : " ID: ") << what << " -- " << id << std::endl;
         return amodel->objects[i];
      }
   }
   //std::cerr << "Not found, returning " << (nodefault ? "NULL" : "DEFAULT : ") << aobject << std::endl;
   //Return selected object unless disabled with nodefault
   return nodefault ? NULL : aobject;
}

ColourMap* LavaVu::findColourMap(std::string what, int id)
{
   //Find by name/ID match in all colour maps
   std::transform(what.begin(), what.end(), what.begin(), ::tolower);
   for (unsigned int i=0; i<amodel->colourMaps.size(); i++)
   {
      if (!amodel->colourMaps[i]) continue;
      std::string namekey = amodel->colourMaps[i]->name;
      std::transform(namekey.begin(), namekey.end(), namekey.begin(), ::tolower);
      if (namekey == what || id > 0 && id == amodel->colourMaps[i]->id)
      {
         return amodel->colourMaps[i];
      }
   }
   return NULL;
}

bool LavaVu::parseCommands(std::string cmd)
{
   if (cmd.length() == 0) return false;
   bool redisplay = true;
   bool norecord = false;
   PropertyParser parsed = PropertyParser();
   static std::string last_cmd = "";
   static std::string multiline = "";

   //Skip comments or empty lines
   if (cmd.length() == 0 || cmd.at(0) == '#') return false;
   //Disable recording for command beginning with @
   if (cmd.at(0) == '@')
   {
      norecord = true;
      cmd = cmd.substr(1);
   }
 
   //If the command contains only one double-quote, append until another received before parsing as a single string
   size_t n = std::count(cmd.begin(), cmd.end(), '"');
   size_t len = multiline.length();
   if (len > 0 || n == 1)
   {
      //Append
      multiline += cmd + "\n";
      //Finished appending?
      if (len > 0 && n == 1)
      {
         cmd = multiline;
         multiline = "";
      }
      else
         return false;
   }

   //Parse the line
   parsed.parseLine(cmd);

   //Verbose command processor
   float fval;
   int ival;

   //******************************************************************************
   //First check for settings commands that don't require a model to be loaded yet!
   if (parsed.exists("file"))
   {
      std::string what = parsed["file"];
      //Attempt to load external file
      FilePath file = FilePath(what);
      loadFile(file);
      return false;
   }
   else if (parsed.exists("script"))
   {
      std::string scriptfile = parsed["script"];
      std::ifstream file(scriptfile.c_str(), std::ios::in);
      if (file.is_open())
      {
         printMessage("Running script: %s", scriptfile.c_str());
         std::string line;
         entry = "";
         while(std::getline(file, line))
            parseCommands(line);
         entry = "";
         file.close();
      }
      else
      {
         if (scriptfile != "init.script")
            printMessage("Unable to open file: %s", scriptfile.c_str());
      }
      return false;
   }
   else if (parsed.has(ival, "cache"))
   {
      TimeStep::cachesize = ival;
      printMessage("Geometry cache set to %d timesteps", TimeStep::cachesize);
      return false;
   }
   else if (parsed.exists("noload"))
   {
      Model::noload = !Model::noload;
      printMessage("Database object loading is %s", Model::noload ? "ON":"OFF");
      return false;
   }
   else if (parsed.exists("hideall"))
   {
      hideall = !hideall;
      printMessage("Object load initial state is %s", hideall ? "VISIBLE":"HIDDEN");
      return false;
   }
   else if (parsed.exists("verbose"))
   {
      std::string what = parsed["verbose"];
      verbose = what != "off";
      printMessage("Verbose output is %s", verbose ? "ON":"OFF");
      //Set info/error stream
      if (verbose && !output)
         infostream = stderr;
      else
        infostream = NULL;
      return false;
   }
   else if (parsed.exists("pngalpha"))
   {
      viewer->alphapng = !viewer->alphapng;
      printMessage("Transparency in PNG output is %s", viewer->alphapng ? "ON":"OFF");
      return false;
   }
   else if (parsed.exists("swapyz"))
   {
      swapY = !swapY;
      printMessage("Y/Z axis swap on OBJ load is %s", swapY ? "ON":"OFF");
      return false;
   }
   else if (parsed.has(ival, "trisplit"))
   {
      trisplit = ival;
      printMessage("Triangle subdivision level set to %d", trisplit);
      return false;
   }
   else if (parsed.exists("globalcam"))
   {
      globalCam = !globalCam;
      printMessage("Global camera is %s", globalCam ? "ON":"OFF");
      return false;
   }
   else if (parsed.exists("localshaders"))
   {
      Shader::path = NULL;
      std::cerr << "Ignoring shader path, using current directory\n";
      printMessage("Using local shaders");
      return false;
   }
   else if (parsed.exists("volres"))
   {
      parsed.has(volres[0], "volres", 0);
      parsed.has(volres[1], "volres", 1);
      parsed.has(volres[2], "volres", 2);
      printMessage("Volume voxel resolution set to %d x %d x %d", volres[0], volres[1], volres[2]);
      return false;
   }
   else if (parsed.exists("volmin"))
   {
      parsed.has(volmin[0], "volmin", 0);
      parsed.has(volmin[1], "volmin", 1);
      parsed.has(volmin[2], "volmin", 2);
      printMessage("Volume minimum bound set to %d x %d x %d", volmin[0], volmin[1], volmin[2]);
      return false;
   }
   else if (parsed.exists("volmax"))
   {
      parsed.has(volmax[0], "volmax", 0);
      parsed.has(volmax[1], "volmax", 1);
      parsed.has(volmax[2], "volmax", 2);
      printMessage("Volume maximum bound set to %d x %d x %d", volmax[0], volmax[1], volmax[2]);
      return false;
   }
   else if (parsed.has(fval, "inscale"))
   {
      inscale = fval;
      printMessage("Geometry input scaling set to %f", inscale);
      return false;
   }
   else if (parsed.exists("createvolume"))
   {
      //Use this to load multiple volumes as timesteps into the same object
      volume = new DrawingObject("volume");
      printMessage("Created static volume object");
      return false;
   }
   else if (parsed.has(ival, "tracersteps"))
   {
      Model::tracers->steps = ival;
      if (Model::tracers->steps > amodel->step()) Model::tracers->steps = amodel->step();
      printMessage("Set tracer steps limit to %d", Model::tracers->steps);
      Model::tracers->redraw = true;
      return true;
   }
   else if (parsed.has(fval, "alpha"))
   {
      if (fval > 1.0)
         GeomData::opacity = fval / 255.0;
      else
         GeomData::opacity = fval;
      printMessage("Set global alpha to %.2f", GeomData::opacity);
      if (amodel)
         redrawViewports();
      return false;
   }
   //TODO: not yet documented
   else if (parsed.exists("pointspheres"))
   {
      Model::pointspheres = !Model::pointspheres;
      printMessage("Points rendered as spheres is %s", Model::pointspheres ? "ON":"OFF");
      return false;
   }
   else if (parsed.exists("linetubes"))
   {
      Lines::tubes = !Lines::tubes;
      printMessage("Lines rendered as tubes is %s", Lines::tubes ? "ON":"OFF");
      redrawViewports();
      return true;
   }
   else if (parsed.exists("open"))
   {
      loadWindow(0, 0, true);
      return false;
   }
   else if (parsed.exists("resize"))
   {
      float w = 0, h = 0;
      if (parsed.has(w, "resize", 0) && parsed.has(h, "resize", 1))
      {
         if (w != viewer->width && h != viewer->height)
         {
            if (viewer->isopen)
            {
               viewer->setsize(w, h);
               resetViews(true);
            }
            else
            {
               //Window not yet open, can simply set the fixed size vars
               fixedwidth = w;
               fixedheight = h;
            }
         }
      }
      return true;
   }

   //******************************************************************************
   //Following commands require a model!
   if (!amodel || !aview || !awin)
   {
      //Attempt to parse as property=value first
      parsePropertySet(cmd);
      return false;
   }

   if (parsed.exists("rotation"))
   {
      float x = 0, y = 0, z = 0, w = 0;
      if (parsed.has(x, "rotation", 0) &&
          parsed.has(y, "rotation", 1) &&
          parsed.has(z, "rotation", 2) &&
          parsed.has(w, "rotation", 3))
      {
        aview->setRotation(x, y, z, w);
        aview->rotated = true;  //Flag rotation finished
      }
   }
   else if (parsed.exists("translation"))
   {
      float x = 0, y = 0, z = 0;
      if (parsed.has(x, "translation", 0) &&
          parsed.has(y, "translation", 1) &&
          parsed.has(z, "translation", 2))
      {
        aview->setTranslation(x, y, z);
      }
   }
   else if (parsed.exists("rotate"))
   {
      float xr = 0, yr = 0, zr = 0;
      std::string axis = parsed["rotate"];
      if (axis == "x")
         parsed.has(xr, "rotate", 1);
      else if (axis == "y")
         parsed.has(yr, "rotate", 1);
      else if (axis == "z")
         parsed.has(zr, "rotate", 1);
      else 
      {
         parsed.has(xr, "rotate", 0);
         parsed.has(yr, "rotate", 1);
         parsed.has(zr, "rotate", 2);
      }
      aview->rotate(xr, yr, zr);
      aview->rotated = true;  //Flag rotation finished
   }
   else if (parsed.has(fval, "rotatex"))
   {
      aview->rotate(fval, 0, 0);
      aview->rotated = true;  //Flag rotation finished
   }
   else if (parsed.has(fval, "rotatey"))
   {
      aview->rotate(0, fval, 0);
      aview->rotated = true;  //Flag rotation finished
   }
   else if (parsed.has(fval, "rotatez"))
   {
      aview->rotate(0, 0, fval);
      aview->rotated = true;  //Flag rotation finished
   }
   else if (parsed.has(fval, "zoom"))
      aview->zoom(fval);
   else if (parsed.has(fval, "translatex"))
      aview->translate(fval, 0, 0);
   else if (parsed.has(fval, "translatey"))
      aview->translate(0, fval, 0);
   else if (parsed.has(fval, "translatez"))
      aview->translate(0, 0, fval);
   else if (parsed.exists("translate"))
   {
      float xt = 0, yt = 0, zt = 0;
      std::string axis = parsed["translate"];
      if (axis == "x")
         parsed.has(xt, "translate", 1);
      else if (axis == "y")
         parsed.has(yt, "translate", 1);
      else if (axis == "z")
         parsed.has(zt, "translate", 1);
      else 
      {
         parsed.has(xt, "translate", 0);
         parsed.has(yt, "translate", 1);
         parsed.has(zt, "translate", 2);
      }
      aview->translate(xt, yt, zt);
   }
   else if (parsed.exists("focus"))
   {
      float xf = 0, yf = 0, zf = 0;
      if (parsed.has(xf, "focus", 0) &&
          parsed.has(yf, "focus", 1) &&
          parsed.has(zf, "focus", 2))
      {
         aview->focus(xf, yf, zf);
      }
   }
   else if (parsed.has(fval, "aperture"))
      aview->adjustStereo(fval, 0, 0);
   else if (parsed.has(fval, "focallength"))
      aview->adjustStereo(0, fval, 0);
   else if (parsed.has(fval, "eyeseparation"))
      aview->adjustStereo(0, 0, fval);
   else if (parsed.has(fval, "zoomclip"))
      aview->zoomClip(fval);
   else if (parsed.has(fval, "nearclip"))
      aview->near_clip = fval;
   else if (parsed.has(fval, "farclip"))
      aview->far_clip = fval;
   else if (parsed.exists("timestep"))  //Absolute
   {
      if (!parsed.has(ival, "timestep"))
      {
         if (parsed["timestep"] == "up")
            ival = amodel->now-1;
         else if (parsed["timestep"] == "down")
            ival = amodel->now+1;
         else
            ival = amodel->now;
      }
      else //Convert to step idx
         ival = amodel->nearestTimeStep(ival);

      if (amodel->setTimeStep(ival) >= 0)
      {
         printMessage("Go to timestep %d", amodel->step());
         resetViews(); //Update the viewports
      }
      else
         return false;  //Invalid
   }
   else if (parsed.has(ival, "jump"))      //Relative
   {
      //Relative jump
      if (amodel->setTimeStep(amodel->now+ival) >= 0)
      {
         printMessage("Jump to timestep %d", amodel->step());
         resetViews(); //Update the viewports
      }
      else
         return false;  //Invalid
   }
   else if (parsed.exists("model"))      //Model switch
   {
      if (window < 0) window = 0; //No window selection yet...
      if (!parsed.has(ival, "model"))
      {
         if (parsed["model"] == "up")
            ival = window-1;
         else if (parsed["model"] == "down")
            ival = window+1;
         else
            ival = window;
      }
      if (ival < 0) ival = windows.size()-1;
      if (ival >= (int)windows.size()) ival = 0;
      if (!loadWindow(ival, amodel->step())) return false;  //Invalid
      printMessage("Load model %d", window);
   }
   else if (parsed.exists("hide") || parsed.exists("show"))
   {
      std::string action = parsed.exists("hide") ? "hide" : "show";
      std::string what = parsed[action];
      Geometry* active = getGeometryType(what);

      //Have selected a geometry type?
      if (active)
      {
         int id;
         std::string range = parsed.get(action, 1);
         if (range.find('-') != std::string::npos)
         {
            std::stringstream rangess(range);
            int start, end;
            char delim;
            rangess >> start >> delim >> end;
            for (int i=start; i<=end; i++)
            {
               if (action == "hide") active->hide(i); else active->show(i);
               printMessage("%s %s range %s", action.c_str(), what.c_str(), range.c_str());
            }
         }
         else if (parsed.has(id, action, 1))
         {
            parsed.has(id, action, 1);
            if (action == "hide")
            {
               if (!active->hide(id)) return false; //Invalid
            }
            else
            {
               if (!active->show(id)) return false; //Invalid
            }
            printMessage("%s %s %d", action.c_str(), what.c_str(), id);
         }
         else
         {
            if (action == "hide")
               active->hideAll();
            else
               active->showAll();
            printMessage("%s all %s %d", action.c_str(), what.c_str(), id);
         }
         redrawViewports();
      }
      else
      {
         //Hide/show by name/ID match in all drawing objects
         //std::string which = parsed.get(action, 1);
         for (int c=0; c<10; c++) //Allow multiple id/name specs on line
         {
            int id = parsed.Int(action, 0, c);
            DrawingObject* obj = findObject(what, id);
            if (obj)
            {
               if (obj->skip)
               {
                  std::ostringstream ss;
                  ss << "load " << obj->id;
                  return parseCommands(ss.str());
               }
               else
               {
                  //Hide/show all data for this object
                  bool state = (action == "show");
                  for (unsigned int i=0; i < Model::geometry.size(); i++)
                     Model::geometry[i]->showById(obj->id, state);
                  obj->visible = state; //This allows hiding of objects without geometry (colourbars)
                  printMessage("%s object %s", action.c_str(), obj->name.c_str());
                  redrawViewports();
               }
            }
         }
      }
   }
   else if (parsed.exists("opacity"))
   {
      //Alpha by name/ID match in all drawing objects
      DrawingObject* obj = aobject;
      int next = 0;
      if (!obj)
      {
         std::string what = parsed["opacity"];
         int id = parsed.Int("opacity", 0);
         obj = findObject(what, id);
         next++;
      }
      if (obj && parsed.has(fval, "opacity", next))
      {
         if (fval > 1.0) fval /= 255.0;
         obj->properties["opacity"] = fval;
         redraw(obj->id);
         printMessage("%s opacity set to %f", obj->name.c_str(), fval);
         redrawViewports();
      }
   }
   else if (parsed.has(ival, "movie"))
   {
      std::string fn = awin->name + ".mp4";
      encodeVideo(fn.c_str(), amodel->step(), ival);
   }
   else if (parsed.exists("record"))
   {
#ifdef HAVE_LIBAVCODEC
      if (!parsed.has(ival, "record")) ival = 30;
      if (!encoder)
      {
         std::string fn = awin->name + ".mp4";
         encoder = new VideoEncoder(fn.c_str(), viewer->width, viewer->height, ival);
      }
      else
      {
         delete encoder;
         encoder = NULL;
      }
#endif
   }
   else if (parsed.has(ival, "play"))
   {
      writeSteps(false, false, amodel->step(), ival, NULL);
   }
   else if (parsed.exists("play"))
   {
      //Play loop
      loop = true;
      OpenGLViewer::commands.push_back(std::string("next"));
   }
   else if (parsed.exists("next"))
   {
      int old = amodel->now;
      if (amodel->timesteps.size() < 2) return false;
      amodel->setTimeStep(amodel->now+1);
      //Allow loop back to start when using next command
      if (amodel->now > 0 && amodel->now == old)
         amodel->setTimeStep(0);
      resetViews(); //Update the viewports

      if (loop)
         OpenGLViewer::commands.push_back(std::string("next"));
   }
   else if (parsed.exists("stop"))
   {
      loop = false;
   }
   else if (parsed.has(ival, "images"))
   {
      writeImages(amodel->step(), ival);
   }
   else if (parsed.exists("animate"))
   {
      if (parsed.has(ival, "animate"))
      {
         animate = ival;
      }
      else if (animate > 0)
      {
         animate = 0;
      }
      else
      {
         animate = 50;
      }
      viewer->notIdle(animate); //Start idle redisplay timer
      printMessage("Animate mode %d millseconds", animate);
      return true; //No record
   }
   else if (parsed.exists("repeat"))
   {
      bool state = recording;
      if (parsed["repeat"] == "history" && parsed.has(ival, "repeat", 1))
      {
         recording = false;
         if (animate > 0 && repeat == 0)
         {
            repeat = ival;
            replay = history;
         }
         else
         {
            for (int r=0; r<ival; r++)
            {
               for (unsigned int l=0; l<history.size(); l++)
                  parseCommands(history[l]);
            }
         }
         recording = state;
         return true; //Skip record
      }
      else if (parsed.has(ival, "repeat"))
      {
         recording = false;
         if (animate > 0)
         {
            repeat = ival;
            replay.push_back(last_cmd);
         }
         else
         {
            for (int r=0; r<ival; r++)
               parseCommands(last_cmd);
         }
         return true;  //Skip record
      }
      else
         return parseCommands(last_cmd);
   }
   else if (parsed.exists("quit"))
   {
      viewer->quitProgram = true;
   }
   else if (parsed.exists("axis"))
   {
      std::string axis = parsed["axis"];
      if (parsed["axis"] == "on")
         aview->properties["axis"] = true;
      else if (parsed["axis"] == "off")
         aview->properties["axis"] = false;
      else
         aview->properties["axis"] = !aview->properties["axis"].ToBool(true);
      printMessage("Axis %s", aview->properties["axis"].ToBool() ? "ON" : "OFF");
   }
   else if (parsed.exists("cullface"))
   {
      GeomData::cullface = !GeomData::cullface;
      redrawViewports();
      printMessage("Back face culling for surfaces is %s", GeomData::cullface ? "ON":"OFF");
   }
   else if (parsed.exists("wireframe"))
   {
      GeomData::wireframe = !GeomData::wireframe;
      redrawViewports();
      printMessage("Wireframe %s", GeomData::wireframe ? "ON":"OFF");
   }
   else if (parsed.exists("lighting"))
   {
      //Lighting ON/OFF
      GeomData::lit = !GeomData::lit;
      printMessage("Lighting is %s", GeomData::lit ? "ON":"OFF");
   }
   else if (parsed.exists("trianglestrips"))
   {
      Model::quadSurfaces->triangles = !Model::quadSurfaces->triangles;
      printMessage("Triangle strips for quad surfaces %s", Model::quadSurfaces->triangles ? "ON":"OFF");
      Model::quadSurfaces->redraw = true;
   }
   else if (parsed.exists("redraw"))
   {
      //for (int type=lucMinType; type<lucMaxType; type++)
      //   Model::geometry[type]->redraw = true;
      redrawViewports();
      printMessage("Redrawing all objects");
   }
   else if (parsed.exists("fullscreen"))
   {
      viewer->fullScreen();
      printMessage("Full-screen is %s", viewer->fullscreen ? "ON":"OFF");
   }
   else if (parsed.exists("scaling"))
   {
      printMessage("Scaling is %s", aview->scaleSwitch() ? "ON":"OFF");
   }
   else if (parsed.exists("fit"))
   {
      aview->zoomToFit();
   }
   else if (parsed.exists("autozoom"))
   {
      aview->autozoom = !aview->autozoom;
      printMessage("AutoZoom is %s", aview->autozoom ? "ON":"OFF");
   }
   else if (parsed.exists("stereo"))
   {
      aview->stereo = !aview->stereo;
      printMessage("Stereo is %s", aview->stereo ? "ON":"OFF");
   }
   else if (parsed.exists("coordsystem"))
   {
      printMessage("%s coordinate system", (aview->switchCoordSystem() == RIGHT_HANDED ? "Right-handed":"Left-handed"));
   }
   else if (parsed.exists("title"))
   {
      //Hide or set title
      if (cmd.length() > 6)
         aview->properties["title"] = parsed["title"];
      else
         aview->properties["title"] = "";
   }
   else if (parsed.exists("rulers"))
   {
      //Show/hide rulers
      aview->properties["rulers"] = !aview->properties["rulers"].ToBool(false);
      printMessage("Rulers %s", aview->properties["rulers"].ToBool() ? "ON" : "OFF");
   }
   else if (parsed.exists("log"))
   {
      ColourMap::logscales = (ColourMap::logscales + 1) % 3;
      bool state = ColourMap::lock;
      ColourMap::lock = false;
      for (unsigned int i=0; i<amodel->colourMaps.size(); i++)
         amodel->colourMaps[i]->calibrate();
      ColourMap::lock = state;  //restore setting
      printMessage("Log scales are %s", ColourMap::logscales  == 0 ? "DEFAULT": ( ColourMap::logscales  == 1 ? "ON" : "OFF"));
      redrawObjects();
   }
   else if (parsed.exists("help"))
   {
      viewer->display();  //Ensures any previous help text wiped
      std::string cmd = parsed["help"];
      if (cmd == "manual")
      {
         std::cout << HELP_MESSAGE;
         std::cout << "\nMiscellanious commands:\n\n";
         std::cout << helpCommand("quit") << helpCommand("repeat") << helpCommand("history");
         std::cout << helpCommand("clearhistory") << helpCommand("pause") << helpCommand("list");
         std::cout << helpCommand("timestep") << helpCommand("jump") << helpCommand("model") << helpCommand("reload");
         std::cout << helpCommand("next") << helpCommand("play") << helpCommand("stop") << helpCommand("script");
         std::cout << "\nView/camera commands:\n\n";
         std::cout << helpCommand("rotate") << helpCommand("rotatex") << helpCommand("rotatey");
         std::cout << helpCommand("rotatez") << helpCommand("rotation") << helpCommand("zoom") << helpCommand("translate");
         std::cout << helpCommand("translatex") << helpCommand("translatey") << helpCommand("translatez");
         std::cout << helpCommand("focus") << helpCommand("aperture") << helpCommand("focallength");
         std::cout << helpCommand("eyeseparation") << helpCommand("nearclip") << helpCommand("farclip");
         std::cout << helpCommand("zerocam") << helpCommand("reset") << helpCommand("camera");
         std::cout << helpCommand("resize") << helpCommand("fullscreen") << helpCommand("fit");
         std::cout << helpCommand("autozoom") << helpCommand("stereo") << helpCommand("coordsystem");
         std::cout << helpCommand("sort") << helpCommand("rotation") << helpCommand("translation");
         std::cout << "\nOutput commands:\n\n";
         std::cout << helpCommand("image") << helpCommand("images") << helpCommand("outwidth") << helpCommand("outheight");
         std::cout << helpCommand("movie") << helpCommand("dump") << helpCommand("json");
         std::cout << "\nObject commands:\n\n";
         std::cout << helpCommand("hide") << helpCommand("show") << helpCommand("delete") << helpCommand("load") << helpCommand("file");
         std::cout << "\nDisplay commands:\n\n";
         std::cout << helpCommand("background") << helpCommand("alpha") << helpCommand("opacity");
         std::cout << helpCommand("axis") << helpCommand("cullface") << helpCommand("wireframe");
         std::cout << helpCommand("trianglestrips") << helpCommand("redraw") << helpCommand("scaling") << helpCommand("rulers") << helpCommand("log");
         std::cout << helpCommand("antialias") << helpCommand("localise") << helpCommand("lockscale");
         std::cout << helpCommand("lighting") << helpCommand("colourmap") << helpCommand("pointtype");
         std::cout << helpCommand("glyphquality");
         std::cout << helpCommand("tracerscale") << helpCommand("tracersteps") << helpCommand("pointsample");
         std::cout << helpCommand("border") << helpCommand("title") << helpCommand("scale") << helpCommand("select");
      }
      else if (cmd.length() > 0)
      {
         displayText(helpCommand(cmd));
         std::cout << helpCommand(cmd);
      }
      else
      {
         displayText(helpCommand("help"));
         std::cout << HELP_MESSAGE;
         std::cout << helpCommand("help");
      }
      viewer->swap();  //Immediate display
      redisplay = false;
   }
   else if (parsed.exists("antialias"))
   {
      aview->properties["antialias"] = !aview->properties["antialias"].ToBool(true);
      printMessage("Anti-aliasing %s", aview->properties["antialias"].ToBool() ? "ON":"OFF");
   }
   else if (parsed.exists("localise"))
   {
      //Find colour value min/max local to each geom element
      for (int type=lucMinType; type<lucMaxType; type++)
         Model::geometry[type]->localiseColourValues();
      printMessage("ColourMap scales localised");
      redrawObjects();
   }
   else if (parsed.exists("export"))
   {
      std::string what = parsed["export"];
      lucExportType type = what == "json" ? lucExportJSON : (what == "csv" ? lucExportCSV : (what == "db" ? lucExportGLDB : lucExportGLDBZ));
      //Export drawing object by name/ID match 
      for (int c=0; c<10; c++) //Allow multiple id/name specs on line
      {
         int id = parsed.Int("export", 0, c);
         DrawingObject* obj = findObject(parsed.get("export", 1), id, true);
         if (obj)
         {
            exportData(type, obj->id);
            printMessage("Dumped object %s to %s", obj->name.c_str(), what.c_str());
         }
         else
         {
            exportData(type);
            printMessage("Dumped all objects to %s", what.c_str());
            break;
         }
      }
   }
   else if (parsed.exists("lockscale"))
   {
      ColourMap::lock = !ColourMap::lock;
      printMessage("ColourMap scale locking %s", ColourMap::lock ? "ON":"OFF");
      redrawObjects();
   }
   else if (parsed.exists("list"))
   {
      if (parsed["list"] == "objects")
      {
         objectlist = !objectlist;
         displayObjectList(true);
      }
      if (parsed["list"] == "elements")
      {
         //Print available elements by id
         for (unsigned int i=0; i < Model::geometry.size(); i++)
            Model::geometry[i]->print();
      }
      if (parsed["list"] == "colourmaps")
      {
         int offset = 0;
         for (unsigned int i=0; i < amodel->colourMaps.size(); i++)
         {
            if (amodel->colourMaps[i])
            {
               std::ostringstream ss;
               ss << std::setw(5) << amodel->colourMaps[i]->id << " : " << amodel->colourMaps[i]->name;

               displayText(ss.str(), ++offset);
               std::cerr << ss.str() << std::endl;
            }
         }
         viewer->swap();  //Immediate display
         return false;
      }
   }
   else if (parsed.exists("reset"))
   {
      aview->reset();     //Reset camera
      aview->init(true);  //Reset camera to default view of model
      printMessage("View reset");
   }
   else if (parsed.exists("reload"))
   {
      //Restore original window data
      if (!loadWindow(window)) return false;
   }
   else if (parsed.exists("zerocam"))
   {
      aview->reset();     //Zero camera
   }
   else if (parsed.exists("colourmap"))
   {
      //Set colourmap on object by name/ID match
      DrawingObject* obj = aobject;
      int next = 0;
      if (!obj)
      {
         std::string what = parsed["colourmap"];
         int id = parsed.Int("colourmap", 0);
         obj = findObject(what, id);
         next++;
      }
      if (obj)
      {
         std::string what = parsed.get("colourmap", next);
         //int cid = parsed.Int("colourmap", 2);
         parsed.has(ival, "colourmap", next);
         ColourMap* cmap = findColourMap(what, ival);
         //Only able to set the value colourmap now
         if (cmap)
         {
            obj->addColourMap(cmap, lucColourValueData);
            printMessage("%s colourmap set to %s (%d)", obj->name.c_str(), cmap->name.c_str(), cmap->id);
         }
         else if (ival < 0 || what.length() == 0)
            printMessage("%s colourmap set to none", obj->name.c_str());
         else
         {
            //No cmap id, parse a colourmap string (must be single line or enclosed in "")
            if (what == "add" || !obj->colourMaps[lucColourValueData])
            {
               obj->colourMaps[lucColourValueData] = addColourMap();
               what = parsed.get("colourmap", next+1);
            }
            if (what.length() > 0)
            {
               obj->colourMaps[lucColourValueData]->loadPalette(what);
               //obj->colourMaps[lucColourValueData]->print();
               obj->colourMaps[lucColourValueData]->calibrate(); //Recalibrate
               obj->colourMaps[lucColourValueData]->calc(); //Recalculate cached colours
            }
         }
         redraw(obj->id);
         redrawViewports();
      }
   }
   else if (parsed.exists("colour"))
   {
      //Set object colour by name/ID match in all drawing objects
      DrawingObject* obj = aobject;
      int next = 0;
      if (!obj)
      {
         std::string what = parsed["colour"];
         int id = parsed.Int("colour", 0);
         obj = findObject(what, id);
         next++;
      }
      if (obj)
      {
         bool append = (parsed.get("colour", next+1) == "append");
         Colour c;
         std::string str = parsed.get("colour", next);
         if (str.find('[') != std::string::npos)
         {
            //Parse colour json array
            json::Value j = json::Deserialize(str);
            c = Colour_FromJson(j);
         }
         else
         {
            //Parse colour as RGB(A) hex, convert to ARGB int
            c.value = parsed.Colour("colour", next);
         }

         if (append)
         {
            //Find the first available geometry container for this drawing object and append a colour
            GeomData* geomdata = getGeometry(obj);
            if (geomdata)
            {
               geomdata->data[lucRGBAData]->read(1, &c.value);
               printMessage("%s colour appended %x", obj->name.c_str(), c.value);
            }
         }
         else
         {
            obj->properties["colour"] = Colour_ToJson(c);
            printMessage("%s colour set to %x", obj->name.c_str(), c.value);
         }
         redraw(obj->id);
         redrawViewports();
      }
   }
   else if (parsed.exists("pointtype"))
   {
      std::string what = parsed["pointtype"];
      if (what == "all")
      {
         //Should use a set method here...
         if (parsed.has(ival, "pointtype", 1))
            Points::pointType = ival;
         else
            Points::pointType++;
         if (Points::pointType > 4) Points::pointType = 0;
         if (Points::pointType < 0) Points::pointType = 4;
         printMessage("Point type %d", Points::pointType);
      }
      else
      {
         //Point type by name/ID match in all drawing objects
         DrawingObject* obj = aobject;
         int next = 0;
         if (!obj)
         {
            int id = parsed.Int("pointtype", 0);
            obj = findObject(what, id);
            next++;
         }
         if (obj)
         {
            if (parsed.has(ival, "pointtype", next))
               obj->properties["pointtype"] = ival;
            else if (parsed.get("pointtype", next) == "up")
               obj->properties["pointtype"] = (obj->properties["pointtype"].ToInt(-1)-1) % 5;
            else if (parsed.get("pointtype", next) == "down")
               obj->properties["pointtype"] = (obj->properties["pointtype"].ToInt(-1)+1) % 5;
            printMessage("%s point type set to %d", obj->name.c_str(), obj->properties["pointtype"].ToInt(-1));
            Model::geometry[lucPointType]->redraw = true;
            redraw(obj->id);
            redrawViewports();
         }
      }
   }
   else if (parsed.has(ival, "glyphquality"))
   {
      if (ival < 0 || ival > 10) return false;
      Model::vectors->redraw = true;
      Model::tracers->redraw = true;
      Model::shapes->redraw = true;
      GeomData::glyphs = ival;
      printMessage("Glyph quality set to %d", GeomData::glyphs);
   }
   else if (parsed.exists("tracerscale"))
   {
      Model::tracers->redraw = true;
      Model::tracers->scaling = Model::tracers->scaling ? false : true;
      printMessage("Scaled tracer rendering is %s", Model::tracers->scaling ? "ON":"OFF");
   }
   else if (parsed.exists("pointsample"))
   {
      if (parsed.has(ival, "pointsample"))
         Points::subSample = ival;
      else if (parsed["pointsample"] == "up")
         Points::subSample /= 2;
      else if (parsed["pointsample"] == "down")
         Points::subSample *= 2;
      if (Points::subSample < 1) Points::subSample = 1;
      Model::points->redraw = true;
      printMessage("Point sampling %d", Points::subSample);
   }
   else if (parsed.exists("image"))
   {
      viewer->snapshot(awin->name.c_str());
      if (viewer->outwidth > 0)
         printMessage("Saved image %d x %d", viewer->outwidth, viewer->outheight);
      else
         printMessage("Saved image %d x %d", viewer->width, viewer->height);
   }
   else if (parsed.has(ival, "outwidth"))
   {
      if (ival < 10) return false;
      viewer->outwidth = ival;
      printMessage("Output image width set to %d", viewer->outwidth);
   }
   else if (parsed.has(ival, "outheight"))
   {
      if (ival < 10) return false;
      viewer->outheight = ival;
      printMessage("Output image height set to %d", viewer->outheight);
   }
   else if (parsed.exists("background"))
   {
      //TODO: fix this, should work as colour
      if (parsed["background"] == "white")
         awin->background.value = 0xffffffff;
      else if (parsed["background"] == "black")
         awin->background.value = 0xff000000;
      else if (parsed["background"] == "grey")
         awin->background.value = 0xff666666;
      else if (parsed["background"] == "invert")
         Colour_Invert(awin->background);
      else if (parsed.has(fval, "background"))
      {
         awin->background.a = 255;
         if (fval <= 1.0) fval *= 255;
         awin->background.r = awin->background.g = awin->background.b = fval;
      }
      else
      {
         if (awin->background.r == 255 || awin->background.r == 0)
            awin->background.value = 0xff666666;
         else
            awin->background.value = 0xff000000;
      }
      viewer->setBackground(awin->background.value);
      printMessage("Background colour set");
   }
   else if (parsed.exists("border"))
   {
      //Frame off/on/filled
      aview->properties["fillborder"] = false;
      int state = aview->properties["border"].ToInt(1);
      if (parsed["border"] == "on")
         aview->properties["border"] = 1;
      else if (parsed["border"] == "off")
         aview->properties["border"] = 0;
      else if (parsed["border"] == "filled")
      {
         aview->properties["fillborder"] = true;
         aview->properties["border"] = 1;
      }
      else
      {
         if (parsed.has(ival, "border"))
           aview->properties["border"] = ival;
         else if (state > 0)
           aview->properties["border"] = 0;
         else
           aview->properties["border"] = 1;
      }
      printMessage("Frame set to %d, filled=%d", aview->properties["border"].ToInt(), aview->properties["fillborder"].ToBool());
   }
   else if (parsed.exists("camera"))
   {
      //Output camera view xml and translation/rotation commands
      aview->print();
   }
   else if (parsed.exists("scale"))
   {
      std::string what = parsed["scale"];
      Geometry* active = getGeometryType(what);
      if (active) 
      {
         if (parsed.has(fval, "scale", 1))
           active->scale = fval;
         else if (parsed.get("scale", 1) == "up")
           active->scale *= 1.5;
         else if (parsed.get("scale", 1) == "down")
           active->scale /= 1.5;
         active->redraw = true;
         printMessage("%s scaling set to %f", what.c_str(), active->scale);
      }
      else
      {
         //X,Y,Z: geometry scaling
         if (what == "x")
         {
            if (parsed.has(fval, "scale", 1))
            {
               aview->setScale(fval, 1, 1);
               redrawViewports();
            }
         }
         else if (what == "y")
         {
            if (parsed.has(fval, "scale", 1))
            {
               aview->setScale(1, fval, 1);
               redrawViewports();
            }
         }
         else if (what == "z")
         {
            if (parsed.has(fval, "scale", 1))
            {
               aview->setScale(1, 1, fval);
               redrawViewports();
            }
         }
         else if (what == "all" && parsed.has(fval, "scale", 1))
         {
            //Scale everything
            aview->setScale(fval, fval, fval);
            redrawViewports();
         }
         else
         {
            //Scale by name/ID match in all drawing objects
            DrawingObject* obj = aobject;
            int next = 0;
            if (!obj)
            {
               int id = parsed.Int("scale", 0);
               obj = findObject(what, id);
               next++;
            }
            if (obj)
            {
               if (parsed.has(fval, "scale", next))
                  obj->properties["scaling"] = fval;
               else if (parsed.get("scale", next) == "up")
                  obj->properties["scaling"] = obj->properties["scaling"].ToFloat(1.0) * 1.5;
               else if (parsed.get("scale", next) == "down")
                  obj->properties["scaling"] = obj->properties["scaling"].ToFloat(1.0) / 1.5;
               printMessage("%s scaling set to %f", obj->name.c_str(), obj->properties["scaling"].ToFloat(1.0));
               for (int type=lucMinType; type<lucMaxType; type++)
                  Model::geometry[type]->redraw = true;
               redraw(obj->id);
               redrawViewports();
            }
         }
      }
   }
   else if (parsed.exists("history"))
   {
      std::string scriptfile = parsed["history"];
      if (scriptfile.length() > 0)
      {
         std::ofstream file(scriptfile.c_str(), std::ios::out);
         if (file.is_open())
         {
            for (unsigned int l=0; l<history.size(); l++)
               file << history[l] << std::endl;
            file.close();
         }
         else
            printMessage("Unable to open file: %s", scriptfile.c_str());
      }
      else
      {
         for (unsigned int l=0; l<history.size(); l++)
            std::cout << history[l] << std::endl;
      }
      return true; //No record
   }
   else if (parsed.exists("clearhistory"))
   {
      history.clear();
      return false; //Skip record
   }
   else if (parsed.has(ival, "pause"))
   {
      PAUSE(ival);
   }
   else if (parsed.exists("delete"))
   {
      //Delete drawing object by name/ID match 
      std::string what = parsed["delete"];
      int id = parsed.Int("delete", 0);
      DrawingObject* obj = findObject(what, id);
      if (obj)
      {
         amodel->deleteObject(obj->id);
         printMessage("%s deleted from database", obj->name.c_str());
         for (unsigned int i=0; i<aview->objects.size(); i++)
         {
            if (!aview->objects[i]) continue;
            if (obj == aview->objects[i])
            {
               aview->objects.erase(aview->objects.begin()+i);
               break;
            }
         }
         loadWindow(window);
      }
   }
   else if (parsed.exists("merge"))
   {
      amodel->mergeDatabases();
      parseCommands("quit");
   }
   else if (parsed.exists("load"))
   {
      //Load drawing object by name/ID match 
      std::string what = parsed["load"];
      for (int c=0; c<10; c++) //Allow multiple id/name specs on line
      {
         int id = parsed.Int("load", 0, c);
         DrawingObject* obj = findObject(what, id);
         if (obj)
         {
            amodel->loadGeometry(obj->id);
            //Update the views
            resetViews(false);
            redrawObjects();
            //Delete the cache as object list changed
            amodel->deleteCache();
         }
      }
   }
   //TODO: NOT YET DOCUMENTED
   else if (parsed.exists("inertia"))
   {
      aview->use_inertia = !aview->use_inertia;
      printMessage("Inertia is %s", aview->use_inertia ? "ON":"OFF");
   }
   else if (parsed.exists("sort"))
   {
      //Always disables sort on rotate mode
      sort_on_rotate = false;
      aview->sort = true;
      viewer->notIdle(1500); //Start idle redisplay timer
      printMessage("Sorting geometry (auto-sort has been disabled)");
   }
   else if (parsed.exists("idle"))
   {
      if (!sort_on_rotate && aview->rotated)
         aview->sort = true;
      //Command playback
      if (repeat != 0)
      {
         bool state = recording;
         recording = false;
         //Playback
         for (unsigned int l=0; l<replay.size(); l++)
            parseCommands(replay[l]);
         repeat--;
         if (repeat == 0) 
         {
            viewer->notIdle(0); //Disable idle redisplay timer
            replay.clear();
         }
         recording = state;
      }
      return true; //Skip record
   }
   //Special commands for passing keyboard/mouse actions directly (web server mode)
   else if (parsed.exists("mouse"))
   {
      std::string data = parsed["mouse"];

      std::replace(data.begin(), data.end(), ',', '\n');
      std::istringstream iss(data);
      PropertyParser props = PropertyParser(iss, '=');

      int x = props.Int("x");
      int y = props.Int("y");
      int button = props.Int("button");
      int spin = props.Int("spin");
      std::string action = props["mouse"];
      std::string modifiers = props["modifiers"];

      //Save shift states
      viewer->keyState.shift = modifiers.find('S')+1;
      viewer->keyState.ctrl = modifiers.find('C')+1;
      viewer->keyState.alt = modifiers.find('A')+1;

      //printf("%s button %d x %d y %d\n", action.c_str(), button, x, y);

      //viewer->button = (MouseButton)button; //if (viewer->keyState.ctrl && viewer->button == LeftButton)
      // XOR state of three mouse buttons to the mouseState variable  
      MouseButton btn = (MouseButton)button;

      if (action.compare("down") == 0)
      {
         if (btn <= RightButton) viewer->mouseState ^= (int)pow(2.0f, btn);
         redisplay = mousePress((MouseButton)button, true, x, y);
      }
      if (action.compare("up") == 0)
      {
         viewer->mouseState = 0;
         redisplay = mousePress((MouseButton)button, false, x, y);
      }
      if (action.compare("move") == 0)
      {
         redisplay = mouseMove(x, y);
      }
      if (action.compare("scroll") == 0)
      {
         redisplay = mouseScroll(spin);
      }
   }
   else if (parsed.exists("key"))
   {
      std::string data = parsed["key"];

      std::replace(data.begin(), data.end(), ',', '\n');
      std::istringstream iss(data);
      PropertyParser props = PropertyParser(iss, '=');

      int x = props.Int("x");
      int y = props.Int("y");
      unsigned char key = props.Int("key");
      std::string modifiers = props["modifiers"];

      //Save shift states
      viewer->keyState.shift = modifiers.find('S')+1;
      viewer->keyState.ctrl = modifiers.find('C')+1;
      viewer->keyState.alt = modifiers.find('A')+1;

      redisplay = keyPress(key, x, y);
   }
   else if (parsed.exists("select"))
   {
      std::string what = parsed["select"];
      int id = parsed.Int("select", 0);
      aobject = findObject(what, id, true); //Don't allow default to currently selected
      if (aobject)
         printMessage("Selected object: %s", aobject->name.c_str());
      else
         printMessage("Object selection cleared");
   }
   else if (parsed.exists("shaders"))
   {
      printMessage("Reloading shaders");
      reloadShaders();
      return false;
   }
   else if (parsed.exists("blend"))
   {
      std::string what = parsed["blend"];
      if (what == "png")
        viewer->blend_mode = BLEND_PNG;
      else if (what == "add")
        viewer->blend_mode = BLEND_ADD;
      else
        viewer->blend_mode = BLEND_NORMAL;
   }
   else if (parsed.exists("props"))
   {
      printProperties();
   }
   else if (parsed.exists("test"))
   {
      createDemoModel();
      resetViews();
   }
   else if (parsed.exists("name"))
   {
      DrawingObject* obj = aobject;
      int next = 0;
      if (!obj)
      {
         std::string what = parsed["name"];
         int id = parsed.Int("name", 0);
         obj = findObject(what, id);
         next++;
      }
      if (obj)
      {
         std::string name = parsed.get("name", next);
         if (name.length() > 0)
         {
           obj->name = name;
           std::cerr << "RENAMED OBJECT: " << obj->id << " " << obj->name << std::endl;
         }
      }
   }
   else if (parsed.exists("newstep"))
   {
      amodel->addTimeStep(amodel->step()+1);
      amodel->setTimeStep(amodel->now+1);
      //Don't record
      return false;
   }
   else
   {
      //If value parses as integer and contains nothing else 
      //interpret as timestep jump
      ival = atoi(cmd.c_str());
      std::ostringstream oss;
      oss << ival;
      if (oss.str() == cmd && amodel->setTimeStep(amodel->nearestTimeStep(ival)) >= 0)
      {
         printMessage("Go to timestep %d", amodel->step());
         resetViews(); //Update the viewports
      }
      else if (cmd.at(0) == '#')
      {
         parseCommands("select " + cmd.substr(1));
      }
      else if (parsePropertySet(cmd))
      {
         return false;
      }
      else
      {
         std::cerr << "# Unrecognised command: \"" << cmd << "\"" << std::endl;
         return false;  //Invalid
      }
   }

   //Always redraw when using multiple viewports in window (urgh sooner this is gone the better)
   if (awin && awin->views.size() > 1 && viewPorts)
       redrawViewports();
   last_cmd = cmd;
   if (!norecord) record(false, cmd);
   if (animate && redisplay) viewer->display();
   return redisplay;
}

bool LavaVu::parsePropertySet(std::string cmd)
{
   std::size_t found = cmd.find("=");
   json::Value jval;
   if (found == std::string::npos) return false;
   parseProperty(cmd);
   if (aobject && aobject->id) redraw(aobject->id);
   return true;
}

std::string LavaVu::helpCommand(std::string cmd)
{
   //Verbose command help
   std::string help = "~~~~~~~~~~~~~~~~~~~~~~~~\n" + cmd + "\n~~~~~~~~~~~~~~~~~~~~~~~~\n";
   if (cmd == "help")
   {
      help += "Command help:\n\nUse:\nhelp * [ENTER]\nwhere * is a command, for detailed help\n"
                  "\nMiscellanious commands:\n\n"
                  "quit, repeat, animate, history, clearhistory, pause, list, timestep, jump, model, reload, file, script\n"
                  "\nView/camera commands:\n\n"
                  "rotate, rotatex, rotatey, rotatez, rotation, translate, translatex, translatey, translatez\n"
                  "focus, aperture, focallength, eyeseparation, nearclip, farclip, zerocam, reset, camera\n"
                  "resize, fullscreen, fit, autozoom, stereo, coordsystem, sort, rotation, translation\n"
                  "\nOutput commands:\n\n"
                  "image, images, outwidth, outheight, movie, play, dump, json\n"
                  "\nObject commands:\n\n"
                  "hide, show, delete, load, select\n"
                  "\nDisplay commands:\n\n"
                  "background, alpha, opacity, axis, cullface, wireframe, trianglestrips, scaling, rulers, log\n"
                  "antialias, localise, lockscale, lighting, colourmap, pointtype, glyphquality\n"
                  "tracerscale, tracersteps, pointsample, border, title, scale\n";
   }
   else if (cmd == "rotation")
   {
      help += "Set model rotation in 3d coords about given axis vector (replaces any previous rotation)\n\n"
                  "Usage: rotation x y z degrees\n\n"
                  "x (number) : axis of rotation x component\n"
                  "y (number) : axis of rotation y component\n"
                  "z (number) : axis of rotation z component\n"
                  "degrees (number) : degrees of rotation\n";
   }
   else if (cmd == "translation")
   {
      help += "Set model translation in 3d coords (replaces any previous translation)\n\n"
                  "Usage: translation x y z\n\n"
                  "x (number) : x axis shift\n"
                  "y (number) : y axis shift\n"
                  "z (number) : z axis shift\n";
   }
   else if (cmd == "rotate")
   {
      help += "Rotate model\n\n"
                  "Usage: rotate axis degrees\n\n"
                  "axis (x/y/z) : axis of rotation\n"
                  "degrees (number) : degrees of rotation\n"
                  "\nUsage: rotate x y z\n\n"
                  "x (number) : x axis degrees of rotation\n"
                  "y (number) : y axis degrees of rotation\n"
                  "z (number) : z axis degrees of rotation\n";
   }
   else if (cmd == "rotatex")
   {
      help += "Rotate model about x-axis\n\n"
                  "Usage: rotatex degrees\n\n"
                  "degrees (number) : degrees of rotation\n";
   }
   else if (cmd == "rotatey")
   {
      help += "Rotate model about y-axis\n\n"
                  "Usage: rotatey degrees\n\n"
                  "degrees (number) : degrees of rotation\n";
   }
   else if (cmd == "rotatez")
   {
      help += "Rotate model about z-axis\n\n"
                  "Usage: rotatez degrees\n\n"
                  "degrees (number) : degrees of rotation\n";
   }
   else if (cmd == "zoom")
   {
      help += "Translate model in Z direction (zoom)\n\n"
                  "Usage: zoom units\n\n"
                  "units (number) : distance to zoom, units are based on model size\n"
                  "                 1 unit is approx the model bounding box size\n"
                  "                 negative to zoom out, positive in\n";
   }
   else if (cmd == "translatex")
   {
      help += "Translate model in X direction\n\n"
                  "Usage: translationx shift\n\n"
                  "shift (number) : x axis shift\n";
   }
   else if (cmd == "translatey")
   {
      help += "Translate model in Y direction\n\n"
                  "Usage: translationy shift\n\n"
                  "shift (number) : y axis shift\n";
   }
   else if (cmd == "translatez")
   {
      help += "Translate model in Z direction\n\n"
                  "Usage: translationz shift\n\n"
                  "shift (number) : z axis shift\n";
   }
   else if (cmd == "translate")
   {
      help += "Translate model in 3 dimensions\n\n"
                  "Usage: translate dir shift\n\n"
                  "dir (x/y/z) : direction to translate\n"
                  "shift (number) : amount of translation\n"
                  "\nUsage: translation x y z\n\n"
                  "x (number) : x axis shift\n"
                  "y (number) : y axis shift\n"
                  "z (number) : z axis shift\n";
   }
   else if (cmd == "focus")
   {
      help += "Set model focal point in 3d coords\n\n"
                  "Usage: focus x y z\n\n"
                  "x (number) : x coord\n"
                  "y (number) : y coord\n"
                  "z (number) : z coord\n";
   }
   else if (cmd == "aperture")
   {
      help += "Set camera aperture (field-of-view)\n\n"
                  "Usage: aperture degrees\n\n"
                  "degrees (number) : degrees of viewing range\n";
   }
   else if (cmd == "focallength")
   {
      help += "Set camera focal length (for stereo viewing)\n\n"
                  "Usage: focallength len\n\n"
                  "len (number) : distance from camera to focal point\n";
   }
   else if (cmd == "eyeseparation")
   {
      help += "Set camera stereo eye separation\n\n"
                  "Usage: eyeseparation len\n\n"
                  "len (number) : distance between left and right eye camera viewpoints\n";
   }
   else if (cmd == "nearclip")
   {
      help += "Set near clip plane, before which geometry is not displayed\n\n"
                  "Usage: nearclip dist\n\n"
                  "dist (number) : distance from camera to near clip plane\n";
   }
   else if (cmd == "farclip")
   {
      help += "Set far clip plane, beyond which geometry is not displayed\n\n"
                  "Usage: farrclip dist\n\n"
                  "dist (number) : distance from camera to far clip plane\n";
   }
   else if (cmd == "timestep")  //Absolute
   {
      help += "Set timestep to view\n\n"
                  "Usage: timestep up/down/value\n\n"
                  "value (integer) : the timestep to view\n"
                  "up : switch to previous timestep if available\n"
                  "down : switch to next timestep if available\n";
   }
   else if (cmd == "jump")      //Relative
   {
      help += "Jump from current timestep forward/backwards\n\n"
                  "Usage: jump value\n\n"
                  "value (integer) : how many timesteps to jump (negative for backwards)\n";
   }
   else if (cmd == "model")      //Model switch
   {
      help += "Set model to view (when multiple models loaded)\n\n"
                  "Usage: model up/down/value\n\n"
                  "value (integer) : the model index to view [1,n]\n"
                  "up : switch to previous model if available\n"
                  "down : switch to next model if available\n";
   }
   else if (cmd == "hide" || cmd == "show")
   {
      help += "Hide/show objects/geometry types\n\n"
                  "Usage: hide/show object_id/object_name\n\n"
                  "object_id (integer) : the index of the object to hide/show (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to hide/show (see: \"list objects\")\n"
                  "\nUsage: hide/show geometry_type id\n\n"
                  "geometry_type : points/labels/vectors/tracers/triangles/quads/shapes/lines\n"
                  "id (integer, optional) : id of geometry to hide/show\n"
                  "eg: 'hide points' will hide all point data\n";
   }
   else if (cmd == "dump")
   {
      help += "Dump object vertices & values to CSV\n\n"
                  "Usage: dump object_id/object_name\n\n"
                  "object_id (integer) : the index of the object to export (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to export (see: \"list objects\")\n";
   }
   else if (cmd == "tracersteps")
   {
      help += "Limit tracer time-steps to plot\n\n"
                  "Usage: tracersteps limit\n\n"
                  "limit (integer) : maximum steps back in time to plot of tracer trajectory\n";
   }
   else if (cmd == "alpha")
   {
      help += "Set global transparency value\n\n"
                  "Usage: alpha value\n\n"
                  "value (integer > 1) : sets alpha as integer in range [1,255] where 255 is fully opaque\n"
                  "value (number [0,1]) : sets alpha as real number in range [0,1] where 1.0 is fully opaque\n";
   }
   else if (cmd == "opacity")
   {
      help += "Set object transparency value\n\n"
                  "Usage: opacity object_id/object_name value\n\n"
                  "object_id (integer) : the index of the object (see: \"list objects\")\n"
                  "object_name (string) : the name of the object (see: \"list objects\")\n"
                  "value (integer > 1) : sets alpha as integer in range [1,255] where 255 is fully opaque\n"
                  "value (number [0,1]) : sets alpha as real number in range [0,1] where 1.0 is fully opaque\n";
   }
   else if (cmd == "movie")
   {
      help += "Encode video of model running from current timestep to specified timestep\n"
                  "(Requires libavcodec)\n\n"
                  "Usage: movie endstep\n\n"
                  "endstep (integer) : last frame timestep\n";
   }
   else if (cmd == "record")
   {
      help += "Encode video of all frames displayed at specified framerate\n"
                  "(Requires libavcodec)\n\n"
                  "Usage: record (framerate)\n\n"
                  "framerate (integer): frames per second (default 30)\n";
   }
   else if (cmd == "play")
   {
      help += "Display model timesteps in sequence from current timestep to specified timestep\n\n"
                  "Usage: play endstep\n\n"
                  "endstep (integer) : last frame timestep\n"
                  "If endstep omitted, will loop back to start until 'stop' command entered\n";
   }
   else if (cmd == "next")
   {
      help += "Go to next timestep in sequence\n";
   }
   else if (cmd == "stop")
   {
      help += "Stop the looping 'play' command\n";
   }
   else if (cmd == "images")
   {
      help += "Write images in sequence from current timestep to specified timestep\n\n"
                  "Usage: images endstep\n\n"
                  "endstep (integer) : last frame timestep\n";
   }
   else if (cmd == "repeat")
   {
      help += "Repeat commands from history\n\n"
                  "Usage: repeat count\n\n"
                  "count (integer) : repeat the last entered command count times\n"
                  "\nUsage: repeat history count (animate)\n\n"
                  "count (integer) : repeat every command in history buffer count times\n";
   }
   else if (cmd == "animate")
   {
      help += "Update display between each command\n\n"
                  "Usage: animate rate\n\n"
                  "rate (integer) : animation timer to fire every (rate) msec, default 50\n"
                  "When on if multiple commands are issued the frame is re-rendered at set framerate\n"
                  "When off all commands will be processed before the display is updated\n";
   }
   else if (cmd == "quit")
   {
      help += "Quit the program\n";
   }
   else if (cmd == "axis")
   {
      help += "Display/hide the axis legend\n\n"
                  "Usage: axis (on/off)\n\n"
                  "on/off (optional) : show/hide the axis, if omitted switches state\n";
   }
   else if (cmd == "cullface")
   {
      help += "Switch surface face culling (global setting)\n\n"
                  "Will not effect objects with cullface set explicitly\n";
   }
   else if (cmd == "wireframe")
   {
      help += "Switch surface wireframe (global setting)\n\n"
                  "Will not effect objects with wireframe set explicitly\n";
   }
   else if (cmd == "trianglestrips")
   {
      help += "Draw quad surfaces with triangle strips\n"
                  "(Default is on, provides better detail for terrain surfaces)\n";
   }
   else if (cmd == "fullscreen")
   {
      help += "Switch viewer to full-screen mode and back to windowed mode\n";
   }
   else if (cmd == "redraw")
   {
      help += "Redraw all object data, required after changing some parameters but may be slow\n";
   }
   else if (cmd == "scaling")
   {
      help += "Disable/enable scaling, default is on, disable to view model un-scaled\n";
   }
   else if (cmd == "fit")
   {
      help += "Adjust camera view to fit the model in window\n";
   }
   else if (cmd == "autozoom")
   {
      help += "Automatically adjust camera view to always fit the model in window (enable/disable)\n";
   }
   else if (cmd == "stereo")
   {
      help += "Enable/disable stereo projection\n"
                  "If no stereo hardware found will use red/cyan anaglyph mode\n";
   }
   else if (cmd == "coordsystem")
   {
      help += "Switch between Right-handed and Lef-handed coordinate system\n";
   }
   else if (cmd == "rulers")
   {
      help += "Enable/disable axis rulers (unfinished)\n";
   }
   else if (cmd == "title")
   {
      help += "Clear title heading\n";
   }
   else if (cmd == "log")
   {
      help += "Over-ride colourmap settings to use log scales\n"
                  "Cycles between ON/OFF/DEFAULT\n"
                  "(default uses original settings for each colourmap)\n";
   }
   else if (cmd == "antialias")
   {
      help += "Enable/disable multi-sample anti-aliasing (if supported by OpenGL drivers)\n";
   }
   else if (cmd == "localise")
   {
      help += "Experimental: adjust colourmaps on each object to fit actual value range\n";
   }
   else if (cmd == "json")
   {
      help += "Dump object data to JSON file for viewing in WebGL viewer\n\n"
                  "Usage: json object_id/object_name\n\n"
                  "object_id (integer) : the index of the object to export (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to export (see: \"list objects\")\n"
                  "If object ommitted all will be exported\n";
   }
   else if (cmd == "lockscale")
   {
      help += "Enable/disable colourmap lock\n"
                  "When locked, dynamic range colourmaps will keep current values\n"
                  "when switching between timesteps instead of being re-calibrated\n";
   }
   else if (cmd == "lighting")
   {
      help += "Enable/disable lighting for objects that support this setting\n";
   }
   else if (cmd == "list")
   {
      help += "List available data\n\n"
                  "Usage: list objects/colourmaps/elements\n\n"
                  "objects : enable object list (stays on screen until disabled)\n"
                  "          (dimmed objects are hidden or not in selected viewport)\n"
                  "colourmaps : show colourmap list (onlt temporarily shown)\n"
                  "elements : show geometry elements by id in terminal\n";
   }
   else if (cmd == "reset")
   {
      help += "Reset the camera to the default model view\n";
   }
   else if (cmd == "reload")
   {
      help += "Reload all data of current model/timestep from database\n";
   }
   else if (cmd == "zerocam")
   {
      help += "Set the camera postiion to the origin (for scripting, not generally advisable)\n";
   }
   else if (cmd == "colourmap")
   {
      help += "Set colourmap on object\n\n"
                  "Usage: colourmap object_id/object_name [colourmap_id/colourmap_name | \"data\"]\n\n"
                  "object_id (integer) : the index of the object to set (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to set (see: \"list objects\")\n"
                  "colourmap_id (integer) : the index of the colourmap to apply (see: \"list colourmaps\")\n"
                  "colourmap_name (string) : the name of the colourmap to apply (see: \"list colourmaps\")\n"
                  "data (string) : data to load into selected object's colourmap\n";
   }
   else if (cmd == "pointtype")
   {
      help += "Set point-rendering type on object\n\n"
                  "Usage: pointtype all/object_id/object_name type/up/down\n\n"
                  "all : use 'all' to set the global default point type\n"
                  "object_id (integer) : the index of the object to set (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to set (see: \"list objects\")\n"
                  "type (integer) : Point type [0,3] to apply (gaussian/flat/sphere/highlight sphere)\n"
                  "up/down : use 'up' or 'down' to switch to the previous/next type in list\n";
   }
   else if (cmd == "glyphquality")
   {
      help += "Set vector/tracer/shape rendering quality\n\n"
                  "Usage: glyphquality value\n\n"
                  "value (integer) : 0=flat, [1-10] increasing quality of 3d glyphs (default 2)\n";
   }
   else if (cmd == "tracerscale")
   {
      help += "Enable/disable scaled tracers (earlier timesteps = narrower lines)\n";
   }
   else if (cmd == "pointsample")
   {
      help += "Set point sub-sampling value\n\n"
                  "Usage: pointsample value/up/down\n\n"
                  "value (integer) : 1=no sub-sampling=50%% sampled etc..\n"
                  "up/down : use 'up' or 'down' to sample more (up) or less (down) points\n";
   }
   else if (cmd == "image")
   {
      help += "Save an image of the current view\n";
   }
   else if (cmd == "outwidth")
   {
      help += "Set output image width (height calculated to match window aspect)\n\n"
                  "Usage: outwidth value\n\n"
                  "value (integer) : width in pixels for output images\n";
   }
   else if (cmd == "outheight")
   {
      help += "Set output image height\n\n"
                  "Usage: outheight value\n\n"
                  "value (integer) : height in pixels for output images\n";
   }
   else if (cmd == "background")
   {
      help += "Set window background colour\n\n"
                  "Usage: background value/white/black/grey/invert\n\n"
                  "value (number [0,1]) : sets to greyscale value with luminosity in range [0,1] where 1.0 is white\n";
                  "white/black/grey : sets background to specified value\n"
                  "invert (or omitted value) : inverts current background colour\n";
   }
   else if (cmd == "border")
   {
      help += "Set model border frame\n\n"
                  "Usage: border on/off/filled\n\n"
                  "on : line border around model bounding-box\n"
                  "off : no border\n"
                  "filled : filled background bounding box"
                  "(no value) : switch between possible values\n";
   }
   else if (cmd == "camera")
   {
      help += "Output camera view as XML for use in model scripts\n";
   }
   else if (cmd == "sort")
   {
      help += "Sort geometry on demand (with idle timer)\n";
   }
   else if (cmd == "scale")
   {
      help += "Scale applicable objects up/down in size (points/vectors/shapes)\n\n"
                  "Usage: scale axis value\n\n"
                  "axis : x/y/z\n"
                  "value (number) : scaling value applied to all geometry on specified axis\n\n"
                  "Usage: scale geometry_type value/up/down\n\n"
                  "geometry_type : points/vectors/tracers/shapes\n"
                  "value (number) or 'up/down' : scaling value or use 'up' or 'down' to reduce/increase scaling\n"
                  "\nUsage: scale object_id/object_name value/up/down\n\n"
                  "object_id (integer) : the index of the object to set (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to set (see: \"list objects\")\n"
                  "value (number) or 'up/down' : scaling value or use 'up' or 'down' to reduce/increase scaling\n";
   }
   else if (cmd == "history")
   {
      help += "Write command history to output (eg: terminal)"
                  "Usage: history [filename]\n\n"
                  "filename (string) : optional file to write data to\n";
   }
   else if (cmd == "clearhistory")
   {
      help += "Clear command history\n";
   }
   else if (cmd == "resize")
   {
      help += "Resize view window\n\n"
                  "Usage: resize width height\n\n"
                  "width (integer) : width in pixels\n"
                  "height (integer) : height in pixels\n";
   }
   else if (cmd == "pause")
   {
      help += "Pause execution (for scripting)\n\n"
                  "Usage: pause msecs\n\n"
                  "msecs (integer) : how long to pause for in milliseconds\n";
   }
   else if (cmd == "delete")
   {
      help += "Delete objects from database\n"
                  "WARNING: This is irreversable! Backup your database before using!\n\n"
                  "Usage: delete object_id/object_name\n\n"
                  "object_id (integer) : the index of the object to delete (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to delete (see: \"list objects\")\n";
   }
   else if (cmd == "load")
   {
      help += "Load objects from database\n"
                  "Used when running with deferred loading (-N command line switch)\n\n"
                  "Usage: load object_id/object_name\n\n"
                  "object_id (integer) : the index of the object to load (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to load (see: \"list objects\")\n";
   }
   else if (cmd == "file")
   {
      help += "Load database or model file\n"
                  "Usage: file \"filename\"\n\n"
                  "filename (string) : the path to the file, quotes optional but necessary if path contains spaces\n";
   }
   else if (cmd == "script")
   {
      help += "Run script file\n"
                  "Load a saved script of viewer commands from a file\n\n"
                  "Usage: script filename\n\n"
                  "filename (string) : path and name of the script file to load\n";
   }
   else if (cmd == "select")
   {
      help += "Select object as the active object\n"
                  "Used for setting properties of objects\n\n"
                  "Usage: select object_id/object_name\n\n"
                  "object_id (integer) : the index of the object to select (see: \"list objects\")\n"
                  "object_name (string) : the name of the object to select (see: \"list objects\")\n"
                  "Leave object parameter empty to clear selection.\n";
   }
   else
      return std::string("");
   return help;
}

