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

#include "Win.h"

unsigned int Win::lastid = 0;

//Adds a view to active window
View* Win::addView(View* v)
{
   views.push_back(v);
   //debug_print("Adding view %s at %f,%f", titlestr, xf, yf);
   //debug_print("Added %f,%f\n", aview->x, aview->y);
   //Re-calculate viewports ( x,y width,height )
   resizeViewports();
   return v;
}

void Win::initViewports(int w, int h)
{
   //Initialise all viewports to window size
   for (unsigned int v=0; v<views.size(); v++)
   {
      //Calculate actual size based on window size
      views[v]->port(w, h);
      //debug_print("Viewport %d/%d at %d,%d %d x %d\n", v+1, views.size(), (int)(width * views[v]->x), (int)(height * views[v]->y), (int)(width * views[v]->w), (int)(height * views[v]->h));
   }
}

void Win::resizeViewports()
{
   //Calculate viewports ( x,y width,height )
   for (unsigned int v=0; v<views.size(); v++)
   {
      float nextx = 1.0, nexty = 1.0;
   
      //Find next largest x & y positions
      if (v+1 < views.size() && views[v+1]->x > views[v]->x)
         nextx = views[v+1]->x;

      for (unsigned int vv=v+1; vv<views.size(); vv++)
      {
         if (views[vv]->y > views[v]->y && views[vv]->y < nexty)
            nexty = views[vv]->y;
      }

      views[v]->w = nextx - views[v]->x;
      views[v]->h = nexty - views[v]->y;
      //debug_print("### Sized viewport %d at %d,%d %d x %d\n", v, (int)(width * views[v]->x), (int)(height * views[v]->y), (int)(width * views[v]->w), (int)(height * views[v]->h));
   }
}
