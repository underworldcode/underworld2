import os
import underworld as uw
import numpy as np
from underworld import function as fn
uw.matplotlib_inline()
import glucifer
from datetime import datetime
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

        print "Doing del"
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

        self.log_titles = ['step','dT', 'time', 'cpu_time']
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

class AnnulusConvection(Model):
    '''
    Build a thermo mechanical model with Q1/dQ0 elements in a 2D annulus geometry
    '''

    def __init__(self, elRes=(10,36), radialLengths=(1.22,2.22), **kwargs):

        super(AnnulusConvection, self).__init__(**kwargs)

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
        self.checkpoint_fields = dict( temperature = self.fields['temperature'],
                                       tDot        = self.fields['tDot']        )

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

        self.dirichletBCs['velocity_freeSlip']      =  uw.conditions.RotatedDirichletCondition(
                                    variable        = vField,
                                    indexSetsPerDof = (inner+outer, None),
                                    basis_vectors   = (annulus.rot_vec_normal, annulus.rot_vec_tangent))

        self.dirichletBCs['velocity_noSlip']        =  uw.conditions.RotatedDirichletCondition(
                                    variable        = vField,
                                    indexSetsPerDof = (inner+outer, None),
                                    basis_vectors   = (annulus.rot_vec_normal, annulus.rot_vec_tangent))

        # set up analytics logging
        self.f = radialLengths[0]/radialLengths[1]

        self.volume          = uw.utils.Integral(fn=1., mesh=annulus)
        self.tField_integral = uw.utils.Integral(fn=tField, mesh=annulus)
        self.dT_dr           = fn.math.dot(tField.fn_gradient,annulus.fn_unitvec_radial())

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

    def postSolve(self):
        stokesSLE = self.system['stokes']
        # remove null space
        uw.libUnderworld.StgFEM.SolutionVector_RemoveVectorSpace(stokesSLE._velocitySol._cself, stokesSLE._asv._cself)
        # realign solution
        uw.libUnderworld.Underworld.AXequalsX( stokesSLE._rot._cself, stokesSLE._velocitySol._cself, False)

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

        vField = self.fields['velocity']
        tField = self.fields['temperature']
        tDot   = self.fields['tDot']

        stokesSolver = self.solver['stokes']
        heatSolver   = self.system['heat']

        # initialise running storage
        ts_t_vol_avg = []
        ts_Nu_t = []
        ts_Nu_b = []

        # controls for timesteping
        tOld = tField.copy()
        er=1.
        its=0

        mycallback = self.postSolve

        while er > 1e-5 and its < maxtimesteps:
            tOld.data[:] = tField.data[:] # record old values

            vField.data[self.meshSets['boundaryNodes'].data] = [0.,0.] # is this reuqired?
            stokesSolver.solve(callback_post_solve=mycallback)

            dt = heatSolver.get_max_dt()
            its+=1
            # visualisation goes here
            self.view.step = its
            if its%visualiseEvery==0:
                self.view.save()
            if its%checkpointEvery==0:
                self.checkpoint(its)

            heatSolver.integrate(dt)

            absErr = uw.utils._nps_2norm(tOld.data-tField.data)
            magT   = uw.utils._nps_2norm(tOld.data)
            er = absErr/magT              # calculate relative variation

            f = self.f
            t_vol_arg = self.tField_integral.evaluate()[0] / self.volume.evaluate()[0]
            Nu_t =   np.log(f)/(2.*np.pi*(1.-f)) * self.dT_dr_outer_integral.evaluate()[0]
            Nu_b = f*np.log(f)/(2.*np.pi*(1.-f)) * self.dT_dr_inner_integral.evaluate()[0]
            cpu_time = MPI.Wtime() - self._start_time # on base class

            # records metrics
            record_string = "{0}\t{1:.5e}\t{2:.5e}\t{3:.5e}\t{4:.5}\t{5:.5e}\n".format(its, dt, cpu_time, t_vol_arg, Nu_t, Nu_b)
            self.record_log(record_string)
            #
            # ts_t_vol_avg.append(t_vol_arg)
            # ts_Nu_t.append(Nu_t)
            # ts_Nu_b.append(Nu_b)

    def initial_temperature_distribution(self, h5file=None, t_outer=0., t_inner=1., k=4.):
        # setup parameters for temperature distribution
        annulus = self.mesh
        tField  = self.fields['temperature']
        inner   = self.meshSets['innerNodes']
        outer   = self.meshSets['outerNodes']

        if h5file is not None:
            if not isinstance(h5file, str) or not os.path.exists(h5file):
                raise ValueError("'h5file' must be of type 'str' and reference a valid hdf5 file")
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
    def checkpoint(self, index):
        self._checkpoint(self.mesh, self.checkpoint_fields, None, None, index)

    def _checkpoint( self, mesh, fieldDict, swarm, swarmDict, index,
                    meshName='mesh', swarmName='swarm',
                    enable_xdmf=True, with_deform_mesh=False):

        prefix = self.outputPath
        # Check the prefix is valid
        if not os.path.exists(prefix) and uw.rank()==0:
            raise RuntimeError("Error checkpointing, can't find {}".format(prefix))
        if not isinstance(index, int):
            raise TypeError("'index' is not of type int")
        ii = str(index)

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
                if enable_xdmf: value.xdmf(filename, handle, key, mh, meshName)

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
                if enable_xdmf: value.xdmf(filename, handle, key, sH, swarmName)
