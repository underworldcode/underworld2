##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##

import underworld as _underworld
import underworld._stgermain as _stgermain
import underworld.swarm as _swarmMod
import underworld.mesh as _uwmesh
from underworld.function import Function as _Function
import libUnderworld as _libUnderworld
import numpy

#TODO: Drawing Objects to implement
# HistoricalSwarmTrajectory
#
# Maybe later...
# SwarmShapes, SwarmRGB, SwarmVectors
# EigenVectors, EigenVectorCrossSection

class ColourMap(_stgermain.StgCompoundComponent):
    """
    The ColourMap class provides functionality for mapping colours to numerical
    values.
    
    Parameters
    ----------
    colours: str, list
        List of colours to use for drawing object colour map. Provided as a string
        or as a list of strings. Example, "red blue", or ["red", "blue"]
    valueRange: tuple, list
        User defined value range to apply to colour map. Provided as a 
        tuple of floats  (minValue, maxValue). If none is provided, the
        value range will be determined automatically.
    logScale: bool
        Bool to determine if the colourMap should use a logarithmic scale.
    discrete: bool
        Bool to determine if a discrete colour map should be used.
        Discrete colour maps do not interpolate between colours and instead
        use nearest neighbour for colouring.

    """
    _selfObjectName = "_cm"
    _objectsDict = { "_cm": "lucColourMap" }
    
    def __init__(self, colours="diverge", valueRange=None, logScale=False, discrete=False, **kwargs):
        if not hasattr(self, "properties"):
            self.properties = {}

        if not isinstance(colours,(str,list)):
            raise TypeError("'colours' object passed in must be of python type 'str' or 'list'")
        if isinstance(colours,(list)):
            self.properties.update({"colours" : ' '.join(colours)})
        else:
            self.properties.update({"colours" : colours})

        #User-defined props in kwargs
        self.properties.update(kwargs)
        dict((k.lower(), v) for k, v in self.properties.items())

        if valueRange != None:
            # is valueRange correctly defined, ie list of length 2 made of numbers
            if not isinstance( valueRange, (list,tuple)):
                raise TypeError("'valueRange' must be of type 'list' or 'tuple'")
            if len(valueRange) != 2:
                raise ValueError("'valueRange' must have 2 real values")
            for item in valueRange:
                if not isinstance( item, (int, float) ):
                    raise TypeError("'valueRange' must contain real numbers")
            if not valueRange[0] < valueRange[1]:
                raise ValueError("The first number of the valueRange list must be smaller than the second number")

            # valueRange arg is good 
            self.properties.update({"range" : [valueRange[0], valueRange[1]]})
        else:
            self.properties.update({"range" : [0.0, 0.0]}) # ignored

        if not isinstance(logScale, bool):
            raise TypeError("'logScale' parameter must be of 'bool' type.")
        self._logScale = logScale
        self.properties.update({"logscale" : logScale})

        if not isinstance(discrete, bool):
            raise TypeError("'discrete' parameter must be of 'bool' type.")
        self.properties.update({"discrete" : discrete})

        # build parent
        super(ColourMap,self).__init__()

    def _add_to_stg_dict(self,componentDictionary):

        # call parents method
        super(ColourMap,self)._add_to_stg_dict(componentDictionary)

    #dict methods
    def update(self, newdict):
        self.properties.update(newdict)

    def __getitem__(self, key):
        return self.properties[key]

    def __setitem__(self, key, item):
        self.properties[key] = item

    def _getProperties(self):
        #Convert properties to string
        return '\n'.join(['%s=%s' % (k,v) for k,v in self.properties.items()]);

class Drawing(_stgermain.StgCompoundComponent):
    """
    This is the base class for all drawing objects but can also be instantiated 
    as is for direct/custom drawing.
    
    Note that the defaults here are often overridden by the child objects.
    
    Parameters
    ----------
    colours: str, list.
        See ColourMap class docstring for further information
    colourMap: glucifer.objects.ColourMap
        A ColourMap object for the object to use.
        This should not be specified if 'colours' is specified.
    opacity: float
        Opacity of object. If provided, must take values from 0. to 1.
    colourBar: bool
        Bool to determine if a colour bar should be rendered.
    valueRange: tuple, list
        See ColourMap class docstring for further information
    logScale: bool
        See ColourMap class docstring for further information
    discrete: bool
        See ColourMap class docstring for further information

        
    """
    _selfObjectName = "_dr"
    _objectsDict = { "_dr": "lucDrawingObject" } # child should replace _dr with own derived type
    
    def __init__(self, name=None, colours=None, colourMap="", colourBar=False,
                       valueRange=None, logScale=False, discrete=False,
                       *args, **kwargs):

        if not hasattr(self, "properties"):
            self.properties = {}

        if colours and colourMap:
            raise RuntimeError("You should specify 'colours' or a 'colourMap', but not both.")
        if colourMap:
            self._colourMap = colourMap
        elif colours:
            self._colourMap = ColourMap(colours=colours, valueRange=valueRange, logScale=logScale)
        elif colourMap is not None:
            self._colourMap = ColourMap(valueRange=valueRange, logScale=logScale)
        else:
            self._colourMap = None

        if not isinstance(discrete, bool):
            raise TypeError("'discrete' parameter must be of 'bool' type.")
        if discrete and self._colourMap:
            self._colourMap["discrete"] = True

        #User-defined props in kwargs
        self.properties.update(kwargs)
        dict((k.lower(), v) for k, v in self.properties.items())

        if not isinstance(colourBar, bool):
            raise TypeError("'colourBar' parameter must be of 'bool' type.")
        self._colourBar = None
        if colourBar and self._colourMap:
            #Create the associated colour bar
            self._colourBar = ColourBar(colourMap=self._colourMap)

        if name:
            self.properties["name"] = str(name)

        self.resetDrawing()

        # build parent
        super(Drawing,self).__init__(*args)

    def _add_to_stg_dict(self,componentDictionary):
        
        # call parents method
        super(Drawing,self)._add_to_stg_dict(componentDictionary)

        # add an empty(ish) drawing object.  children should fill it out.
        componentDictionary[self._dr.name].update( {
            "properties"    :self._getProperties(),
            "ColourMap"     :self._colourMap._cm.name if self._colourMap else None
        } )

    #dict methods
    def update(self, newdict):
        self.properties.update(newdict)

    def __getitem__(self, key):
        return self.properties[key]

    def __setitem__(self, key, item):
        self.properties[key] = item

    def _getProperties(self):
        #Convert properties to string
        return '\n'.join(['%s=%s' % (k,v) for k,v in self.properties.items()]);

    def render(self, viewer):
        #Place any custom geometry output in this method, called after database creation

        #General purpose plotting via LavaVu
        #Plot all custom data drawn on provided object
        try:
            obj = viewer.objects[self.properties["name"]]
            if not obj: raise KeyError("Object not found")
        except KeyError as e:
            print(self.properties["name"] + " Object lookup error: " + str(e))
            return

        obj["geometry"] = self.geomType

        output = False
        if len(self.vertices):
            obj.vertices(self.vertices)
            output = True
        if len(self.vectors):
            obj.vectors(self.vectors)
            output = True
        if len(self.scalars):
            obj.values(self.scalars)
            output = True
        if len(self.labels):
            obj.labels(self.labels)
            output = True

        #Update the database
        if output:
            viewer.app.update(obj.ref, True)

    def resetDrawing(self):
        #Clear direct drawing data
        self.vertices = []
        self.vectors = []
        self.scalars = []
        self.labels = []
        self.geomType = None

    #Direct drawing methods
    def label(self, text, pos=(0.,0.,0.), font="sans", scaling=1):
        """  
        Writes a label string
        
        Parameters
        ----------
        text: str
            label text.
        pos: tuple
            X,Y,Z position to place the label.
        font : str
            label font (small/fixed/sans/serif/vector).
        scaling : float
            label font scaling (for "vector" font only).
        """
        self.geomType = "labels"
        self.vertices.append(pos)
        self.labels.append(text)
        self.properties.update({"font" : font, "fontscale" : scaling}) #Merge

    def point(self, pos=(0.,0.,0.)):
        """  
        Draws a point
            
        Parameters
        ----------
        pos : tuple
            X,Y,Z position to place the point
        """
        self.geomType = "points"
        self.vertices.append(pos)

    def line(self, start=(0.,0.,0.), end=(0.,0.,0.)):
        """  
        Draws a line
            
        Parameters
        ----------
        start : tuple
            X,Y,Z position to start line
        end : tuple
            X,Y,Z position to end line
        """
        self.geomType = "lines"
        self.vertices.append(start)
        self.vertices.append(end)

    def vector(self, position=(0.,0.,0.), vector=(0.,0.,0.)):
        """  
        Draws a vector
            
        Parameters
        ----------
        position : tuple
            X,Y,Z position to centre vector on
        vector : tuple
            X,Y,Z vector value
        """
        self.geomType = "vectors"
        self.vertices.append(position)
        self.vectors.append(vector)


    @property
    def colourBar(self):
        """    
        colourBar (object): return colour bar of drawing object, create if
        doesn't yet exist.
        """
        if not self._colourBar:
            self._colourBar = ColourBar(colourMap=self._colourMap)
        return self._colourBar

    @property
    def colourMap(self):
        """
        colourMap (object): return colour map of drawing object
        """
        return self._colourMap


    def getdata(self, viewer, typename):
        #Experimental: grabbing data from a LavaVu object
        alldata = []
        obj = viewer.objects[self.properties["name"]]
        if obj:
            #Force viewer open to trigger surface optimisation etc
            viewer.app.resetViews()
            #Get data elements list
            dataset = obj.data()
            for geom in dataset:
                #Grab a copy of the data
                data = geom.copy(typename)
                alldata.append(data)

        return alldata


class ColourBar(Drawing):
    """
    The ColourBar drawing object draws a colour bar for the provided colour map.
    
    Parameters
    ----------
    colourMap: glucifer.objects.ColourMap
        Colour map for which the colour bar will be drawn.
    """

    def __init__(self, colourMap, *args, **kwargs):
        #Default properties
        self.properties = {"colourbar" : 1}
    
        # build parent
        super(ColourBar,self).__init__(colourMap=colourMap, *args, **kwargs)

class CrossSection(Drawing):
    """  
    This drawing object class defines a cross-section plane, derived classes 
    plot data over this cross section
    
    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    mesh : underworld.mesh.FeMesh
        Mesh over which cross section is rendered.
    fn : underworld.function.Function
        Function used to determine values to render.
    crossSection : str
        Cross Section definition, eg. z=0.
    resolution : list(unsigned)
        Surface sampling resolution.
    onMesh : boolean
        Sample the mesh nodes directly, as opposed to sampling across a regular grid. This flag
        should be used in particular where a mesh has been deformed.
        
    """
    _objectsDict = { "_dr": "lucCrossSection" }

    def __init__(self, mesh, fn, crossSection="", resolution=[100,100,1],
                       colourBar=True,
                       offsetEdges=None, onMesh=False,
                       *args, **kwargs):

        self._onMesh = onMesh

        self._fn = _underworld.function.Function.convert(fn)
        
        if not isinstance(mesh,_uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        if not isinstance(crossSection,str):
            raise ValueError("'crossSection' parameter must be of python type 'str'")
        self._crossSection = crossSection
        self._offsetEdges = offsetEdges
        
        if not isinstance( resolution, (list,tuple)):
            if isinstance(resolution,int):
                resolution = [resolution,resolution,1]
            else:
                raise TypeError("'resolution' passed in must be of type 'int', 'list' or 'tuple'")
        for el in resolution:
            if not isinstance(el,int):
                raise TypeError("'resolution' elements must be of python type 'int'")
            if el < 1:
                raise ValueError("'resolution' elements must be greater than zero")
        self._resolution = resolution

        # build parent
        super(CrossSection,self).__init__(colourBar=colourBar, *args, **kwargs)

    def _setup(self):
        _libUnderworld.gLucifer._lucCrossSection_SetFn( self._cself, self._fn._fncself )
        if self._offsetEdges != None:
            self._dr.offsetEdges = self._offsetEdges

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(CrossSection,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {
                   "Mesh": self._mesh._cself.name,
                   "crossSection": self._crossSection,
                   "resolutionA" : self._resolution[0],
                   "resolutionB" : self._resolution[1],
                   "onMesh" : self._onMesh
            } )

    @property
    def crossSection(self):
        """    crossSection (str): Cross Section definition, eg;: z=0.
        """
        return self._crossSection

class Surface(CrossSection):
    """  
    This drawing object class draws a surface using the provided scalar field.

    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    mesh : underworld.mesh.FeMesh
        Mesh over which cross section is rendered.
    fn : underworld.function.Function
        Function used to determine values to render.
    drawSides : str
        Sides (x,y,z,X,Y,Z) for which the surface should be drawn. 
        For example, "xyzXYZ" would render the provided function across
        all surfaces of the domain in 3D. In 2D, this object always renders
        across the entire domain.
    """
    
    _objectsDict = {  "_dr"  : "lucScalarField" }

    def __init__(self, mesh, fn, drawSides="xyzXYZ",
                       colourBar=True,
                       *args, **kwargs):

        if not isinstance(drawSides,str):
            raise ValueError("'drawSides' parameter must be of python type 'str'")
        self._drawSides = drawSides

        # build parent
        super(Surface,self).__init__( mesh=mesh, fn=fn, colourBar=colourBar, *args, **kwargs)

        #Merge with default properties
        is3d = len(self._crossSection) == 0
        defaults = {"cullface" : is3d, "lit" : is3d}
        defaults.update(self.properties)
        self.properties = defaults

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Surface,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name]["drawSides"] = self._drawSides

    def _setup(self):
        _libUnderworld.gLucifer._lucCrossSection_SetFn( self._cself, self._fn._fncself )

    def __del__(self):
        super(Surface,self).__del__()

class Contours(CrossSection):
    """  
    This drawing object class draws contour lines in a cross section using the provided scalar field.

    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    mesh : underworld.mesh.FeMesh
        Mesh over which cross section is rendered.
    fn : underworld.function.Function
        Function used to determine values to render.
    labelFormat: str
        Format string (printf style) used to print a contour label, eg: " %g K"
    unitScaling:
        Scaling factor to apply to value when printing labels
    interval: float
        Interval between contour lines
    limits: tuple, list
        User defined minimum and maximum limits for the contours. Provided as a 
        tuple/list of floats  (minValue, maxValue). If none is provided, the
        limits will be determined automatically.

    """
    
    _objectsDict = {  "_dr"  : "lucContourCrossSection" }

    def __init__(self, mesh, fn, labelFormat="", unitScaling=1.0, interval=0.33, limits=(0.0, 0.0),
                       *args, **kwargs):

        if not isinstance(labelFormat,str):
            raise ValueError("'labelFormat' parameter must be of python type 'str'")
        self._labelFormat = labelFormat

        if not isinstance( unitScaling, (int, float) ):
            raise TypeError("'unitScaling' must contain a number")
        self._unitScaling = unitScaling

        if not isinstance( interval, (int, float) ):
            raise TypeError("'interval' must contain a number")
        self._interval = interval

        # is limits correctly defined, ie list of length 2 made of numbers
        if not isinstance( limits, (list,tuple)):
            raise TypeError("'limits' must be of type 'list' or 'tuple'")
        if len(limits) != 2:
            raise ValueError("'limits' must have 2 real values")
        for item in limits:
            if not isinstance( item, (int, float) ):
                raise TypeError("'limits' must contain real numbers")
        if not limits[0] <= limits[1]:
            raise ValueError("The first number of the limits list must be smaller than the second number")
        self._limits = limits

        # build parent
        super(Contours,self).__init__( mesh=mesh, fn=fn, *args, **kwargs)

        #Default properties
        self.properties.update({"lit" : False})

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Contours,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name]["unitScaling"] = self._unitScaling
        componentDictionary[self._dr.name][   "interval"] = self._interval
        componentDictionary[self._dr.name]["minIsovalue"] = self._limits[0]
        componentDictionary[self._dr.name]["maxIsovalue"] = self._limits[1]

    def _setup(self):
        #Override dictionary setting, it seems to left-trim strings
        self._dr.labelFormat = self._labelFormat
        _libUnderworld.gLucifer._lucCrossSection_SetFn( self._cself, self._fn._fncself )

    def __del__(self):
        super(Contours,self).__del__()


class Points(Drawing):
    """  
    This drawing object class draws a swarm of points.
    
    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    swarm : underworld.swarm.Swarm
        Swarm which provides locations for point rendering.
    fn_colour : underworld.function.Function
        Function used to determine colour to render particle.
        This function should return float/double values.
    fn_mask : underworld.function.Function
        Function used to determine if a particle should be rendered. 
        This function should return bool values. 
    fn_size : underworld.function.Function
        Function used to determine size to render particle.
        This function should return float/double values.
        

    """
    _objectsDict = { "_dr": "lucSwarmViewer" }

    def __init__(self, swarm, fn_colour=None, fn_mask=None, fn_size=None, colourVariable=None,
                       colourBar=True, *args, **kwargs):

        if not isinstance(swarm,_swarmMod.Swarm):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = swarm

        self._fn_colour = None
        if fn_colour != None:
           self._fn_colour = _underworld.function.Function.convert(fn_colour)
        else:
           colourBar = False
        self._fn_mask = None
        if fn_mask != None:
           self._fn_mask = _underworld.function.Function.convert(fn_mask)
        self._fn_size = None
        if fn_size != None:
           self._fn_size = _underworld.function.Function.convert(fn_size)

        # build parent
        super(Points,self).__init__(colourBar=colourBar, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        super(Points,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][ "Swarm" ] = self._swarm._cself.name
        
    def _setup(self):
        fnc_ptr = None
        if self._fn_colour:
            fnc_ptr = self._fn_colour._fncself
        fnm_ptr = None
        if self._fn_mask:
            fnm_ptr = self._fn_mask._fncself
        fns_ptr = None
        if self._fn_size:
            fns_ptr = self._fn_size._fncself

        _libUnderworld.gLucifer._lucSwarmViewer_SetFn( self._cself, fnc_ptr, fnm_ptr, fns_ptr, None )


class _GridSampler3D(CrossSection):
    """  This drawing object class samples a regular grid in 3D.

    resolution : list(unsigned)
        Number of samples in the I,J,K directions.
    resolutionI : unsigned
        DEPRECATED: Number of samples in the I direction.
    resolutionJ : unsigned
        DEPRECATED: Number of samples in the J direction.
    resolutionK : unsigned
        DEPRECATED: Number of samples in the K direction.
    """
    _objectsDict = { "_dr": None } #Abstract class, Set by child

    def __init__(self, resolution=[16,16,16], resolutionI=None, resolutionJ=None, resolutionK=None, *args, **kwargs):

        #Convert deprecated parameters
        if resolutionI:
            print("Parameter 'resolutionI' is deprecated, please use resolution=[I,J,K]")
            if not isinstance(resolutionI,int):
                raise TypeError("'resolutionI' must be of python type 'int'")
            resolution[0] = resolutionI
        if resolutionJ:
            print("Parameter 'resolutionJ' is deprecated, please use resolution=[I,J,K]")
            if not isinstance(resolutionJ,int):
                raise TypeError("'resolutionJ' must be of python type 'int'")
            resolution[1] = resolutionJ
        if resolutionK:
            print("Parameter 'resolutionK' is deprecated, please use resolution=[I,J,K]")
            if not isinstance(resolutionK,int):
                raise TypeError("'resolutionK' must be of python type 'int'")
            resolution[2] = resolutionK

        self._resolution = resolution

        # build parent
        super(_GridSampler3D,self).__init__(resolution=resolution, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(_GridSampler3D,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {
            "resolutionX": self._resolution[0],
            "resolutionY": self._resolution[1],
            "resolutionZ": self._resolution[2]
            } )


class VectorArrows(_GridSampler3D):
    """  
    This drawing object class draws vector arrows corresponding to the provided vector field.

    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    mesh : underworld.mesh.FeMesh
        Mesh over which vector arrows are rendered.
    fn : underworld.function.Function
        Function used to determine vectors to render. 
        Function should return a vector of floats/doubles of appropriate
        dimensionality.
    arrowHead : float
         The size of the head of the arrow.
         If > 1.0 is ratio to arrow radius
         If in range [0.,1.] is ratio to arrow length
    scaling : float
        Scaling for entire arrow.
    autoscale : bool
        Scaling based on field min/max
    glyphs : int
        Type of glyph to render for vector arrow.
        0: Line, 1 or more: 3d arrow, higher number => better quality.
    resolution : list(unsigned)
        Number of samples in the I,J,K directions.

    """
    _objectsDict = { "_dr": "lucVectorArrows" }

    def __init__(self, mesh, fn, resolution=[16, 16, 16], autoscale=True, *args, **kwargs):

        self._autoscale = autoscale

        # build parent
        super(VectorArrows,self).__init__( mesh=mesh, fn=fn, resolution=resolution,
                        colours=None, colourMap=None, colourBar=False, autoscale=autoscale, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(VectorArrows,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {
            "dynamicRange": self._autoscale
            } )

class Volume(_GridSampler3D):
    """  
    This drawing object class draws a volume using the provided scalar field.
    
    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    mesh : underworld.mesh.FeMesh
        Mesh over which object is rendered.
    fn : underworld.function.Function
        Function used to determine colour values.
        Function should return a vector of floats/doubles of appropriate
        dimensionality.
    resolution : list(unsigned)
        Number of samples in the I,J,K directions.

    """
    _objectsDict = { "_dr": "lucFieldSampler" }

    def __init__(self, mesh, fn, 
                       resolution=[64,64,64], 
                       colourBar=True,
                       *args, **kwargs):

        # build parent
        if mesh.dim == 2:
            raise ValueError("Volume rendering requires a three dimensional mesh.")
        super(Volume,self).__init__( mesh=mesh, fn=fn, resolution=resolution,
                                     colourBar=colourBar, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(Volume,self)._add_to_stg_dict(componentDictionary)

class Sampler(Drawing):
    """
    The Sampler class provides functionality for sampling a field at
    a number of provided vertices.
    
    Parameters
    ----------
    vertices: list,array
        List of vertices to sample the field at, either a list or numpy array
    mesh : underworld.mesh.FeMesh
        Mesh over which the values are sampled
    fn : underworld.function.Function
        Function used to get the sampled values.

    """

    _objectsDict = { "_dr": "lucSampler" }
    
    def __init__(self, mesh, fn, *args, **kwargs):

        if not isinstance(mesh,_uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        self._fn = None
        if fn != None:
           self._fn = _underworld.function.Function.convert(fn)

        # build parent
        super(Sampler,self).__init__(*args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):

        # call parents method
        super(Sampler,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name]["Mesh"] = self._mesh._cself.name

    def _setup(self):
        fnc_ptr = None
        if self._fn:
            fn_ptr = self._fn._fncself

        _libUnderworld.gLucifer._lucSampler_SetFn( self._cself, fn_ptr )

    def sample(self, vertices):
        sz = len(vertices)/3*self._cself.fieldComponentCount
        values = numpy.zeros(sz, dtype='float32')
        _libUnderworld.gLucifer.lucSampler_SampleField( self._cself, vertices, values)
        return values


class IsoSurface(Volume):
    """  
    This drawing object class draws an isosurface using the provided scalar field.
    
    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    mesh : underworld.mesh.FeMesh
        Mesh over which object is rendered.
    fn : underworld.function.Function
        Function used to determine surface position.
        Function should return a vector of floats/doubles.
    fn_colour : underworld.function.Function
        Function used to determine colour of surface.
    resolution : list(unsigned)
        Number of samples in the I,J,K directions.
    isovalue : float
        Isovalue to plot.
    isovalues : list of float
        List of multiple isovalues to plot.

    """

    def __init__(self, mesh, fn, fn_colour=None, resolution=[64,64,64],
                       colourBar=True, isovalue=None, *args, **kwargs):

        # build parent
        if mesh.dim == 2:
            raise ValueError("Isosurface requires a three dimensional mesh.")

        # Validate isovalue(s) params
        if not "isovalues" in kwargs:
            if isovalue is None:
                raise ValueError("Isosurface requires either 'isovalue' value or 'isovalues' list parameter.")
            kwargs["isovalues"] = [isovalue]

        self._sampler = None
        if fn_colour != None:
           self._sampler = Sampler(mesh, fn_colour)

        super(IsoSurface,self).__init__( mesh=mesh, fn=fn, resolution=resolution,
                                         colourBar=colourBar, *args, **kwargs)
        self.geomType = "triangles"

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(IsoSurface,self)._add_to_stg_dict(componentDictionary)

    def _setup(self):
        if self._sampler:
            self._sampler._setup()

    def render(self, viewer):
        # FieldSampler has exported a 3d volume to the database,
        # now we can use LavaVu to generate isosurface triangles
        isobj = viewer.objects[self.properties["name"]]
        if isobj:
            #Force viewer open to trigger surface optimisation
            viewer.app.resetViews()
            #Generate isosurface in same object, convert and delete volume, update db
            isobj.isosurface(name=None, convert=True, updatedb=True)
        else:
            print("Object not found: " + self.properties["name"])

    def parallel_render(self, viewer, rank):
        #If this method defined, is run by all procs to process
        # any render output that must be done in parallel
        isobj = viewer.objects[self.properties["name"]]
        if isobj and self._sampler:
            #If coloured by another field, get the vertices, sample and load values
            #Clear existing values
            isobj.cleardata()
            #Get data elements list
            dataset = isobj.data()
            for geom in dataset:
                #Grab a view of the vertex data
                verts = geom.get("vertices")
                if len(verts):
                    #Sample over tri vertices
                    values = self._sampler.sample(verts)
                    #Update data on root
                    if rank == 0:
                        #Update element with the sampled data values
                        geom.set("sampledfield", values)

            #Write the colour data back to db on root
            if rank == 0:
                isobj.update("triangles")

class Mesh(Drawing):
    """  
    This drawing object class draws a mesh.
    
    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ----------
    mesh : underworld.mesh.FeMesh
        Mesh to render.
    nodeNumbers : bool
        Bool to determine whether global node numbers should be rendered. 
    segmentsPerEdge : unsigned
        Number of segments to render per cell/element edge. For higher 
        order mesh, more segments are useful to render mesh curvature correctly.

    """
    _objectsDict = { "_dr": "lucMeshViewer" }

    def __init__( self, mesh, nodeNumbers=False, segmentsPerEdge=1, *args, **kwargs ):

        if not isinstance(mesh,_uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        if not isinstance(nodeNumbers,bool):
            raise TypeError("'nodeNumbers' flag must be of type 'bool'")
        self._nodeNumbers = nodeNumbers

        if not isinstance(segmentsPerEdge,int) or segmentsPerEdge < 1:
            raise TypeError("'segmentsPerEdge' must be a positive 'int'")
        self._segmentsPerEdge = segmentsPerEdge

        #Default properties
        self.properties = {"linesmooth" : False, "lit" : False, "font" : "small", "fontscale" : 0.5,
                           "pointsize" : 5 if self._nodeNumbers else 1, 
                           "pointtype" : 2 if self._nodeNumbers else 4}
        
        # build parent
        super(Mesh,self).__init__( colourMap=None, colourBar=False, *args, **kwargs )

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Mesh,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name][       "Mesh"] = self._mesh._cself.name
        componentDictionary[self._dr.name]["nodeNumbers"] = self._nodeNumbers
        componentDictionary[self._dr.name][   "segments"] = self._segmentsPerEdge
