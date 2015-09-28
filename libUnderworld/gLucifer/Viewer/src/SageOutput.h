//Output for tiled displays (http://sagecommons.org)
#ifndef SageOutput__
#define SageOutput__

#ifdef HAVE_SAGE

#include "OutputInterface.h"
#include "OpenGLViewer.h"
#include "libsage.h"
#define SHOW_WIDGETS 1

class SageOutput : public OutputInterface
{
   //Singleton class, construct with SageOutput::Instance()
  protected:
   SageOutput(OpenGLViewer* viewer);

   static SageOutput* _self;

   //SAGE stuff
   static sail *sageInf; // sail object
#ifdef SHOW_WIDGETS
   // some global widgets
   static appWidget *mm;
   static sizer *s1;
   static appWidget *ll;
   static bool doUpdatelabel;
   static int frameNum;
#endif

   OpenGLViewer* viewer; 

  public:

   //Public instance constructor/getter
   static SageOutput* Instance(OpenGLViewer* viewer);
   virtual ~SageOutput();
   
   // virtual functions for window management
   virtual void open(int width, int height);
   virtual void resize(int new_width, int new_height);
   virtual void display();
   virtual void close();
   virtual void idle();

#ifdef SHOW_WIDGETS
   //Static callbacks
   static void onMenuItem(menuItem * mi);
   static void removeMenu(int eventId, button * btnObj);
   static void playpause(int eventId, button * btnObj);   
   static void quit(int eventId, button * btnObj);   
#endif
};

#endif //HAVE_SAGE
#endif //SageOutput__
