#ifdef HAVE_SAGE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SageOutput.h"

SageOutput* SageOutput::_self = NULL; //Static
bool SageOutput::doUpdatelabel = false;
int SageOutput::frameNum = 0;
sail* SageOutput::sageInf; // sail object
#ifdef SHOW_WIDGETS
// some global widgets
appWidget* SageOutput::mm = NULL;
sizer* SageOutput::s1 = NULL;
appWidget* SageOutput::ll = NULL;
#endif

SageOutput* SageOutput::Instance(OpenGLViewer* viewer)
{
   if (!_self)   // Only allow one instance of class to be generated.
      _self = new SageOutput(viewer);

   return _self;
}

SageOutput::SageOutput(OpenGLViewer* viewer) : viewer(viewer)
{
}

SageOutput::~SageOutput()
{
   deleteSAIL(sageInf);
}

// virtual functions for window management
void SageOutput::open(int width, int height)
{
   sageInf = createSAIL("gLucifer", viewer->width, viewer->height, PIXFMT_888, NULL);

#ifdef SHOW_WIDGETS
   // add ui widgets
   char l[256];
   sprintf(l, "Frame# %d", frameNum);
   label *lbl1 = new label;
   lbl1->setSize(150, 20);
   lbl1->alignLabel(LEFT);
   lbl1->setLabel(l, 10);
   lbl1->align(CENTER, BOTTOM_OUTSIDE);
   ll = lbl1;

   thumbnail *pauseBtn = new thumbnail;
   pauseBtn->setSize(50,50);
   pauseBtn->alignY(BOTTOM, 2);
   pauseBtn->setY(2);
   pauseBtn->setUpImage("images/pause2.png");
   pauseBtn->setDownImage("images/play2.png");
   pauseBtn->setToggle(true);
   pauseBtn->onDown(&playpause);
   pauseBtn->onUp(&playpause);
   pauseBtn->setTransparency(150);

   button *btn1 = new button;
   btn1->alignLabel(LEFT);
   btn1->setSize(100,20);
   btn1->setLabel("Quit (app button)", 10);
   btn1->onUp(&quit);
   btn1->setFontColor(255,255,0);
   mm = btn1;

   menu *m = new menu;
   m->setLabel("Some options", 10);
       m->alignY(TOP,2);
   m->addMenuItem("Update label", &onMenuItem);
   m->addMenuItem("Dont update label", &onMenuItem);
   m->addMenuItem("Add widget", &onMenuItem);
   m->addMenuItem("Remove widget", &onMenuItem);
   m->setUpImage("images/default_button_up.png");
   m->setFontColor(0, 255, 0);
   m->setTransparency(200);

   // sizer 1
   s1 = new sizer(VERTICAL);
   s1->addChild(btn1);
   s1->addChild(ll);
   s1->alignY(TOP, 5);

   panel *p = new panel(55,55,55);
   p->alignY(TOP,5);
   p->addChild(s1);

   // sizer 2
   sizer *s2 = new sizer(HORIZONTAL);
   s2->addChild(pauseBtn);
   s2->addChild(m);
   s2->alignY(BOTTOM,5);

   // main sizer
   sizer *mainSizer = new sizer(HORIZONTAL);
   mainSizer->addChild(p);
   mainSizer->addChild(s2);
   mainSizer->align(CENTER, CENTER, 5,5);

   // bottom panel
   panel *bottomPanel = new panel(37,37,37);
   bottomPanel->align(CENTER, BOTTOM_OUTSIDE,0,2);
   bottomPanel->setSize(viewer->width, 100);
   bottomPanel->fitInWidth(false);
   bottomPanel->setTransparency(150);
   bottomPanel->addChild(mainSizer);
   sageInf->addWidget(bottomPanel);
#endif

   std::cout << "sail initialized " << std::endl;
   viewer->notIdle(100); //Start idle redisplay timer
}

void SageOutput::resize(int new_width, int new_height)
{
}

void SageOutput::display()
{
#ifdef SHOW_WIDGETS
   frameNum++;
   if (frameNum % 10 == 0)
   {
      // update every 10th frame
      char l[256];
      sprintf(l, "Frame# %d", frameNum);
      if (ll != NULL && doUpdatelabel) 
         ll->setLabel(l, 10);
   }
#endif
    
   if (viewer->width > 0)
   {
      GLubyte *rgbBuffer = nextBuffer(sageInf);
      viewer->pixels(rgbBuffer, false, false);
      swapBuffer(sageInf);
   }
}

void SageOutput::close()
{
}

void SageOutput::idle()
{
   // this also allows app widget callbacks to be performed
   processMessages(sageInf,NULL,NULL,NULL);
}

#ifdef SHOW_WIDGETS
void SageOutput::onMenuItem(menuItem * mi)
{
   switch (mi->getItemId())
   {
   case 0:
      doUpdatelabel = true;
      break;
   case 1:
      doUpdatelabel = false;
      break;
   case 2:
   {
      char l[256];
      sprintf(l, "Frame# %d", frameNum);
      label *lbl = new label; 
      lbl->setSize(150, 20);
      lbl->setLabel(l, 10);
      lbl->alignLabel(RIGHT);
      lbl->alignX(CENTER);
      s1->addChild(lbl);
      ll = lbl;
      break;
   }
   case 3:
      sageInf->removeWidget(ll);
      ll = NULL;
      break;
   }
}

void SageOutput::removeMenu(int eventId, button * btnObj)
{
   sageInf->removeWidget(mm);
}

void SageOutput::playpause(int eventId, button * btnObj)
{
   static bool moving = true;
   if (moving)
   {
      moving = false;
      _self->viewer->notIdle(0); //Stop idle redisplay timer
   }
   else
   {
      moving = true;
      _self->viewer->notIdle(100); //Start idle redisplay timer
   }
}

void SageOutput::quit(int eventId, button * btnObj)
{
   deleteSAIL(sageInf);
   exit(0);
}
#endif

#endif  //HAVE_SAGE
