/********************************************************************************************************************** 
 * LavaVu + OmegaLib
 *********************************************************************************************************************/
#ifdef USE_OMEGALIB

#include <omega.h>
#include <omegaGl.h>
#include <omegaToolkit.h>
#include "../src/ViewerApp.h"
#include "../src/LavaVu.h"
#include "../src/Server.h"
#include <dirent.h>

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

class LavaVuApplication;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LavaVuRenderPass: public RenderPass, ViewerApp
{
public:
  LavaVuRenderPass(Renderer* client, LavaVuApplication* app, OpenGLViewer* viewer): RenderPass(client, "LavaVuRenderPass"), app(app), ViewerApp(viewer) {}
  virtual void initialize();
  virtual void render(Renderer* client, const DrawContext& context);

   // Virtual functions for interactivity (from ViewerApp/ApplicationInterface)
   virtual bool mouseMove(int x, int y) {}
   virtual bool mousePress(MouseButton btn, bool down, int x, int y) {}
   virtual bool mouseScroll(int scroll) {}
   virtual bool keyPress(unsigned char key, int x, int y) {}

private:
  LavaVuApplication* app;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LavaVuApplication: public EngineModule
{
public:
  OpenGLViewer* viewer;
  LavaVu* glapp;
  bool redisplay;
  int argc;
  char** argv;
  //Copy of commands
  std::deque<std::string> commands;
  //Widgets
  Ref<Label> statusLabel;
  Ref<Label> titleLabel;

  LavaVuApplication(): EngineModule("LavaVuApplication") { redisplay = true; enableSharedData(); menuOpen = false;}

    virtual void initialize()
    {
      //Create a label for text info
      DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
      // Create and initialize the UI management module.
      myUiModule = UiModule::createAndInitialize();
      myUi = myUiModule->getUi();

      int sz = 100;
      statusLabel = Label::create(myUi);
      statusLabel->setText("");
      statusLabel->setColor(Color::Gray);
      statusLabel->setFont(ostr("fonts/arial.ttf %1%", %sz));
      statusLabel->setHorizontalAlign(Label::AlignLeft);
      statusLabel->setPosition(Vector2f(100,300));

      sz = 150;
      titleLabel = Label::create(myUi);
      titleLabel->setText("");
      titleLabel->setColor(Color::Gray);
      titleLabel->setFont(ostr("fonts/arial.ttf %1%", %sz));
      titleLabel->setHorizontalAlign(Label::AlignLeft);
      titleLabel->setPosition(Vector2f(100,100));

    }

  virtual void initializeRenderer(Renderer* r) 
  { 
    viewer = new OpenGLViewer(false, false);
    r->addRenderPass(new LavaVuRenderPass(r, this, viewer));
  }

  float setArgs(int argc, char** argv) {this->argc = argc; this->argv = argv;}

  virtual void handleEvent(const Event& evt);
  virtual void cameraSetup(bool init=false);
  virtual void commitSharedData(SharedOStream& out);
  virtual void updateSharedData(SharedIStream& in);

private:
  // The ui manager
  Ref<UiModule> myUiModule;
  // The root ui container
  Ref<Container> myUi;  
  std::string labelText;
  bool menuOpen;
  Vector3f lastpos;
  Vector4f lastrot;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LavaVuRenderPass::initialize()
{
  RenderPass::initialize();

  //Init fractal app
  std::vector<std::string> arglist;

  //Get the executable path
  std::string expath = GetBinaryPath("LavaVR", "LavaVR");

   //Add any output attachments to the viewer
#ifndef DISABLE_SERVER
   SystemManager* sys = SystemManager::instance();
   if (sys->isMaster())
      //Quality = 0, don't serve images
      viewer->addOutput(Server::Instance(viewer, expath + "../bin/html", 8080, 0, 4));
#endif

  // Initialize the omegaToolkit python API
  omegaToolkitPythonApiInit();

  //Attempt to run script in cwd then in executable path
  bool found = false;
  PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();
  std::string filename = "init.py";
  std::ifstream infile(filename.c_str());
  if (infile.good())
  {
     infile.close();
     found = true;
  }
  else
  {
    filename = expath + filename;
    std::ifstream infile(filename.c_str());
    if (infile.good())
    {
      infile.close();
      found = true;
    }
  } 

  if (found)
  {
    // Run the system menu script
    pi->runFile(filename, 0);
    // Call the function from the script that will setup the menu.
    pi->eval("_onAppStart('" + expath + "')");
  }

  //Create the app
  app->glapp = new LavaVu(arglist, viewer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LavaVuRenderPass::render(Renderer* client, const DrawContext& context)
{
  if (context.task == DrawContext::SceneDrawTask)
  {
    client->getRenderer()->beginDraw3D(context);

    if (!viewer->isopen)
    {
      //Load vis data for first window
      FilePath init("init.script");
      app->glapp->loadFile(init);
      app->glapp->cacheLoad();
      app->glapp->loadWindow(0, -1, true);

      //Add menu items to hide/show all objects
      Model* amodel = app->glapp->amodel;
      PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();
      for (unsigned int i=0; i < amodel->objects.size(); i++)
      {
         if (amodel->objects[i])
         {
            std::ostringstream ss;
            ss << std::setw(5) << amodel->objects[i]->id << " : " << amodel->objects[i]->name;
            if (!amodel->objects[i]->skip)
            {
               //pi->eval("_addMenuItem('" + amodel->objects[i]->name + "', 'toggle " + amodel->objects[i]->name + "')");
               pi->eval("_addObjectMenuItem('" + amodel->objects[i]->name + (amodel->objects[i]->visible ? "', True)" : "', False)"));
               //std::cerr << "ADDING " << amodel->objects[i]->name << std::endl;
            }
         }
      }

      //Add scripts menu
      DIR *dir;
      struct dirent *ent;
      if ((dir = opendir(".")) != NULL)
      {
         while ((ent = readdir(dir)) != NULL)
         {
            FilePath fe = FilePath(ent->d_name);
            if (fe.type == "script" && fe.base != "init")
            {
               printf ("%s\n", fe.full.c_str());
               pi->eval("_addFileMenuItem('" + fe.full + "')");
            }
         }
         closedir (dir);
      }

      if (context.tile->isInGrid)
      {
        //app->master = glapp; //Copy to master ref
        if (context.tile->gridX > 0 || context.tile->gridY > 0)
           app->glapp->quiet = true;  //Disable text output
      }

      DisplaySystem* ds = app->getEngine()->getDisplaySystem();
      Colour& bg = viewer->background;
      ds->setBackgroundColor(Color(bg.rgba[0]/255.0, bg.rgba[1]/255.0, bg.rgba[2]/255.0, 0));
      //Omegalib 5.1+
      Camera* cam = Engine::instance()->getDefaultCamera();
      //cam->setBackgroundColor(Color(bg.rgba[0]/255.0, bg.rgba[1]/255.0, bg.rgba[2]/255.0, 0));

      viewer->open(context.tile->pixelSize[0], context.tile->pixelSize[1]);
      viewer->init();

      //Transfer LavaVu camera settings to Omegalib
      app->cameraSetup(true);

      //Default nav speed
      float navSpeed = Geometry::properties["navspeed"].ToFloat(0);
      CameraController* cc = cam->getController();
      View* view = app->glapp->aview;
      //cc->setSpeed(view->model_size * 0.03);
      float rotate[4], translate[3], focus[3];
      view->getCamera(rotate, translate, focus);
      if (navSpeed <= 0.0) navSpeed = abs(translate[2]) * 0.05;
      cc->setSpeed(navSpeed);

    }

    //Copy commands before consumed
    app->commands = OpenGLViewer::commands;

    //Update status label
    if (app->statusLabel->getText() != app->glapp->message)
    {
       //std::cerr << statusLabel->getText() << " != " << glapp->message << std::endl;
       app->statusLabel->setAlpha(0.5);
       app->statusLabel->setText(app->glapp->message);
    }
    //Update title label
    if (app->glapp->viewer->title.length() > 0)
    {
       std::string titleText = app->glapp->viewer->title + " " + app->glapp->aview->title;
       if (app->titleLabel->getText() != titleText)
          app->titleLabel->setText(app->glapp->viewer->title);
    }

    //Fade out status label (doesn't seem to work in cave)
    float alpha = app->statusLabel->getAlpha();
    if (alpha < 0.01)
    {
       app->statusLabel->setText("");
       app->glapp->message[0] = '\0';
    }
    else
       app->statusLabel->setAlpha(alpha * 0.95);
     
    //if (app->redisplay)
    {
      //Apply the model rotation/scaling
      View* view = app->glapp->aview;   
      //view->apply();
      view->apply(false); //Fixes vol-rend rotate origin issue but breaks connectome initial pos
         
      glEnable(GL_BLEND);
      viewer->display();
      //app->redisplay = false;
    }

    client->getRenderer()->endDraw();

    //Process timer based commands
    OpenGLViewer::pollInput();
  }
  else if(context.task == DrawContext::OverlayDrawTask)
  {
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LavaVuApplication::cameraSetup(bool init)
{
   //Setup camera using omegalib functions
   Camera* cam = Engine::instance()->getDefaultCamera();
   View* view = glapp->aview;
   float rotate[4], translate[3], focus[3];
   Vector3f oldpos = cam->getPosition();
   view->getCamera(rotate, translate, focus);

   //Set position from translate
   Vector3f newpos = Vector3f(translate[0], translate[1], -translate[2]);
      if (!init && newpos == oldpos)
      {
         newpos = lastpos; //cam->setPosition(lastpos);
         //view->setTranslation(lastpos[0], lastpos[1], -lastpos[2]);
         view->setRotation(lastrot[0], lastrot[1], lastrot[2], lastrot[3]);
         view->print();
         //return;
      }
      else
      {
         if (!init) view->setRotation(0, 0, 0, 1);
         lastpos = oldpos;
         lastrot = Vector4f(rotate[0], rotate[1], rotate[2], rotate[3]);
      }

   //view->reset();
   //view->setRotation(0, 0, 0, 1);

   cam->setPosition(newpos);
   //From viewing distance
   //cam->setPosition(Vector3f(focus[0], focus[1], (focus[2] - view->model_size)) * view->orientation);
   //At center
   //cam->setPosition(Vector3f(focus[0], focus[1], focus[2] * view->orientation));

   //Default eye separation, TODO: set this via LavaVu property controllable via init.script
   cam->setEyeSeparation(view->eye_sep_ratio);

   ///Setting clip planes can kill menu! Need to check using MenuManager::getDefaultMenuDistance()
   //MenuManager* mm = MenuManager::createAndInitialize();
   //float menuDist = mm->getDefaultMenuDistance();
   //Menu* main = mm->getMainMenu(); //float menuDist = mm->getDefaultMenuDistance();
   //main->addButton("SetCamera", "getDefaultCamera().setPosition(Vector3(0, 0, 0))"));

   //cam->setNearFarZ(view->near_clip*0.01, view->far_clip);
   //NOTE: Setting near clip too close is bad for eyes, too far kills negative parallax stereo
   //TODO: Make sure this can be controlled via script
   cam->setNearFarZ(view->near_clip, view->far_clip);
   //cam->setNearFarZ(view->near_clip*0.1, view->far_clip);
   //cam->setNearFarZ(view->near_clip*0.01, view->far_clip);
   //cam->setNearFarZ(cam->getNearZ(), view->far_clip);

   cam->lookAt(Vector3f(focus[0], focus[1], focus[2] * view->orientation), Vector3f(0,1,0));
   cam->setPitchYawRoll(Vector3f(0, 0, 0));
}

void LavaVuApplication::handleEvent(const Event& evt)
{
  //printf(". %d %d %d\n", evt.getType(), evt.getServiceType(), evt.getFlags());
  if(evt.getServiceType() == Service::Pointer)
  {
    int x = evt.getPosition().x();
    int y = evt.getPosition().y();
    int flags = evt.getFlags();
    MouseButton button = NoButton;
    if (flags & 1)
      button = LeftButton;
    else if (flags & 2)
      button = RightButton;
    else if (flags & 4)
      button = MiddleButton;

    switch (evt.getType())
    {
      case Event::Down:
        //printf("%d %d\n", button, flags);
        if (button <= RightButton) viewer->mouseState ^= (int)pow(2, (int)button);
        viewer->mousePress(button, true, x, y);
          redisplay = true;
          break;
      case Event::Up:
        viewer->mouseState = 0;
        viewer->mousePress(button, false, x, y);
          break;
      case Event::Zoom:
        viewer->mouseScroll(evt.getExtraDataInt(0));
         break;
      case Event::Move:
         if (viewer->mouseState)
         {
            viewer->mouseMove(x, y);
            //redisplay = true;
         }
          break;
      default:
        printf("? %d\n", evt.getType());
    }
  }
  else if(evt.getServiceType() == Service::Keyboard)
  {
    int x = evt.getPosition().x();
    int y = evt.getPosition().y();
    int key = evt.getSourceId();
    if (evt.isKeyDown(key))
    {
      if (key > 255)
      {
      //printf("Key %d %d\n", key, evt.getFlags());
        if (key == 262) key = KEY_UP;
        else if (key == 264) key = KEY_DOWN;
        else if (key == 261) key = KEY_LEFT;
        else if (key == 263) key = KEY_RIGHT;
        else if (key == 265) key = KEY_PAGEUP;
        else if (key == 266) key = KEY_PAGEDOWN;
        else if (key == 260) key = KEY_HOME;
        else if (key == 267) key = KEY_END;
      }
      //viewer->keyPress(key, x, y);
    }
  }
  else if(evt.getServiceType() == Service::Wand)
  {
    int x = evt.getPosition().x();
    int y = evt.getPosition().y();
    int key = evt.getSourceId();
    if (evt.isButtonDown(Event::Button2)) //Circle
    {
       menuOpen = true;
       printf("Menu opened\n");
    }
    else if (evt.isButtonDown(Event::Button3)) //Cross
    {
       if (menuOpen)
       {
           menuOpen = false;
           printf("Menu closed\n");
       }
       else
       {
           //Restore camera
           cameraSetup();
       }
    }
    else if (evt.isButtonDown(Event::Button7))
    {
       //L2 Trigger (large)
      // std::cout << "L2 Trigger " << std::endl;
      if (evt.isButtonDown(Event::ButtonLeft ))
      {
        glapp->parseCommands("zoomclip -0.01");
        Camera* cam = Engine::instance()->getDefaultCamera();
        cam->setNearFarZ(glapp->aview->near_clip*0.1, glapp->aview->far_clip);
        evt.setProcessed();
      }
      else if (evt.isButtonDown(Event::ButtonRight ))
      {

        glapp->parseCommands("zoomclip 0.01");
        Camera* cam = Engine::instance()->getDefaultCamera();
        cam->setNearFarZ(glapp->aview->near_clip*0.1, glapp->aview->far_clip);
        evt.setProcessed();
      }
      else if (evt.isButtonDown(Event::ButtonUp))
      {
         //evt.setProcessed();
      }
      else if (evt.isButtonDown(Event::ButtonDown))
      {
         //evt.setProcessed();
      }
      if (evt.isButtonDown(Event::Button5))
      {
         //Depth sort geometry
         glapp->aview->sort = true;
      }

    }
    else if (evt.isButtonDown(Event::Button5))
    {
      //L1 Trigger (small) - Multi-press to fine tune
      if (evt.isButtonDown(Event::ButtonLeft ))
      {
         glapp->parseCommands("scale all 0.95");
         evt.setProcessed();
      }
      else if (evt.isButtonDown(Event::ButtonRight ))
      {
         glapp->parseCommands("scale all 1.05");
         evt.setProcessed();
      }
      else if (evt.isButtonDown(Event::ButtonUp))
      {
         //Reduce eye separation
         Camera* cam = Engine::instance()->getDefaultCamera();
         cam->setEyeSeparation(cam->getEyeSeparation()-0.01);
         printf("Eye-separation set to %f\n", cam->getEyeSeparation());
         evt.setProcessed();
      }
      else if (evt.isButtonDown(Event::ButtonDown))
      {
         //Increase eye separation
         Camera* cam = Engine::instance()->getDefaultCamera();
         cam->setEyeSeparation(cam->getEyeSeparation()+0.01);
         printf("Eye-separation set to %f\n", cam->getEyeSeparation());
         evt.setProcessed();
      }
      else
      {
         //Depth sort geometry
         //glapp->aview->sort = true;
      }
    }
    else if (evt.isButtonDown(Event::ButtonUp ))
    {
        //if (GeomData::opacity > 0.0) GeomData::opacity -= 0.05;
        //glapp->redrawViewports();
    }
    else if (evt.isButtonDown(Event::ButtonDown ))
    {
        //if (GeomData::opacity < 1.0) GeomData::opacity += 0.05;
        //glapp->redrawViewports();
        glapp->parseCommands("timestep 0");
    }
    else if (evt.isButtonDown(Event::ButtonLeft ))
    {
        glapp->parseCommands("model up");
    }
    else if (evt.isButtonDown( Event::ButtonRight ))
    {
        glapp->parseCommands("model down");
    }
    //else
    {
        //Grab the analog stick horizontal axis
        float analogLR = evt.getAxis(0);
        //Grab the analog stick vertical axis
        float analogUD = evt.getAxis(1);
        if (abs(analogUD) + abs(analogLR) > 0.001)
        {
           //TODO: default is model rotate, enable timestep sweep mode via menu option
           bool rotateStick = Geometry::properties["sweep"].ToInt(0) == 0;
           if (rotateStick)
           {
               //L2 Trigger (large)
               if (abs(analogUD) > 0.01)
               {
                  std::stringstream rcmd;
                  rcmd << "rotate x " << analogUD;
                  glapp->parseCommands(rcmd.str());
               }
               if (abs(analogLR) > 0.01)
               {
                  std::stringstream rcmd;
                  rcmd << "rotate y " << analogLR;
                  glapp->parseCommands(rcmd.str());
               }
               evt.setProcessed();
            }
            else if (abs(analogUD) > abs(analogLR))
            {
               if (analogUD > 0.01)
                 glapp->parseCommands("timestep down");
               else if (analogUD < 0.01)
                 glapp->parseCommands("timestep up");
               evt.setProcessed();
            }
        }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LavaVuApplication::commitSharedData(SharedOStream& out)
{
   std::stringstream oss;
   for (int i=0; i < commands.size(); i++)
      oss << commands[i] << std::endl;
   out << oss.str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LavaVuApplication::updateSharedData(SharedIStream& in)
{
   std::string commandstr;
   in >> commandstr;

   SystemManager* sys = SystemManager::instance();
   if (!sys->isMaster())
   {
      OpenGLViewer::commands.clear();
      std::stringstream iss(commandstr);
      std::string line;
      while(std::getline(iss, line))
      {
         OpenGLViewer::commands.push_back(line);
         //glapp->parseCommands(line);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ApplicationBase entry point
int main(int argc, char** argv)
{
  Application<LavaVuApplication> app("LavaVR");
  return omain(app, argc, argv);
}

#endif

