import underworld as uw
import errno
import weakref
import underworld._stgermain as _stgermain
import _drawing
from subprocess import Popen, PIPE, STDOUT
import os
import subprocess
import urllib2
import time
from base64 import b64encode
import libUnderworld

# lets create somewhere to dump data for this session
import os
try:
    tmpdir = os.environ['TMPDIR']
except:
    tmpdir = "/tmp"
os.path.join(tmpdir,"glucifer")
try:
    os.makedirs(tmpdir)
except OSError as exception:
    if exception.errno != errno.EEXIST:
        raise

class Figure(_stgermain.StgCompoundComponent):
    """  The Figure class provides the base container for gLucifer drawing objects, and associated routines for image generation.
         Generally the user will use the figure() routine to generate or recall Figure instances.
         
         A minimal workflow might be as follows:
            fig = plt.figure()                  # generate a Figure instance
            fig.Surface(field="PressureField")  # add Surface drawing object
            fig.show()                          # show the image (in ipython notebook)
            
         Currently, the Surface, Points & VectorArrows are the supported drawing objects.  See help() on these objects for their respective options.
    """
    _objectsDict = { "_db":"lucDatabase",
                    "_win":"lucWindow",
                     "_vp":"lucViewport",
                    "_cam":"lucCamera"}
    _selfObjectName = "_db"
    _viewerProc = None
    _stdout = ""
    _cmdline = ""

    def __init__(self, num=None, figsize=(640,480), facecolour="white", edgecolour="black", title=None, axis=False, **kwargs):
        """ The initialiser takes as arguments 'num', 'figsize', 'facecolour', 'edgecolour', 'title' and 'axis'.   See help(Figure) for full details on these options.
        """
        if num and not isinstance(num,(str,int)):
            raise TypeError("'num' object passed in must be of python type 'str' or 'int'")
        if num and isinstance(num,str) and (" " in num):
            raise ValueError("'num' object passed in must not contain any spaces.")
        self._num = num

        if not isinstance(figsize,tuple):
            raise TypeError("'figsize' object passed in must be of python type 'tuple'")
        self._figsize = figsize

        if not isinstance(facecolour,str):
            raise TypeError("'facecolour' object passed in must be of python type 'str'")
        self._facecolour = facecolour

        if not isinstance(edgecolour,str):
            raise TypeError("'edgecolour' object passed in must be of python type 'str'")
        self._edgecolour = edgecolour

        if title and not isinstance(title,str):
            raise TypeError("'title' object passed in must be of python type 'str'")
        self._title = title

        if not isinstance(axis,bool):
            raise TypeError("'axis' object passed in must be of python type 'bool'")
        self._axis = axis

        self._drawingObjects = []
        
        self._script = []
        
        super(Figure,self).__init__(**kwargs)

    def __del__(self):
        self.close_viewer()

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        super(Figure,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._db.name].update( {
                            "filename"          :"gluciferDB"+self._id,
                            "blocking"          :True,
                            "splitTransactions" :True,
                            "dbPath"            :tmpdir
        } )
        componentDictionary[self._win.name].update( {
                            "Database"          :self._db.name,
                            "Viewport"          :[self._vp.name],
                            "width"             :self.figsize[0],
                            "height"            :self.figsize[1],
                            "backgroundColour"  :self.facecolour,
                            "useModelBounds"    :False
        } )
        componentDictionary[self._vp.name].update( {
                            "Camera"            :self._cam.name,
                            "borderColour"      :self.edgecolour,
                            "border"            :1,
                            "title"             :self.title,
                            "axis"              :self.axis
        } )
        componentDictionary[self._cam.name].update ( {
                            "useBoundingBox"    : True
        } )


    @property
    def num(self):
        """    num (str,int): integer or string figure identifier. Must not contain spaces. optional, default: none
        """
        return self._num

    @property
    def figsize(self):
        """    figsize (tuple(int,int)): size of window in pixels, default: (640,480)
        """
        return self._figsize

    @property
    def facecolour(self):
        """    facecolour : colour of face background, default: white
        """
        return self._facecolour

    @property
    def edgecolour(self):
        """    edgecolour : colour of figure border, default: white
        """
        return self._edgecolour

    @property
    def title(self):
        """    title : a title for the image, default: None
        """
        return self._title

    @property
    def axis(self):
        """    axis : Axis enabled if true.  Default False.
        """
        return self._axis

    @property
    def drawingObjects(self):
        """    drawingObjects : list of objects to be drawn within the figure.
        """
        return self._drawingObjects

    def show(self, type="image"):
        """    Shows the generated image inline within an ipython notebook
        
                Args:
                    type(str) : Type of visualisation to display ('Image' or 'WebGL'). Default is 'Image'.
                    
                Returns:
                    Ipython Image object (will be displayed inline in an ipython notebook).
                    If Ipython is not found, nothing is returned.

        """

        try:
            from IPython.display import Image,HTML
            self._generate_DB()
            if uw.rank() == 0:
                self._generate_image()
                if type.lower() == "webgl":
                    return HTML('<iframe src='+self._find_generated_HTML()+' width=1000 height=500></iframe>')
                else:
                    return Image(filename=self._find_generated_file())
        except ImportError:
            pass
        except RuntimeError:
            pass
        except:
            raise

    def _find_generated_file(self):
        # lets determine what we are outputting (jpg,png)
        foundFile = None
        fname = None
        for extension in ("jpg", "jpeg", "png"):
            fname = os.path.join(tmpdir,self._win.name+".00000."+extension)
            if os.path.isfile(fname):
                foundFile = fname
                break
        
        if not foundFile:
            #raise RuntimeError("The required rendered image does not appear to have been created. Please contact developers. (" + fname + ")")
            print "The required rendered image does not appear to have been created. Please contact developers. "
            print "(" + fname + ")"
            print "COMMAND: %s" % self._cmdline
            print "STDOUT: %s" % self._stdout
            raise RuntimeError("The required rendered image does not appear to have been created. Please contact developers.")
        
        return os.path.abspath(foundFile)

    def _find_generated_HTML(self):
        # lets determine what we are outputting (jpg,png)
        fname = os.path.join(tmpdir,'html','index.html')
        url = 'http://localhost:8000/html/index.html?../' +self._win.name+".00000.jsonp"
        return url

    def _find_generated_DB(self):
        fname = os.path.join(tmpdir,"gluciferDB"+self._id+".gldb")
        if not os.path.isfile(fname):
            raise RuntimeError("The database does not appear to have been created. Please contact developers.")
        
        return os.path.abspath(fname)

    def save_image(self,filename):
        """  Saves the generated image to the provided filename.
            
             Args:
               filename (str):  Filename to save file to.  May include an absolute or relative path.
        """
        self._generate_DB()
        if uw.rank() == 0:
            self._generate_image()
            generatedFilename=self._find_generated_file()
            
            absfilename = os.path.abspath(filename)
            
            # lets set the final extension to that of the glucifer generated file
            splitabsfilename = os.path.splitext(absfilename)
            splitgenfilename = os.path.splitext(generatedFilename)
            if splitabsfilename[1].lower() in [".png", ".jpg", ".jpeg"]:
                frontpart = splitabsfilename[0]
            else:
                frontpart = absfilename
            finaloutFile = frontpart+splitgenfilename[1]
            from shutil import copyfile
            copyfile(generatedFilename,finaloutFile)

    def save_database(self,filename):
        """  Saves the generated database to the provided filename.
            
             Args:
               filename (str):  Filename to save file to.  May include an absolute or relative path.
        """
        self._generate_DB()
        if uw.rank() == 0:
            self._generate_image()
            generatedDB=self._find_generated_DB()
            absfilename = os.path.abspath(filename)

            from shutil import copyfile
            copyfile(generatedDB,absfilename)


    def _generate_DB(self):
        # remove any existing
        for ii in range(self._vp.drawingObject_Register.objects.count,0,-1):
            libUnderworld.StGermain._Stg_ObjectList_RemoveByIndex(self._vp.drawingObject_Register.objects,ii-1, libUnderworld.StGermain.KEEP)
        # first add drawing objects to viewport
        if len(self.drawingObjects) == 0:
            raise RuntimeError("There appears to be no drawing objects to render.")
        for object in self.drawingObjects:
            object._cself.id = 0
            libUnderworld.StGermain.Stg_ObjectList_Append(self._vp.drawingObject_Register.objects,object._cself)
            if object._colourBar == True:
                object._cb.id = 0
                libUnderworld.StGermain.Stg_ObjectList_Append(self._vp.drawingObject_Register.objects,object._cb)
                
        # go ahead and fill db
        libUnderworld.gLucifer.lucDatabase_DeleteWindows(self._db)
        libUnderworld.gLucifer.lucDatabase_OutputWindow(self._db, self._win)
        libUnderworld.gLucifer._lucDatabase_Execute(self._db,None)
        libUnderworld.gLucifer._lucWindow_Execute(self._win,None)

    def _generate_image(self):
        if uw.rank() == 0:
            # go ahead and draw
            scriptpath = self._db.dump_script
            lvpath = scriptpath.replace("dump.sh", "LavaVuOS")
            if not os.path.isfile(lvpath):
                lvpath = scriptpath.replace("dump.sh", "LavaVu")
                if not os.path.isfile(lvpath):
                    raise RuntimeError("LavaVu rendering engine does not appear to exist. Perhaps it was not compiled.\nPlease check your configuration, or contact developers.")
            timestep = "-" + str(self._db.timeStep)
            dbfile = self._db.path
            args = [lvpath, timestep, "-I", dbfile]
            #Script commands to run...
            script = '\n'.join(self._script + ['image', 'quit'])
            #Start the viewer process
            proc = subprocess.Popen(args, stdout=PIPE, stdin=PIPE, stderr=STDOUT)
            #Run the script
            self._stdout = proc.communicate(input=script)[0]
            self._cmdline = ' '.join(args)

    def script(self, cmd=None):
        #Append to or get contents of the saved script
        if cmd:
            if isinstance(cmd, list):
                self._script += cmd
            else:
                self._script += [cmd]
        else:
            self._script = []
        #Returns contents as newline separated string
        return '\n'.join(self._script)

    def image(self):
        #Get image from current viewer window
        from IPython.display import Image,HTML
        if not self._viewerProc:
            return
        url = "http://localhost:8080/image=0"
        response = urllib2.urlopen(url).read()
        image_data = "data:image/png;base64,%s" % b64encode(response)
        return HTML("<img src='%s'>" % image_data)

    def open_viewer(self, args=[], ):
        self._generate_DB()
        if uw.rank() == 0:
            #Open viewer with local web server for interactive/iterative use
            if self._viewerProc:
                return
            scriptpath = self._db.dump_script
            lvpath = scriptpath.replace("dump.sh", "LavaVu")
            if not os.path.isfile(lvpath):
                lvpath = scriptpath.replace("dump.sh", "LavaVuOS")
                if not os.path.isfile(lvpath):
                    raise RuntimeError("LavaVu rendering engine does not appear to exist. Perhaps it was not compiled.\nPlease check your configuration, or contact developers.")
            timestep = "-" + str(self._db.timeStep)
            dbfile = self._db.path
            self._viewerProc = subprocess.Popen([lvpath, timestep, "-L", "-p8080", "-q90", "-Q", "-v", dbfile] + args, stdout=PIPE, stdin=PIPE, stderr=STDOUT)
            #Wait a second so server has time to start
            time.sleep(1)
            from IPython.display import HTML
            return HTML('''<a href='#' onclick='window.open("http://" + location.hostname + ":8080");'>Open Viewer Interface</a>''')

    def close_viewer(self):
        if self._viewerProc:
           self._viewerProc.kill()
           self._viewerProc = None

    def run_script(self):
        #Run the saved script on an open viewer instance
        self.open_viewer()
        url = "http://localhost:8080/command=" + urllib2.quote(';'.join(self._script))
        response = urllib2.urlopen(url).read()
        #print response
  
    def send_command(self, cmd):
        #Run command on an open viewer instance
        self.open_viewer()
        url = "http://localhost:8080/command=" + urllib2.quote(cmd)
        #print url
        response = urllib2.urlopen(url).read()
        #print response

    def clear(self):
        """    Clears all the figure's drawing objects 
        """
        del self.drawingObjects[:]

    def Surface(self, fn, mesh, drawSides="xyzXYZ", **kwargs):
        """    Add a surface drawing object to the current figure.
               See 'help(Surface)' for information on the Surface class and it's options.
               
               Returns the generated Surface object.
        """
        guy = _drawing.Surface(fn, mesh, drawSides=drawSides, **kwargs)
        self.drawingObjects.append(guy)
        return guy

    def Points(self, swarm, colourVariable=None, sizeVariable=None, opacityVariable=None, pointSize=1.0, **kwargs):
        """    Add a points drawing object to the current figure.
               See 'help(Points)' for information on the Points class and it's options.
               
               Returns the generated Points object.
        """
        guy = _drawing.Points( swarm=swarm, colourVariable=colourVariable, sizeVariable=sizeVariable,
                                            opacityVariable=opacityVariable, pointSize=pointSize, **kwargs)
        self.drawingObjects.append(guy)
        return guy

    def VectorArrows(self, fn, mesh, resolutionX=16, resolutionY=16, resolutionZ=16,
                                  arrowHeadSize=0.3, lengthScale=0.3, glyphs=3, **kwargs):
        """    Add a vector arrow drawing object to the current figure.
               See 'help(VectorArrows)' for information on the VectorArrows class and it's options.
               
               Returns the generated Points object.
        """
        guy = _drawing.VectorArrows( fn, mesh, resolutionX=resolutionX, resolutionY=resolutionY, resolutionZ=resolutionZ,
                                                  arrowHeadSize=arrowHeadSize, lengthScale=lengthScale, glyphs=glyphs, colourBar=False, **kwargs)
        self.drawingObjects.append(guy)
        return guy

    def Mesh(self, mesh, nodeNumbers=False, segmentsPerEdge=1, **kwargs):
        """    Add a mesh drawing object to the current figure.
               See 'help(Mesh)' for information on the Mesh class and it's options.
               
               Returns the generated Mesh object.
        """
        guy = _drawing.Mesh(mesh=mesh, nodeNumbers=nodeNumbers, segmentsPerEdge=segmentsPerEdge, **kwargs)
        self.drawingObjects.append(guy)
        return guy
