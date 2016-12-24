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

#TODO: Drawing Objects to implement
# IsoSurface, IsoSurfaceCrossSection
# MeshSurface/MeshSampler (surface/volumes using MeshCrossSection sampler)
# Contour, ContourCrossSection
# HistoricalSwarmTrajectory
# VectorArrowMeshCrossSection?
#
# Maybe later...
# TextureMap
# SwarmShapes, SwarmRGB, SwarmVectors
# EigenVectors, EigenVectorCrossSection
# FeVariableSurface

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

class ColourMap(_stgermain.StgCompoundComponent):
    """
    The ColourMap class provides functionality for mapping colours to numerical
    values.
    
    Parameters
    ----------
    colours: str, list.  default="#288FD0 #50B6B8 #989878 #C68838 #FF7520"
        List of colours to use for drawing object colour map. Provided as a string
        or as a list of strings. Example, "red blue", or ["red", "blue"]
        This should not be specified if 'colourMap' is specified.
    valueRange: tuple,list. default=None.
        User defined value range to apply to colour map. Provided as a 
        tuple of floats  (minValue, maxValue). If none is provided, the
        value range will be determined automatically.
    logScale: bool. default=False.
        Bool to determine if the colourMap should use a logarithmic scale.
    discrete: bool.  default=False.
        Bool to determine if a discrete colour map should be used.
        Discrete colour maps do not interpolate between colours and instead
        use nearest neighbour for colouring.

    """
    _selfObjectName = "_cm"
    _objectsDict = { "_cm": "lucColourMap" }
    
    #Default is a cool-warm map with low variance in luminosity/lightness
    def __init__(self, colours=None, valueRange=None, logScale=False, discrete=False, **kwargs):

        if not hasattr(self, "properties"):
            self.properties = {}

        if colours == None:
            colours = colourMaps["diverge"]
        if not isinstance(colours,(str,list)):
            raise TypeError("'colours' object passed in must be of python type 'str' or 'list'")
        if isinstance(colours,(list)):
            self.properties.update({"colours" : ' '.join(colours)})
        else:
            self.properties.update({"colours" : colours})

        #User-defined props in kwargs
        self.properties.update(kwargs)
        dict((k.lower(), v) for k, v in self.properties.iteritems())

        if valueRange != None:
            # is valueRange correctly defined, ie list of length 2 made of numbers
            if not isinstance( valueRange, (list,tuple)):
                raise TypeError("'valueRange' objected passed in must be of type 'list' or 'tuple'")
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
        return '\n'.join(['%s=%s' % (k,v) for k,v in self.properties.iteritems()]);

class Drawing(_stgermain.StgCompoundComponent):
    """
    This is the base class for all drawing objects but can also be instantiated 
    as is for direct/custom drawing.
    
    Note that the defaults here are often overridden by the child objects. Please
    inspect child constructor (__init__) itself to determine exact default settings.
    
    Parameters
    ----------
    colours: str, list.
        See ColourMap class docstring for further information
    colourMap: ColourMap. default=None
        A ColourMap object for the object to use.
        This should not be specified if 'colours' is specified.
    opacity: float. default=None.
        Opacity of object. If provided, must take values from 0. to 1. 
    colourBar: bool. default=False
        Bool to determine if a colour bar should be rendered.
    valueRange: tuple,list. default=None.
        See ColourMap class docstring for further information
    logScale: bool. default=False.
        See ColourMap class docstring for further information
    discrete: bool.  default=False.
        See ColourMap class docstring for further information

        
    """
    _selfObjectName = "_dr"
    _objectsDict = { "_dr": "lucDrawingObject" } # child should replace _dr with own derived type
    
    def __init__(self, name=None, colours=None, colourMap=None, colourBar=False,
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
        else:
            self._colourMap = ColourMap(valueRange=valueRange, logScale=logScale)

        #User-defined props in kwargs
        self.properties.update(kwargs)
        dict((k.lower(), v) for k, v in self.properties.iteritems())

        if not isinstance(colourBar, bool):
            raise TypeError("'colourBar' parameter must be of 'bool' type.")
        self._colourBar = None
        if colourBar:
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
            "ColourMap"     :self._colourMap._cm.name
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
        return '\n'.join(['%s=%s' % (k,v) for k,v in self.properties.iteritems()]);

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
        self.geomType = _libUnderworld.gLucifer.lucLabelType
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
        self.geomType = _libUnderworld.gLucifer.lucPointType
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
        self.geomType = _libUnderworld.gLucifer.lucLineType
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
        self.geomType = _libUnderworld.gLucifer.lucVectorType
        self.vertices.append(position)
        self.vectors.append(vector)


    @property
    def colourBar(self):
        """    
        colourBar (dict): return colour bar of drawing object, create if 
        doesn't yet exist.
        """
        if not self._colourBar:
            self._colourBar = ColourBar(colourMap=self._colourMap)
        return self._colourBar

class ColourBar(Drawing):
    """
    The ColourBar drawing object draws a colour bar for the provided colour map.
    
    Parameters
    ----------
    colourMap: ColourMap
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
    mesh : uw.mesh.Mesh
        Mesh over which cross section is rendered.
    fn : uw.function.Function
        Function used to determine values to render.
    crossSection : str, default=""
        Cross Section definition, eg. z=0.
    resolution : unsigned, default=100
        Surface rendered sampling resolution.
        
    """
    _objectsDict = { "_dr": "lucCrossSection" }

    def __init__(self, mesh, fn, crossSection="", resolution=100,
                       colours=None, colourMap=None, colourBar=True,
                       valueRange=None, logScale=False, discrete=False, offsetEdges=None,
                       *args, **kwargs):

        self._fn = _underworld.function.Function.convert(fn)
        
        if not isinstance(mesh,_uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        if not isinstance(crossSection,str):
            raise ValueError("'crossSection' parameter must be of python type 'str'")
        self._crossSection = crossSection
        self._offsetEdges = offsetEdges
        
        if not isinstance(resolution,int):
            raise TypeError("'resolution' parameter must be of python type 'int'")
        if resolution < 1:
            raise ValueError("'resolution' parameter must be greater than zero")
        self._resolution = resolution

        # build parent
        super(CrossSection,self).__init__(colours=colours, colourMap=colourMap, colourBar=colourBar,
                       valueRange=valueRange, logScale=logScale, discrete=discrete, *args, **kwargs)

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
                   "resolution" : self._resolution
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
    mesh : uw.mesh.Mesh
        Mesh over which cross section is rendered.
    fn : uw.function.Function
        Function used to determine values to render.
    drawSides : str, default="xyzXYZ"
        Sides (x,y,z,X,Y,Z) for which the surface should be drawn.  
    drawOnMesh : bool, default=False.
        Note that this option is current disabled.
        Bool to determine whether the surface rendering should explicitly
        use the mesh object to generate the rendered surface. This may 
        result in better quality rendering for deformed mesh, and may 
        also be faster.
    """
    
    # let's just build both objects because we aint sure yet which one we want to use yet
    _objectsDict = {  "_dr"  : "lucScalarField",
                      "_dr2" : "lucScalarFieldOnMesh" }

    def __init__(self, mesh, fn, drawSides="xyzXYZ", drawOnMesh=False,
                       colours=None, colourMap=None, colourBar=True,
                       valueRange=None, logScale=False, discrete=False,
                       *args, **kwargs):

        if drawOnMesh:
            raise RuntimeError("The 'drawOnMesh' option is currently disabled.")
        if not isinstance(drawSides,str):
            raise ValueError("'drawSides' parameter must be of python type 'str'")
        self._drawSides = drawSides

        # if we wish to draw on mesh, switch live object
        if not isinstance(drawOnMesh, bool):
            raise TypeError("'drawOnMesh parameter must be of type 'bool'.")
        self._drawOnMesh = drawOnMesh


        #Default properties
        self.properties = {"cullface" : True}
        # TODO: disable lighting if 2D (how to get dims?)
        #self.properties["lit"] = False
        
        # build parent
        super(Surface,self).__init__( mesh=mesh, fn=fn,
                        colours=colours, colourMap=colourMap, colourBar=colourBar,
                        valueRange=valueRange, logScale=logScale, discrete=discrete, *args, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Surface,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name]["drawSides"] = self._drawSides
        componentDictionary[self._dr.name][     "Mesh"] = self._mesh._cself.name
        componentDictionary[self._dr2.name]["drawSides"] = self._drawSides
        componentDictionary[self._dr2.name][     "Mesh"] = self._mesh._cself.name

    def _setup(self):
        if self._drawOnMesh:
            self._drOrig = self._dr
            self._dr     = self._dr2
            self._cself  = self._dr2
        _libUnderworld.gLucifer._lucCrossSection_SetFn( self._cself, self._fn._fncself )

    def __del__(self):
        # lets unwind the kludge from _setup to avoid any double deletions or memory leaks.
        if self._drawOnMesh:
            self._dr    = self._drOrig
            self._cself = self._drOrig
        super(Surface,self).__del__()


class Points(Drawing):
    """  
    This drawing object class draws a swarm of points.
    
    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    swarm : uw.swarm.Swarm
        Swarm which provides locations for point rendering.
    fn_colour : uw.function.Function
        Function used to determine colour to render particle.
        This function should return float/double values.
    fn_mask : uw.function.Function
        Function used to determine if a particle should be rendered. 
        This function should return bool values. 
    fn_size : uw.function.Function
        Function used to determine size to render particle.
        This function should return float/double values.
        

    """
    _objectsDict = { "_dr": "lucSwarmViewer" }

    def __init__(self, swarm, fn_colour=None, fn_mask=None, fn_size=None, colourVariable=None,
                       colours=None, colourMap=None, colourBar=True,
                       valueRange=None, logScale=False, discrete=False,
                       *args, **kwargs):

        #DEPRECATE
        if colourVariable != None:
            raise RuntimeError("'colourVariable' parameter is deprecated. Use the fn_colour parameter instead.")
        if not isinstance(swarm,_swarmMod.Swarm):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = swarm

        self._fn_colour = None
        if fn_colour != None:
           self._fn_colour = _underworld.function.Function.convert(fn_colour)
        self._fn_mask = None
        if fn_mask != None:
           self._fn_mask = _underworld.function.Function.convert(fn_mask)
        self._fn_size = None
        if fn_size != None:
           self._fn_size = _underworld.function.Function.convert(fn_size)

        # build parent
        super(Points,self).__init__(
                        colours=colours, colourMap=colourMap, colourBar=colourBar,
                        valueRange=valueRange, logScale=logScale, discrete=discrete, *args, **kwargs)

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
    """
    _objectsDict = { "_dr": None } #Abstract class, Set by child

    def __init__(self, resolutionI=None, resolutionJ=None, resolutionK=None, *args, **kwargs):

        # set defaults here
        if resolutionI == None:
            resolutionI = 16
        if resolutionJ == None:
            resolutionJ = 16
        if resolutionK == None:
            resolutionK = 16

        if resolutionI:
            if not isinstance(resolutionI,(int)):
                raise TypeError("'resolutionI' object passed in must be of python type 'int'")
        if resolutionJ:
            if not isinstance(resolutionJ,(int)):
                raise TypeError("'resolutionJ' object passed in must be of python type 'int'")
        if resolutionK:
            if not isinstance(resolutionK,(int)):
                raise TypeError("'resolutionK' object passed in must be of python type 'int'")

        self._resolutionI = resolutionI
        self._resolutionJ = resolutionJ
        self._resolutionK = resolutionK

        # build parent
        super(_GridSampler3D,self).__init__(*args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(_GridSampler3D,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {
            "resolutionX": self._resolutionI,
            "resolutionY": self._resolutionJ,
            "resolutionZ": self._resolutionK
            } )


class VectorArrows(_GridSampler3D):
    """  
    This drawing object class draws vector arrows corresponding to the provided vector field.

    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    mesh : uw.mesh.Mesh
        Mesh over which vector arrows are rendered.
    fn : uw.function.Function
        Function used to determine vectors to render. 
        Function should return a vector of floats/doubles of appropriate
        dimensionality.
    arrowHead : float
         The size of the head of the arrow compared with the arrow length.
         Must be in [0.,1.].
    scaling : float
        Scaling for entire arrow.
    glyphs : int
        Type of glyph to render for vector arrow.
        0: Line, 1 or more: 3d arrow, higher number => better quality.
    resolutionI : unsigned, default=16.
        Number of samples in the I direction.
    resolutionJ : unsigned, default=16.
        Number of samples in the J direction.
    resolutionK : unsigned, default=16.
        Number of samples in the K direction.

    """
    _objectsDict = { "_dr": "lucVectorArrows" }

    def __init__(self, mesh, fn,
                       resolutionI=None, resolutionJ=None, resolutionK=None, 
                       *args, **kwargs):

        # build parent
        super(VectorArrows,self).__init__( mesh=mesh, fn=fn, resolutionI=resolutionI, resolutionJ=resolutionJ, resolutionK=resolutionK,
                        colours=None, colourMap=None, colourBar=False, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(VectorArrows,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {} )

class Volume(_GridSampler3D):
    """  
    This drawing object class draws a volume using the provided scalar field.
    
    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ---------
    mesh : uw.mesh.Mesh
        Mesh over which object is rendered.
    fn : uw.function.Function
        Function used to determine colour values.
        Function should return a vector of floats/doubles of appropriate
        dimensionality.
    resolutionI : unsigned, default=16.
        Number of samples in the I direction.
    resolutionJ : unsigned, default=16.
        Number of samples in the J direction.
    resolutionK : unsigned, default=16.
        Number of samples in the K direction.

    """
    _objectsDict = { "_dr": "lucFieldSampler" }

    def __init__(self, mesh, fn, resolutionI=None, resolutionJ=None, resolutionK=None,
                       colours=None, colourMap=None, colourBar=True,
                       valueRange=None, logScale=False, discrete=False,
                       *args, **kwargs):
        # build parent
        if mesh.dim == 2:
            raise ValueError("Volume rendered requires a three dimensional mesh.")
        super(Volume,self).__init__( mesh=mesh, fn=fn, resolutionI=resolutionI, resolutionJ=resolutionJ, resolutionK=resolutionK,
                        colours=colours, colourMap=colourMap, colourBar=colourBar,
                        valueRange=valueRange, logScale=logScale, discrete=discrete, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(Volume,self)._add_to_stg_dict(componentDictionary)

class Mesh(Drawing):
    """  
    This drawing object class draws a mesh.
    
    See parent class for further parameter details. Also see property docstrings.
    
    Parameters
    ----------
    mesh : uw.mesh.Mesh
        Mesh to render.
    nodeNumbers : bool. default=False
        Bool to determine whether global node numbers should be rendered. 
    segmentsPerEdge : unsigned. default=1
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
        super(Mesh,self).__init__( colours=None, colourMap=None, colourBar=False, *args, **kwargs )

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Mesh,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name][       "Mesh"] = self._mesh._cself.name
        componentDictionary[self._dr.name]["nodeNumbers"] = self._nodeNumbers
        componentDictionary[self._dr.name][   "segments"] = self._segmentsPerEdge
