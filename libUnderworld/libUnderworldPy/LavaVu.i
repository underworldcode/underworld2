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

%typemap(in) (int argc, char **argv) {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int i;
    $1 = PyList_Size($input);
    $2 = (char **) malloc(($1+1)*sizeof(char *));
    for (i = 0; i < $1; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
        $2[i] = PyString_AsString(PyList_GetItem($input,i));
      else {
        PyErr_SetString(PyExc_TypeError,"list must contain strings");
        free($2);
        return NULL;
      }
    }
    $2[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

%typemap(freearg) (int argc, char **argv) {
  free((char *) $2);
}

namespace std {
%template(Line)  vector <float>;
%template(Array) vector < vector <float> >;
}

%pythoncode %{
#Helper functions
app = None
def load(args=["LavaVu", "-a"]):
    global app
    if not app:
      app = LavaVu()
    execute(args, app)
    return app
%}

void execute(int argc, char **argv, LavaVu* app);
void execute(int argc, char **argv);

class LavaVu
{
public:
  Model* amodel;
  View* aview;
  DrawingObject* aobject;

  LavaVu();
  ~LavaVu();

  void run();

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

