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

#include "../GraphicsUtil.h"
#include "../LavaVu.h"
#include "../Server.h"
#include "../SageOutput.h"
#include "../OpenGLViewer.h"
#include "SDLViewer.h"
#include "X11Viewer.h"
#include "GlutViewer.h"
#include "OSMesaViewer.h"
#include "AGLViewer.h"

#define X11_WINDOW 0
#define GLUT_WINDOW 1
#define SDL_WINDOW 2
#define OSMESA_WINDOW 3
#define AGL_WINDOW 4

// Main function
int main(int argc, char *argv[]) 
{
   OpenGLViewer* viewer = NULL;
   LavaVu* app;
   int port = 8080, quality = 0, threads = 4;
   bool stereo = false;
   bool fullscreen = false;
   int width = 0, height = 0;
   int window;
   std::vector<std::string> args;

//Evil platform specific extension handling stuff
#if defined _WIN32
  //GetProcAddress = (getProcAddressFN)wglGetProcAddress;
#elif defined HAVE_OSMESA
  GetProcAddress = (getProcAddressFN)OSMesaGetProcAddress;
#elif defined HAVE_X11  //not defined __APPLE__
  //GetProcAddress = (getProcAddressFN)glXGetProcAddress;
  GetProcAddress = (getProcAddressFN)glXGetProcAddressARB;
#elif defined HAVE_GLUT and not defined __APPLE__
  GetProcAddress = (getProcAddressFN)glXGetProcAddress;
#endif

   //Set default viewer
#if defined HAVE_X11
   window = X11_WINDOW;
#elif defined HAVE_GLUT
   window = GLUT_WINDOW;
#elif defined HAVE_SDL
   window = SDL_WINDOW;
#elif defined HAVE_OSMESA
   window = OSMESA_WINDOW;
#elif defined HAVE_AGL
   window = AGL_WINDOW;
#else
   abort_program("No windowing system configured (requires X11, GLUT, SDL, AGL or OSMesa)");
#endif

   //Shader path (default to program path if not set)
   std::string xpath = GetBinaryPath(argv[0], "LavaVu");
   if (!Shader::path) Shader::path = xpath.c_str();

   //Read command line
   for (int i=1; i<argc; i++)
   {
      //Help?
      if (strstr(argv[i], "?") || strstr(argv[i], "help"))
      {
         std::cout << "Viewer command line options:\n\n";
         std::cout << "\nStart and end timesteps\n";
         std::cout << " -# : Any integer entered as a switch will be interpreted as the initial timestep to load\n";
         std::cout << "      Any following integer switch will be interpreted as the final timestep for output\n";
         std::cout << "      eg: -10 -20 will run all output commands on timestep 10 to 20 inclusive\n";
         std::cout << " -c#: caching, set # of timesteps to cache data in memory for\n";
         std::cout << " -P#: subsample points\n";
         std::cout << " -A : All objects hidden initially, use 'show object' to display\n";
         std::cout << " -N : No load, deferred loading mode, use 'load object' to load & display from database\n";
         std::cout << "\nGeneral options\n";
         std::cout << " -v : Verbose output, debugging info displayed to console\n";
         std::cout << " -o : output mode: all commands entered dumped to standard output,\n";
         std::cout << "      useful for redirecting to a script file to recreate a visualisation setup.\n";
         std::cout << " -a#: set global alpha to # [0,255] where 255 is fully opaque\n";
         std::cout << " -p#: port, web server interface listen on port #\n";
         std::cout << " -q#: quality, web server jpeg quality (0=don't serve images)\n";
         std::cout << " -n#: number of threads to launch for web server #\n";
         std::cout << " -l: use local shaders, locate in working directory not executable directory\n";
         std::cout << "\nImage/Video output\n";
         std::cout << " -w: write images of all loaded timesteps/windows then exit\n";
         std::cout << " -i: as above\n";
         std::cout << " -W: write images as above but using input database path as output path for images\n";
         std::cout << " -I: as above\n";
         std::cout << " -t: write transparent background png images (if supported)\n";
         std::cout << " -m: write movies of all loaded timesteps/windows then exit (if supported)\n";
         std::cout << " -xWIDTH,HEIGHT: set output image width (height optional, will be calculated if omitted)\n";
         std::cout << "\nData export\n";
         std::cout << " -d#: export object id # to CSV vertices + values\n";
         std::cout << " -j#: export object id # to JSON, if # omitted will output all compatible objects\n";
         std::cout << " -g#: export object id # to GLDB, if # omitted will output all compatible objects\n";
         std::cout << "\nWindow settings\n";
         std::cout << " -rWIDTH,HEIGHT: resize initial viewer window to width x height\n";
         std::cout << " -h: hidden window, will exit after running any provided input script and output options\n";
         std::cout << " -s: enable stereo mode if hardware available\n";
         std::cout << " -f: enable full-screen mode if supported\n";
         std::cout << " -GLUT: attempt to use GLUT window if available\n";
         std::cout << " -SDL: attempt to use SDL window if available\n";
         return 0;
      }
      //Switches can be before or after files but not between
      if (argv[i][0] == '-' && strlen(argv[i]) > 1)
      {
         char x;
         std::istringstream ss(argv[i]+2);

         //Window type requests
         if (strcmp(argv[i], "-SDL") == 0)
         {
#if defined HAVE_SDL
            window = SDL_WINDOW;
#else
            std::cerr << "SDL support not available\n";
#endif
            continue;
         }
         else if (strcmp(argv[i], "-GLUT") == 0)
         {
#if defined HAVE_GLUT
            window = GLUT_WINDOW;
#else
            std::cerr << "GLUT support not available\n";
#endif
            continue;
         }

         switch (argv[i][1])
         {
         case 'r':
            ss >> width >> x >> height;
            break;
         case 's':
            //Stereo window requested
            stereo = true;
            break;
         case 'f':
            //Fullscreen window requested
            fullscreen = true;
            break;
         case 'p':
            //Web server enable
            ss >> port;
            break;
         case 'q':
            //Web server JPEG quality
            ss >> quality;
            break;
         case 'n':
            //Web server threads
            ss >> threads;
            break;
         default:
            if (strlen(argv[i]) > 0)
               args.push_back(argv[i]);
         }
      }
      else if (strlen(argv[i]) > 0)
         args.push_back(argv[i]);
   }

   //Create viewer window
#if defined HAVE_X11
   if (window == X11_WINDOW) viewer = new X11Viewer(stereo, fullscreen);
#endif
#if defined HAVE_GLUT
   if (window == GLUT_WINDOW) viewer = new GlutViewer(stereo, fullscreen);
#endif
#if defined HAVE_SDL || defined _WIN32
   if (window == SDL_WINDOW) viewer = new SDLViewer(stereo, fullscreen);
#endif
#if defined HAVE_OSMESA
   if (window == OSMESA_WINDOW) viewer = new OSMesaViewer();
#endif
#if defined HAVE_AGL
   if (window == AGL_WINDOW) viewer = new AGLViewer();
#endif
   if (!viewer) abort_program("No Viewer available\n");

   //Add any output attachments to the viewer
#ifndef DISABLE_SERVER
   if (port)
   {
      //Use executable path as base for html path
      std::string htmlpath = xpath + "html";
      viewer->addOutput(Server::Instance(viewer, htmlpath, port, quality, threads));
   }
#endif
#ifdef HAVE_SAGE
   viewer->addOutput(SageOutput::Instance(viewer));
#endif

   //Create & run application
   app = new LavaVu(args, viewer, width, height);
   app->run(port > 0); //If server running, always stay open (persist flag)

   delete viewer;
   delete app;
   
   return 0;
}

