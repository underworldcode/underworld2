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

#ifdef HAVE_GLUT

#ifndef GlutViewer__
#define GlutViewer__

#include "../GraphicsUtil.h"
#include "../OpenGLViewer.h"

#if not defined __APPLE__
#include <GL/glut.h>
//Need this for glxGetProcAddress
#include <GL/glx.h>
#else
#include <GLUT/glut.h>
#endif

//Derived from window class
class GlutViewer  : public OpenGLViewer
{
  public:
   GlutViewer(bool stereo, bool fullscreen);
   
   //Function implementations
   void open(int width, int height);
   void setsize(int width, int height);
   void show();
   void display();
   void swap();
   void execute();
   void fullScreen();
   void animate(int msec);

  private:
   //Static self-reference
   static GlutViewer* self;
   int win;
   //Callback functions
   static void glut_timer(int value);
   static void glut_display(void);
   static void glut_resize(int width, int height);
   static void glut_mouseclick(int btn, int state, int x, int y);
   static void glut_mousemove(int x, int y);
   static void glut_keyboard(unsigned char key, int x, int y);
   static void glut_keyboard_ext(int code, int x, int y);
   static void get_modifiers(void);
};

#endif //GlutViewer__

#endif //HAVE_GLUT


