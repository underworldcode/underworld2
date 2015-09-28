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

#ifdef HAVE_X11

#include "X11Viewer.h"

#include <signal.h>
#include <sys/time.h>

int X11_error(Display* Xdisplay, XErrorEvent* error)
{
   char error_str[256];
   XGetErrorText(Xdisplay, error->error_code, error_str, 256);
   debug_print("X11 Error: %d -> %s\n", error->error_code, error_str);
   return error->error_code;
}

// Create a new X11 window 
X11Viewer::X11Viewer(bool stereo, bool fullscreen) : OpenGLViewer(stereo, fullscreen)
{
   // Setup display name 
   strcpy(host, "localhost");
   displayNumber = 0;
   displayScreen = 0;
   sprintf(displayName, "%s:%u.%u", host, displayNumber, displayScreen);

   XSetErrorHandler(X11_error);
   debug_print("X11 viewer created\n");
   Xdisplay = NULL;
   sHints = NULL;
   wmHints = NULL;
}

X11Viewer::~X11Viewer()
{
   if (Xdisplay)
   {
      if (sHints) XFree(sHints);
      if (wmHints) XFree(wmHints);
      XDestroyWindow(Xdisplay ,win);
      XFree(vi);
      if (glxcontext) glXDestroyContext(Xdisplay, glxcontext);
      XSetCloseDownMode(Xdisplay, DestroyAll);
      XCloseDisplay(Xdisplay);
   }
}

void X11Viewer::open(int w, int h)
{
   //Call base class open to set width/height
   OpenGLViewer::open(w, h);
   
   if (!Xdisplay)
   {
      //********************** Create Display *****************************
      Xdisplay = XOpenDisplay(NULL);
      if (Xdisplay == NULL)
      {
         debug_print("In func %s: Function XOpenDisplay(NULL) returned NULL\n", __func__);

         // Second Try 
         Xdisplay = XOpenDisplay(displayName);
         if (Xdisplay == NULL)
         {
            debug_print("In func %s: Function XOpenDisplay(%s) didn't work.\n", __func__ , displayName);

            // Third Try 
            Xdisplay = XOpenDisplay(":0.0");
            if (Xdisplay == NULL)
            {
               debug_print("In func %s: Function XOpenDisplay(\":0.0\") returned NULL\n", __func__);
               return;
            }
         }
      }

      // Check to make sure display we've just opened has a glx extension 
      if (!glXQueryExtension(Xdisplay, NULL, NULL))
      {
         debug_print("In func %s: X server has no OpenGL GLX extension\n", __func__);
         return;
      }

      // Get visual 
      if (!chooseVisual()) return;

      // Setup window manager hints 
      sHints = XAllocSizeHints();
      wmHints = XAllocWMHints();

      // Create a window or buffer to render to 
      createWindow(width, height);
   }
   else
   {
      //Resize
      XDestroyWindow(Xdisplay, win );
      if (glxcontext) glXDestroyContext(Xdisplay, glxcontext);
      createWindow(width, height);
      show();
      XSync(Xdisplay, false);  // Flush output buffer 
   }

   //Call OpenGL init
   OpenGLViewer::init();
}

void X11Viewer::setsize(int width, int height)
{
   if (width == 0 || height == 0) return;
   XResizeWindow(Xdisplay, win, width, height);
   //Call base class setsize
   OpenGLViewer::setsize(width, height);
}

void X11Viewer::show()
{
   if (!visible) return;
   OpenGLViewer::show(); 
   XMapRaised( Xdisplay, win ); // Show the window 

   // Update title 
   XTextProperty Xtitle;
   char* titlestr = (char*)title.c_str();
   XStringListToTextProperty(&titlestr, 1, &Xtitle);  //\/ argv, argc, normal_hints, wm_hints, class_hints 
   XSetWMProperties(Xdisplay, win, &Xtitle, &Xtitle, NULL, 0, sHints, wmHints, NULL);
}

void X11Viewer::display()
{
   OpenGLViewer::display();
   swap();
}

void X11Viewer::swap()
{
   // Swap buffers 
   if (doubleBuffer)
      glXSwapBuffers(Xdisplay, win);
}

void X11Viewer::execute()
{
   XEvent         event;
   MouseButton button;
   unsigned char key;
   KeySym keysym;
   char buf[64];
   int count;
   bool hidden = false;
   bool redisplay = true;

   //Ensure window visible for interaction
   show();

   //Get file descriptor of the X11 display
   int x11_fd = ConnectionNumber(Xdisplay);
   //Create a File Description Set containing x11_fd
   fd_set in_fds;
   FD_ZERO(&in_fds);
   FD_SET(x11_fd, &in_fds);
   struct timeval tv;
   tv.tv_sec   = 0;

   // Enter event loop processing
   while (!quitProgram)
   {
      if (timer > 0)
      {

         // Wait for X Event or timer
         tv.tv_usec  = timer*1000; //Convert to microseconds
         if (select(x11_fd+1, &in_fds, 0, 0, &tv) == 0)
         {
            //Timer fired
            if (postdisplay || OpenGLViewer::pollInput())
               display();
         }
      }

      //Process all pending events
      while (XPending(Xdisplay))
      {
         // Get next event
         XNextEvent(Xdisplay, &event);

         //Save shift states
         keyState.shift = (event.xkey.state & ShiftMask);
         keyState.ctrl = (event.xkey.state & ControlMask);
#ifdef __APPLE__
         keyState.alt = (event.xkey.state & Mod1Mask | event.xkey.state & Mod2Mask);
#else
         keyState.alt = (event.xkey.state & Mod1Mask);
#endif

         switch (event.type)
         {
         case ButtonRelease:
            mouseState = 0;
            button = (MouseButton)event.xbutton.button;
            mousePress(button, false, event.xmotion.x, event.xmotion.y);
            redisplay = true;
            break;
         case ButtonPress:
            // XOR state of three mouse buttons to the mouseState variable  
            button = (MouseButton)event.xbutton.button;
            if (button <= RightButton) mouseState ^= (int)pow(2, button);
            mousePress(button, true, event.xmotion.x, event.xmotion.y);
            break;
         case MotionNotify:
            if (mouseState)
            {
               mouseMove(event.xmotion.x, event.xmotion.y);
               redisplay = true;
            }
            break;
         case KeyPress:
            // Convert to cross-platform codes for event handler
            count = XLookupString((XKeyEvent *)&event, buf, 64, &keysym, NULL);
            if (count) key = buf[0];
            else if (keysym == XK_KP_Up || keysym == XK_Up) key = KEY_UP;
            else if (keysym == XK_KP_Down || keysym == XK_Down) key = KEY_DOWN;
            else if (keysym == XK_KP_Left || keysym == XK_Left) key = KEY_LEFT;
            else if (keysym == XK_KP_Right || keysym == XK_Right) key = KEY_RIGHT;
            else if (keysym == XK_KP_Page_Up || keysym == XK_Page_Up) key = KEY_PAGEUP;
            else if (keysym == XK_KP_Page_Down || keysym == XK_Page_Down) key = KEY_PAGEDOWN;
            else if (keysym == XK_KP_Home || keysym == XK_Home) key = KEY_HOME;
            else if (keysym == XK_KP_End || keysym == XK_End) key = KEY_END;
            else key = keysym;

            if (keyPress(key, event.xkey.x, event.xkey.y))
               redisplay = true;

            break;
         case ClientMessage:
            if (event.xclient.data.l[0] == (long)wmDeleteWindow)
               quitProgram = true;
            break;
         case MapNotify:
            // Window shown 
            if (hidden) redisplay = true;
            hidden = false;
            break;
         case UnmapNotify:
            // Window hidden, iconized or switched to another desktop
            hidden = true;
            redisplay = false;
            break;
         case ConfigureNotify:
         {
            // Notification of window actions, including resize 
            resize(event.xconfigure.width, event.xconfigure.height);
            break;
         }
         case Expose:
            if (!hidden) redisplay = true;
            break;
         default:
            redisplay = false;
         }
      }

      //Redisplay if required.®.
      if (redisplay)
      {
          // Redraw Viewer (Call virtual to display)
         display();
         redisplay = false;
      }
   }
}

void X11Viewer::fullScreen()
{
   XEvent xev;
   Atom wm_state = XInternAtom(Xdisplay, "_NET_WM_STATE", False);
   Atom full_screen = XInternAtom(Xdisplay, "_NET_WM_STATE_FULLSCREEN", False);

   memset(&xev, 0, sizeof(xev));
   xev.type = ClientMessage;
   xev.xclient.window = win;
   xev.xclient.message_type = wm_state;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = fullscreen ? 0 : 1;
   xev.xclient.data.l[1] = full_screen;
   xev.xclient.data.l[2] = 0;

   XSendEvent(Xdisplay, DefaultRootWindow(Xdisplay), False, SubstructureNotifyMask, &xev);

   fullscreen = fullscreen ? 0 : 1;
}

bool X11Viewer::chooseVisual()
{
   int i, count = 7;
   static int configuration[] = { GLX_STEREO, GLX_SAMPLE_BUFFERS, 1, GLX_SAMPLES, 4,
                                  GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, GLX_DEPTH_SIZE, 8,
                                  GLX_ALPHA_SIZE, 8, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
                                  GLX_RGBA, None
                                 };
   const char* configStrings[] = {"Stereo", "4 x MultisampleSample", "Double-buffered", "Depth", "Stencil", "Alpha", "RGB"};
   int configs[] = {0, 1, 5, 6, 8, 10, 12};

   // find an OpenGL-capable display - trying different configurations if nessesary 
   // Note: only attempt to get stereo and double buffered visuals when in interactive mode 
   vi = NULL;
   for (i = stereo ? 0 : 1; i < count; i += 1)
   {
      int j;
      debug_print("Attempting config %d: (", i+1);
      for (j = i; j < 7; j++)
         debug_print("%s%s", (j==i) ? "" : ", ", configStrings[j]);
      debug_print(") ");

      vi = glXChooseVisual(Xdisplay, DefaultScreen(Xdisplay), &configuration[configs[i]]);
      if (vi)
      { 
         debug_print("Success!\n");
         break; // Success? 
      }
      debug_print("Failed\n");
   }

   if (vi == NULL)
   {
      fprintf(stderr,  "In func %s: Couldn't open display\n", __func__);
      return false;
   }
   return true;
}

bool X11Viewer::createWindow(int width, int height)
{
   Colormap             cmap;
   XSetWindowAttributes swa;

   // Create Colourmap 
   cmap = XCreateColormap(Xdisplay, RootWindow(Xdisplay, vi->screen), vi->visual, AllocNone);
   swa.colormap = cmap;
   swa.border_pixel = 0;
   swa.background_pixel = 0;
   swa.event_mask = ExposureMask | StructureNotifyMask | ButtonReleaseMask | ButtonPressMask | ButtonMotionMask | KeyPressMask;

   if (sHints && wmHints)
   {
      sHints->min_width = 32;
      sHints->min_height = 32;
      sHints->max_height = 4096;
      sHints->max_width = 4096;

      sHints->flags = PMaxSize | PMinSize | USPosition;
      // Center 
      sHints->x = (DisplayWidth(Xdisplay, vi->screen) - width) / 2;
      sHints->y = (DisplayHeight(Xdisplay, vi->screen) - height) / 2;


      // Create X window 
      win = XCreateWindow(Xdisplay, RootWindow(Xdisplay, vi->screen),
                          sHints->x, sHints->y, width, height,
                          0, vi->depth, InputOutput, vi->visual,
                          CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &swa);

      wmHints->initial_state = NormalState;
      wmHints->flags = StateHint;

      wmDeleteWindow = XInternAtom(Xdisplay, "WM_DELETE_WINDOW", 1);
      XSetWMProtocols(Xdisplay, win, &wmDeleteWindow, 1);

      // Create an OpenGL rendering context
      glxcontext = glXCreateContext(Xdisplay, vi, 0, 1);
      if (!glxcontext) // Failed? Try an indirect context 
         glxcontext = glXCreateContext(Xdisplay, vi, 0, 0);

      if (glxcontext)
      {
         if (glXIsDirect(Xdisplay, glxcontext))
            debug_print("GLX: Direct rendering enabled.\n");
         else
            debug_print("GLX: No direct rendering context available, using indirect.\n");

         //if (visible)
         //   XMapRaised( Xdisplay, win ); // Show the window 

         glXMakeCurrent(Xdisplay, win, glxcontext);

         //XFlush(Xdisplay);  // Flush output buffer 
      }
      else
         fprintf(stderr, "In func %s: Could not create GLX rendering context.\n", __func__);
   }
   else
      return false;


   if (!glxcontext) abort_program("No context!\n");
   if (!glxcontext) return false;
   return true;
}

#endif


