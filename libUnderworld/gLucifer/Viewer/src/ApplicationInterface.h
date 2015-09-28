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

#ifndef ApplicationInterface__
#define ApplicationInterface__

//Mouse/keyboard input definitions
typedef enum {
   NoButton     = 0,
   LeftButton   = 1,
   MiddleButton = 2,
   RightButton  = 3,
   WheelUp      = 4,
   WheelDown    = 5
} MouseButton;

typedef struct {
   int shift;
   int ctrl;
   int alt;
   int meta;
} ShiftState;

class ApplicationInterface
{
  public:

   virtual void open(int width, int height) = 0;
   virtual void close() = 0;
   virtual void resize(int new_width, int new_height) = 0;
   virtual void setsize(int width, int height) {}
   virtual void display() = 0;
   virtual void swap() = 0;

   // Virtual functions for interactivity
   virtual bool mouseMove(int x, int y) = 0;
   virtual bool mousePress(MouseButton btn, bool down, int x, int y) = 0;
   virtual bool mouseScroll(int scroll) = 0;
   virtual bool keyPress(unsigned char key, int x, int y) = 0;

   virtual bool parseCommands(std::string cmd) {return false;}

   virtual std::string requestData(std::string key) {return std::string("");}

   ApplicationInterface() {}
};

//Some key codes, using ascii range
#define KEY_BACKSPACE 8
#define KEY_TAB 9
#define KEY_ENTER 13
#define KEY_ESC 27
#define KEY_DELETE 127
#define KEY_PRINTSCR 44
//Using unused ascii vals
#define KEY_UP 17
#define KEY_DOWN 18
#define KEY_RIGHT 19
#define KEY_LEFT 20
#define KEY_INSERT 21
#define KEY_HOME 22
#define KEY_END 23
#define KEY_PAGEUP 24
#define KEY_PAGEDOWN 25

#define KEY_F1 190
#define KEY_F2 191
#define KEY_F3 192
#define KEY_F4 193
#define KEY_F5 194
#define KEY_F6 195
#define KEY_F7 196
#define KEY_F8 197
#define KEY_F9 198
#define KEY_F10 199

#endif //ApplicationInterface__
