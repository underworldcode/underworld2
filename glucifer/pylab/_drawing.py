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
import underworld
import underworld._stgermain as _stgermain
import underworld.fevariable as fevariable
import underworld.swarm as swarmMod
import underworld.mesh as uwmesh
from underworld.function import Function as _Function
from libUnderworld import *

class Drawing(_stgermain.StgCompoundComponent):
    _selfObjectName = "_dr"
    _objectsDict = { "_dr": None, # child should set _dr
                     "_cm": "lucColourMap",
                     "_cb": "lucColourBar" }
    
    def __init__(self, colours="Purple Blue Turquoise Bisque Orange Red Brown".split(), properties=None, opacity=-1, logScale=False, colourBar=True, **kwargs):
    
        if not isinstance(colours,(str,list)):
            raise TypeError("'colours' object passed in must be of python type 'str' or 'list'")
        if isinstance(colours,(str)):
            self._colours = colours.split()
        else:
            self._colours = colours

        if not properties:
            properties = {}
        if not isinstance(properties,dict):
            raise TypeError("'properties' object passed in must be of python type 'dict'")
        self._properties = properties

        if kwargs.has_key('valueRange'):
            valueRange = kwargs.get('valueRange')
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
            self._dynamicRange=False
            self._valueRange = valueRange
        else:
           self._dynamicRange=True
           self._valueRange = [0.0,1.0] # dummy value - not important

        if not isinstance(opacity,(int,float)):
            raise TypeError("'opacity' object passed in must be of python type 'int' or 'float'")
        if float(opacity) > 1. or float(opacity) < -1.:
            raise ValueError("'opacity' object must takes values from 0. to 1., while a value of -1 explicitly disables opacity.")
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
        #Convert properties to string
        propstr = ''
        for key in self._properties:
            propstr += key + '=' + str(self._properties[key]) + '\n'

        componentDictionary[self._dr.name].update( {
            "properties"    :propstr,
            "ColourMap"     :self._cm.name,
            "opacity"       :self.opacity
        } )

        #Set default properties for now, TODO: allow setting ColourBar props, needs to be in own class for this
        cbprops = ["colourbar=1", "height=10", "lengthfactor=0.8", 
                   "margin=16", "border=1", 
                   "precision=2", "scientific=false", 
                   "ticks=0", "printticks=true", "printunits=false", "scalevalue=1.0"] #tick0-tick10=val

        componentDictionary[self._cb.name].update( {
            "ColourMap"     :self._cm.name,
            "properties"    :'\n'.join(cbprops),
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

class CrossSection(Drawing):
    """  This drawing object class defines a cross-section plane, derived classes plot data over this cross section
    """
    _objectsDict = { "_dr": "lucCrossSection" }

    def __init__(self, fn, mesh, crossSection="", **kwargs):
        self._fn = underworld.function.Function._CheckIsFnOrConvertOrThrow(fn)
        
        if not isinstance(mesh,uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        if not isinstance(crossSection,str):
            raise ValueError("'crossSection' argument must be of python type 'str'")
        self._crossSection = crossSection

        # build parent
        super(CrossSection,self).__init__(**kwargs)

    def _setup(self):
        gLucifer._lucCrossSection_SetFn( self._cself, self._fn._fncself )

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(CrossSection,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {
                   "Mesh": self._mesh._cself.name,
                   "crossSection": self._crossSection
            } )

    @property
    def crossSection(self):
        """    crossSection (str): Cross Section definition, eg;: z=0.
        """
        return self._crossSection

class Surface(CrossSection):
    """  This drawing object class draws a surface using the provided scalar field.
    """
    _objectsDict = { "_dr": None }

    def __new__(cls, drawSides="xyzXYZ", useMesh=False, *args, **kwargs):

        #Use the mesh sampler if requested
        if useMesh:
            cls._objectsDict = { "_dr": "lucScalarFieldOnMesh" }
        else:
            cls._objectsDict = { "_dr": "lucScalarField" }
        return super(Surface, cls).__new__(cls, *args, **kwargs)

    def __init__(self, drawSides="xyzXYZ", useMesh=False, *args, **kwargs):

        if not isinstance(drawSides,str):
            raise ValueError("'drawSides' argument must be of python type 'str'")
        self._drawSides = drawSides
        
        # build parent
        super(Surface,self).__init__(**kwargs)

        # Enable cullface prop by default
        self._properties["cullface"] = True;
        # TODO: disable lighting if 2D (how to get dims?)
        #self._properties["lit"] = False;

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Surface,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name]["drawSides"] = self.drawSides

    @property
    def drawSides(self):
        """    drawSides (str): sides (x,y,z,X,Y,Z) for which the surface should be drawn.  default is all sides ("xyzXYZ").
        """
        return self._drawSides

class Points(Drawing):
    """  This drawing object class draws a swarm of points.
    """
    _objectsDict = { "_dr": "lucSwarmViewer" }

    def __init__(self, swarm, colourVariable=None, sizeVariable=None, opacityVariable=None, pointSize=1.0, pointType=None, **kwargs):
        if not isinstance(swarm,swarmMod.Swarm):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = swarm

        if colourVariable:
            if not isinstance(colourVariable,swarmMod.SwarmVariable):
                raise TypeError("'colourVariable' object passed in must be of type 'SwarmVariable'")
            if colourVariable.swarm != swarm:
                raise ValueError("colourVariable must correspond to provided swarm.")
        self._colourVariable = colourVariable
        if sizeVariable:
            if not isinstance(sizeVariable,swarmMod.SwarmVariable):
                raise TypeError("'sizeVariable' object passed in must be of type 'SwarmVariable'")
            if sizeVariable.swarm != swarm:
                raise ValueError("sizeVariable must correspond to provided swarm.")
        self._sizeVariable = sizeVariable
        if opacityVariable:
            if not isinstance(opacityVariable,swarmMod.SwarmVariable):
                raise TypeError("'opacityVariable' object passed in must be of type 'SwarmVariable'")
            if opacityVariable.swarm != swarm:
                raise ValueError("opacityVariable must correspond to provided swarm.")
        self._opacityVariable = opacityVariable

        if not isinstance(pointSize,(float,int)):
            raise TypeError("'pointSize' object passed in must be of python type 'float' or 'int'")

        if not isinstance(pointType,(int)):
            raise TypeError("'pointType' object passed in must be of python type 'int'")

        # build parent
        super(Points,self).__init__(**kwargs)

        #Set properties dict
        self._properties["pointsize"] = pointSize
        if pointType != None:
            self._properties["pointtype"] = pointType

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        super(Points,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][ "Swarm" ] = self.swarm._cself.name
        
    def _setup(self):
        if self.colourVariable:
            self._cself.colourVariable = self.colourVariable._cself
        if self.sizeVariable:
            self._cself.sizeVariable = self.sizeVariable._cself
        if self.opacityVariable:
            self._cself.opacityVariable = self.opacityVariable._cself
    
    @property
    def swarm(self):
        """    swarm (str): name of live underworld swarm for which points will be rendered.
        """
        return self._swarm
    @property
    def colourVariable(self):
        """    colourVariable (str): name of live underworld swarm variable which will determine the point colours.
        """
        return self._colourVariable
    @property
    def sizeVariable(self):
        """    sizeVariable (str): name of live underworld swarm variable which will determine the point size.
        """
        return self._sizeVariable
    @property
    def opacityVariable(self):
        """    opacityVariable (str): name of live underworld swarm variable which will determine the point opacity.
        """
        return self._opacityVariable

    @property
    def pointSize(self):
        """    pointSize (float): size of points
        """
        return self._properties["pointsize"]
        self._properties["pointType"]

    @property
    def pointType(self):
        """    pointType (int): points type [0-4]
        """
        return self._properties["pointType"]

class GridSampler3D(CrossSection):
    """  This drawing object class samples a regular grid in 3D.
    """
    _objectsDict = { "_dr": None } #Abstract class, Set by child

    def __init__(self, resolutionX=16, resolutionY=16, resolutionZ=16, **kwargs):
        if resolutionX:
            if not isinstance(resolutionX,(int)):
                raise TypeError("'resolutionX' object passed in must be of python type 'int'")
        if resolutionY:
            if not isinstance(resolutionY,(int)):
                raise TypeError("'resolutionY' object passed in must be of python type 'int'")
        if resolutionZ:
            if not isinstance(resolutionZ,(int)):
                raise TypeError("'resolutionZ' object passed in must be of python type 'int'")

        self._resolutionX = resolutionX
        self._resolutionY = resolutionY
        self._resolutionZ = resolutionZ

        # build parent
        super(GridSampler3D,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(GridSampler3D,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {
            "resolutionX": self.resolutionX,
            "resolutionY": self.resolutionY,
            "resolutionZ": self.resolutionZ
            } )

    @property
    def resolutionX(self):
        """    resolutionX (int): Number of samples in the X direction. Default is 16.
        """
        return self._resolutionX
    @property
    def resolutionY(self):
        """    resolutionY (int): Number of samples in the Y direction. Default is 16.
        """
        return self._resolutionY
    @property
    def resolutionZ(self):
        """    resolutionZ (int): Number of samples in the Z direction. Default is 16.
        """
        return self._resolutionZ

class VectorArrows(GridSampler3D):
    """  This drawing object class draws vector arrows corresponding to the provided vector field.
    """
    _objectsDict = { "_dr": "lucVectorArrows" }

    def __init__(self, arrowHeadSize=0.3, lengthScale=0.3, glyphs=3, **kwargs):
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

        # build parent
        super(VectorArrows,self).__init__(**kwargs)

        #Set properties dict
        self._properties["arrowhead"] = arrowHeadSize
        self._properties["scaling"] = lengthScale
        self._properties["glyphs"] = glyphs

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(VectorArrows,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {} )

    @property
    def arrowHeadSize(self):
        """    arrowHeadSize (float): The size of the head of the arrow compared with the arrow length. Must be between [0, 1].   Default is 0.3.
        """
        return self._properties["arrowhead"]
    @property
    def lengthScale(self):
        """    lengthScale (float): A factor to scale the size of the arrows by.  Default is 0.3.
        """
        return self._properties["scaling"]
    @property
    def glyphs(self):
        """    glyphs (int): Type of glyph to render for vector arrow. (0: Line, 1 or more: 3d arrow, higher number => better quality)
        """
        return self._properties["glyphs"]

class Volume(GridSampler3D):
    """  This drawing object class draws a volume using the provided scalar field.
    """
    _objectsDict = { "_dr": "lucFieldSampler" }

    def __init__(self, **kwargs):
        # build parent
        super(Volume,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        
        # call parents method
        super(Volume,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name].update( {
            } )

class Mesh(Drawing):
    """  This drawing object class draws a mesh.
    """
    _objectsDict = { "_dr": "lucMeshViewer" }

    def __init__(self, mesh, nodeNumbers=False, segmentsPerEdge=1, *args, **kwargs):

        if not isinstance(mesh,uwmesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        if not isinstance(nodeNumbers,bool):
            raise TypeError("'nodeNumbers' flag must be of type 'bool'")
        self._nodeNumbers = nodeNumbers

        if not isinstance(segmentsPerEdge,int) or segmentsPerEdge < 1:
            raise TypeError("'segmentsPerEdge' must be a positive 'int'")
        self._segmentsPerEdge = segmentsPerEdge
        
        # build parent
        super(Mesh,self).__init__(**kwargs)

        #No colour bar
        self._colourBar = False

        #Set properties dict
        self._properties["lit"] = False;
        self._properties["linewidth"] = 0.1;
        self._properties["pointsize"] = 5 if self._nodeNumbers else 1
        self._properties["pointtype"] = 2 if self._nodeNumbers else 4


    def _add_to_stg_dict(self,componentDictionary):
        # lets build up component dictionary
        # append random string to provided name to ensure unique component names
        # call parents method
        
        super(Mesh,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._dr.name][       "Mesh"] = self._mesh._cself.name
        componentDictionary[self._dr.name]["nodeNumbers"] = self._nodeNumbers
        componentDictionary[self._dr.name][   "segments"] = self._segmentsPerEdge

