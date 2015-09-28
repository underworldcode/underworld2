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

#include <stdlib.h>
#include <signal.h>
#include <math.h>

#include "GlutViewer.h"

GlutViewer* GlutViewer::self; //Declare self-referential static pointer (must be outside class)

// Create a new Glut window 
GlutViewer::GlutViewer(bool stereo, bool fullscreen) : OpenGLViewer(stereo, fullscreen)
{
   GlutViewer::self = NULL;

   int argc = 0;
   char** argv = NULL;   
   glutInit(&argc, argv);
   //   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
   glutInitDisplayString( "rgba alpha depth double samples");
   win = 0;

   debug_print("GLUT viewer created\n");
}

void GlutViewer::open(int w, int h)
{
   //Call base class open to set width/height
   OpenGLViewer::open(w, h);
   
   if (!GlutViewer::self)
   {
      GlutViewer::self = this;

      if (visible)
         glutInitWindowSize(width, height);
      else
         glutInitWindowSize(1, 1);

      win = glutCreateWindow("");

      if (!visible) glutHideWindow();

      glutDisplayFunc(glut_display);
      glutReshapeFunc(glut_resize);
      glutKeyboardFunc(glut_keyboard);
      glutSpecialFunc(glut_keyboard_ext);
      glutMouseFunc(glut_mouseclick);
      glutMotionFunc(glut_mousemove);
   }

   //Call OpenGL init
   OpenGLViewer::init();
}

void GlutViewer::setsize(int width, int height)
{
   if (width == 0 || height == 0) return;
   //Resize
   glutReshapeWindow(width, height);

   //Call base class setsize
   OpenGLViewer::setsize(width, height);

   //No glut callbacks when window is hidden so manually call resize
   if (!visible)
      resize(width, height);
}

void GlutViewer::show()
{
   OpenGLViewer::show(); 
   if (!visible)
   {
      glutShowWindow();
      //Restore to set width & height
      glutReshapeWindow(width, height);
   }
   // Update title 
   glutSetWindowTitle(title.c_str());
}

void GlutViewer::display()
{
   OpenGLViewer::display();
   swap();
}

void GlutViewer::swap()
{
   // Swap buffers 
   if (doubleBuffer)
      glutSwapBuffers();
}

void GlutViewer::execute()
{
   //Ensure window visible for interaction
   if (visible) 
   {
     show();
     glutMainLoop();
   }
   else
   {
      //Run our own event loop
      OpenGLViewer::execute();
   }
}

void GlutViewer::fullScreen()
{
   static int savewidth, saveheight;
   fullscreen = fullscreen ? 0 : 1;
   if (fullscreen)
   {
      savewidth = width;
      saveheight = height;
      glutFullScreen();
   }
   else
   {
      glutFullScreen();
      glutReshapeWindow(savewidth, saveheight);
      glutPositionWindow(100,100);
      //self->resize(savewidth, saveheight);
   }
   debug_print("fullscreen %d sw %d sh %d\n", fullscreen, savewidth, saveheight);
}

void GlutViewer::animate(int msec)
{
   glutTimerFunc(msec, glut_timer, msec);
}

//Glut callback functions...
void GlutViewer::glut_display(void)
{
   self->display();
}

void GlutViewer::glut_timer(int value)
{
   if (self->postdisplay || OpenGLViewer::pollInput())
      glutPostRedisplay();

   //Have to force quit in GLUT as we don't control event loop
   if (self->quitProgram) exit(0);

   glutTimerFunc(value, glut_timer, value);
}

void GlutViewer::glut_resize(int width, int height)
{
   self->resize(width, height);
}

void GlutViewer::glut_mouseclick(int btn, int state, int x, int y)
{
   get_modifiers();
   MouseButton button = MouseButton(btn+1);
   if (state == 0)
   {
      // XOR state of three mouse buttons to the mouseState variable  
      if (button <= RightButton) self->mouseState ^= (int)pow(2,button+1);
      self->mousePress(button, true, x, y);
   }
   else
   {
      // Release
      self->mouseState = 0;
      self->mousePress(button, false, x, y);
   }
   glutPostRedisplay();
}

void GlutViewer::glut_mousemove(int x, int y)
{
   if (self->mouseState)
   {
      self->mouseMove(x, y);
      glutPostRedisplay();
   }
}

void GlutViewer::glut_keyboard(unsigned char key, int x, int y)
{
   get_modifiers();
   if (self->keyPress(key, x, y)) glutPostRedisplay();

   if (self->quitProgram)
   {
      exit(0);
   }
}

void GlutViewer::glut_keyboard_ext(int code, int x, int y)
{
   unsigned char key;
   get_modifiers();
   switch (code)
   {
   case GLUT_KEY_LEFT:
      key = KEY_LEFT; break;
   case GLUT_KEY_RIGHT:
      key = KEY_RIGHT; break;
   case GLUT_KEY_DOWN:
      key = KEY_DOWN; break;
   case GLUT_KEY_UP:
      key = KEY_UP; break;
   case GLUT_KEY_PAGE_UP:
      key = KEY_PAGEUP; break;
   case GLUT_KEY_PAGE_DOWN:
      key = KEY_PAGEDOWN; break;
   case GLUT_KEY_HOME:
      key = KEY_HOME; break;
   case GLUT_KEY_END:
      key = KEY_END; break;
   default:
      return;
   }

   if (self->keyPress(key, x, y)) glutPostRedisplay();
}

void GlutViewer::get_modifiers(void)
{
   //Save shift states
   int modifiers = glutGetModifiers();
   self->keyState.shift = (modifiers & GLUT_ACTIVE_SHIFT);
   self->keyState.ctrl = (modifiers & GLUT_ACTIVE_CTRL);
   self->keyState.alt = (modifiers & GLUT_ACTIVE_ALT);;
}

#endif
