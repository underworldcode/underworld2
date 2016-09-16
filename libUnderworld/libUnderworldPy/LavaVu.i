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
#include "../gLucifer/Viewer/src/ViewerTypes.h"
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
%template(ULine) vector <unsigned int> ;
%template(Array) vector < vector <float> >;
%template(List) vector <string>;
}

%include "../gLucifer/Viewer/src/ViewerTypes.h"

%pythoncode %{
import json
#Helper functions
def load(app=None, arglist=[], binary="LavaVu", database=None, figure=None, startstep=None, endstep=None, 
         port=0, verbose=False, interactive=False, hidden=True, cache=True,
         quality=2, writeimage=False, res=None, script=None):
  #Convert options to args
  args = [] + arglist
  if verbose:
    args += ["-v"]
  #Automation: scripted mode, no interaction
  if not interactive:
    args += ["-a"]
  #Hidden window
  if hidden:
    args += ["-h"]
  #Timestep cache
  if cache:
    args += ["-c1"]
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

#Wrapper class for drawing object
#handles property updating via internal dict
class Obj():
  def __init__(self, idict, instance, *args, **kwargs):
    self.dict = idict
    self.instance = instance
    self.name = str(self.dict["name"])

  def get(self):
    #Retrieve updated props
    props = json.loads(self.instance.getObject(self.name))
    self.dict.clear()
    self.dict.update(props)
    self.name = str(self.dict["name"])

  def set(self):
    #Send updated props
    self.instance.setObject(self.name, json.dumps(self.dict))
    self.get()

  def __getitem__(self, key):
    self.get()
    return self.dict[key]

  def __setitem__(self, key, value):
    self.get()
    #Set new value and send
    self.dict[key] = value
    self.set()

  def __contains__(self, key):
    return key in self.dict

  def __str__(self):
    self.get()
    return str('\n'.join(['%s=%s' % (k,json.dumps(v)) for k,v in self.dict.iteritems()]))

  #Interface for setting filters
  def include(self, *args, **kwargs):
    return self.filter(*args, out=False, **kwargs)

  def includemap(self, *args, **kwargs):
    return self.filter(*args, out=False, map=True, **kwargs)

  def exclude(self, *args, **kwargs):
    return self.filter(*args, out=True, **kwargs)
      
  def excludemap(self, *args, **kwargs):
    return self.filter(*args, out=True, map=True, **kwargs)

  def filter(self, label, values, out=False, map=False):
    #Pass a tuple for exclusive range (min < val < max), list for inclusive range (min <= val <= max)
    filter = {"by" : label, "minimum" : values[0], "maximum" : values[1], "map" : map, "out" : out, "inclusive" : False}
    if isinstance(values, list):
      filter["inclusive"] = True
    self["filters"].append(filter)
    self.set()
    return len(self["filters"])-1

  def data(self):
    #Return json data set list
    return json.dumps(self.dict["data"])

  def vertices(self, data):
    self.instance.parseCommands("select " + self.name)
    self.instance.loadVectors(data, lucVertexData)

  def vectors(self, data):
    self.instance.parseCommands("select " + self.name)
    self.instance.loadVectors(data, lucVectorData)

  def values(self, data, type=lucColourValueData, label="", min=0., max=0.):
    self.instance.parseCommands("select " + self.name)
    self.instance.loadScalars(data, type, label, min, max)

  def colours(self, data):
    self.instance.parseCommands("select " + self.name)
    self.instance.parseCommands("colours=" + str(data))
    self.instance.parseCommands("read colours")

  def indices(self, data):
    self.instance.parseCommands("select " + self.name)
    self.instance.loadUnsigned(data, lucIndexData)

  def labels(self, data):
    self.instance.parseCommands("select " + self.name)
    self.instance.labels(data)

  #Property control interface
  #...TODO...

#Wrapper dict+list of objects
class Objects(dict):
  def __init__(self, instance):
    self.instance = instance
    pass

  def update(self):
    idx = 0
    self.list = []
    for obj in self.instance.state["objects"]:
      if obj["name"] in self:
        self[obj["name"]].get()
        self.list.append(self[obj["name"]])
      else:
        o = Obj(obj, self.instance)
        self[obj["name"]] = o
        self.list.append(o)

  def __str__(self):
    return '\n'.join(self.keys())

#Some preset colourmaps
# aim to reduce banding artifacts by being either 
# - isoluminant
# - smoothly increasing in luminance
# - diverging in luminance about centre value
colourMaps = {}
#Isoluminant blue-orange
colourMaps["isolum"] = "#288FD0 #50B6B8 #989878 #C68838 #FF7520"
#Diverging blue-yellow-orange
colourMaps["diverge"] = "#288FD0 #fbfb9f #FF7520"
#Isoluminant rainbow blue-green-orange
colourMaps["rainbow"] = "#5ed3ff #6fd6de #7ed7be #94d69f #b3d287 #d3ca7b #efc079 #ffb180"
#CubeLaw indigo-blue-green-yellow
colourMaps["cubelaw"] = "#440088 #831bb9 #578ee9 #3db6b6 #6ce64d #afeb56 #ffff88"
#CubeLaw indigo-blue-green-orange-yellow
colourMaps["cubelaw2"] = "#440088 #1b83b9 #6cc35b #ebbf56 #ffff88"
#CubeLaw heat blue-magenta-yellow)
colourMaps["smoothheat"] = "#440088 #831bb9 #c66f5d #ebbf56 #ffff88"
#Paraview cool-warm (diverging)
colourMaps["coolwarm"] = "#3b4cc0 #7396f5 #b0cbfc #dcdcdc #f6bfa5 #ea7b60 #b50b27"
#colourMaps["coolwarm"] = "#4860d1 #87a9fc #a7c5fd #dcdcdc #f2c8b4 #ee8669 #d95847"

%}

class LavaVu
{
public:
  Model* amodel;
  View* aview;
  DrawingObject* aobject;
  std::string binpath;

  LavaVu(std::string binary="");
  ~LavaVu();

  void run(std::vector<std::string> args={});

  bool loadFile(const std::string& file);
  bool parseCommands(std::string cmd);
  void render();
  void init();
  std::string image(std::string filename="", int width=0, int height=0, bool frame=false);
  std::string web(bool tofile=false);
  void defaultModel();
  void addObject(std::string name, std::string properties="");
  void setObject(std::string name, std::string properties);
  std::string getObject(std::string name);
  int colourMap(std::string name, std::string colours="");
  void setState(std::string state);
  std::string getState();
  std::string getFigures();
  std::string getTimeSteps();
  void loadVectors(std::vector< std::vector <float> > array, lucGeometryDataType type=lucVertexData);
  void loadScalars(std::vector <float> array, lucGeometryDataType type=lucColourValueData, std::string label="", float minimum=0, float maximum=0);
  void loadUnsigned(std::vector <unsigned int> array, lucGeometryDataType type=lucIndexData);
  void labels(std::vector <std::string> labels);
  void close();

%pythoncode %{
  #dict methods
  def __getitem__(self, key):
    #Get view/global property
    #self.state = json.loads(self.getState())
    view = self.state["views"][0]
    if key in self.state:
      return self.state[key]
    if key in view:
      return view[key]
    else:
      return self.state["properties"][key]
    return None

  def __setitem__(self, key, item):
    #Set view/global property
    #self.parseCommands("select") #Ensure no object selected
    #self.parseCommands(key + '=' + str(item))
    #self.get()
    self.state = json.loads(self.getState())
    view = self.state["views"][0]
    if key in self.state:
      self.state[key] = item
    if key in view:
      view[key] = item
    else:
      self.state["properties"][key] = item
    self.set()

  def __contains__(self, key):
    return key in self.state or key in self.state["properties"] or key in self.state["views"][0]

  def __str__(self):
    #View/global props to string
    self.get()
    self.properties = self.state["properties"]
    self.properties.update(self.state["views"][0])
    return str('\n'.join(['  %s=%s' % (k,json.dumps(v)) for k,v in self.properties.iteritems()]))

  def get(self):
    #Import state from lavavu
    self.state = json.loads(self.getState())
    if not isinstance(self.objects, Objects):
      self.objects = Objects(self)
    self.objects.update()

  def set(self):
    #Export state to lavavu
    #(include current object list state)
    #self.state["objects"] = [obj.dict for obj in self.objects.list]
    self.setState(json.dumps(self.state))

  def commands(self, cmds):
    if isinstance(cmds, list):
      self.parseCommands('\n'.join(cmds))
    else:
      self.parseCommands(cmds)
    #Always sync the state after running commands
    self.get()

  #Callable with commands...
  def __call__(self, cmds):
    self.commands(cmds)

  #Undefined methods supported directly as LavaVu commands
  def __getattr__(self, key):
    def any_method(*args, **kwargs):
      argstr = key
      for arg in args:
        argstr += " " + str(arg)
      self.commands(argstr)
    return any_method

  def add(self, name, properties={}):
    if not self.amodel:
      self.defaultModel()

    if isinstance(self.objects, Objects) and name in self.objects:
      print "Object exists: " + name
      return self.objects[name]

    #Adds a new object
    self.addObject(name, json.dumps(properties))

    #Get state and update object list
    self.get()

    #Read any property data sets (allows object creation and load with single prop dict)
    for key in properties:
      if key in ["vertices", "normals", "vectors", "colours", "values"]:
        self.parseCommands("read " + key)

    #Return wrapper obj
    return self.objects.list[-1]

  def file(self, filename, name=None, properties={}):
    if not self.amodel:
      self.defaultModel()

    if isinstance(self.objects, Objects) and name in self.objects:
      print "Object exists: " + name
      return self.objects[name]

    #Load a new object from file
    #self.loadFile(filename)
    self.parseCommands("file " + filename)
    if name:
      properties["name"] = name

    #Get state and update object list
    self.get()

    #Update properties and return wrapper object
    obj = self.objects.list[-1] #Use last in list, most recently added
    self.setObject(obj.name, json.dumps(properties))
    return obj

  def clear(self):
    self.close()

  def timesteps(self):
    return json.loads(self.getTimeSteps())

  def frame(self, resolution=(640,480)):
      #Jpeg encoded frame data
      return self.image("", resolution[0], resolution[1], True);

  def display(self, resolution=(640,480)):
      """    
      Shows the generated image inline within an ipython notebook.
      
      If IPython is installed, displays the result image content inline

      If IPython is not installed, this method will call the default image 
      output routines to save the result with a default filename in the current directory

      """
      #Sync state first
      self.set()

      try:
        if __IPYTHON__:
          from IPython.display import display,Image,HTML
          #Return inline image result
          img = self.image("", resolution[0], resolution[1])
          display(HTML("<img src='%s'>" % img))
      except NameError, ImportError:
        #Not in IPython, call default image save routines (autogenerated filenames)
        self.image("test", resolution[0], resolution[1])

  def webgl(self, resolution=(640,480)):
      """    
      Shows the generated model inline within an ipython notebook.
      
      If IPython is installed, displays the result WebGL content inline

      If IPython is not installed, this method will call the default web 
      output routines to save the result with a default filename in the current directory

      """

      try:
        if __IPYTHON__:
          from IPython.display import display,Image,HTML
          #Write files to disk first, can be passed directly on url but is slow for large datasets
          filename = "input.json"
          #Create link to web content directory
          import os
          if not os.path.isdir("html"):
            os.symlink(os.path.join(self.binpath, 'html'), 'html')
          text_file = open("html/" + filename, "w")
          text_file = open("html/" + filename, "w")
          text_file.write(self.web());
          text_file.close()
          from IPython.display import IFrame
          display(IFrame("html/index.html#" + filename, width=resolution[0], height=resolution[1]))
      except NameError, ImportError:
        self.web(True)
      except Exception,e:
        print "WebGL output error: " + str(e)
        pass
%}
};

