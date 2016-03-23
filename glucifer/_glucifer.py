##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##


import underworld as uw
import errno
import underworld._stgermain as _stgermain
import os
import urllib2
import time
from base64 import b64encode
import libUnderworld
import _LavaVu as lavavu
import subprocess
from subprocess import Popen, PIPE, STDOUT
from . import objects

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
    """  
    The Figure class provides a window within which gLucifer drawing objects
    may be rendered. It also provides associated routines for image generation
    and visualisation.
    
    In addition to parameter specification below, see property docstrings for
    further information.

    Parameters
    ----------
    figsize: tuple, default=(640,480)
        Image resolution provided as a tuple.
    boundingBox: tuple, default=None
        Tuple of coordiante tuples defining figure bounding box.
        For example ( (0.1,0.1), (0.9,0.9) )
    facecolour: str, default="white"
        Background colour for figure.
    edgecolour: str, default="black"
        Edge colour for figure.
    title: str, default=None
        Figure title.
    axis: bool, default=False
        Bool to determine if figure axis should be drawn.
    quality: unsigned, default=1
        Antialiasing oversampling quality. For a value of 2, the image will be
        rendered at twice the resolution, and then downsampled. Setting
        this to 1 disables antialiasing, values higher than 3 are not recommended..
    properties: str, default=None
        Further properties to set on the figure.
            
    Example
    -------
        
    Create a figure:
    >>> import glucifer
    >>> fig = glucifer.Figure()

    We need a mesh
    >>> import underworld as uw
    >>> mesh = uw.mesh.FeMesh_Cartesian()

    Add drawing objects:
    >>> fig.append( glucifer.objects.Surface( mesh, 1.) )
    
    Draw image (note, in a Jupyter notebook, this will render the image within the notebook).
    >>> fig.show()
    <IPython.core.display.HTML object>
    
    Save the image
    >>> imgfile = fig.save_image("test_image")

    Save the database
    >>> dbfile = fig.save_database("test_db")
    
    Clean up:
    >>> if uw.rank() == 0: 
    ...     import os; 
    ...     os.remove( dbfile )
    ...     os.remove( imgfile )

    """
    _objectsDict = { "_db":"lucDatabase",
                    "_win":"lucWindow",
                     "_vp":"lucViewport",
                    "_cam":"lucCamera"}
    _selfObjectName = "_db"
    _viewerProc = None

    def __init__(self, figsize=(640,480), boundingBox=None, facecolour="white",
                 edgecolour="black", title="", axis=False, quality=1, properties=None, **kwargs):
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

        if not isinstance(title,str):
            raise TypeError("'title' object passed in must be of python type 'str'")

        if not isinstance(axis,bool):
            raise TypeError("'axis' object passed in must be of python type 'bool'")

        if quality and not isinstance(quality,(int,float)):
            raise TypeError("'quality' object passed in must be of python type 'float' or 'int'")
        self.quality=quality

        #Setup default properties
        self._properties = {"title" : title, "axis" : axis, "axislength" : 0.2, "antialias" : True,
            "margin" : 32, "border" : (1 if edgecolour else 0), "borderColour" : edgecolour, "rulers" : False, "timestep" : False, "zoomstep" : 0} 
        
        if properties and not isinstance(properties,dict):
            raise TypeError("'properties' object passed in must be of python type 'dict'")
        if properties:
            self._properties.update(properties)

        self.draw = objects.Drawing()
        self._drawingObjects = [self.draw]
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
                            "filename"          :None,
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
                            "properties"        :self._getProperties()
        } )
        componentDictionary[self._cam.name].update ( {
                            "useBoundingBox"    : True
        } )

    def _getProperties(self):
        #Convert properties to string
        return '\n'.join(['%s=%s' % (k,v) for k,v in self._properties.iteritems()]);

    def _setProperties(self, newProps):
        #Update the properties values (merge)
        #values of any existing keys are replaced and viewport is updated
        self._properties.update(newProps)
        _libUnderworld.gLucifer.lucViewport_SetProperties(self._vp, self._getProperties());

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
        return self._properties["bordercolour"]

    @property
    def title(self):
        """    title : a title for the image, default: None
        """
        return self._properties["title"]

    @property
    def axis(self):
        """    axis : Axis enabled if true.  Default False.
        """
        return self._properties["axis"]

    @property
    def drawingObjects(self):
        """    drawingObjects : list of objects to be drawn within the figure.
        """
        return self._drawingObjects

    @property
    def properties(self):
        """    
        properties (dict): visual properties of viewport, passed to LavaVu to control 
        rendering output of figure.
        
        When using the property setter, new properties are set, overwriting any duplicate 
        keys but keeping existing values otherwise.
        """
        return self._properties
    @properties.setter
    def properties(self, value):
        self._setProperties(value)

    def show(self, type="image"):
        """    
        Shows the generated image inline within an ipython notebook
        
        Parameters
        ----------
        type: str
            Type of visualisation to display ('Image' or 'WebGL'). Default is 'Image'.
        
        Returns
        -------
        Ipython HTML object (for type 'Image')
        Ipython IFrame object (for type 'Webgl')
        Note that if IPython is not installed, this method will return nothing.

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

    def save_image(self, filename, size=(0,0)):
        """  
        Saves the generated image to the provided filename.
        
        Parameters
        ----------
        filename :str
            Filename to save file to.  May include an absolute or relative path.
            size (tuple(int,int)): size of image in pixels, defaults to original figsize setting
        """
        if not isinstance(filename, str):
            raise TypeError("Provided parameter 'filename' must be of type 'str'. ")
        if size and not isinstance(size,tuple):
            raise TypeError("'size' object passed in must be of python type 'tuple'")

        self._generate_DB()
        if uw.rank() == 0:
            return self._generate_image(filename, size)

    def save_database(self,filename,regen=True):
        """  
        Saves the generated database to the provided filename.
            
        Parameters
        ----------
        filename :str
            Filename to save file to.  May include an absolute or relative path.
        regen :bool, default=True
            Regenerate the database, only required if show() has not been called previously.
        """
        if regen:
            self._generate_DB()
        if uw.rank() == 0:
            if not isinstance(filename, str):
                raise TypeError("Provided parameter 'filename' must be of type 'str'. ")
            if not filename.lower().endswith('.gldb'):
                filename += '.gldb'
            libUnderworld.gLucifer.lucDatabase_BackupDbFile(self._db, filename)

    def _generate_DB(self):
        # remove any existing
        libUnderworld.gLucifer.lucDatabase_DeleteGeometry(self._db, 0, 0); #Delete existing at timestep 0 (is this necessary?)
        for ii in range(self._vp.drawingObject_Register.objects.count,0,-1):
            libUnderworld.StGermain._Stg_ObjectList_RemoveByIndex(self._vp.drawingObject_Register.objects,ii-1, libUnderworld.StGermain.KEEP)

        # first add drawing objects to viewport
        if len(self._drawingObjects) == 0:
            raise RuntimeError("There appears to be no drawing objects to render.")

        #Add drawing objects to register and output any custom data on them
        for object in self._drawingObjects:

# I'M COMMENTING THIS OUT FOR NOW, AS WITHOUT IT, RENDERING AN EMPTY FIGURE EXPLODES.. KABOOM!  JM 1/16 
#            #Only add default object if used
#            if object == self.draw and len(self.draw.vertices) == 0:
#                continue

            #Add the object to the drawing object register for the (default) viewport
            object._cself.id = 0
            libUnderworld.StGermain.Stg_ObjectList_Append(self._vp.drawingObject_Register.objects,object._cself)
            #For default colour bars created with drawing objects rather than added on their own, add manually
            if object._colourBar:
                object._colourBar._dr.id = 0
                libUnderworld.StGermain.Stg_ObjectList_Append(self._vp.drawingObject_Register.objects,object._colourBar._dr)
            #Set ids to 0 as when > 0 flagged as already written to db and skipped
            if object._colourMap:
                object._colourMap._cself.id = 0

        # go ahead and fill db
        libUnderworld.gLucifer.lucDatabase_DeleteWindows(self._db)
        libUnderworld.gLucifer.lucDatabase_OutputWindow(self._db, self._win)
        libUnderworld.gLucifer._lucDatabase_Execute(self._db,None)
        libUnderworld.gLucifer._lucWindow_Execute(self._win,None)

        #Output any custom geometry on objects
        for object in self._drawingObjects:
            self._plotObject(object)

        # Detect if viewer was built by finding executable...
        # (even though no longer needed as using libLavaVu 
        #  will keep this for now as is useful to know if the executable was built
        #  and to pass it as first command line arg or if needed to get html path)
        self._lvpath = self._db.dump_script.replace("dump.sh", "")
        self._lvbin = self._lvpath + "LavaVu"
        if not os.path.isfile(self._lvbin):
            raise RuntimeError("LavaVu rendering engine does not appear to exist. Perhaps it was not compiled.\nPlease check your configuration, or contact developers.")
        
    def _plotObject(self, drawingObject):
        #General purpose plotting using db output
        #Plot all custom data drawn on provided object
        farr = libUnderworld.gLucifer.new_farray(3)
        for pos in drawingObject.vertices:
            #Write vertex position
            i = 0
            for item in pos:
                libUnderworld.gLucifer.farray_setitem(farr,i,item)  # Set values
                i += 1
            libUnderworld.gLucifer.lucDatabase_AddVertices(self._db, 1, drawingObject.geomType, farr)

        #Write vectors
        for vec in drawingObject.vectors:
            i = 0
            for item in vec:
                libUnderworld.gLucifer.farray_setitem(farr,i,item)  # Set values
                i += 1
            libUnderworld.gLucifer.lucDatabase_AddVectors(self._db, 1, drawingObject.geomType, 0, 0, farr)

        #Write values
        for value in drawingObject.scalars:
            libUnderworld.gLucifer.farray_setitem(farr,0,value)  # Set values
            libUnderworld.gLucifer.lucDatabase_AddValue(self._db, 1, drawingObject.geomType, farr)

        libUnderworld.gLucifer.delete_farray(farr)

        #Write labels
        for label in drawingObject.labels:
            libUnderworld.gLucifer.lucDatabase_AddLabel(self._db, drawingObject.geomType, label);

        #Write the custom geometry to the database
        libUnderworld.gLucifer.lucDatabase_OutputGeometry(self._db, drawingObject._dr.id)

    def _generate_image(self, filename="", size=(0,0)):
        if uw.rank() == 0:
            #Render with viewer
            args = [self._lvbin, self._db.path, "-" + str(self._db.timeStep), "-p0", "-z" + str(self.quality), "-a", "-h", ":"] + self._script
            lavavu.execute(args)
            imagestr = lavavu.image(filename, size[0], size[1])
            #Return the generated filename
            if filename: return imagestr
            #Return inline image result
            from IPython.display import Image,HTML
            return HTML("<img src='%s'>" % imagestr)

    def _generate_HTML(self):
        if uw.rank() == 0:
            #Export encoded json string
            jsonstr = lavavu.execute([self._lvbin, "-" + str(self._db.timeStep), "-U", "-p0", self._db.path, ":"] + self._script)
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
        """ 
        Append to or get contents of the saved script.
        
        Parameters
        ----------
        cmd: str
            Command to add to script.
        """
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
        """ Get image from current viewer window.
        """
        from IPython.display import Image,HTML
        if not self._viewerProc:
            return
        url = "http://localhost:8080/image=0"
        response = urllib2.urlopen(url).read()
        image_data = "data:image/png;base64,%s" % b64encode(response)
        return HTML("<img src='%s'>" % image_data)

    def open_viewer(self, args=[]):
        """ Open the viewer.
        """
        if uw.rank() == 0:
            fname = os.path.join(tmpdir,"gluciferDB"+self._id+".gldb")
            self.save_database(fname)
            if self._viewerProc and self._viewerProc.poll() == None:
                return
            #Open viewer with local web server for interactive/iterative use
            args = [self._lvbin, "-" + str(self._db.timeStep), "-L", "-p8080", "-q90", "-Q", fname] + args
            self._viewerProc = subprocess.Popen(args, stdout=PIPE, stdin=PIPE, stderr=STDOUT)

            from IPython.display import HTML
            return HTML('''<a href='#' onclick='window.open("http://" + location.hostname + ":8080");'>Open Viewer Interface</a>''')

    def close_viewer(self):
        """ Close the viewer.
        """
        #poll() returns None if the process is alive
        if self._viewerProc and self._viewerProc.poll() == None:
           self.send_command("quit")
           self._viewerProc.kill()
        self._viewerProc = None

    def run_script(self):
        """ Run the saved script on an open viewer instance
        """
        self.open_viewer()
        url = "http://localhost:8080/command=" + urllib2.quote(';'.join(self._script))
        response = urllib2.urlopen(url).read()
        #print response
  
    def send_command(self, cmd):
        """ 
        Run command on an open viewer instance.
        
        Parameters
        ----------
        cmd: str
            Command to send to open viewer.
        """
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
        """    Clears all the figure's drawing objects and colour maps.
        """
        del self._drawingObjects[:]

    def __add__(self,drawing_object):
        # DEPRECATE
        raise RuntimeError("This method is now deprecated.\n" \
                           "To add drawing objects to figures, use the append() method:\n" \
                           "    someFigure.append(someDrawingObject)")
    
    def append(self,drawingObject):
        """
        Add a drawing object to the figure. 
        
        Parameters
        ----------
        drawingObject: objects.Drawing
            The drawing object to add to the figure
                
        
        """
        if not isinstance(drawingObject, objects.Drawing):
            raise TypeError("Object your are trying to add to figure does not appear to be of type 'Drawing'.")

        self._drawingObjects.append( drawingObject )
