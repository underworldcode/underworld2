import underworld as _underworld
import underworld._stgermain as _stgermain
import underworld.swarm as _swarmMod
import underworld.mesh as _uwmesh
from underworld.function import Function as _Function
import libUnderworld as _libUnderworld
#import underworld.fevariable as fevariable


class Drawing(_stgermain.StgCompoundComponent):
    _selfObjectName = "_dr"
    _objectsDict = { "_dr": None, # child should set _dr
                     "_cm": "lucColourMap",
                     "_cb": "lucColourBar" }
    
    def __init__(self, colours=None, opacity=None, logScale=False, colourBar=True, valueRange=None, **kwargs ):
    
        if colours == None:
            colours = ["black", "white"]
        if not isinstance(colours,(str,list)):
            raise TypeError("'colours' object passed in must be of python type 'str' or 'list'")
        if isinstance(colours,(str)):
            self._colours = colours.split()
        else:
            self._colours = colours

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

            # valueRange arg is good - turn off dynamicRange and use input 
            self._dynamicRange = False
            self._valueRange   = valueRange
        else:
           self._dynamicRange = True
           self._valueRange   = [0.0,1.0] # dummy value - not important

        self._opacity = -1
        if opacity != None:
            if not isinstance(opacity,(int,float)):
                raise TypeError("'opacity' object passed in must be of python type 'int' or 'float'")
            if float(opacity) > 1. or float(opacity) < 0.:
                raise ValueError("'opacity' must takes values within [0,1].")
            self._opacity = opacity
        
        if not isinstance(logScale, bool):
            raise TypeError("'logScale' parameter must be of 'bool' type.")
        self._logScale = logScale

        if not isinstance(colourBar, bool):
            raise TypeError("'colourBar' parameter must be of 'bool' type.")
        self._colourBar = colourBar
     
        # build parent
        super(Drawing,self).__init__()

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(Drawing,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._cm.name].update( {
            "colours"       :" ".join(self.colours),
            "logScale"      :self._logScale,
            "maximum"       :self._valueRange[1],
            "minimum"       :self._valueRange[0],
            "dynamicRange"  :self._dynamicRange
        } )
        # add an empty(ish) drawing object.  children should fill it out.
        componentDictionary[self._dr.name].update( {
            "ColourMap"     :self._cm.name,
            "opacity"       :self.opacity
        } )
        componentDictionary[self._cb.name].update( {
            "ColourMap"     :self._cm.name
        } )


    @property
    def valueRange(self):
        """     valueRange (list) : list of 2 numbers that define the min and max of the colour bar values 
        """
        return self._valueRange

    @property
    def dynamicRange(self):
        """     dynamicRange (bool) : if True the max and min values of the field will automatically define the colour bar value 
                                      range and the valueRange list is ignored. If False the valueRange is used to define the 
                                      colour bar value range
        """
        return self._dynamicRange

    @property
    def colours(self):
        """    colours (list): list of colours to use to draw object.  Should be provided as a list or a string.
        """
        return self._colours

    @property
    def opacity(self):
        """    opacity (float): Opacity of drawing object.  Takes values from 0. to 1., while a value of -1 explicitly disables opacity.
        """
        return self._opacity

    @property
    def logScale(self):
        """    logScale (bool): Use a logarithm scale for the colourmap.
        """
        return self._logScale

class Surface(Drawing):
    """  This drawing object class draws a surface using the provided scalar field.
    """
    _objectsDict = { "_dr": "lucScalarField" }

    def __init__(self, mesh, fn, drawSides="xyzXYZ",
                       colours=None, opacity=None, logScale=False, colourBar=True, valueRange=None, *args, **kwargs):
        # DEPRECATE
        if isinstance(mesh, _underworld.function.Function):
            raise TypeError("Note that the first two arguments for this constructor have been switched. "
                            "Please specify the mesh (mesh), and then the function (fn), or use keyword arguments.")
        self._fn = _underworld.function.Function._CheckIsFnOrConvertOrThrow(fn)
        
        if not isinstance(mesh,_uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh
        
        if not isinstance(drawSides,str):
            raise ValueError("'drawSides' argument must be of python type 'str'")
        self._drawSides = drawSides
        
        # build parent
        super(Surface,self).__init__(colours, opacity, logScale, colourBar, valueRange, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Surface,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name]["drawSides"] = self.drawSides
        componentDictionary[self._dr.name][     "Mesh"] = self._mesh._cself.name

    def _setup(self):
        _libUnderworld.gLucifer._lucCrossSection_SetFn( self._cself, self._fn._fncself )

    @property
    def drawSides(self):
        """    drawSides (str): sides (x,y,z,X,Y,Z) for which the surface should be drawn.  default is all sides ("xyzXYZ").
        """
        return self._drawSides




class Points(Drawing):
    """  This drawing object class draws a swarm of points.
    """
    _objectsDict = { "_dr": "lucSwarmViewer" }

    def __init__(self, swarm, fn_colour=None, fn_mask=None, fn_size=None, pointSize=1.0,
                       colours=None, opacity=None, logScale=False, colourBar=True, valueRange=None, colourVariable=None, *args, **kwargs):

        #DEPRECATE
        if colourVariable != None:
            raise RuntimeError("'colourVariable' parameter is deprecated. Use the fn_colour parameter instead.")
        if not isinstance(swarm,_swarmMod.Swarm):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = swarm

        self._fn_colour = None
        if fn_colour != None:
           self._fn_colour = _underworld.function.Function._CheckIsFnOrConvertOrThrow(fn_colour)
        self._fn_mask = None
        if fn_mask != None:
           self._fn_mask = _underworld.function.Function._CheckIsFnOrConvertOrThrow(fn_mask)
        self._fn_size = None
        if fn_size != None:
           self._fn_size = _underworld.function.Function._CheckIsFnOrConvertOrThrow(fn_size)

        if not isinstance(pointSize,(float,int)):
            raise TypeError("'pointSize' object passed in must be of python type 'float'")
        self._pointSize = pointSize

        # build parent
        super(Points,self).__init__(colours, opacity, logScale, colourBar, valueRange, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        super(Points,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][ "Swarm" ] = self.swarm._cself.name
        componentDictionary[ self._cself.name ][ "pointSize" ] = self.pointSize
        
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


    @property
    def swarm(self):
        """    swarm (str): name of live underworld swarm for which points will be rendered.
        """
        return self._swarm
    @property
    def fn_colour(self):
        """    fn_colour (uw.Function): Function evaluated to determine particle colour.
        """
        return self._colourVariable
    @property
    def pointSize(self):
        """    pointSize (float): size of points
        """
        return self._pointSize


class VectorArrows(Drawing):
    """  This drawing object class draws vector arrows corresponding to the provided vector field.
    """
    _objectsDict = { "_dr": "lucVectorArrows" }

    def __init__(self, mesh, fn, resolutionX=16, resolutionY=16, resolutionZ=16, arrowHeadSize=0.3, lengthScale=0.3, glyphs=3,
                       colours=None, opacity=None, logScale=False, colourBar=True, valueRange=None, *args, **kwargs):
        # DEPRECATE
        if isinstance(mesh, _underworld.function.Function):
            raise TypeError("Note that the first two arguments for this constructor have been switched. "
                            "Please specify the mesh (mesh), and then the function (fn), or use keyword arguments.")
        self._fn = _underworld.function.Function._CheckIsFnOrConvertOrThrow(fn)
        
        if not isinstance(mesh,_uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        if resolutionX:
            if not isinstance(resolutionX,(int)):
                raise TypeError("'resolutionX' object passed in must be of python type 'int'")
        if resolutionY:
            if not isinstance(resolutionY,(int)):
                raise TypeError("'resolutionY' object passed in must be of python type 'int'")
        if resolutionZ:
            if not isinstance(resolutionZ,(int)):
                raise TypeError("'resolutionZ' object passed in must be of python type 'int'")
        if arrowHeadSize:
            if not isinstance(arrowHeadSize,(float,int)):
                raise TypeError("'arrowHeadSize' object passed in must be of python type 'int' or 'float'")
            if arrowHeadSize < 0 or arrowHeadSize > 1:
                raise ValueError("'arrowHeadSize' can only take values between zero and one. Value provided is " + str(arrowHeadSize)+".")
        if lengthScale:
            if not isinstance(lengthScale,(float,int)):
                raise TypeError("'lengthScale' object passed in must be of python type 'int' or 'float'")
        if glyphs:
            if not isinstance(glyphs,(int)):
                raise TypeError("'glyphs' object passed in must be of python type 'int'")

        self._resolutionX = resolutionX
        self._resolutionY = resolutionY
        self._resolutionZ = resolutionZ
        self._arrowHeadSize = arrowHeadSize
        self._lengthScale = lengthScale
        self._glyphs = glyphs

        # build parent
        super(VectorArrows,self).__init__(colours, opacity, logScale, colourBar, valueRange, *args, **kwargs)

    def _setup(self):
        _libUnderworld.gLucifer._lucCrossSection_SetFn( self._cself, self._fn._fncself )


    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(VectorArrows,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {
                   "Mesh": self._mesh._cself.name,
            "resolutionX": self.resolutionX,
            "resolutionY": self.resolutionY,
            "resolutionZ": self.resolutionZ,
          "arrowHeadSize": self.arrowHeadSize,
            "lengthScale": self.lengthScale,
                 "glyphs": self.glyphs
            } )


    @property
    def resolutionX(self):
        """    resolutionX (int): Number of vector arrows to render in the X direction. Default is 16.
        """
        return self._resolutionX
    @property
    def resolutionY(self):
        """    resolutionY (int): Number of vector arrows to render in the Y direction. Default is 16.
        """
        return self._resolutionY
    @property
    def resolutionZ(self):
        """    resolutionZ (int): Number of vector arrows to render in the Z direction. Default is 16.
        """
        return self._resolutionZ
    @property
    def arrowHeadSize(self):
        """    arrowHeadSize (float): The size of the head of the arrow compared with the arrow length. Must be between [0, 1].   Default is 0.3.
        """
        return self._arrowHeadSize
    @property
    def lengthScale(self):
        """    lengthScale (float): A factor to scale the size of the arrows by.  Default is 0.3.
        """
        return self._lengthScale
    @property
    def glyphs(self):
        """    glyphs (int): Type of glyph to render for vector arrow.
        """
        return self._glyphs

class Mesh(Drawing):
    """  This drawing object class draws a mesh.
    """
    _objectsDict = { "_dr": "lucMeshViewer" }

    def __init__(self, mesh, nodeNumbers=False,
                       colours=None, opacity=None, logScale=False, colourBar=False, valueRange=None, *args, **kwargs):

        if not isinstance(mesh,_uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        if not isinstance(nodeNumbers,bool):
            raise TypeError("'nodeNumbers' flag must be of type 'bool'")
        self._nodeNumbers = nodeNumbers

        if not isinstance(segmentsPerEdge,int) or segmentsPerEdge < 1:
            raise TypeError("'segmentsPerEdge' must be a positive 'int'")
        self._segmentsPerEdge = segmentsPerEdge
        
        # build parent
        super(Mesh,self).__init__(colours, opacity, logScale, colourBar, valueRange, *args, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Mesh,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name][       "Mesh"] = self._mesh._cself.name
        componentDictionary[self._dr.name]["nodeNumbers"] = self._nodeNumbers
        componentDictionary[self._dr.name][   "segments"] = self._segmentsPerEdge
