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

#if defined HAVE_SDL

#include <stdlib.h>
#include <signal.h>
#include <math.h>

#include "SDLViewer.h"

/* Timer callback */
Uint32 sdl_timer(Uint32 interval, void* param)
{
   // Create a user event and post
   SDLViewer* self = (SDLViewer*) param;
   if (self->postdisplay || OpenGLViewer::pollInput())
   {
      SDL_Event event;// = {SDL_USEREVENT, 1, 0, 0};
      event.type = SDL_USEREVENT;
      event.user.code = 1;
      event.user.data1 = 0;
      event.user.data2 = 0;
      SDL_PushEvent(&event);
   }

   return interval;
}

// Create a new SDL window 
SDLViewer::SDLViewer(bool stereo, bool fullscreen) : OpenGLViewer(stereo, fullscreen), screen(NULL)
{
   const SDL_VideoInfo *pSDLVideoInfo;
putenv(strdup("SDL_VIDEO_WINDOW_POS=center"));
   // Initialise SDL Video subsystem 
   if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 ) 
      abort_program("Unable to initialize SDL: %s", SDL_GetError());

   pSDLVideoInfo = SDL_GetVideoInfo();

   if( !pSDLVideoInfo )
   {
      SDL_Quit();
      abort_program("SDL_GetVideoInfo() failed. SDL Error: %s", SDL_GetError());
   }

   timer_id = 0;

   // NOTE: still want Ctrl-C to work, undo the SDL redirections 
#ifndef _WIN32
   signal(SIGINT, SIG_DFL);
   signal(SIGQUIT, SIG_DFL);
#endif

   // Keyboard setup 
   SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
//#ifndef PDF_CAPTURE
   SDL_EnableUNICODE(1);   //Enable unicode character translation
//Above prevents adobe 3d capture from detecting print screen key?
//#endif

   // Save fullscreen width/height 
   savewidth = pSDLVideoInfo->current_w;
   saveheight = pSDLVideoInfo->current_h;

   resized = false;
   screen = NULL;

   debug_print("SDL viewer created\n");
}

SDLViewer::~SDLViewer()
{
   SDL_Quit();
   screen = NULL;
}

void SDLViewer::open(int w, int h)
{
   //Call base class open to set width/height
   OpenGLViewer::open(w, h);
   
   createWindow(width, height);
   
   //Call OpenGL init
   OpenGLViewer::init();
}

void SDLViewer::setsize(int width, int height)
{
   if (width == 0 || height == 0) return;
   close();
   open(width, height);
}

void SDLViewer::animate(int msec)
{
   if (timer_id)
     SDL_RemoveTimer(timer_id);
   timer_id = SDL_AddTimer(msec, sdl_timer, this);
}

void SDLViewer::createWindow(int width, int height)
{
   int SDL_Flags;
   if (fullscreen)
      //SDL_Flags = SDL_OPENGL | SDL_FULLSCREEN;
      SDL_Flags = SDL_OPENGL | SDL_NOFRAME;
   else
      SDL_Flags = SDL_OPENGL | SDL_RESIZABLE;

    // set opengl attributes 
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,      8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,    0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,    1);
    if (stereo) SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
    // Enable 4xsample Antialiasing 
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

#ifdef _WIN32
   //GL context lost on resize in windows
    if (screen != NULL) close();
#endif

   // Create our rendering surface 
   screen = SDL_SetVideoMode( width, height, 32, SDL_Flags);
   if (!screen)
   {
      // Try without stereo 
      debug_print("Stereo hardware not available\n");
      SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
      screen = SDL_SetVideoMode( width, height, 32, SDL_Flags );
      if (!screen)
      {
         SDL_Quit();
         abort_program("Call to SDL_SetVideoMode() failed! - SDL_Error: %s", SDL_GetError());
      }
   }  
}

void SDLViewer::show() 
{
   OpenGLViewer::show();
   // Update title 
   SDL_WM_SetCaption(title.c_str(), NULL);
}

void SDLViewer::display()
{
   OpenGLViewer::display();
   swap();
}

void SDLViewer::swap()
{
   // Swap buffers 
   if (doubleBuffer)
      SDL_GL_SwapBuffers();
}

void SDLViewer::execute()
{
   bool redisplay = true;
   MouseButton button = NoButton;
   unsigned char key;
   int x, y;
   SDL_Event event;
   int modifiers = 0;
   static unsigned int stimer;

   //Ensure window visible for interaction
   show();

   // Enter event loop processing
   while ( !quitProgram ) 
   {
      // Check for events 
      // Delay redisplay & resize until event queue empty 
      if (SDL_PollEvent(&event) == 0)
      {
         // Resize window 
         if (resized) 
         {
            if (stimer < SDL_GetTicks()) 
            {
               // Create in new dimensions 
               resized = false;
               open(width, height);
               redisplay = true;
            } 
            else
               continue;   //Cycle until timer runs out
         } 

         if (redisplay)
         {
             // Redraw Viewer (Call virtual to display) 
            display();
            redisplay = false;
         }

         // Wait for next event 
         SDL_WaitEvent( &event );
      }

      //Save shift states
      modifiers = SDL_GetModState();
      keyState.shift = (modifiers & KMOD_SHIFT);
      keyState.ctrl = (modifiers & KMOD_CTRL);
      keyState.alt = (modifiers & KMOD_ALT);

      // Process event 
      switch (event.type)
      {
      case SDL_QUIT:
         quitProgram = true;
         break;   
      case SDL_VIDEORESIZE:
         //Adjust viewport metrics etc...
         resize(event.resize.w, event.resize.h);
         resized = true;
         //Start timer to wait for sizing events to cease before calling actual context resize
         stimer = SDL_GetTicks() + 200;
         break;
      case SDL_KEYDOWN:
         //Pass keystrokes on KEYDOWN only, char info not provided by SDL on KEYUP
         key = event.key.keysym.unicode;
         if (!key)
         {
            int code = (int)event.key.keysym.sym;
            if (code == SDLK_KP8 || code == SDLK_UP) key = KEY_UP;
            if (code == SDLK_KP2 || code == SDLK_DOWN) key = KEY_DOWN;
            if (code == SDLK_KP4 || code == SDLK_LEFT) key = KEY_LEFT;
            if (code == SDLK_KP6 || code == SDLK_RIGHT) key = KEY_RIGHT;
            if (code == SDLK_KP9 || code == SDLK_PAGEUP) key = KEY_PAGEUP;
            if (code == SDLK_KP3 || code == SDLK_PAGEDOWN) key = KEY_PAGEDOWN;
            if (code == SDLK_KP7 || code == SDLK_HOME) key = KEY_HOME;
            if (code == SDLK_KP1 || code == SDLK_END) key = KEY_END;
            //key = (unsigned char)code;
            if (!key) continue;
         }
         SDL_GetMouseState(&x, &y);
         if (keyPress(key, x, y)) redisplay = true;
         break;
      case SDL_MOUSEMOTION:
         if (mouseState)
         {
            mouseMove(event.motion.x, event.motion.y);
            redisplay = true;
         }
         break;
      case SDL_MOUSEBUTTONDOWN: 
         button = (MouseButton)event.button.button;
         // XOR state of three mouse buttons to the mouseState variable  
         if (button <= RightButton) mouseState ^= (int)pow(2.0,button);
         mousePress( button, true, event.button.x, event.button.y);
         break;
      case SDL_MOUSEBUTTONUP:
         mouseState = 0;
         button = (MouseButton)event.button.button;
         mousePress( button, false, event.button.x, event.button.y);
         redisplay = true;
         break;
      case SDL_USEREVENT:
         // Timer event
         redisplay = true;
         break;
      case SDL_ACTIVEEVENT:
         if (event.active.state == SDL_APPACTIVE && event.active.gain == 1)   // Restored from icon 
            redisplay = true;
         break;
      case SDL_VIDEOEXPOSE:
         redisplay = true;   // Repaint 
         break;
      }
   }
}

void SDLViewer::fullScreen()
{
   int w = width, h = height;
   width = savewidth;
   height = saveheight;
   savewidth = w;
   saveheight = h;
   fullscreen = fullscreen ? 0 : 1;
   resized = true;
}
#endif

