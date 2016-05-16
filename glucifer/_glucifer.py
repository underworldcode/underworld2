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
import json
from base64 import b64encode
import libUnderworld
import subprocess
from subprocess import Popen, PIPE, STDOUT
from . import objects
import libUnderworld as _libUnderworld

haveLavaVu = True
try:
    import _LavaVu as lavavu
except:
    haveLavaVu = False

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

class Store(_stgermain.StgCompoundComponent):
    """  
    The Store class provides a database which stores gLucifer drawing objects
    as they are rendered in figures. It also provides associated routines for saving
    and reloading this database to external files
    
    In addition to parameter specification below, see property docstrings for
    further information.

    Parameters
    ----------
    filename: str, default=None
        Filename to use for a disk database, default is in memory only unless saved.
    properties: str, default=None
        Global properties to set on all figures stored.
            
    Example
    -------
        
    Create a database:
    >>> import glucifer
    >>> store = glucifer.Store()

    We need a mesh
    >>> import underworld as uw
    >>> mesh = uw.mesh.FeMesh_Cartesian()

    Add drawing objects:
    >>> store.append( glucifer.objects.Surface( mesh, 1.) )
    
    Draw image (note, in a Jupyter notebook, this will render the image within the notebook).
    >>> store.show()
    
    Save the database
    >>> dbfile = store.save("test_db")

    """
    _objectsDict = { "_db":"lucDatabase" }
    _selfObjectName = "_db"

    def __init__(self, filename=None, properties=None, **kwargs):

        self.step = 0
        self.filename = filename
        self._objects = []
        self.viewonly = False

        #Open an existing db?
        if filename and os.path.isfile(filename):
            self.viewonly = True

        #Setup default properties
        self._properties = {} 
        
        if properties and not isinstance(properties,dict):
            raise TypeError("'properties' object passed in must be of python type 'dict'")
        if properties:
            self._properties.update(properties)

        super(Store,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        super(Store,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._db.name].update( {
                            "filename"          :self.filename,
                            "blocking"          :True,
                            "splitTransactions" :True,
                            "viewonly"          :self.viewonly,
                            "dbPath"            :".",
        } )

    def _setup(self):
        # Detect if viewer was built by finding executable...
        # (even though no longer needed as using libLavaVu 
        #  will keep this for now as is useful to know if the executable was built
        #  and to pass it as first command line arg or if needed to get html path)
        self._lvpath = self._db.bin_path
        self._lvbin = os.path.join(self._db.bin_path, "LavaVu")
        global haveLavaVu
        if haveLavaVu and not os.path.isfile(self._lvbin):
            print("LavaVu rendering engine does not appear to exist. Perhaps it was not compiled.\n"
                  "Please check your configuration, or contact developers.")
            haveLavaVu = False

    def save(self,filename):
        """  
        Saves the database to the provided filename.
            
        Parameters
        ----------
        filename :str
            Filename to save file to.  May include an absolute or relative path.
        """
        if uw.rank() == 0:
            if not isinstance(filename, str):
                raise TypeError("Provided parameter 'filename' must be of type 'str'. ")
            if not filename.lower().endswith('.gldb') and not filename.lower().endswith('.db'):
                filename += '.gldb'
            libUnderworld.gLucifer.lucDatabase_BackupDbFile(self._db, filename)

    def _generate(self, objects, viewprops):
        #Add nested colourbar objects
        for obj in objects:
            if obj._colourBar:
                objects.append(obj._colourBar)

        #First merge object list with active
        allobjects = self._objects + list(set(objects) - set(self._objects))

        #Set default names on objects where omitted by user
        #Needs to be updated every time as indices may have changed
        for o in range(len(allobjects)):
            #Default name + idx if no user set name
            obj = allobjects[o]
            if not "name" in obj.properties or obj.properties["name"][0] == '_':
                if obj.properties.get("colourbar"):
                   obj.properties["name"] = '_ColourBar_' + str(o)
                else:
                    obj.properties["name"] = '_' + obj._dr.type[3:] + '_' + str(o)

        # Remove any existing data at current timestep
        if not self.viewonly:
            libUnderworld.gLucifer.lucDatabase_DeleteGeometry(self._db, self.step, self.step)
            self._db.timeStep = self.step

            #Delete all drawing objects in register
            for ii in range(self._db.drawingObject_Register.objects.count,0,-1):
                libUnderworld.StGermain._Stg_ObjectList_RemoveByIndex(self._db.drawingObject_Register.objects,ii-1, libUnderworld.StGermain.KEEP)

            #Add drawing objects to register and output any custom data on them
            for object in allobjects:
                #Hide objects not in this figure
                object._properties["visible"] = object in objects

                #Add the object to the drawing object register for the database
                libUnderworld.StGermain.Stg_ObjectList_Append(self._db.drawingObject_Register.objects,object._cself)

            # go ahead and fill db
            libUnderworld.gLucifer._lucDatabase_Execute(self._db,None)

            #Output any custom geometry on objects
            for object in allobjects:
                self._plotObject(object)

            #Write visualisation state as json data
            libUnderworld.gLucifer.lucDatabase_WriteState(self._db, self._get_state(allobjects, viewprops))

        else:
            #Open db, get state and update it to match active figure
            if not self._db.db:
                libUnderworld.gLucifer.lucDatabase_OpenDatabase(self._db)
            args = [self._lvbin, self._db.path, "-" + str(self.step), "-p0", "-a", "-h"]
            lavavu.execute(args)
            #Get state, includes the list of objects in the loaded database
            statestr = lavavu.getState()
            state = json.loads(statestr)
            if len(objects):
                #For each obj in db, lookup in local list by name, replace properties if found
                #if not found locally, objects in db are hidden from view
                for obj in state["objects"]:
                    obj["visible"] = False
                    for object in objects:
                        if obj["name"] == object._properties["name"]:
                            #Merge/replace
                            obj.update(object._properties)
                            obj["visible"] = True
            else:
                #No objects passed in with figure, simply plot them all
                for obj in state["objects"]:
                    obj["visible"] = True
            lavavu.clear() #Close and free memory
            export = dict()
            #Global properties stored on this object
            state["properties"].update(self._properties)
            #View properties passed from figure
            state["views"][0].update(viewprops)
            #Write updated visualisation state as json data
            libUnderworld.gLucifer.lucDatabase_WriteState(self._db, json.dumps(state, indent=2))

        #Save the active object list
        self._objects = allobjects

    def _get_state(self, objects, viewprops):
        export = dict()
        #Global properties stored on this object
        export["properties"] = self._properties
        #View properties passed from figure
        export["views"] = [viewprops]
        #Objects passed from figure
        objlist = []
        for obj in objects:
            objlist.append(obj._properties)

        export["objects"] = objlist
        #TODO: ColourMap properties

        #print json.dumps(export, indent=2)
        return json.dumps(export, indent=2)

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

    def empty(self):
        """    Empties all the cached drawing objects
        """
        self._objects = []

class Figure():
    """  
    The Figure class provides a window within which gLucifer drawing objects
    may be rendered. It also provides associated routines for image generation
    and visualisation.
    
    In addition to parameter specification below, see property docstrings for
    further information.

    Parameters
    ----------
    store: Store, default=None
        Database to collect visualisation data, this may be shared among figures
        to collect their data into a single file
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
    >>> imgfile = fig.save("test_image")

    Clean up:
    >>> if imgfile: 
    ...     import os; 
    ...     os.remove( imgfile )

    """
    _viewerProc = None

    def __init__(self, store=None, figsize=(640,480), boundingBox=((0,0,0),(0,0,0)), facecolour="white",
                 edgecolour="black", title="", axis=False, quality=1, properties=None, **kwargs):

        #Create a default database just for this figure if none provided
        if store and isinstance(store, Store):
            self.db = store
        else:
            self.db = Store()

        if not isinstance(figsize,tuple):
            raise TypeError("'figsize' object passed in must be of python type 'tuple'")
        self._figsize = figsize

        if boundingBox and not isinstance(boundingBox,tuple):
            raise TypeError("'boundingBox' object passed in must be of type 'tuple'")
        if boundingBox and not isinstance(boundingBox[0],tuple):
            raise TypeError("'boundingBox[0]' object passed in must be of type 'tuple'")
        if boundingBox and not isinstance(boundingBox[1],tuple):
            raise TypeError("'boundingBox[1]' object passed in must be of type 'tuple'")

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
        self._properties = {"title" : title, "axis" : axis, "axislength" : 0.2, "antialias" : True, "background" : facecolour,
            "margin" : 34, "border" : (1 if edgecolour else 0), "bordercolour" : edgecolour, "rulers" : False, "zoomstep" : 0} 
        self._properties["min"] = boundingBox[0]
        self._properties["max"] = boundingBox[1]
        
        if properties and not isinstance(properties,dict):
            raise TypeError("'properties' object passed in must be of python type 'dict'")
        if properties:
            self._properties.update(properties)

        self.draw = objects.Drawing()
        self._drawingObjects = []
        self._script = []

    def __del__(self):
        self.close_viewer()

    def _getProperties(self):
        #Convert properties to string
        return '\n'.join(['%s=%s' % (k,v) for k,v in self._properties.iteritems()]);

    def _setProperties(self, newProps):
        #Update the properties values (merge)
        #values of any existing keys are replaced
        self._properties.update(newProps)

    @property
    def figsize(self):
        """    figsize (tuple(int,int)): size of window in pixels, default: (640,480)
        """
        return self._figsize

    @property
    def facecolour(self):
        """    facecolour : colour of face background, default: white
        """
        return self._properties["background"]

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
    def objects(self):
        """    objects : list of objects to be drawn within the figure.
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
        Shows the generated image inline within an ipython notebook.
        
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
                    #Return inline image result
                    filename = self._generate_image()
                    from IPython.display import Image,HTML
                    return HTML("<img src='%s'>" % filename)
        except ImportError:
            pass
        except RuntimeError, e:
            print "Error creating image: "
            print e
            pass
        except:
            raise

    def save_image(self, filename, size=(0,0)):
        # For back compatibility
        return self.save(filename, size)

    def save_database(self, filename, regen=True):
        # For back compatibility
        if regen:
            self._generate_DB()
        return self.db.save(filename)

    def save(self, filename, size=(0,0)):
        """  
        Saves the generated image to the provided filename.
        
        Parameters
        ----------
        filename :str
            Filename to save file to.  May include an absolute or relative path.
            size (tuple(int,int)): size of image in pixels, defaults to original figsize setting
            
        Returns
        -------
        filename: str
            The final filename (including extension) used to save the image will be returned. Note
            that only the root process will return this filename. All other processes will not return
            anything.
        """
        if not isinstance(filename, str):
            raise TypeError("Provided parameter 'filename' must be of type 'str'. ")
        if size and not isinstance(size,tuple):
            raise TypeError("'size' object passed in must be of python type 'tuple'")

        self._generate_DB()
        return self._generate_image(filename, size)

    def _generate_DB(self):
        objects = self._drawingObjects[:]
        self._properties["resolution"] = self._figsize
        #Only add default object if used
        if len(self.draw.vertices) > 0:
            objects.append(self.draw)
        self.db._generate(objects, self._properties)

    def _generate_image(self, filename="", size=(0,0)):
        if haveLavaVu and uw.rank() == 0:
            #Render with viewer
            args = [self.db._lvbin, self.db._db.path, "-" + str(self.db.step), "-p0", "-z" + str(self.quality), "-a", "-h", ":"] + self._script
            lavavu.execute(args)
            imagestr = lavavu.image(filename, size[0], size[1])
            lavavu.clear() #Close and free memory
            #Return the generated filename
            return imagestr

    def _generate_HTML(self):
        if haveLavaVu and uw.rank() == 0:
            #Export encoded json string
            jsonstr = lavavu.execute([self.db._lvbin, "-" + str(self.db.step), "-U", "-p0", self.db._db.path, ":"] + self._script)
            if not os.path.isdir("html"):
                #Create link to web content directory
                os.symlink(self.db._lvpath + 'html', 'html')
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

    def open_viewer(self, args=[], background=True):
        """ Open the viewer.
        """
        if haveLavaVu and uw.rank() == 0:
            fname = self.db.filename
            if not fname:
                fname = os.path.join(tmpdir,"gluciferDB"+self.db._id+".gldb")
                self.db.save(fname)
            if self._viewerProc and self._viewerProc.poll() == None:
                return

            #Open viewer with local web server for interactive/iterative use
            args = [self.db._lvbin, "-" + str(self.db.step), "-L", "-p8080", "-q90", fname] + args
            if background:
                self._viewerProc = subprocess.Popen(args, stdout=PIPE, stdin=PIPE, stderr=STDOUT)
                from IPython.display import HTML
                return HTML('''<a href='#' onclick='window.open("http://" + location.hostname + ":8080");'>Open Viewer Interface</a>''')
            else:
                lavavu.execute(args)

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
  
    def send_command(self, cmd, retry=True):
        """ 
        Run command on an open viewer instance.
        
        Parameters
        ----------
        cmd: str
            Command to send to open viewer.
        """
        if haveLavaVu and uw.rank() == 0:
            self.open_viewer()
            url = "http://localhost:8080/command=" + urllib2.quote(cmd)
            try:
                #print url
                response = urllib2.urlopen(url).read()
                #print response
            except:
                print "Send command '" + cmd + "' failed, no response"
                if retry:
                    #Wait a few seconds so server has time to start then try again
                    print "... retrying in 1s ..."
                    time.sleep(1)
                    self.send_command(cmd, False)
                else:
                    print "... failed, skipping ..."
                    pass

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
