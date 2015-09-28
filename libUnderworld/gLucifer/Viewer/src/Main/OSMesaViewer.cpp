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

#ifdef HAVE_OSMESA

#include "OSMesaViewer.h"

#include <sys/select.h>

// Create a new OSMesa window 
OSMesaViewer::OSMesaViewer() : OpenGLViewer(false, false)
{
   visible = false;
   pixelBuffer = NULL;
   osMesaContext = NULL;
   fbo_enabled = false;
}

OSMesaViewer::~OSMesaViewer()
{
   animate(0);
   OSMesaDestroyContext(osMesaContext);
   delete[] pixelBuffer;
}

void OSMesaViewer::open(int w, int h)
{
   //Call base class open to set width/height
   OpenGLViewer::open(w, h);
   fbo_enabled = false; //Force fbo disabled
   
   if (osMesaContext)
      OSMesaDestroyContext(osMesaContext);
   if (pixelBuffer)
      delete[] pixelBuffer;

   // Init OSMesa display buffer
   pixelBuffer = new GLubyte[width * height * 4];
   // 24 bit depth, 1 bit stencil, no accum, no shared display lists
   osMesaContext = OSMesaCreateContextExt(OSMESA_RGBA, 24, 1, 0, NULL);
   OSMesaMakeCurrent(osMesaContext, pixelBuffer, GL_UNSIGNED_BYTE, width, height);
   debug_print("OSMesa viewer created\n");
   timer = 0;
   //const char* gl_version = (const char*)glGetString(GL_VERSION);

   //Call OpenGL init
   OpenGLViewer::init();
}

void OSMesaViewer::setsize(int width, int height)
{
   if (width == 0 || height == 0) return;
   close();
   open(width, height);
}

void OSMesaViewer::display()
{
   // Make sure we are using the correct context when more than one are created
   OSMesaContext context = OSMesaGetCurrentContext();
   if (context != osMesaContext)
      OSMesaMakeCurrent(osMesaContext, pixelBuffer, GL_UNSIGNED_BYTE, width, height);

   OpenGLViewer::display();
}

#endif   //HAVE_OSMESA

