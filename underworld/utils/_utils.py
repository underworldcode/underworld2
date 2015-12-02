##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld as uw
import underworld._stgermain as _stgermain
import underworld.mesh as mesh
import underworld.fevariable as fevariable
import underworld.function
import libUnderworld
import libUnderworld.libUnderworldPy.Function as _cfn

class Integral(_stgermain.StgCompoundComponent):
    """
    This class constructs a surface or volume integral of the provided function over a 
    given mesh.
    
    Calculate volume of mesh:
    
    >>> import underworld as uw
    >>> mesh = uw.mesh.FeMesh_Cartesian(minCoord=(0.,0.), maxCoord=(1.,1.))
    >>> volumeIntegral = uw.utils.Integral(fn=1.,mesh=mesh)
    >>> volumeIntegral.evaluate()
    [1.0]
    
    Calculate surface area of mesh:
    
    >>> surfaceIntegral = uw.utils.Integral(fn=1.,mesh=mesh, integrationType='surface', surfaceIndexSet=mesh.specialSets["AllWalls_VertexSet"])
    >>> surfaceIntegral.evaluate()
    [4.0]
    
    """
    _objectsDict = { "_integral": "Fn_Integrate" }
    _selfObjectName = "_integral"

    def __init__(self, fn, mesh=None, integrationType="volume", surfaceIndexSet=None, integrationSwarm=None, feMesh=None, **kwargs):
        """
        Parameters
        ----------
        fn : uw.function.Function
            Function to be integrated.
        mesh : uw.mesh.FeMesh
            The mesh over which integration is performed.
        integrationType : str
            Type of integration to perform.  Options are "volume" or "surface".
        surfaceIndexSet : uw.mesh.FeMesh_IndexSet
            Must be provided where integrationType is "surface".
            This IndexSet determines which surface is to be integrated over.
            Note that surface integration over interior nodes is not currently supported.
        integrationSwarm : uw.swarm.IntegrationSwarm (optional)
            User provided integration swarm.
        
        """
        
        if feMesh:  # DEPRECATE
            raise ValueError("This parameter has been renamed to 'mesh'.")

        if not mesh:
            raise ValueError("A mesh object must be provided")
        if not isinstance(mesh, uw.mesh.FeMesh):
            raise TypeError("'feMesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh
        self._cself.mesh = self._mesh._cself
        
        self._maskFn = None
        
        self._fn = uw.function.Function._CheckIsFnOrConvertOrThrow(fn)

        if integrationType and integrationSwarm:
            raise RuntimeError("Either an 'integrationType' or an 'integrationSwarm' may be provided, but not both.\n"
                              +"You may need to set 'integrationType' to None.")
        
        if integrationType:
            if not isinstance( integrationType, str ):
                raise TypeError( "'integrationType' provided must be a string.")
            integrationType = integrationType.lower()
            if integrationType not in ["volume", "surface"]:
                raise ValueError( "'integrationType' string provided must be either 'volume' or 'surface'.")
            if integrationType == "volume":
                self._cself.isSurfaceIntegral = False
                integrationSwarm = uw.swarm.GaussIntegrationSwarm(mesh)
            else:
                self._cself.isSurfaceIntegral = True
                if not surfaceIndexSet:
                    raise RuntimeError("For surface integration, you must provide a 'surfaceIndexSet'.")
                if not isinstance(surfaceIndexSet, uw.mesh.FeMesh_IndexSet ):
                    raise TypeError("'surfaceIndexSet' must be of type 'FeMesh_IndexSet'.")
                if surfaceIndexSet.object != mesh:
                    raise ValueError("'surfaceIndexSet' mesh does not appear to correspond to mesh provided to Integral object.")
                if surfaceIndexSet.topologicalIndex != 0:
                    raise ValueError("'surfaceIndexSet' must correspond to vertex objects.")
                # check that nodes are boundary nodes
                try:
                    allBoundaryNodes = mesh.specialSets['AllWalls_VertexSet']
                except:
                    raise ValueError("Mesh does not appear to provide a 'AllWalls_VertexSet' special set. This is required for surface integration.")
                for guy in surfaceIndexSet:
                    inSet = int(guy) in allBoundaryNodes
                    if not inSet:
                        raise ValueError("Your surfaceIndexSet appears to contain node(s) which do not belong to the mesh boundary. Surface integration across internal nodes is not currently supported.")
                # create feVariable
                deltaFeVariable = uw.fevariable.FeVariable(mesh, 1)
                # init to zero
                deltaFeVariable.data[:] = 0.
                # set to 1 on provided vertices
                deltaFeVariable.data[surfaceIndexSet.data] = 1.
                # replace fn with delta*fn
                # note that we need to use this condition so that we only capture border swarm particles
                # on the surface itself. for those directly adjacent, the deltaFeVariable will evaluate
                # to non-zero (but less than 1.), so we need to remove those from the integration as well.
                self._maskFn = underworld.function.branching.conditional(
                                                  [  ( deltaFeVariable > 0.999, 1. ),
                                                     (                    True, 0. )   ] )
                self._fn = self._fn * self._maskFn
                integrationSwarm = uw.swarm.GaussBorderIntegrationSwarm(mesh)
        else:
            if not isinstance(integrationSwarm, uw.swarm.IntegrationSwarm):
                raise TypeError("'integrationSwarm' object passed in must be of type 'IntegrationSwarm'")

        self._integrationSwarm = integrationSwarm
        self._cself.integrationSwarm = integrationSwarm._cself
        self._cself.dim = mesh.dim

        # lets setup fn tings
        libUnderworld.Underworld._Fn_Integrate_SetFn( self._cself, self._fn._fncself)
        
        super(Integral,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        pass

    def evaluate(self):
        """
        Perform integration.
        
        Returns
        -------
        result : list of floats
            Integration result. For vector integrals, a vector is returned.
        
        """
        val = libUnderworld.Underworld.Fn_Integrate_Integrate( self._cself )
        result = []
        for ii in range(0,val.size()):
            result.append(val.value(ii))
        return result

    def integrate(self): # DEPRECATE
        raise RuntimeError("This method has been renamed to 'evaluate'.")
    
    @property
    def maskFn(self):
        """
        The integration mask used where surface integration is performed.
        """
        if not self._maskFn:
            raise RuntimeError("No mask function appears to have been set.\n"+
                               "Note that mask functions are only set for surface integration.")
        return self._maskFn


### Code for XDMF output ###
def _spacetimeschema( elementMesh, time, filename ):
    """
    Writes out the initial portion of an xmf file.
    The mesh is output as a <Grid> with the <Time> also recorded
    """

    dim=elementMesh.dim
    nGlobalNodes = elementMesh.nodesGlobal
    nGlobalEls = elementMesh.elementsGlobal

    out = "<Grid Name=\"FEM_Mesh_{0}\">\n".format("HSEM")
    out += "\n\t<Time Value=\"{0}\" />\n\n".format(time)

    if elementMesh.elementType=='Q1':
        # for linear meshes
        if elementMesh.dim == 2:
            topologyType = "Quadrilateral"
            nodesPerElement = 4
        else:
            nodesPerElement = 8
            topologyType = "Hexahedron"

        out += "\t<Topology Type=\"{0}\" NumberOfElements=\"{1}\">\n".format( topologyType,  nGlobalEls)
        out += "\t\t<DataItem Format=\"HDF\" DataType=\"Int\" Dimensions=\"{0} {1}\">{2}:/connectivity</DataItem>\n".format(nGlobalEls, nodesPerElement, filename)

    elif elementMesh.elementType=='Q2':
        # for quadratic meshes
        if elementMesh.dim == 2:
            topologyType = "Quadrilateral_9"
            nodesPerElement = 9
            out += "\t<Topology Type=\"{0}\" NumberOfElements=\"{1}\">\n".format( topologyType,  nGlobalEls)
            out += "\t\t<DataItem ItemType=\"Function\" Dimensions=\"{0} {1}\" Function=\"JOIN($0, $2, $8, $6, $1, $5, $7, $3, $4)\">\n".format( nGlobalEls, nodesPerElement )
        else:
            nodesPerElement = 27
            topologyType = "Hexahedron_27"
            out += "\t<Topology Type=\"{0}\" NumberOfElements=\"{1}\">\n".format( topologyType,  nGlobalEls)
            out += ( "\t\t<DataItem ItemType=\"Function\"  Dimensions=\"{0} {1}\" Function=\"JOIN( $0,  $9,  $2, $12, $22, $10,  $4, $11,  $3, "+
                       "$13, $26, $14, $24, $21, $25, $16, $27, $15, $5, $17,  $6, $20, $23, $18,  $8, $19,  $7)\">\n".format( nGlobalEls, nodesPerElement ) )

        for n_i in xrange(nodesPerElement):
            out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"{0} 1\" Name=\"C{1}\">\n".format( nGlobalEls, n_i )
            out += "\t\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 {0} 1 1 {1} 1 </DataItem>\n".format( n_i, nGlobalEls )
            out += "\t\t\t\t<DataItem Format=\"HDF\" NumberType=\"Int\" Dimensions=\"{0} 1\">{1}:/connectivity</DataItem>\n".format( nGlobalEls, filename )
            out += "\t\t</DataItem>\n"

        out += "\t\t</DataItem>\n"

    else:
        raise RuntimeError("XDMF code doesn't support mesh with 'elementType' {0}".format(elementMesh.elementType))
        

    variableType = "NumberType=\"Float\" Precision=\"8\""

    out += "\t</Topology>\n"
    out += "\t<Geometry Type=\"XYZ\">\n"
    if dim == 2:
        out += "\t\t<DataItem ItemType=\"Function\"  Dimensions=\"{0} 3\" Function=\"JOIN($0, $1, 0*$1)\">\n".format(nGlobalNodes)
        out += "\t\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"{0} 1\" Name=\"XCoords\">\n".format(nGlobalNodes)
        out += "\t\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 0 1 1 {0} 1 </DataItem>\n".format(nGlobalNodes)
        out += "\t\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} 2\">{2}:/vertices</DataItem>\n".format(variableType, nGlobalNodes, filename) 
        out += "\t\t\t</DataItem>\n"
        out += "\t\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"{0} 1\" Name=\"YCoords\">\n".format(nGlobalNodes)
        out += "\t\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 1 1 1 {0} 1 </DataItem>\n".format(nGlobalNodes)
        out += "\t\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} 2\">{2}:/vertices</DataItem>\n".format(variableType, nGlobalNodes, filename )
        out += "\t\t\t</DataItem>\n"
        out += "\t\t</DataItem>\n"
    if dim == 3:
        out += "\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} 3\">{2}:/vertices</DataItem>\n".format(variableType, nGlobalNodes, filename)
        
    out += "\t</Geometry>\n"
    
    return out

def _fieldschema((field_name, field), filename, elementMesh ):
    """
    Writes output the xmf portion for a FeVariable
    """

    # Error check
    if not isinstance(field_name, str):
        raise TypeError("'field_name', must be of type str")
    if not isinstance(field, uw.fevariable.FeVariable):
        raise TypeError("'field', must be of type FeVariable")
    if not isinstance(filename, str):
        raise TypeError("'field_name', must be of type str")
    if not isinstance(elementMesh, uw.mesh.FeMesh):
        raise TypeError("'elementMesh', must be of type FeMesh")

    # get information about the field
    dim = elementMesh.dim
    dof_count = field.data.shape[1]
    nodesGlobal = field.feMesh.nodesGlobal
    
    variableType = "NumberType=\"Float\" Precision=\"8\""
    offset = 0 #OK: Temporary to get 3D running

    # get the location of the field nodes on the mesh
    if( nodesGlobal == elementMesh.nodesGlobal ):
        centering = "Node"
    elif (nodesGlobal == elementMesh.elementsGlobal ):
        centering = "Cell"
    else:
        raise RuntimeError("Can't output field '{}', unsupported elementType '{}'\n".format(field_name, field.feMesh.elementType) )
       # more conditions needed above for various pressure elementTypes??? 
       # valid XDMF centers are "Node | Cell | Grid | Face | Edge" - http://www.xdmf.org/index.php/XDMF_Model_and_Format


    if dof_count==1:
        out = "\t<Attribute Type=\"Scalar\" Center=\"%s\" Name=\"%s\">\n" % (centering, field_name)
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" >\n" % (nodesGlobal)
        out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 0 1 1 %u 1 </DataItem>\n" % (nodesGlobal)
        out += "\t\t\t<DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s:/data</DataItem>\n" % (variableType, nodesGlobal, dof_count, filename )
        out += "\t\t</DataItem>\n"
        out += "\t</Attribute>\n"
    
    elif dof_count==2:
        out = "\t<Attribute Type=\"Vector\" Center=\"%s\" Name=\"%s\">\n" % (centering, field_name)
        out += "\t<DataItem ItemType=\"Function\"  Dimensions=\"%u 3\" Function=\"JOIN($0, $1, 0*$1)\">\n" % (nodesGlobal)
        # X values
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"XValue\">\n" % (nodesGlobal)
        out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 0 1 1 %u 1 </DataItem>\n" % (nodesGlobal)
        out += "\t\t\t<DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s:/data</DataItem>\n" % (variableType, nodesGlobal, dof_count, filename )
        out += "\t\t</DataItem>\n"
        # Y values
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"YValue\">\n" % (nodesGlobal)
        out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 1 1 1 %u 1 </DataItem>\n" % (nodesGlobal)
        out += "\t\t\t<DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s:/data</DataItem>\n" % (variableType, nodesGlobal, dof_count, filename )
        out += "\t\t</DataItem>\n"
        out += "\t</DataItem>\n"
        out += "\t</Attribute>\n"

    elif dof_count==3:
        out =  "\t<Attribute Type=\"Vector\" Center=\"%s\" Name=\"%s\">\n".format(centering,  field_name)
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 3\" >\n".format(nodesGlobal)
        out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u 3 </DataItem>\n".format(offset, nodesGlobal)
        out += "\t\t\t<DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s:/data</DataItem>\n".format(variableType, nodesGlobal, dof_count, filename)
        out += "\t\t</DataItem>\n"
        out += "\t</Attribute>\n"
    
    elif dof_count==6 and dim==3:
        out =  "\t<Attribute Type=\"Tensor6\" Center=\"%s\" Name=\"%s\">\n".format(centering,  field_name)
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"%u %u\" >\n".format(nodesGlobal, dof_count)
        out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u %u </DataItem>\n".format(offset, nodesGlobal, dof_count)
        out += "\t\t\t<DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s:/data</DataItem>\n".format(variableType, nodesGlobal, dof_count, filename)
        out += "\t\t</DataItem>\n"
        out += "\t</Attribute>\n"
    elif dof_count==9 and dim==3:
        out =  "\t<Attribute Type=\"Tensor\" Center=\"%s\" Name=\"%s\">\n".format(centering,  field_name)
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"%u %u\" >\n".format(nodesGlobal, dof_count)
        out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u %u </DataItem>\n".format(offset, nodesGlobal, dof_count)
        out += "\t\t\t<DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s:/data</DataItem>\n".format(variableType, nodesGlobal, dof_count, filename)
        out += "\t\t</DataItem>\n"
        out += "\t</Attribute>\n"
    else:
        for d_i in xrange(dof_count):
            out = "\t<Attribute Type=\"Scalar\" Center=\"%s\" Name=\"%s-Component-%u\">\n".format(centering, field_name, d_i)
            out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" >\n".format(nodesGlobal)
            out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u 1 </DataItem>\n".format(offset, nodesGlobal)
            out += "\t\t\t<DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s:/data</DataItem>\n".format(variableType, nodesGlobal, dof_count, filename)
            out += "\t\t</DataItem>\n"
            out += "\t</Attribute>\n"
    
    return out

    
    return out

import os
#### TO Deprecated Nov 2015
def xdmf_write( objects, mesh, outputDir='./output', time=None):
    """
    Depreciated function, instead use the LogBook class

    log = LogBook( objects, mesh, outputDir= )
    log.write(time=time)

    """
    raise RuntimeError("Depreciated function, instead use the LogBook class, eg:\n"+
                       "log = LogBook( objects, mesh, outputDir= )\n" +
                       "log.write(time=time)\n" )

class LogBook(object):
    """
    Class for recording Underworld information onto disk
    """

    def __init__(self, objects, mesh, outputDir="./output" ):
        """
        The LogBook class saves (and maybe later will read) Underworld objects to disk in XDMF format. eg. hdf5 format for heavy data,
        (eg: fields & swarms) xml format for metadata that describes heavy data. The resultant data
        XDMF data file is '<outputDir>/XDMF.temporalFiles.xdmf'. 
        
        Parameters
        ----------
        objects : dict
            Dictionary containing strings that map to FeVariables. The strings label the 
            paired FeVariables in the XDMF output. Eg: {'vField': foo, 'pField: foo2 }
            FeVariable foo will be labelled 'vField', FeVariable foo2 will be labelled 'pField'
        
        mesh : feMesh
            The elementMesh that all fields are defined over
        
        outputDir : string
            The path to record all hdf5 and xml files. By default this path is './output'.
            xdmf_write() only writes to this directory and will overwrite existing files.

        """

        ### Error Check input ###
        # test 'mesh' arg
        if not isinstance(mesh, uw.mesh.FeMesh):
            raise TypeError("'mesh' must be of type 'FeMesh'")
        self._mesh=mesh

        # test 'objects' arg
        if objects:
            if not isinstance( objects, dict):
                raise TypeError("'objects' passed in must be of type 'dict'")
            if len(objects) < 1:
                raise ValueError("'objects' dictionary must contain one 'name' : field pair\n" +
                                 "e.g. {'vfield' : myVelocityField }")
            for (k,v) in objects.items():
                if not isinstance(k, str):
                    raise TypeError("'objects' keys must be of type 'str'")
                if not isinstance(v, uw.fevariable.FeVariable):
                    raise TypeError("object with key '{}' must be of type 'FeVariable'".format(k))
                # check if we support field elementType on master mesh
                # get the location of the field nodes on the mesh
                if( v.feMesh.nodesGlobal == mesh.nodesGlobal ):
                    pass
                elif ( v.feMesh.nodesGlobal == mesh.elementsGlobal ):
                    pass
                else:
                    raise RuntimeError("Can't write field '{}' XDMF file, unsupported elementType '{}'\n".format(k, v.feMesh.elementType) )
                   # more conditions needed above for various pressure elementTypes??? 
                   # valid XDMF centers are "Node | Cell | Grid | Face | Edge" - http://www.xdmf.org/index.php/XDMF_Model_and_Format

        self._objects=objects

        if not isinstance(outputDir, str):
            raise TypeError("'outputDir', must be of type 'str'")
        self._outputDir = outputDir

        # initialise 'internalCount' - used for a unique index for each output
        self._internalCount = 0

        ### only proc 0 does this ###
        if uw.rank() != 0:
            return None

        # Build outputDir and write to XDMF
        if not os.path.exists(outputDir):
            try:
                os.makedirs(outputDir)
            except:
                print("Can not make directory {}".format(outputDir))
                raise
        
        # create the xdmf file name
        self._xdmfFN = xdmfFN = outputDir+"/XDMF.Files.xdmf"
        xdmfTemporalFiles = outputDir+"/XDMF.temporalFiles.xdmf"

        if os.path.exists( xdmfFN ):
            os.remove( xdmfFN )

        # create the files "XDMF.Files.xdmf" and "XDMF.temporalFiles.xdmf"
        xdmfFH = open(xdmfFN, "w")
        string = ("<?xml version=\"1.0\" ?>\n" +
                  "<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\">\n" + 
                  "<Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"FEM_Mesh_Fields\">\n" +
                  "</Grid>\n" + 
                  "</Xdmf>")
        xdmfFH.write(string)
        xdmfFH.close()

        
        # create "XDMF.temporalFiles.xdmf" once only
        refxdmfFN = os.path.basename(xdmfFN)
        xdmfTemporalFiles = outputDir+"/XDMF.temporalFiles.xdmf"
        xdmfFH = open( xdmfTemporalFiles, "w" )
        xdmfFH.write("<?xml version=\"1.0\" ?>\n" +
                     "<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\">\n" +
                     "<Domain>\n" + 
                     "<xi:include href=\"{0}\" xpointer=\"xpointer(//Xdmf/Grid)\"/>\n".format(refxdmfFN) +
                     "</Domain>\n" +
                     "</Xdmf>\n")
        xdmfFH.close()

    @property
    def mesh(self):
        """
        The element mesh used for the LogBook's XDMF output
        """
        return self._mesh
    @property
    def objects(self):
        """
        The objects this LogBook will output to disk
        """
        return self._objects
    @property
    def outputDir(self):
        """
        The output directory the LogBook writes to
        """
        return self._outputDir

    def write(self, time=None):
        """
        Writes out Underworld objects to disk in XDMF format. eg. hdf5 format for heavy data,
        (eg: fields & swarms) xml format for metadata that describes heavy data. The resultant data
        XDMF data file is '<outputDir>/XDMF.temporalFiles.xdmf'

        Parameters
        ----------
        time : scalar
            An optional parameter to record the model time when the XDMF file is written.
            If not provided the XDMF time will be the nth occasion this function has been called.

        """
        # if the 'time' arg is empty use the _internalCount number to specify the time for XDMF
        if time==None:
            time = self._internalCount

        if not isinstance(time, (float,int)):
            raise TypeError("'time' argument must be of type float or float")

        mesh = self.mesh
        objects = self.objects
        outputDir = self.outputDir

        # setup a unique id 'uniId' with ZFILL!
        uniId = str(self._internalCount).zfill(5)

        # increment the _internalCount for next write operation
        self._internalCount += 1

        # setup filenames
        meshFN = outputDir+"/mesh." + uniId +".h5"    
        refmeshFN = os.path.basename(meshFN)    

        # the xmf file is stored in 'string'
        # 1st write header
        string = ("<?xml version=\"1.0\" ?>\n" +
                  "<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\">\n" +
                  "<Domain>\n")
    
        # Save the mesh under file <outputDir/mesh.uniId.h5>    
        mesh.save(meshFN)
        
        # append to xmf
        string += _spacetimeschema(mesh, time, refmeshFN)
    
        ## Save the fields under the file <outputDir/name.uniId.h5>
        for (k,feVar) in objects.items():
            # not sure multi meshes work so far - further testing required
            #if( feVar.feMesh != mesh ):
            #    raise RuntimeError("Unexpected mesh, {} xmf writer needs further implementation\n".format(k)+
            #                       "to handle multiple meshes.\n")
            
            # setup filenames
            fieldFN = outputDir+"/{}.".format(k)+uniId+".h5"
            reffieldFN = os.path.basename(fieldFN)

            feVar.save(fieldFN)
            string += _fieldschema( (k,feVar), reffieldFN, mesh )
        
        # write the footer to the xmf    
        string += ("</Grid>\n" + 
                   "</Domain>\n" + 
                   "</Xdmf>\n" )
    

        ### if PARALLEL only proc 0 is to write the xdmf ###
        if uw.rank() != 0:
            return

        # open the would be xmf file
        xmfFN = outputDir+"/XDMF."+uniId+".xmf"
        refxmfname = os.path.basename(xmfFN)
        xdmfFN = self._xdmfFN

        # open or overwrite xmfFN
        try:
            xmfFH=open(xmfFN, "w")
        except:
            print("Cannot make the file {}".format(xmfFN))
            raise
        
        # write string to xmf file
        xmfFH.write(string)
        xmfFH.close()
        
        # now write 'XDMF.Files.xdmf' - stores all timesteps
        try:
            xdmfFH = open(xdmfFN, "r+")
        except:
            raise

        # append to the xdmf 
        xdmfFH.seek(-16,2)  # exact move of filePtr 16 bytes from the end of the file - before the "</Grid>"
        xdmfFH.truncate()   # nuke all of file after this point
        string = ("\n    <xi:include href=\"{0}\" ".format(refxmfname) + 
                  "xpointer=\"xpointer(//Xdmf/Domain/Grid[1])\"/>\n" +
                  "</Grid>\n" + 
                  "</Xdmf>")
        xdmfFH.write(string)
        xdmfFH.close()


