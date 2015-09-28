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

#ifdef HAVE_AGL

#include "AGLViewer.h"

#include <sys/select.h>

// Create a new AGL window 
AGLViewer::AGLViewer() : OpenGLViewer(false, false)
{
   visible = false;
   PixelBuffer = NULL;
   graphicsContext = NULL;
}

AGLViewer::~AGLViewer()
{
   if (PixelBuffer)
      aglDestroyPBuffer(PixelBuffer);
   aglSetCurrentContext(0);
   aglDestroyContext(graphicsContext );
}

void AGLViewer::open(int w, int h)
{
   //Call base class open to set width/height
   OpenGLViewer::open(w, h);
   
   if (PixelBuffer)
      aglDestroyPBuffer(PixelBuffer);

   AGLPixelFormat format; // OpenGL pixel format
  
   // OpenGL attributes
   static GLint attributes[] =
   {
      AGL_RGBA,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_ALPHA_SIZE, 8,
      AGL_DEPTH_SIZE, 16,
      AGL_STENCIL_SIZE, 1,
      AGL_NONE
   };
   static GLint aaAttributes[] =
   {
      AGL_RGBA,
      AGL_RED_SIZE, 8,
      AGL_GREEN_SIZE, 8,
      AGL_BLUE_SIZE, 8,
      AGL_ALPHA_SIZE, 8,
      AGL_DEPTH_SIZE, 16,
      AGL_STENCIL_SIZE, 1,
      AGL_SAMPLE_BUFFERS_ARB, 1, //Multisample
      AGL_SAMPLES_ARB, 16,
      AGL_NONE
   };
   int* attribs;
   attribs = aaAttributes;
   //if (antialias) attribs = aaAttributes;
   //else attribs = attributes;
  
   // Create the OpenGL context (shared) and bind it to the window or pixelbuffer.
   format = aglChoosePixelFormat(NULL, 0, attribs);
   graphicsContext = aglCreateContext(format, NULL);
   assert(graphicsContext);
   aglDestroyPixelFormat(format);
  
   if (!fbo_enabled) // No PBuffer necessary if using fbo render mode
   {
      aglCreatePBuffer(width, height, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, 0, &PixelBuffer);
      aglSetPBuffer(graphicsContext, PixelBuffer, 0, 0, 0);
   }
  
   aglSetCurrentContext(graphicsContext); 

   debug_print("AGL viewer created\n");

   //Call OpenGL init
   OpenGLViewer::init();
}

void AGLViewer::setsize(int width, int height)
{
   if (width == 0 || height == 0) return;
   close();
   open(width, height);
}

void AGLViewer::show()
{
   OpenGLViewer::show(); 
}

void AGLViewer::display()
{
   OpenGLViewer::display();
   swap();
}

void AGLViewer::swap()
{
   // Swap buffers
   if (doubleBuffer)
      aglSwapBuffers(graphicsContext); 

   OpenGLViewer::display();
}

void AGLViewer::execute()
{
   // Set the current window's context as active
   aglSetCurrentContext(graphicsContext);
   aglUpdateContext(graphicsContext); 

   show();
   display();

   // Enter fake event loop processing
   while (!quitProgram)
   {
      //New frame? call display
      if (postdisplay || OpenGLViewer::pollInput())
         display();
   }
}

#endif   //HAVE_AGL

