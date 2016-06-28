/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module LavaVu
%include <std_string.i>
%include <std_vector.i>

%{
#include "../gLucifer/Viewer/src/LavaVu.h"
%}

%include "exception.i"
%exception {
    try {
        $action
    } catch (const std::runtime_error& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

namespace std {
%template(Line)  vector <float>;
%template(Array) vector < vector <float> >;
%template(List) vector <string>;
}

%pythoncode %{
#Helper functions
def load(app=None, arglist=[], binary="LavaVu", database=None, figure=None, startstep=None, endstep=None, 
         port=0, verbose=False, interactive=False, hidden=True, quality=2, writeimage=False, res=None, script=None):
    args = [] + arglist
    #Convert options to args
    if verbose:
      args += ["-v"]
    #Automation: scripted mode, no interaction
    if not interactive:
      args += ["-a"]
    #Hidden window
    if hidden:
      args += ["-h"]
    #Subsample anti-aliasing for image output
    args += ["-z" + str(quality)]
    #Timestep range
    if startstep != None:
      args += ["-" + str(startstep)]
      if endstep != None:
        args += ["-" + str(endstep)]
    #Web server
    args += ["-p" + str(port)]
    #Database file
    if database:
      args += [database]
    #Initial figure
    if figure != None:
      args += ["-f" + str(figure)]
    #Output resolution
    if res != None and isinstance(res,tuple):
      args += ["-x" + str(res[0]) + "," + str(res[1])]
    #Save image and quit
    if writeimage:
      args += ["-I"]
    if script and isinstance(script,list):
      args += script

    if not app:
      app = LavaVu(binary)
    app.run(args)
    return app
%}

class LavaVu
{
public:
  Model* amodel;
  View* aview;
  DrawingObject* aobject;

  LavaVu(std::string binary="");
  ~LavaVu();

  void run(std::vector<std::string> args={});

  bool parseCommands(std::string cmd);
  std::string image(std::string filename="", int width=0, int height=0);
  std::string web(bool tofile=false);
  void addObject(std::string name, std::string properties);
  void setState(std::string state);
  std::string getStates();
  std::string getTimeSteps();
  void vertices(std::vector< std::vector <float> > array);
  void values(std::vector <float> array);
  void close();

%pythoncode %{
  def commands(self, cmds):
      self.parseCommands(cmds)

  def add(self, name="(unnamed)", props={}):
      if self.amodel:
          propstr = '\n'.join(['%s=%s' % (k,v) for k,v in props.iteritems()])
          self.addObject(name, propstr)

  def clear(self):
      self.close()

%}
};

