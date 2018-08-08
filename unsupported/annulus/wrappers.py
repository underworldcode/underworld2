import os
import underworld as uw
import numpy as np
from underworld import function as fn
uw.matplotlib_inline()
import glucifer
from datetime import datetime
from mpi4py import MPI
from xml.etree import ElementTree as ET
from xml.etree.ElementTree import Element, SubElement, ElementTree
from xml.dom import minidom
import h5py
from mpi4py import MPI

# available to all
comm = MPI.COMM_WORLD

# if uw.rank() ==0 and not uw.utils.is_kernel():
#
#     from shutil import copyfile
#     from datetime import datetime
#
#     timestamped_file_name = '{}+{}'.format(datetime.now().strftime("%Y.%m.%d-%H.%M.%S"),__file__)
#     copyfile(__file__,  os.path.join(outputPath,timestamped_file_name))

class OutputFile(object):
    def __init__(self, filename='FrequentOutput'):
        # error check input
        if not isinstance( filename, str):
            raise ValueError("'filename' must be of type 'str', not type {}".format(type(filename)))
        self._filename = filename
        # open the file
        if uw.rank() == 0:
            timestamped_file_name = '{}-{}'.format(filename,datetime.now().strftime("%Y.%m.%d-%H.%M.%S"))
            self.oFile = open(timestamped_file_name, 'w',0)    # zero for no buffering

            # create a link with a simple name, check if it already exists
            if os.path.exists(self._filename):
                os.remove(self._filename) # unlink or remove
            # create symlink
            os.symlink( os.path.basename(self.oFile.name), self._filename)
        else:
            self.oFile = None

    def __del__(self):
        if uw.rank() != 0:
            return

        # close the file
        self.oFile.close()

    def emit(self, data):
        if uw.rank() != 0:
            return

        if data is None:
            self.oFile.write("No data given")
        elif not isinstance( data, str ):
            raise ValueError("Can't process data in Output.emit() class")

        if not data.endswith("\n"): data += "\n" # optionally add new line
        self.oFile.write(data)

# create and enum
class EquationType(object):
    BA, EBA, TALA, ALA = range(4)

class Model(object):
    def __init__(self, outputPath='./output'):
        '''
        Base class for a model. It stores the 'outputPath' directory, animation 'store' and 'log'
        '''
        # record start wall time
        self._start_time = MPI.Wtime()

        # check the outputPath is valid
        if not isinstance( outputPath, str):
            raise ValueError("'outputPath' must be of type 'str', not type {}".format(type(outputPath)))
        # add a backslash
        if not outputPath.endswith('/'): outputPath += '/'
        # create path only on rank 0
        if not os.path.exists(outputPath) and uw.rank()==0:
            print "Creating directory: ",outputPath
            os.makedirs(outputPath)
        uw.barrier()
        self.outputPath = outputPath

        self.animation = glucifer.Store(outputPath+'animation')

        # version of code stored
        self._version = uw.__version__
        self._extradata = None

        self.log_titles = ['step','time','dT','cpu_time']
        self.log = OutputFile(filename=outputPath+'FrequentOutput.dat')

    def checkpoint(self):
        pass

    def init_log(self):
        # print header in metric_log
        titles = self.log_titles
        self.log.emit("# "+'\t'.join(map(str,titles)) ) # print header
        self.log.emit("# Version:"+self._version+" "+self._extradata)

    def record_log(self, data):
        self.log.emit(data)

# class jSwarm(object):
class Swarm(uw.swarm.Swarm):
    '''
    New swarm class, this inherits from the original uw.swarm.Swarm (can be used interchangably) and
    has new features to simplify use.

    Parameters
    ----------
    mesh : underworld.mesh.FeMesh
        The FeMesh the swarm is supported by. See Swarm.mesh property docstring
        for further information.
    particleEscape : bool
        If set to true, particles are deleted when they leave the domain. This
        may occur during particle advection, or when the mesh is deformed.
    particlePerCell : int
        Only used if loadFile is 'None'.
        The number of particles per element to generate.
    varDict : dict
        Only used if loadFile is 'None'.
        A python dictionary defining the swarm variables create on the swarm.
        Each variable is defined by a name and a 2-tuple including the dataType and size.
        eg.
           varDict = {'prevT':('int', 1), 'fooVec':('double', 3))}
        defines 2 swarm variables. 1) An integer variable called 'prevT' and 2) a floating-point vector of size 3 called 'fooVec'
    loadFile : str
        Path to an underworld checkpoint file (eg, swarm.xxx.xdmf) to reload.
    outputPath : str, (default='swarmOutput')
        The path to produce swarm output.

    '''
    '''
    Real difference btw swarm and annulus.swarm is a python dictionary is used to interface with swarm variables. This allows for
    name-based operations to be used. But will cause issues if python views are created and not correctly removed by the user.

    PROBLEMS:
    Swarm & child annulus.Swarm use _setup(). Only after _setup are the swarm variables build.
    It appears a different load from checkpoint strategy is required because the setup phase must perform
    differently in the swarm creation vs swarm load styles.
    '''

    def __init__(self, mesh, particleEscape=True, particlesPerCell=20, varDict=None, loadFile=None, outputPath='swarmOutput', **kwargs ):
        # create swarm discretisation
        super(Swarm, self).__init__(mesh, particleEscape, **kwargs)

        if varDict is not None and not isinstance(varDict, dict):
            raise ValueError("'varDict' must be of type dictionary")
        self._varDict = varDict # save varDict because we must process it after setup

        if loadFile is not None:
            if not os.path.exists(loadFile):
                raise ValueError("The 'loadFile' can't be found")
        self._loadFile = loadFile

        if not isinstance(outputPath, str):
            raise ValueError("'outputPath' must be of type str")
        if not outputPath.endswith('/'): outputPath += '/'
        self._outputPath = outputPath
        # save xdmf enabled timeseries in tuple
        self._xdmf_timeseries = (outputPath+'XDMF.FileSwarm.xdmf', outputPath+'XDMF.temporalSwarm.xmf')

        self._particlesPerCell = particlesPerCell # should be error checked by layout

        # create outputPath and files only with proc 0
        if uw.rank() == 0:
            dirname = os.path.dirname(outputPath)
            if not os.path.exists(dirname):
                os.makedirs(dirname)

            ### build the output files
            with open(self._xdmf_timeseries[0], 'w') as oF:
                xdmf = Element('Xdmf', attrib={'Version':"3.0",
                                               'xmlns:xi':"http://www.w3.org/2001/XInclude"})
                domain = SubElement( xdmf,  'Grid', attrib={'CollectionType':"Temporal",'GridType':"Collection"}) ; domain.text = "\n"
                myXDMF = ET.tostring(xdmf, method='xml')
                output = minidom.parseString(myXDMF).toprettyxml(indent="")
                oF.write(output)

            with open(self._xdmf_timeseries[1], 'w') as oF:
                xdmf = Element('Xdmf', attrib={'Version':"3.0",
                                               'xmlns:xi':"http://www.w3.org/2001/XInclude"})
                domain = SubElement( xdmf,  'Domain')
                grid   = SubElement( domain, 'xi:include', attrib={'href':"XDMF.FileSwarm.xdmf", 'xpointer':"xpointer(//Xdmf/Grid)"} )
                myXDMF = ET.tostring(xdmf, method='xml')
                output = minidom.parseString(myXDMF).toprettyxml(indent="  ")
                oF.write(output)

        if loadFile is not None:
            if not os.path.exists(loadFile):
                raise ValueError("The 'loadFile' can't be found")
        self._loadFile = loadFile

        # create vars dictionary
        self.vars = dict()

    def _setup(self):
        # 'owingCell' and 'particleCoordinates' already created by uw.swarm.Swarm's _setup() func
        self.vars['owningCell'] = self.owningCell
        self.vars['coordinates'] = self.particleCoordinates

        # if loading from checkpoint
        if self._loadFile is not None:
            loadFile = self._loadFile
            path     = os.path.dirname(loadFile)+'/'

            # parse the checkpoint directory for all variables, aka Attributes in xdmf
            tree = ET.parse(loadFile)
            root = tree.getroot()

            count = 0
            for geo in root.iter('Geometry'):
                count += 1
                text=geo.find('DataItem').text
            if count > 1:
                raise RuntimeError("More than one 'Geometry' tags was found. This is not compatible with the current python code")

            # load coordinates first, they are not attribute of the xdmf file
            coord_file = text.split(':')[0]
            self.load(path+coord_file)

            to_load = dict()
            for var in root.iter('Attribute'):
                # get textual name of data
                name = var.attrib.get('Name')
                # get path to hdf5 file
                raw  = list(var)[0].text
                filename = raw.split(':')[0]
                to_load[name] = filename

            # load each swarm variable from the hdf5 file
            for name in to_load:
                filename = path + to_load[name]

                # parse hdf5 data size and shape
                sFile = h5py.File(name=filename, mode='r', driver='mpio', comm=MPI.COMM_WORLD)
                file_data = sFile.get('data')
                if file_data is None:
                    raise RuntimeError("Can't parse {} correctly".format(filename))

                shape, dtype = file_data.shape, str(file_data.dtype)
                if dtype.find('float') > -1: dtype = 'double'
                elif dtype.find('int') > -1: dtype = 'int'

                # create the correct storage and load variable
                if uw.rank() == 0:
                    print("Reloading swarm variables - {} - with shape {}".format(name, shape[1]))
                self.vars[name] = self.add_variable( count=shape[1], dataType=dtype )
                self.vars[name].load(filename)
                sFile.close()

            return # finish here if checkpointing

        # if creating the swarm from scrach
        elif self._particlesPerCell > 0:
            # perform particle build using perCellSpaceFilling
            self.layout = uw.swarm.layouts.PerCellSpaceFillerLayout(self, particlesPerCell=self._particlesPerCell)
            self.populate_using_layout(self.layout)

        # add variables from varDict
        varDict = self._varDict
        if varDict is not None:
            self._addSwarmVar( varDict )


    def _addSwarmVar(self, varDict):

        if not isinstance(varDict, dict):
            raise ValueError("'varDict' must be of type dict")

        registeredVars = 0 # used to report the number of vars created

        # import pdb; pdb.set_trace()
        # for each entry apply the correct swarmvariable declaration
        for name, value in varDict.items():
            if not isinstance(name, str):
                raise ValueError("Name must be 'str' type")

            if self.vars.get(name) is not None:
                # should implement the options to override in future but for now error
                print("Can't add the swarm variable {}. It already exists on the swarm, please delete it first\n".format(name))
                continue

            # if plain swarm variable declaration, ie {'velocity':('double',2)}
            if isinstance(value, (list,tuple)) and len(value) == 2:
                if not (isinstance(value[1], int) and isinstance(value[0], str)):
                    raise ValueError("Invalid swarm variable tuple for {}".format(name))
                self.vars[name] = self.add_variable(count=value[1], dataType=value[0])

            # if uw.function based .. TODO
            # else isinstance(value, uw.function.Function):
            #     print("Read {} as function\n".format(name))

            # Just an idea but doesn't work well with 'stateId'
            # if reading from checkpoint file
            # elif isinstance(value, str):
            #     # check if swarm var exists
            #     if not os.path.exists(value):
            #         raise ValueError("Can't find path {}".format(value) )
            #
            #     # parse h5 file for data configuration
            #     import h5py
            #     from mpi4py import MPI
            #     sFile = h5py.File(name=value, mode='r', driver='mpio', comm=MPI.COMM_WORLD)
            #     file_data = sFile.get('data')
            #     if file_data is None:
            #         print("Can't correctly read input file {} - not creating variable {}".format(value,name))
            #     shape, dtype = file_data.shape, file_data.dtype
            #
            #     # create the correct storage and load variable
            #     self.vars[name] = self.add_variable( count=shape[1], dataType=dtype )
            #     self.vars[name].load(filename=value)
            else:
                print("Can't create swarmvar named {}".format(name))

            print("Success creating var {}".format(name))
            registeredVars += 1

    def _removeSwarmVars(self, varName):
        '''
        Delete a swarm variable from the swarm
        '''

        # remove from vars dictionary
        swarmVar = self.vars.pop(varName)
        # remove from vars list
        swarm.variables.remove(swarmVar)
        del swarmVar # not sure how possible this is due to possible remaining python references

    def save_swarm(self, varDict=None, chkpid=None, modeltime=None):
        '''
        Save swarm to xdmf format to disk, using h5py and libxml.
        By default all swarm variables are saved unless 'varDict' is specified
        Parameters
        ----------
        varDict : dict, default 'None'
            Optional python dictionary defining the swarm variables to save.
            Each entry should defined a name and swarm variable.
            eg.
               varDict = {'prevT':swarm.vars['temperature'], 'fooVec':swarm.vars['prev_velocity'])}
        chkpid : str
            String identifier for the checkpoint made, eg, 'helloWorld'.
        modeltime : float, default 'None'
            Optional model time used in xdmf checkpoint
        '''
        if not isinstance(modeltime, (float, int)):
            modeltime = 0.0
        if varDict is not None and not isinstance(varDict, dict):
            raise ValueError("'varDict', must be of type 'dict'")

        path     = self._outputPath
        filename = path+'coordinates.'+chkpid+'.h5'

        # save the swarm particle .h5 file
        sH = self.particleCoordinates.save(filename, swarmHandle=None)

        # begin writing the xdmf file
        xdmf = Element('Xdmf', attrib={'Version':"3.0",
                                   'xmlns:xi':"http://www.w3.org/2001/XInclude"})
        domain = SubElement( xdmf,  'Domain')

        gc  = str(self.particleGlobalCount)
        dim = str(self.mesh.dim)

        grid   = SubElement( domain, 'Grid', attrib={'Name':"Swarm", 'GridType':"Uniform"} )
        time   = SubElement( grid, 'Time', attrib={'TimeType':"Single", 'Value':str(modeltime)})
        topo   = SubElement( grid, 'Topology', attrib={ 'Type':"POLYVERTEX",'NodesPerElement':gc}) ; topo.text = " "
        geo    = SubElement( grid, 'Geometry', attrib={'Type':'XY'})

        # part of geometry
        x_0 = "{} {}".format(gc, dim)
        d_attrib = {'Format':'HDF', 'NumberType':'Float', 'Precision':'8', 'Dimensions':x_0}

        # d_attrib = {'Format':'HDF', 'NumberType':'Float'self.particleCoordinates.dataType, 'Precision':'8', 'Dimensions':x_0}
        d1      = SubElement( geo, 'DataItem', attrib=d_attrib)
        d1.text =  os.path.basename(sH.filename)+':/data'

        # select python dictionary of vars to save
        varToSave = self.vars
        if varDict is not None:
            varToSave = varDict

        for name,var in self.vars.items():
            if name is 'coordinates': # skip coordinates
                continue

            # save the swarm variable .h5
            filename = path+name+'.'+chkpid+'.h5'
            var.save(filename, sH)

            # GET THE ATTRIBUTES GOING HERE
            dof_count = var.count
            x_0 = "{} {}".format(gc, dof_count)
            lazy = {'NumberType':'Float', 'Precision':'8'}

            if dof_count == 1:
                x_type = 'Scalar'
                d_attrib = {'Type':x_type, 'Center':'Node', 'Name':name}
                att = SubElement( grid, 'Attribute', attrib=d_attrib )

                d_attrib = {'Format':'HDF','Dimensions':x_0}
                # d_attrib = {'Format':'HDF', 'NumberType':var.dataType, 'Precision':'8', 'Dimensions':x_0}
                d1       = SubElement( att, 'DataItem', attrib=d_attrib )
                d1.text  = os.path.basename(filename)+':/data'
            elif dof_count == 2:
                x_type = 'Vector'
                d_attrib = {'Type':x_type, 'Center':'Node', 'Name':name}
                att = SubElement( grid, 'Attribute', attrib=d_attrib )
                d_attrib = {'ItemType':"Function", 'Dimensions':gc+" 3", 'Function':"JOIN($0, $1, 0*$1)"}
                d1       = SubElement( att, 'DataItem', attrib=d_attrib)
                # x-values
                d_attrib = {'ItemType':"HyperSlab", 'Dimensions':gc+" 1", 'Name':"x-component"}
                d2       = SubElement( d1, 'DataItem', attrib=d_attrib)
                d_attrib = {'Format':"XML", 'Dimensions':"3 2"}
                d3       = SubElement( d2, 'DataItem', attrib=d_attrib) ; d3.text  = "0 0 1 1 "+gc+" 1"
                d_attrib = {'Format':"HDF", 'Dimensions':x_0}
                d4       = SubElement( d2, 'DataItem', attrib=d_attrib ) ; d4.text  = os.path.basename(filename)+':/data'
                # y-values
                d_attrib = {'ItemType':"HyperSlab", 'Dimensions':gc+" 1", 'Name':"y-component"}
                d5       = SubElement( d1, 'DataItem', attrib=d_attrib)
                d_attrib = {'Format':"XML", 'Dimensions':"3 2"}
                d6       = SubElement( d5, 'DataItem', attrib=d_attrib) ; d6.text  = "0 1 1 1 "+gc+" 1"
                d_attrib = {'Format':"HDF", 'Dimensions':x_0}
                d7       = SubElement( d6, 'DataItem', attrib=d_attrib ) ; d7.text  = os.path.basename(filename)+':/data'

            elif dof_count == 3:
                x_type = 'Vector'
                d_attrib = {'Type':x_type, 'Center':'Node', 'Name':name}
                att = SubElement( grid, 'Attribute', attrib=d_attrib )

                d_attrib = {'Format':'HDF','Dimensions':x_0}
                # d_attrib = {'Format':'HDF', 'NumberType':var.dataType, 'Precision':'8', 'Dimensions':x_0}
                d1       = SubElement( att, 'DataItem', attrib=d_attrib )
                d1.text  = os.path.basename(filename)+':/data'
            elif dof_count == 6:
                x_type = 'Tensor6'
                d_attrib = {'Type':x_type, 'Center':'Node', 'Name':name}
                att = SubElement( grid, 'Attribute', attrib=d_attrib )
                d_attrib = {'ItemType':'Function','Dimensions':gc+" 6", 'Function':"JOIN($0, $3, $4, $1, $5, $2)"}
                d1       = SubElement( att, 'DataItem', attrib=d_attrib)
                for d_i in xrange(dof_count):
                    ixx = "{}".format(d_i)
                    d_attrib = {'ItemType':"HyperSlab", 'Dimensions':gc+" 1", 'Name':ixx}
                    d2       = SubElement(d1, "DataType", attrib=d_attrib)

                    d_attrib = {'Dimensions':"3 2", 'Format':"XML"}
                    d3       = SubElement(d2, "DataType", attrib=d_attrib) ; d3.text = "0 {} 1 1 {} 1".format(d_i, gc)


                    d_attrib = {'Dimensions':gc+" 2", 'Format':"HDF"}
                    d4       = SubElement(d2, "DataType", attrib=d_attrib) ; d4.text = os.path.basename(filename)+':/data'

            elif dof_count == 9:
                x_type = 'Tensor'
                d_attrib = {'Type':x_type, 'Center':'Node', 'Name':name}
                att = SubElement( grid, 'Attribute', attrib=d_attrib )

                d_attrib = {'Format':'HDF', 'NumberType':'Float', 'Precision':'8', 'Dimensions':x_0}
                # d_attrib = {'Format':'HDF', 'NumberType':var.dataType, 'Precision':'8', 'Dimensions':x_0}
                d1       = SubElement( att, 'DataItem', attrib=d_attrib)
                d1.text  = os.path.basename(filename)+':/data'
            # more to implementent


        if uw.rank()==0:
            # write the single time step
            filename = self._outputPath + '/swarm.'+chkpid+'.xdmf'
            with open(filename, 'w') as oF:
                myXDMF = ET.tostring(xdmf, method='xml')
                output = minidom.parseString(myXDMF).toprettyxml(indent="  ")
                oF.write(output)

            # write xdmf file into the temporal xdmf file
            with open(self._xdmf_timeseries[0], 'r+') as oF:
                oF.seek(-16, 2) # 20 characters in from the EOF
                oF.write('<xi:include href="{}" xpointer="xpointer(//Xdmf/Domain/Grid)"/>\n\n'.format(os.path.basename(filename)))
                oF.write('</Grid>\n</Xdmf>'.format(filename))


        return True


def Annulus_loadswarm(mesh, path):
    '''
    Given a path this function will try and load a swarm.xdmf for it.
    If successful a swarm is returned, else False is returned.
    '''

    if not isinstance(mesh, uw.mesh.FeMesh): # the swarm should have nothing to do with the mesh
        raise ValueError
    if not isinstance(path, str):
        raise ValueError

    if not path.endswith('/'): path += '/'

    swarm = annulus.Swarm(mesh, particleEscape=True, outputPath='./foo/bar/s2')
    swarm.load(path+'/coordinates.h5')

    # import pdb; pdb.set_trace()
    if uw.rank() == 0:
        tree = ET.parse(path+'/swarm.xdmf')
        root = tree.getroot()
        i = 0
        for var in root.iter('Attribute'):
            i += 1
            print("Found Attribute called {}".format( var.attrib.get('Name') ))


    # now read the number of attributes (swarm variables)
    # and load them correctly, building the swarm.vars python dictionary.
    return swarm


class AnnulusConvection(Model):
    '''
    Build a thermo mechanical model with Q1/dQ0 elements in a 2D annulus geometry.
    To use this model 3 stages must be run:
      parameter_setup() ... setup the model, parameters and system of equations
      simulate()        ... run the model
      
    The 3 stages have been chosen to allow for user flexibility.
    '''

    def __init__(self, elRes=(10,36), radialLengths=(1.22,2.22), swarmVars=None, **kwargs):

        # swarmVars can be a dictionary of swarm Variables as below, or a path to a previous swarm checkpoint file
        #  swarmVars is a dictionary of variable names and (optional) hdf5 files where the info can be loaded.
        #  Eg, withSwarm={'plasticStrain':('double',3), 'materialId':('int',1)}, where None would indicate the user will initialise

        super(AnnulusConvection, self).__init__(**kwargs)

        # Because we must build the mesh first, before the fields, this wrapper is difficult to seperate into
        # a straight themo-mechanical wrapper for any mesh discretisation

        # create the FEM mesh
        self.mesh = annulus = uw.mesh._FeMesh_Annulus(elementRes=elRes,
                                      radialLengths=radialLengths, angularExtent=(0.,360.),
                                      periodic = [False, True])

        # create the fields
        self.fields = dict()
        
        # stokes fields
        vField = self.fields['velocity']    = uw.mesh.MeshVariable(annulus, nodeDofCount=2)
        self.fields['pressure']             = uw.mesh.MeshVariable(annulus.subMesh, nodeDofCount=1)
        # heat equation fields
        tField = self.fields['temperature'] = uw.mesh.MeshVariable(annulus, nodeDofCount=1)
        self.fields['tDot']                 = uw.mesh.MeshVariable(annulus, nodeDofCount=1)

        # fields to checkpoint by default
        # self.checkpoint_fields = dict(self.fields.items()) # possibility to chkp all fields
        self.checkpoint_fields = dict( temperature = tField,
                                       tDot        = self.fields['tDot'],
                                       velocity    = vField             )

        # create the nodes Sets
        self.meshSets=dict()
        outer = self.meshSets['outerNodes'] = annulus.specialSets["MaxI_VertexSet"]
        inner = self.meshSets['innerNodes'] = annulus.specialSets["MinI_VertexSet"]
        self.meshSets['boundaryNodes'] = self.meshSets['outerNodes']+self.meshSets['innerNodes']

        # create the dirichet boundary condtions
        self.dirichletBCs=dict()
        # the three types of conditions available
        self.dirichletBCs['temperature'] = uw.conditions.DirichletCondition(
                                                    variable=tField,
                                                    indexSetsPerDof=(inner+outer) )

        self.dirichletBCs['velocity_freeSlip'] =  uw.conditions.RotatedDirichletCondition(
                                    variable        = vField,
                                    indexSetsPerDof = (inner+outer, None),
                                    basis_vectors   = (annulus.bnd_vec_normal, annulus.bnd_vec_tangent))

        self.dirichletBCs['velocity_noSlip']   =  uw.conditions.RotatedDirichletCondition(
                                    variable        = vField,
                                    indexSetsPerDof = (inner+outer, None),
                                    basis_vectors   = (annulus.bnd_vec_normal, annulus.bnd_vec_tangent))

        # set up analytics logging
        self.f     = radialLengths[0]/radialLengths[1]
        self.dT_dr = fn.math.dot( tField.fn_gradient, annulus.fn_unitvec_radial() )

        self.dT_dr_outer_integral  = uw.utils.Integral( mesh=annulus, fn=self.dT_dr,
                                                   integrationType="surface", surfaceIndexSet=outer )

        self.dT_dr_inner_integral  = uw.utils.Integral( mesh=annulus, fn=self.dT_dr,
                                                   integrationType="surface", surfaceIndexSet=inner )

        # start the log file
        self.log_titles += ['<T>','Nu_u','Nu_b']

        # setup visualisation with lavavu
        animation = self.animation
        self.view = glucifer.Figure(store=animation, name="scene1")
        # self.view.append(glucifer.objects.Mesh(annulus))
        self.view.append(glucifer.objects.Surface(mesh=annulus, fn=self.fields['temperature'],
                                              onMesh=True, name='temperature'))
        self.view.append(glucifer.objects.VectorArrows(mesh=annulus, fn=self.fields['velocity'],
                                                  onMesh=True, name='velocity'))

        # Swarm Definition - use wrapper next
        if swarmVars is not None:
            if isinstance(swarmVars, dict):
                self.swarm = Swarm(mesh=self.mesh, particlesPerCell=20, particleEscape=True,
                                    varDict = swarmVars, outputPath=self.outputPath+'/swarm/' )
            elif isinstace(swarmVars, str):
                swarm = annulus.Swarm(mesh=self.mesh,
                                    loadFile=swarmVars, outputPath=self.output+'/swarm/')
            # create swarm advector class - maybe later on???
            self.swarmadvector = uw.systems.SwarmAdvector(velocityField=self.fields['velocity'], swarm=self.swarm)


    def parameter_setup(self, Ra, Di, viscosity_mode=0, benchmark=1, eqn='BA' ):
        # if viscosity_mode is 1: viscosity is 1.
        # else viscosity is temperature dependent
        annulus = self.mesh
        vField  = self.fields['velocity']
        tField  = self.fields['temperature']
        self.Ra = fn.misc.constant(Ra)
        self.Di = fn.misc.constant(Di)
        T_s     = fn.misc.constant(0.091)

        self._extradata = "eqn: "+str(eqn)

        if viscosity_mode == 0:  # isoviscous
            self.fn_eta = fn.misc.constant(1.0)
        else:                    # temp dependent
            self.fn_eta = fn.math.exp(-np.log(1000)*tField)

        # default value
        self.fn_source = 0.

        # define function the strain rate 2nd invariant
        fn_sr_2ndinv = fn.tensor.second_invariant(fn.tensor.symmetric( vField.fn_gradient ))
        if eqn == 'EBA':
            # should give these better names
            self.viscous_dissipation = 2.0*self.fn_eta*fn_sr_2ndinv
            self.adiabatic_heating   = -self.Di*fn.math.dot(annulus.fn_unitvec_radial(), vField) * (tField + T_s)
            self.fn_source = self.Di / self.Ra * self.viscous_dissipation + self.adiabatic_heating

        self.fn_force = tField * Ra * annulus.fn_unitvec_radial()
        
        # setups the systems
        self.model_init()
        
        # flag setups as staged
        self._staged_setup = 1

    def postSolve(self):
        stokesSLE = self.system['stokes']
        # realign solution
        uw.libUnderworld.Underworld.AXequalsX( stokesSLE._rot._cself, stokesSLE._velocitySol._cself, False)
        # remove null space
        uw.libUnderworld.StgFEM.SolutionVector_RemoveVectorSpace(stokesSLE._velocitySol._cself, stokesSLE._vnsVec._cself)

    def model_init(self):
        self.init_log()

        # prepare numerics
        vField = self.fields['velocity']
        pField = self.fields['pressure']
        tField = self.fields['temperature']
        tDot   = self.fields['tDot']

        tBC    = self.dirichletBCs['temperature']
        vBC    = self.dirichletBCs['velocity_freeSlip']

        self.system = dict()
        self.system['heat'] = uw.systems.AdvectionDiffusion(tField, tDot, vField,
                                    fn_diffusivity=1.0, conditions=tBC, fn_sourceTerm = self.fn_source)

        self.system['stokes'] = uw.systems.Stokes(vField, pField,
                                  fn_viscosity = self.fn_eta,
                                  fn_bodyforce = self.fn_force,
                                  conditions   = vBC,
                                  _removeBCs   = False)
        self.solver = dict()
        self.solver['stokes'] = uw.systems.Solver(self.system['stokes'])

    def simulate(self, maxtimesteps=10, visualiseEvery=2, checkpointEvery=5, temp_rtol=1e-5):
        if not hasattr(self,'_staged_setup'):
            raise RuntimeError("Must run 'parameter_setup()' before 'simulate()'")
            
        vField = self.fields['velocity']
        tField = self.fields['temperature']
        tDot   = self.fields['tDot']
        annulus = tField.mesh

        stokesSolver = self.solver['stokes']
        heatSolver   = self.system['heat']

        # controls for timesteping
        time = 0.
        tOld = tField.copy()
        er=1.
        its=chk_its=0

        mycallback = self.postSolve

        while er > 1e-5 and its < maxtimesteps:
            tOld.data[:] = tField.data[:] # record old values

            vField.data[self.meshSets['boundaryNodes'].data] = [0.,0.] # is this reuqired?
            stokesSolver.solve(callback_post_solve=mycallback)

            # visualisation goes here
            self.view.step = its
            if its%visualiseEvery==0:
                self.view.save()
            if its%checkpointEvery==0:
                self.checkpoint(chk_its, time)
                chk_its+=1

            dt = heatSolver.get_max_dt()
            heatSolver.integrate(dt)
            time += dt
            its  += 1

            absErr = uw.utils._nps_2norm(tOld.data-tField.data)
            magT   = uw.utils._nps_2norm(tOld.data)
            er = absErr/magT              # calculate relative variation

            f = self.f
            t_vol_arg = self.mesh.integrate(tField)[0] / self.mesh.integrate(1.)[0]
            Nu_t =   np.log(f)/(2.*np.pi*(1.-f)) * self.dT_dr_outer_integral.evaluate()[0]
            Nu_b = f*np.log(f)/(2.*np.pi*(1.-f)) * self.dT_dr_inner_integral.evaluate()[0]
            cpu_time = MPI.Wtime() - self._start_time # on base class

            # records metrics
            record_string = "{0}\t{1:.5e}\t{2:.5e}\t{3:.5e}\t{4:.5}\t{5:.5e}\t{6:.5e}\n".format(its, time, dt, cpu_time, t_vol_arg, Nu_t, Nu_b)
            self.record_log(record_string)

        # final checkpoint
        self.checkpoint(its)


    def initial_temperature_distribution(self, h5file=None, t_outer=0., t_inner=1., k=4.):
        # setup parameters for temperature distribution
        annulus = self.mesh
        tField  = self.fields['temperature']
        inner   = self.meshSets['innerNodes']
        outer   = self.meshSets['outerNodes']

        if h5file is not None:
            if not isinstance(h5file, str) or not os.path.exists(h5file):
                raise ValueError("'h5file' path is invalid")
            tField.load(h5file, interpolate=True)
            return


        dr = annulus.radialLengths[1] - annulus.radialLengths[0]
        # parameters for linear temperature profile
        dT_dr = (t_outer-t_inner)/(dr)
        c0 = t_inner - dT_dr*annulus.radialLengths[0]

        # wavenumber for perturbation
        k = k
        for ind,coord in enumerate(annulus.data):
            r = np.sqrt(coord[0]**2 + coord[1]**2)
            theta = np.arctan2(coord[1], coord[0])

            # linear component
            linearT = r*dT_dr + c0
            # periodic perturbation component
            pert = 0.2 * np.cos(k*theta)
            tField.data[ind] = min([max([0.,linearT + 1.*pert]),1])

        tField.data[inner.data] = t_inner
        tField.data[outer.data] = t_outer

    # create checkpoint function
    def checkpoint(self, index, time=0.):
        self._checkpoint(self.mesh, self.checkpoint_fields, None, None, index, modeltime=time)

    def _checkpoint( self, mesh, fieldDict, swarm, swarmDict, index,
                    meshName='mesh', swarmName='swarm',
                    enable_xdmf=True, modeltime=0., with_deform_mesh=False):

        prefix = self.outputPath
        # Check the prefix is valid
        if not os.path.exists(prefix) and uw.rank()==0:
            raise RuntimeError("Error checkpointing, can't find {}".format(prefix))
        if not isinstance(index, int):
            raise TypeError("'index' is not of type int")
        ii = str(index).zfill(5)

        # only is there's a mesh then check for fields to save
        if mesh is not None:

            # Error check the mesh and fields
            if not isinstance(mesh, uw.mesh.FeMesh):
                raise TypeError("'mesh' is not of type uw.mesh.FeMesh")
            if not isinstance(fieldDict, dict):
                raise TypeError("'fieldDict' is not of type dict")
            for key, value in fieldDict.iteritems():
                if not isinstance( value, uw.mesh.MeshVariable ):
                    raise TypeError("'fieldDict' must contain uw.mesh.MeshVariable elements")

            # Save the mesh each call of checkpoint() if with_deform_mesh is enabled
            if with_deform_mesh is True:
                mh = mesh.save(prefix+meshName+".h5")
            else:
                # see if we have already saved the mesh. It only needs to be saved once
                if not hasattr( self, 'mH' ):
                    self.mH = mesh.save(prefix+meshName+".h5")
                mh = self.mH

            # save xdmf files
            for key,value in fieldDict.iteritems():
                filename = prefix+key+'-'+ii
                handle = value.save(filename+'.h5', mh)
                if enable_xdmf: value.xdmf(filename, handle, key, mh, meshName, modeltime=modeltime)

        # is there a swarm
        if swarm is not None:

            # Error check the swarms
            if not isinstance(swarm, uw.swarm.Swarm):
                raise TypeError("'swarm' is not of type uw.swarm.Swarm")
            if not isinstance(swarmDict, dict):
                raise TypeError("'swarmDict' is not of type dict")
            for key, value in swarmDict.iteritems():
                if not isinstance( value, uw.swarm.SwarmVariable ):
                    raise TypeError("'fieldDict' must contain uw.swarm.SwarmVariable elements")

            # save xdmf files
            sH = swarm.save(prefix+swarmName+"-"+ii+".h5")
            for key,value in swarmDict.iteritems():
                filename = prefix+key+'-'+ii
                handle = value.save(filename+'.h5')
                if enable_xdmf: value.xdmf(filename, handle, key, sH, swarmName,modeltime=modeltime)
