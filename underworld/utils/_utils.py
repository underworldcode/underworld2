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
import underworld.function
import libUnderworld
import libUnderworld.libUnderworldPy.Function as _cfn
from timeit import default_timer as timer
from mpi4py import MPI
import h5py
import numpy as np
import sys
import shutil
import os

class Integral(_stgermain.StgCompoundComponent):
    """
    This class constructs a surface or volume integral of the provided function over a
    given mesh.

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

    Notes
    -----
    Constructor must be called by collectively all processes.

    Example
    -------
    Calculate volume of mesh:

    >>> import underworld as uw
    >>> mesh = uw.mesh.FeMesh_Cartesian(minCoord=(0.,0.), maxCoord=(1.,1.))
    >>> volumeIntegral = uw.utils.Integral(fn=1.,mesh=mesh)
    >>> np.isclose( 1., volumeIntegral.evaluate(), rtol=1e-8)
    array([ True], dtype=bool)

    Calculate surface area of mesh:

    >>> surfaceIntegral = uw.utils.Integral(fn=1., mesh=mesh, integrationType='surface', surfaceIndexSet=mesh.specialSets["AllWalls_VertexSet"])
    >>> np.isclose( 4., surfaceIntegral.evaluate(), rtol=1e-8)
    array([ True], dtype=bool)

    """
    _objectsDict = { "_integral": "Fn_Integrate" }
    _selfObjectName = "_integral"

    def __init__(self, fn, mesh=None, integrationType="volume", surfaceIndexSet=None, integrationSwarm=None, **kwargs):
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
                # create MeshVariable
                deltaMeshVariable = uw.mesh.MeshVariable(mesh, 1)
                # init to zero
                deltaMeshVariable.data[:] = 0.
                # set to 1 on provided vertices
                deltaMeshVariable.data[surfaceIndexSet.data] = 1.
                # replace fn with delta*fn
                # note that we need to use this condition so that we only capture border swarm particles
                # on the surface itself. for those directly adjacent, the deltaMeshVariable will evaluate
                # to non-zero (but less than 1.), so we need to remove those from the integration as well.
                self._maskFn = underworld.function.branching.conditional(
                                                  [  ( deltaMeshVariable > 0.999, 1. ),
                                                     (                      True, 0. )   ] )
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

        Notes
        -----
        Method must be called collectively by all processes.

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

    @property
    def maskFn(self):
        """
        The integration mask used where surface integration is performed.
        """
        if not self._maskFn:
            raise RuntimeError("No mask function appears to have been set.\n"+
                               "Note that mask functions are only set for surface integration.")
        return self._maskFn

class SavedFileData(object):
    '''
    A class used to define saved data.

    Parameters
    ----------
    pyobj: object
        python object saved data relates to.
    filename: str
        filename for saved data, full path
    '''
    def __init__(self, pyobj, filename):
        self.pyobj = pyobj
        self.filename = os.path.abspath(filename)


### Code for XDMF output ###
##  Seperate functions for writing geometry to variable in
def _xdmfheader():
    out = ("<?xml version=\"1.0\" ?>\n" +
           "<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\">\n" +
           "<Domain>\n")
    return out

def _xdmffooter():
    out = ("</Grid>\n" +
           "</Domain>\n" +
           "</Xdmf>\n" )
    return out

def _swarmspacetimeschema( swarmSavedData, swarmname, time ):
    """
    Writes the swarm geometry schema for a swarm variable xdmf file

    Parameters:
    ----------
    swarmSavedData : SavedFileData
        The SavedFileData handle to the saved Swarm.
    swarmname : str
        The name, in xdmf, to give the swam
    time : float
        The time stored in the xdmf file

    Returns
    -------
    out : str
        string containing the xdmf schema

    """
    # retrieve bits about previously saved swarm file
    swarm = swarmSavedData.pyobj
    filename = swarmSavedData.filename

    # get swarm parameters - serially read from hdf5 file to get size
    h5f = h5py.File(name=filename, mode="r")
    dset = h5f.get('data')
    if dset == None:
        raise RuntimeError("Can't find 'data' in file '{}'.\n".format(filename))
    globalCount = len(dset)
    h5f.close()

    dim = swarm.mesh.dim

    out = "<Grid Name=\"{0}\" GridType=\"Uniform\">\n".format(swarmname)
    out += "\n\t<Time Value=\"{0}\" />\n\n".format(time)

    out += "\t<Topology Type=\"POLYVERTEX\" NodesPerElement=\"{0}\"> </Topology>\n".format(globalCount)
    if dim == 2:
        out += "\t\t<Geometry Type=\"XY\">\n"
    elif dim == 3:
        out += "\t\t<Geometry Type=\"XYZ\">\n"
    else:
        raise RuntimeError( "Unexpected dim value of {0}, supported value 2 and 3 only".format(dim) )

    out += "\t\t\t<DataItem Format=\"HDF\" NumberType=\"Float\" Precision=\"8\" Dimensions=\"{0} {1}\">{2}:/data</DataItem>\n".format(globalCount, dim, filename)
    out += "\t\t</Geometry>\n"

    return out

def _xdmfAttributeschema( varname, variableType, centering, globalcount, dof_count, datafile ):
    """
    Function to write out an xdmf attribute schema
    It saves rewriting this chunk for MeshVariables and SwarmVariable
    """
    if dof_count==1:
        out = "\t<Attribute Type=\"Scalar\" Center=\"{0}\" Name=\"{1}\">\n".format(centering, varname)
        out += "\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} {2}\">{3}:/data</DataItem>\n".format(variableType, globalcount, dof_count, datafile )
        out += "\t</Attribute>\n"
    elif dof_count==2:
        out = "\t<Attribute Type=\"Vector\" Center=\"{0}\" Name=\"{1}\">\n".format(centering, varname)
        out += "\t<DataItem ItemType=\"Function\"  Dimensions=\"{0} 3\" Function=\"JOIN($0, $1, 0*$1)\">\n".format(globalcount)
        # X values
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"{0} 1\" Name=\"XValue\">\n".format(globalcount)
        out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 0 1 1 {0} 1 </DataItem>\n".format(globalcount)
        out += "\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} {2}\">{3}:/data</DataItem>\n".format(variableType, globalcount, dof_count, datafile )
        out += "\t\t</DataItem>\n"
        # Y values
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"{0} 1\" Name=\"YValue\">\n".format(globalcount)
        out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 1 1 1 {0} 1 </DataItem>\n".format(globalcount)
        out += "\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} {2}\">{3}:/data</DataItem>\n".format(variableType, globalcount, dof_count, datafile )
        out += "\t\t</DataItem>\n"
        out += "\t</DataItem>\n"
        out += "\t</Attribute>\n"
    elif dof_count==3:
        out = "\t<Attribute Type=\"Vector\" Center=\"{0}\" Name=\"{1}\">\n".format(centering, varname)
        out += "\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} {2}\">{3}:/data</DataItem>\n".format(variableType, globalcount, dof_count, datafile )
        out += "\t</Attribute>\n"
    elif dof_count==6:
        out = "\t<Attribute Type=\"Tensor6\" Center=\"{0}\" Name=\"{1}\">\n".format(centering, varname)
        out += "\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} {2}\">{3}:/data</DataItem>\n".format(variableType, globalcount, dof_count, datafile )
        out += "\t</Attribute>\n"
    elif dof_count==9:
        out = "\t<Attribute Type=\"Tensor\" Center=\"{0}\" Name=\"{1}\">\n".format(centering, varname)
        out += "\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} {2}\">{3}:/data</DataItem>\n".format(variableType, globalcount, dof_count, datafile )
        out += "\t</Attribute>\n"
    else:
        out = ""
        for d_i in xrange(dof_count):
            out += "\t<Attribute Type=\"Scalar\" Center=\"{0}\" Name=\"{1}-Component-{2}\">\n".format(centering, varname, d_i)
            out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"{0} 1\" >\n".format(globalcount)
            out += "\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 {0} 1 1 {1} 1 </DataItem>\n".format(offset, globalcount)
            out += "\t\t\t<DataItem Format=\"HDF\" {0} Dimensions=\"{1} {2}\">{3}:/data</DataItem>\n".format(variableType, globalcount, dof_count, datafile)
            out += "\t\t</DataItem>\n"
            out += "\t</Attribute>\n"

    return out

def _swarmvarschema( varSavedData, varname ):
    """"
    Writes the attribute schema for a swarm variable xdmf file

    Parameters:
    ----------
    varSavedData : SavedFileData
        The SavedFileData handle to the saved SwarmVariable
    varname : str
        The name, in xdmf, to give the SwarmVariable

    Returns
    -------
    out : str
        string containing the xdmf schema
    """

    # retrieve bits from varSavedData
    var = varSavedData.pyobj
    varfilename = varSavedData.filename

    # set parameters - serially open the varfilename
    h5f = h5py.File(name=varfilename, mode="r")
    dset = h5f.get('data')
    if dset == None:
        raise RuntimeError("Can't find 'data' in file '{}'.\n".format(filename))
    globalCount = len(dset)
    h5f.close()

    dof_count = var.data.shape[1]
    variableType = "NumberType=\"Float\" Precision=\"8\""

    out = _xdmfAttributeschema( varname, variableType, "Node", globalCount, dof_count, varfilename )

    return out



def _spacetimeschema( savedMeshFile, meshname, time ):
    """
    Writes the geometry schema portion for a MeshVariable xdmf file

    Parameters:
    ----------
    savedMeshFile : SavedFileData
        The SavedFileData handle to the saved Mesh.
    meshname : str
        The name, in xdmf, to give the mesh
    time : float
        The time stored in the xdmf file

    Returns
    -------
    out : str
        string containing the xdmf schema
    """

    elementMesh = savedMeshFile.pyobj
    filename = os.path.basename(savedMeshFile.filename) # short ref only

    dim=elementMesh.dim
    nGlobalNodes = elementMesh.nodesGlobal
    nGlobalEls = elementMesh.elementsGlobal

    out = "<Grid Name=\"FEM_Mesh_{0}\">\n".format(meshname)
    out += "\n\t<Time Value=\"{0}\" />\n\n".format(time)

    if elementMesh.elementType=='Q1':
        # for linear meshes
        if elementMesh.dim == 2:
            topologyType = "Quadrilateral"
            nodesPerElement = 4
            out += "\t<Topology Type=\"{0}\" NumberOfElements=\"{1}\">\n".format( topologyType,  nGlobalEls)
            out += "\t\t<DataItem ItemType=\"Function\" Dimensions=\"{0} {1}\" Function=\"JOIN($0, $1, $3, $2)\">\n".format(nGlobalEls, nodesPerElement)
        else:
            nodesPerElement = 8
            topologyType = "Hexahedron"
            out += "\t<Topology Type=\"{0}\" NumberOfElements=\"{1}\">\n".format( topologyType,  nGlobalEls)
            out += "\t\t<DataItem ItemType=\"Function\" Dimensions=\"{0} {1}\" Function=\"JOIN($0, $1, $3, $2, $4, $5, $7, $6)\">\n".format(nGlobalEls, nodesPerElement)

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

    else:
        raise RuntimeError("XDMF code doesn't support mesh with 'elementType' {0}".format(elementMesh.elementType))


    # define each hyperslab element for the element-node map
    for n_i in xrange(nodesPerElement):
        out += "\t\t<DataItem ItemType=\"HyperSlab\" Dimensions=\"{0} 1\" Name=\"C{1}\">\n".format( nGlobalEls, n_i )
        out += "\t\t\t\t<DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 {0} 1 1 {1} 1 </DataItem>\n".format( n_i, nGlobalEls )
        out += "\t\t\t\t<DataItem Format=\"HDF\" NumberType=\"Int\" Dimensions=\"{0} 1\">{1}:/en_map</DataItem>\n".format( nGlobalEls, filename )
        out += "\t\t</DataItem>\n"

    out += "\t\t</DataItem>\n"

    variableType = "NumberType=\"Float\" Precision=\"8\""
    out += "\t</Topology>\n"
    out += "\t<Geometry Type=\"XYZ\">\n"
    if dim == 2:
        # think this xdmf block can be defined with <Geometry Type="XY"> instead
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

def _fieldschema(varSavedFile, varname ):
    """
    Writes output the xmf portion for a MeshVariable


    Parameters:
    ----------
    varSavedData : SavedFileData
        The SavedFileData handle to the saved SwarmVariable
    varname : str
        The name, in xdmf, to give the SwarmVariable

    Returns
    -------
    out : str
        string containing the xdmf schema
    """


    # Error check
    if not isinstance(varSavedFile, uw.utils.SavedFileData):
        raise TypeError("'varSavedFile', must be of type uw.utils.SavedFileData")
    if not isinstance(varname, str):
        raise TypeError("'varname', must be of type str")

    # get information about the saved field
    field = varSavedFile.pyobj
    filename = os.path.basename(varSavedFile.filename)

    # get the element mesh the field is defined on, ie don't use subMesh
    mesh = field.mesh
    if hasattr(mesh.generator, "geometryMesh"):
        mesh = field.mesh.generator.geometryMesh

    dim = mesh.dim
    dof_count = field.data.shape[1]
    nodesGlobal = field.mesh.nodesGlobal

    variableType = "NumberType=\"Float\" Precision=\"8\""
    offset = 0 #OK: Temporary to get 3D running

    # get the location of the field nodes on the mesh
    if( nodesGlobal == mesh.nodesGlobal ):
        centering = "Node"
    elif (nodesGlobal == mesh.elementsGlobal ):
        centering = "Cell"
    else:
        raise RuntimeError("Can't output field '{}', unsupported elementType '{}'\n".format(varname, field.mesh.elementType) )
       # more conditions needed above for various pressure elementTypes???
       # valid XDMF centers are "Node | Cell | Grid | Face | Edge" - http://www.xdmf.org/index.php/XDMF_Model_and_Format

    out = _xdmfAttributeschema( varname, variableType, centering, nodesGlobal, dof_count, filename )

    return out

def _createMeshName( mesh, outputDir='./output', index=None):
    """
    Returns a string - "outputDir/Mesh_res_time.h5"

    """
    if not index == None:
        if not isinstance(index, int):
            ValueError("'index' must be None or type int")
    # get resolution string
    tmp=map(str,mesh.elementRes)
    st=''
    for x in tmp:
        st += str(x)+'x'
    st = st[:-1]

    if index == None:
        return "{0}/Mesh_{1}.h5".format(outputDir,st)
    else:
        return "{0}/Mesh_{1}.{2}.h5".format(outputDir, st, index )

class ProgressBar(object):
    """
    Class that provides a commandline Progress bar that plays well with piping to file.
    This class is unaware of parallelism and should be appropriately called.

    >>> rank = MPI.COMM_WORLD.Get_rank()
    >>> end = 10.0
    >>> bar = ProgressBar( start=0.0, end=end, title="Ordem e Progresso")
    >>> bar.update(end)   # doctest: +ELLIPSIS
    Ordem e Progresso: 0%----25%----50%----75%----100% | time ... |
    """
    def __init__( self, start=0.0, end=1.0, title=None ):
        if not isinstance( start, (float,int) ):
            raise TypeError( "In ProgressBar, 'start', must be a scalar" )
        if not isinstance( end, (float, int) ):
            raise TypeError( "In ProgressBar, 'end', must be a scalar" )

        self._start=float(start)
        self._end=float(end)
        self._markers = [0, 25, 50, 75]
        self._history=0
        self._startTime=0
        if title != None:
            if not isinstance( title, str ):
                raise TypeError( "In ProgressBar, 'title', must be a scalar" )
            self._title = title
            self._printTitle=True

    def update(self, progress):
        if isinstance( progress, int ):
            progress = float(progress)
        if not isinstance( progress, float ):
            raise TypeError( "In ProgressBar, 'progress', must be a scalar" )

        start = self._start
        end = self._end
        markers = self._markers
        length = end-start
        relprog = int(100.0*(progress-start)/length)
        h = self._history

        if progress < start:
            raise RuntimeError( "Error in ProgressBar: 'progress' < 'start' " )

        if relprog > 100:
            sys.stdout.write("Warning: "+ str(self._title)+ "'s ProgressBar is done\n")
            return

        if self._printTitle:
            sys.stdout.write(str(self._title)+': ')
            self._printTitle=False
            self._startTime=timer()

        while h < relprog:
            if h%5 == 0:
                if h % 25 == 0:
                    sys.stdout.write("{0}%".format(h))
                else:
                    sys.stdout.write("-")
            h += 1

        if h == 100:
            totalTime = timer() - self._startTime
            sys.stdout.write("100% | time {0:.4g} |\n".format(totalTime))

        self._history = relprog

        sys.stdout.flush()

def _nps_2norm( v, comm=MPI.COMM_WORLD ):
    """
    Calculates the 2-norm of a numpy vector v.
    The vector may be decomposed across multiple processors as specified by 'comm'.

    Parameters
    ----------
    v : numpy.ndarray, numpy object
        Assumed to be decomposed across processors as per 'comm'
    comm : MPI.Intracomm, default = MPI.COMM_WORLD
        The communicator that defines the group of processor to calculate the
        vector norm. By default it is all processors.

    Returns
    -------
      $$ ||v||_{2} $$

      Returns this value to all processors

    Note: This is a collective call and must be called by all processors with v
    """


    if not isinstance(comm, MPI.Intracomm):
        raise TypeError("'comm' is not of value type 'MPI.Intracomm'")
    if not isinstance(v, np.ndarray):
        raise TypeError("'v' is not of value type 'numpy.ndarray'")

    lnorm_sq = np.linalg.norm(v)**2 # sq as this is 2-norm

    # comm all gather across procs
    gnorm_sq = np.sum( comm.allgather(lnorm_sq) )

    return np.sqrt(gnorm_sq)
