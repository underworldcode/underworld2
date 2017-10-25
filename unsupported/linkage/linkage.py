import time
import numpy as np
from scipy.interpolate import griddata, interp1d
from scipy.ndimage.filters import gaussian_filter
import underworld as uw

from tempfile import gettempdir
from uw_utilities import get_UW_velocities

_tempdir = gettempdir()

class LinkageSolver(object):

    def __init__(self, solver, **kwargs):

        self.function = None
        self.args = None

        if callable(solver):
            self.function = solver
        else:
            self.solver = solver
        
        self.args = kwargs
            
    def solve(self):
        if self.function != None:
            self.function(**self.args)
        else:
            self.solver.solve(**self.args)


class LinkageModel(object):
    """
    A LinkageModel joins an Underworld and a Badlands model. Underworld models
    the domain in 3D and Badlands models the surface processes.

    LinkageModel hides the details of how the two models communicate so you can
    concentrate on building them.

    To define a linked model, instantiate a LinkageModel object and set, at
    least, the following members:

    mesh: the FeMesh that the model is defined over
    swarm: TODO
    material_index: an array of material indices. These will be changed when Badlands runs a tick.
    velocity_field: an Underworld velocity field. It must be defined over the same domain as the Badlands surface.

        self.velocityField is used to advect the Badlands surface, so it should
        be representative of the movement of the surface you are modelling.

        On each iteration, the material types of self.mesh will be altered
        according to Badland's processes. Particles which have transitioned
        from air to sediment will be given the self.sedimentIndex material
        type. Particles which have transitioned from sediment to air will be
        given the self.airIndex material type. You can override these if you
        wish.

        NOTE: the Badlands surface and Underworld mesh must be defined over the
        same coordinate system.


    badlands_model: the Badlands model which you would like to use. It must be initalised (XML and DEM loaded). It must be at time=0.
    update_function: a function which is called on each iteration. It must perform all of the per-iteration processing required for your Underworld model (usually, solving the Stokes equation and advecting the system.) It has the interface func(linkage, update_seconds)
    You can get the current time from linkage.t
    You should not run more than max_seconds or your checkpoints will no longer be synchronised.
    You must return the number of seconds that you *did* run (usually advector.get_max_dt())

        You might also use this to output interesting data to disk or to modify
        the behaviour of the model at a specific time.
        The update_function member can be changed at any time (say, if you're running in a Jupyter Notebook and want to adjust the model mid-run).

        Remember that you can store data on the linkage object like so:
            linkage.private_stuff = some object
        and get it back during the update/checkpoint calls:
            some object = linkage.private_stuff

    checkpoint_function: a function which is called at the end of each checkpoint interval. You should write any relevant Underworld state to disk. Badlands state is written to disk automatically.
        The function is called with arguments 'linkage' and 'checkpoint_number'
        linkage: the linkage model object
        checkpoint_number: integer increasing by 1 per checkpoint. The initial state of the system is checkpointed as number 0, and subsequent states are 1..n.
        time_years: current time of the model (in years) - note that this happens AFTER advection. Usually you will write this to disk so output can be loaded synchronised with Badlands.

    air_material_indices
    sediment_material_indices

    You can then run the model by calling the run_for_years() function.

    Output will be saved as files on disk.
    """

    def __init__(self):
        # These MUST be set after you instantiate the object
        self.velocity_field = None
        self.material_index = None
        self.update_function = None
        self.badlands_model = None
        self.mesh = None
        self.swarm = None
        self.solver = None
        self._tmp = _tempdir
        #self.outputDir = "outblds"

        # If the grid is deformed during your coupled simulation
        # this parameter needs to be set to False.
        self.fixvelfield = True

        # Badlands and Underworld will write synchronised output this many
        # years apart
        self.checkpoint_interval = 10000

        # Default material map
        # On the Underworld side, we assume two materials: air (index 0) and
        # sediment (index 1)
        # On the Badlands side, we assume one erosion layer, so there is only
        # air and sediment
        # See https://github.com/UnderworldBadlandsLinkage/linkage/wiki/Material-maps
        # for more information
        self.material_map = [
            [0],
            [1],
        ]

        # --- You don't need to modify any settings below this line ---
        try:
            import sys
            sca = sys.modules["unsupported.scaling"]
            self.scaleDIM = 1.0 / sca.scaling["[length]"]
            self.scaleTIME = 1.0 / sca.scaling["[time]"]
        
        except KeyError:
       
            self.scaleDIM = 1.0
            self.scaleTIME = 1.0
        
        self.time_years = 0.  # Simulation time in years. We start at year 0.
        self._model_started = False  # Have we performed one-time initialisation yet?
        self._disp_inserted = False

        self.SECONDS_PER_YEAR = float(365 * 24 * 60 * 60)
        self._checkpoint_number = 0  # used to give each checkpoint a unique index
        self._next_checkpoint_years = None

        # Set this variable to stop Badlands changes from modifying the
        # Underworld material types. This is usually used for open-loop testing
        # to ensure that BL and UW do not desync after many timesteps.
        self.disable_material_changes = False

    def run_for_years(self, years, sigma=0, verbose=False):
        """
        Run the model for a number of years. Possibility to smooth Underworld velocity
        field using a Gaussian filter.
        """
        if not self._model_started:
            self._startup()

        end_years = self.time_years + years

        write_checkpoint = True

        while self.time_years < end_years:

            if verbose and uw.rank() == 0:
                t0 = time.clock()

            # Get solution from Stokes solver.
            self.solver.solve()

            uw.barrier()
            if verbose and uw.rank() == 0:
                tloop = time.clock() - t0
                print '- Solver function took %0.02f seconds' % (time.clock() - t0)
                t0 = time.clock()

            # Checkpointing fields and swarm
            if write_checkpoint or self.time_years == 0.:
                self.checkpoint_function(self, self._checkpoint_number, self.time_years)
                self._checkpoint_number += 1
                self._next_checkpoint_years += self.checkpoint_interval
                if verbose and uw.rank() == 0:
                    tloop = time.clock() - t0
                    print '- Checkpointing function took %0.02f seconds' % (time.clock() - t0)
                    t0 = time.clock()
            write_checkpoint = False

            # What's the longest we can run before we have to write a
            # checkpoint or stop?
            max_years = self._next_checkpoint_years - self.time_years
            max_years = min(end_years - self.time_years, max_years)
            max_seconds = max_years * self.SECONDS_PER_YEAR

            # Ask the Underworld model to update
            dt_seconds = self.update_function(self, max_seconds)
            assert int(dt_seconds * 100.) <= int(max_seconds * 100.), "Maximum dt (seconds) for the update function was %s, but it ran for more than that (%s seconds)" % (max_seconds, dt_seconds)

            uw.barrier()
            if verbose and uw.rank() == 0:
                tloop = time.clock() - t0
                print '- Update function took %0.02f seconds' % (time.clock() - t0)
                t0 = time.clock()

            # Do we need to write a checkpoint later?
            # TODO: make sure floating point imperfections don't desync the seconds/years counters on both sides,
            # especially around writing checkpoints
            if dt_seconds == max_seconds:
                write_checkpoint = True

            dt_years = dt_seconds / self.SECONDS_PER_YEAR

            rg = self.badlands_model.recGrid

            if self.mesh.dim == 2:
                zVals = rg.regZ.mean(axis = 1)
                np_surface = np.column_stack((rg.regX, zVals)) * self.scaleDIM

            if self.mesh.dim == 3:
                np_surface = np.column_stack((rg.rectX, rg.rectY, rg.rectZ))*self.scaleDIM

            #tracer_velocity_mps = np_velocity_field.evaluate(np_surface) * self.scaleTIME / self.scaleDIM
            tracer_velocity_mps = get_UW_velocities(np_surface, self.velocity_field) * self.scaleTIME / self.scaleDIM

            uw.barrier()
            if verbose and uw.rank() == 0:
                tloop = time.clock() - t0
                print '- Evaluate velocity field function took %0.02f seconds' % (time.clock() - t0)
                t0 = time.clock()

            ### INTERFACE PART 1: UW->BL
            # Use the tracer vertical velocities to deform the Badlands TIN
            # convert from meters per second to meters displacement over the whole iteration
            tracer_disp = tracer_velocity_mps * self.SECONDS_PER_YEAR * dt_years
            self._inject_badlands_displacement(self.time_years, dt_years, tracer_disp, sigma)

            uw.barrier()
            if verbose and uw.rank() == 0:
                tloop = time.clock() - t0
                print '- Build displacement function took %0.02f seconds' % (time.clock() - t0)
                t0 = time.clock()

            # Run the Badlands model to the same time point
            self.badlands_model.run_to_time(self.time_years + dt_years)
            
            uw.barrier()
            if verbose and uw.rank() == 0:
                tloop = time.clock() - t0
                print '- Running badlands took %0.02f seconds' % (time.clock() - t0)
                t0 = time.clock()

            # Advance time
            self.time_years += dt_years

            ### INTERFACE PART 2: BL->UW
            # TODO: Improve the performance of this function
            self._update_material_types()
            
            uw.barrier()
            if verbose and uw.rank() == 0:
                tloop = time.clock() - t0
                print '- Update material type took %0.02f seconds' % (time.clock() - t0)
                t0 = time.clock()

        # Get solution from Stokes solver.
        self.solver.solve()

        # Checkpointing fields and swarm for last time step
        self.checkpoint_function(self, self._checkpoint_number, self.time_years)
        self._checkpoint_number += 1
        self._next_checkpoint_years += self.checkpoint_interval

    @staticmethod
    def generate_flat_dem(minCoord, maxCoord, resolution, elevation, scale=1.):
        """
        Generate a flat DEM. This can be used as the initial Badlands state.

        minCoord: tuple of (X, Y, Z) coordinates defining the minimum bounds of
                  the DEM. Only the X and Y coordinates are used.
        maxCoord: tuple of (X, Y, Z) coordinates defining the maximum bounds of
                  the DEM. Only the X and Y coordinates are used.
        resolution: resolution of the model. Badlands assumes dx = dy.
        elevation: the Z parameter that each point is created at
        scale: the scaling factor between the 2 codes

        For your convenience, minCoord and maxCoord are designed to have the
        same formatting as the Underworld FeMesh_Cartesian minCoord and
        maxCoord parameters.

        IMPORTANT: minCoord and maxCoord are defined in terms of the Underworld
        coordinate system, but the returned DEM uses the Badlands coordinate
        system.

        Note that the initial elevation of the Badlands surface should coincide
        with the material transition in Underworld.
        """

        # Calculate number of nodes from required resolution.
        nx = np.int((maxCoord[0] - minCoord[0]) / resolution) 
        ny = np.int((maxCoord[1] - minCoord[1]) / resolution)

        items = []
        # FIXME: there should be a fast numpy way to do this
        for y in np.linspace(minCoord[1]/scale, maxCoord[1]/scale, ny):
            for x in np.linspace(minCoord[0]/scale, maxCoord[0]/scale, nx):
                items.append([x, y, elevation/scale])

        # NOTE: Badlands uses the difference in X coord of the first two points to determine the resolution.
        # This is something we should fix.
        # This is why we loop in y/x order instead of x/y order.
        return np.array(items)

    def _startup(self):
        """
        Perform one-time initialisation of the models.

        We load everything, transfer the initial surface elevation from
        Badlands to Underworld, then write the initial state to disk.
        """
        assert not self._model_started

        # Make sure the linkage has been correctly configured
        for k in ['velocity_field', 'material_index', 'update_function', 'badlands_model', 'mesh', 'swarm']:
            assert getattr(self, k) is not None, "You must configure your LinkageModel with a '%s' member" % k

        # Make sure UW and BL are operating over the same XY domain
        if self.mesh.dim == 3:
            rg = self.badlands_model.recGrid
            bl_xy = np.array([rg.rectX.min(),
                              rg.rectX.max(),
                              rg.rectY.min(),
                              rg.rectY.max()])
            uw_xy = np.array([self.mesh.minCoord[0]/self.scaleDIM,
                              self.mesh.maxCoord[0]/self.scaleDIM,
                              self.mesh.minCoord[1]/self.scaleDIM,
                              self.mesh.maxCoord[1]/self.scaleDIM])
            tolerance = 1e-5
            differences = np.abs(bl_xy - uw_xy)
            assert np.all(differences < tolerance), "Badlands and Underworld must operate over the same domain (Badlands has %s, but Underworld has %s)" % (bl_xy, uw_xy)

        # If we are running a T-model, compare dimension in x only...
        if self.mesh.dim == 2:
            rg = self.badlands_model.recGrid
            bl_x = np.array([rg.rectX.min(), 
                             rg.rectX.max()])
            uw_x = np.array([self.mesh.minCoord[0]/self.scaleDIM,
                             self.mesh.maxCoord[0]/self.scaleDIM])
            tolerance = 1e-5
            differences = np.abs(bl_x - uw_x)
            assert np.all(differences < tolerance), "Badlands and Underworld must operate over the same domain (Badlands has %s, but Underworld has %s)" % (bl_x, uw_x)

        self.badlands_model.input.disp3d = True  # enable 3D displacements
        self.badlands_model.input.region = 0  # TODO: check what this does
        
        # Override the checkpoint/display interval in the Badlands model to
        # ensure BL and UW are synced
        self.badlands_model.input.tDisplay = self.checkpoint_interval

        # Set Badlands minimal distance between nodes before regridding
        self.badlands_model.force.merge3d = self.badlands_model.input.Afactor * self.badlands_model.recGrid.resEdges * 0.5

        # Transfer the initial DEM state to Underworld
        self._update_material_types()
        self._next_checkpoint_years = 0.

        # Bodge Badlands to perform an initial checkpoint
        # FIXME: we need to run the model for at least one iteration before this is generated. It would be nice if this wasn't the case.
        self.badlands_model.force.next_display = 0

        self._model_started = True

    def _determine_particle_state_2D(self):
        known_xy = self.badlands_model.recGrid.tinMesh['vertices']*self.scaleDIM  # points that we have known elevation for
        known_z = self.badlands_model.elevation*self.scaleDIM  # elevation for those points
        
        xs = self.badlands_model.recGrid.regX*self.scaleDIM
        ys = self.badlands_model.recGrid.regY*self.scaleDIM
        
        grid_x, grid_y = np.meshgrid(xs, ys)
        interpolate_z = griddata(known_xy, known_z, (grid_x, grid_y), method='nearest').T
        interpolate_z = interpolate_z.mean(axis=1)
      
        f = interp1d(xs, interpolate_z)
        
        uw_surface = self.swarm.particleCoordinates.data
        bdl_surface = f(uw_surface[:,0])
        
        flags = uw_surface[:,1] < bdl_surface
        
        return flags

    def _determine_particle_state(self):
        # Given Badlands' mesh, determine if each particle in 'volume' is above
        # (False) or below (True) it.

        # To do this, for each X/Y pair in 'volume', we interpolate its Z value
        # relative to the mesh in blModel. Then, if the interpolated Z is
        # greater than the supplied Z (i.e. Badlands mesh is above particle
        # elevation) it's sediment (True). Else, it's air (False).

        # TODO: we only support air/sediment layers right now; erodibility
        # layers are not implemented

        known_xy = self.badlands_model.recGrid.tinMesh['vertices']*self.scaleDIM  # points that we have known elevation for
        known_z = self.badlands_model.elevation*self.scaleDIM  # elevation for those points

        volume = self.swarm.particleCoordinates.data

        interpolate_xy = volume[:, [0, 1]]

        # NOTE: we're using nearest neighbour interpolation. This should be
        # sufficient as Badlands will normally run at a much higher resolution
        # than Underworld. 'linear' interpolation is much, much slower.
        interpolate_z = griddata(points=known_xy, values=known_z, xi=interpolate_xy, method='nearest')

        # True for sediment, False for air
        flags = volume[:, 2] < interpolate_z

        return flags

    def load_badlands_dem_file(self, filename):
        self.badlands_model.build_mesh(filename, verbose=False)

    def load_badlands_dem_array(self, array):
        # for now, write it out to a temp file and load that into badlands
        np.savetxt(self._tmp+'/dem.csv', array)
        self.load_badlands_dem_file(self._tmp+'/dem.csv')

    def _inject_badlands_displacement(self, time, dt, disp, sigma):
        """
        Takes a plane of tracer points and their DISPLACEMENTS in 3D over time
        period dt applies a gaussian filter on it. Injects it into Badlands as 3D
        tectonic movement.
        """

        # The Badlands 3D interpolation map is the displacement of each DEM
        # node at the end of the time period relative to its starting position.
        # If you start a new displacement file, it is treated as starting at
        # the DEM starting points (and interpolated onto the TIN as it was at
        # that tNow).


        # kludge; don't keep adding new entries
        if self._disp_inserted:
            self.badlands_model.force.T_disp[0, 0] = time
            self.badlands_model.force.T_disp[0, 1] = (time + dt)
        else:
            self.badlands_model.force.T_disp = np.vstack(([time, time + dt], self.badlands_model.force.T_disp))
            self._disp_inserted = True

        # Extent the velocity field in the third dimension
        if self.mesh.dim == 2:
            dispX = np.tile(disp[:,0], self.badlands_model.recGrid.rny)
            dispY = np.zeros((self.badlands_model.recGrid.rnx * self.badlands_model.recGrid.rny,))
            dispZ = np.tile(disp[:,1], self.badlands_model.recGrid.rny)

            disp = np.zeros((self.badlands_model.recGrid.rnx * self.badlands_model.recGrid.rny,3))
            disp[:,0] = dispX
            disp[:,1] = dispY
            disp[:,2] = dispZ
        
        # Gaussian smoothing
        if sigma>0:
            dispX = np.copy(disp[:,0]).reshape(self.badlands_model.recGrid.rnx, self.badlands_model.recGrid.rny)
            dispY = np.copy(disp[:,1]).reshape(self.badlands_model.recGrid.rnx, self.badlands_model.recGrid.rny)
            dispZ = np.copy(disp[:,2]).reshape(self.badlands_model.recGrid.rnx, self.badlands_model.recGrid.rny)
            smoothX = gaussian_filter(dispX, sigma)
            smoothY = gaussian_filter(dispY, sigma)
            smoothZ = gaussian_filter(dispZ, sigma)
            disp[:,0] = smoothX.flatten()
            disp[:,1] = smoothY.flatten()
            disp[:,2] = smoothZ.flatten()

        self.badlands_model.force.injected_disps = disp

    def _update_material_types(self):
        if self.disable_material_changes:
            return

        # What do the materials (in air/sediment terms) look like now?
        if self.mesh.dim == 3:
            material_flags = self._determine_particle_state()
        if self.mesh.dim == 2:
            material_flags = self._determine_particle_state_2D()

        # If any materials changed state, update the Underworld material types
        mi = self.material_index.data

        # convert air to sediment
        sedimented_mask = np.logical_and(np.in1d(mi, self.material_map[0]), material_flags)
        mi[sedimented_mask] = self.material_map[1][0]

        # convert sediment to air
        eroded_mask = np.logical_and(np.in1d(mi, self.material_map[1]), ~material_flags)
        mi[eroded_mask] = self.material_map[0][0]
