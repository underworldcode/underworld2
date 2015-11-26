import underworld as uw
import errno
import weakref
import underworld._stgermain as _stgermain
import _drawing
import os
import urllib2
import time
from base64 import b64encode
import libUnderworld
import _LavaVu as lavavu
import _gLucifer
from multiprocessing import Process

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

    def __init__(self, database=None, num=None, figsize=(640,480), boundingBox=None, facecolour="white", edgecolour="black", title=None, axis=False, **kwargs):
        """ The initialiser takes as arguments 'num', 'figsize', 'boundingBox', 'facecolour', 'edgecolour', 'title' and 'axis'.   See help(Figure) for full details on these options.
        """
        if database and not isinstance(database,str):
            raise TypeError("'database' object passed in must be of python type 'str'")
        self._database = database
        if num and not isinstance(num,(str,int)):
            raise TypeError("'num' object passed in must be of python type 'str' or 'int'")
        if num and isinstance(num,str) and (" " in num):
            raise ValueError("'num' object passed in must not contain any spaces.")
        self._num = num

        if not isinstance(figsize,tuple):
            raise TypeError("'figsize' object passed in must be of python type 'tuple'")
        self._figsize = figsize

        if boundingBox and not isinstance(boundingBox,tuple):
            raise TypeError("'boundingBox' object passed in must be of type 'tuple'")
        if boundingBox and not isinstance(boundingBox[0],tuple):
            raise TypeError("'boundingBox[0]' object passed in must be of type 'tuple'")
        if boundingBox and not isinstance(boundingBox[1],tuple):
            raise TypeError("'boundingBox[1]' object passed in must be of type 'tuple'")
        self._boundingBox = boundingBox

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

        self._defaultDrawingObject = _drawing.Drawing() #Default object for custom drawing
        self._drawingObjects = [self._defaultDrawingObject]
        self._colourMaps = [_drawing.Drawing._defaultColourMap]
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
                            "filename"          :self._database, #"gluciferDB"+self._id,
                            "blocking"          :True,
                            "splitTransactions" :True,
                            "dbPath"            :tmpdir,
        } )

        if self._boundingBox:
            componentDictionary[self._db.name]["minX"] = self._boundingBox[0][0]
            componentDictionary[self._db.name]["minY"] = self._boundingBox[0][1]
            componentDictionary[self._db.name]["minZ"] = self._boundingBox[0][2]
            componentDictionary[self._db.name]["maxX"] = self._boundingBox[1][0]
            componentDictionary[self._db.name]["maxY"] = self._boundingBox[1][1]
            componentDictionary[self._db.name]["maxZ"] = self._boundingBox[1][2]

        componentDictionary[self._win.name].update( {
                            "Database"          :self._db.name,
                            "Viewport"          :[self._vp.name],
                            "width"             :self.figsize[0],
                            "height"            :self.figsize[1],
                            "backgroundColour"  :self.facecolour,
                            "useModelBounds"    :self._boundingBox != None
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
    def filename(self):
        """    database (str): filename for storing generated figure content, default: none (in-memory only)
        """
        return self._database

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

    @property
    def colourMaps(self):
        """    colourMaps : list of colour maps available within the figure.
        """
        return self._colourMaps

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
                if type.lower() == "webgl":
                    return self._generate_HTML()
                else:
                    return self._generate_image()
        except ImportError:
            pass
        except RuntimeError, e:
            print "Error creating image: "
            print e
            pass
        except:
            raise

    def _find_generated_file(self):
        # lets determine what we are outputting (jpg,png)
        foundFile = None
        fname = None
        for extension in ("jpg", "jpeg", "png"):
            #fname = os.path.join(tmpdir,self._win.name+".00000."+extension)
            fname = self._win.name+".00000."+extension
            if os.path.isfile(fname):
                foundFile = fname
                break
        
        if not foundFile:
            raise RuntimeError("The required rendered image does not appear to have been created. Please contact developers. (" + fname + ")")
        
        return os.path.abspath(foundFile)

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
            self._generate_image(asfile=True)
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
            os.rename(generatedFilename,finaloutFile)

    def save_database(self,filename):
        """  Saves the generated database to the provided filename.
            
             Args:
               filename (str):  Filename to save file to.  May include an absolute or relative path.
        """
        if uw.rank() == 0:
            libUnderworld.gLucifer.lucDatabase_BackupDbFile(self._db, filename)

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
            #For default colour bars created with drawing objects rather than added on their own, add manually
            if object._colourBar:
                object._colourBar._dr.id = 0
                libUnderworld.StGermain.Stg_ObjectList_Append(self._vp.drawingObject_Register.objects,object._colourBar._dr)
        #Set ids to 0 as when > 0 flagged as already written to db and skipped
        for object in self.colourMaps:
            object._cself.id = 0
                
        # go ahead and fill db
        libUnderworld.gLucifer.lucDatabase_DeleteWindows(self._db)
        libUnderworld.gLucifer.lucDatabase_OutputWindow(self._db, self._win)
        libUnderworld.gLucifer._lucDatabase_Execute(self._db,None)
        libUnderworld.gLucifer._lucWindow_Execute(self._win,None)

        # Detect if viewer was built by finding executable...
        # (even though no longer needed as using libLavaVu 
        #  will keep this for now as is useful to know if the executable was built
        #  and to pass it as first command line arg or if needed to get html path)
        self._lvpath = self._db.dump_script.replace("dump.sh", "")
        self._lvbin = self._lvpath + "LavaVu"
        if not os.path.isfile(self._lvbin):
            raise RuntimeError("LavaVu rendering engine does not appear to exist. Perhaps it was not compiled.\nPlease check your configuration, or contact developers.")
        
    def _generate_image(self, asfile=False):
        if uw.rank() == 0:
            #Render with viewer
            if asfile:
                starting_directory = os.getcwd()
                lavavu.initViewer([self._lvbin, "-" + str(self._db.timeStep), "-I", "-p0", self._db.path, ":"] + self._script)
            else:
                imagestr = lavavu.initViewer([self._lvbin, "-" + str(self._db.timeStep), "-u", "-h", "-p0", self._db.path, ":"] + self._script)
                from IPython.display import Image,HTML
                return HTML("<img src='%s'>" % imagestr)

    def _generate_HTML(self):
        if uw.rank() == 0:
            #Export encoded json string
            jsonstr = lavavu.initViewer([self._lvbin, "-" + str(self._db.timeStep), "-U", "-h", "-p0", self._db.path, ":"] + self._script)
            if not os.path.isdir("html"):
                #Create link to web content directory
                os.symlink(self._lvpath + 'html', 'html')
            text_file = open("html/input.json", "w")
            text_file.write(jsonstr);
            text_file.close()
            from IPython.display import IFrame
            return IFrame("html/index.html#input.json", width=1000, height=800)
        return ""

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
            if self._viewerProc:
                return
            #Open viewer with local web server for interactive/iterative use
            self._viewerProc = Process(target=lavavu.initViewer, args=([self._lvbin, "-" + str(self._db.timeStep), "-L", "-p8080", "-q90", "-Q", self._db.path], ))
            self._viewerProc.start()

            from IPython.display import HTML
            return HTML('''<a href='#' onclick='window.open("http://" + location.hostname + ":8080");'>Open Viewer Interface</a>''')

    def close_viewer(self):
        if self._viewerProc:
           self.send_command("quit")
           self._viewerProc.join()
           #self._viewerProc.terminate()
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
        try:
            #print url
            response = urllib2.urlopen(url).read()
            #print response
        except:
            #Wait a second so server has time to start then try again
            time.sleep(1)
            response = urllib2.urlopen(url).read()

    def clear(self):
        """    Clears all the figure's drawing objects and colour maps
        """
        del self.drawingObjects[:]
        del self.colourMaps[:]

    def ColourMap(self, **kwargs):
        """    Add a Colour Map to the current figure.
               See 'help(ColourMap)' for information on the ColourMap class and it's options.
               
               Returns the generated ColourMap object.
        """
        guy = _drawing.ColourMap(**kwargs)
        self.colourMaps.append(guy)
        return guy

    def ColourBar(self, colourMap, **kwargs):
        """    Add a Colour Bar to the current figure.
               See 'help(ColourBar)' for information on the ColourMap class and it's options.
               
               Returns the generated ColourBar object.
        """
        guy = _drawing.ColourBar(colourMap=colourMap, **kwargs)
        self.drawingObjects.append(guy)
        return guy

    def Surface(self, fn, mesh, useMesh=False, drawSides="xyzXYZ", colourBar=True, **kwargs):
        """    Add a surface drawing object to the current figure.
               See 'help(Surface)' for information on the Surface class and it's options.
               
               Returns the generated Surface object.
        """
        guy = _drawing.Surface(fn=fn, mesh=mesh, useMesh=useMesh, drawSides=drawSides, colourBar=colourBar, **kwargs)
        self.drawingObjects.append(guy)
        return guy

    def Points(self, swarm, colourBar=True, colourVariable=None, sizeVariable=None, opacityVariable=None, pointSize=1.0, pointType=1, **kwargs):
        """    Add a points drawing object to the current figure.
               See 'help(Points)' for information on the Points class and it's options.
               
               Returns the generated Points object.
        """
        guy = _drawing.Points( swarm=swarm, colourBar=colourBar, colourVariable=colourVariable, sizeVariable=sizeVariable,
                                            opacityVariable=opacityVariable, pointSize=pointSize, pointType=pointType, **kwargs)
        self.drawingObjects.append(guy)
        return guy

    def VectorArrows(self, fn, mesh, resolutionX=16, resolutionY=16, resolutionZ=16,
                                  arrowHead=0.3, scaling=0.3, glyphs=3, **kwargs):
        """    Add a vector arrow drawing object to the current figure.
               See 'help(VectorArrows)' for information on the VectorArrows class and it's options.
               
               Returns the generated VectorArrows object.
        """
        guy = _drawing.VectorArrows( fn=fn, mesh=mesh, resolutionX=resolutionX, resolutionY=resolutionY, resolutionZ=resolutionZ,
                                                  arrowHead=arrowHead, scaling=scaling, glyphs=glyphs, **kwargs)
        self.drawingObjects.append(guy)
        return guy

    def Volume(self, fn, mesh, useMesh=False, colourBar=True, resolutionX=16, resolutionY=16, resolutionZ=16, **kwargs):
        """    Add a volume drawing object to the current figure.
               See 'help(Volume)' for information on the Volume class and it's options.
               
               Returns the generated Volume object.
        """
        guy = _drawing.Volume( fn=fn, mesh=mesh, useMesh=useMesh, colourBar=colourBar, resolutionX=resolutionX, resolutionY=resolutionY, resolutionZ=resolutionZ, **kwargs)
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

    #Direct drawing methods
    def label(self, text, pos=(0.,0.,0.), font="sans", scaling=1):
        """  Writes a label string
            
             Args:
               pos     (tuple):  X,Y,Z position to place the label
               text      (str):  label text
               font      (str):  label font (small/fixed/sans/serif/vector)
               scaling (float):  label font scaling (for "vector" font only)
        """
        self._vertex(geomType=libUnderworld.gLucifer.lucPointType, 
                     pos=pos, label=text, props={"font" : font, "fontscale" : scaling})

    def _vertex(self, drawingObject=None, geomType=None, pos=(0.,0.,0.), vec=None, value=None, label=None, props=None):
        #General purpose plotting using db output
        if not drawingObject:
            drawingObject = self._defaultDrawingObject

        #Replace props, overwriting keys if existing, then update
        if props:
            drawingObject._updateProperties(props, overwrite=True)
            libUnderworld.gLucifer.lucDrawingObject_SetProperties(drawingObject._dr, drawingObject._getProperties());

        #Wwrite vertex position
        nitems = len(pos)
        farr = _gLucifer.new_farray(nitems)
        i = 0
        for item in pos:
            _gLucifer.farray_setitem(farr,i,item)  # Set values
            i = i + 1
        libUnderworld.gLucifer.lucDatabase_AddVertices(self._db, 1, geomType, farr)

        #Write vector if any
        if vec:
            i = 0
            for item in vec:
                _gLucifer.farray_setitem(farr,i,item)  # Set values
                i = i + 1
            libUnderworld.gLucifer.lucDatabase_AddVectors(self._db, 1, geomType, 0, 0, farr)

        #Write value if any
        if value != None:
            _gLucifer.farray_setitem(farr,0,value)  # Set values
            libUnderworld.gLucifer.lucDatabase_AddValue(self._db, 1, geomType, farr)

        _gLucifer.delete_farray(farr)

        #Write label if any
        if label != None:
            libUnderworld.gLucifer.lucDatabase_AddLabel(self._db, geomType, label);

