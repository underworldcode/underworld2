from pyBadlands.model import Model as BadlandsModel
from scipy.interpolate import griddata, interp1d
from scipy.ndimage.filters import gaussian_filter
import numpy as np

from tempfile import gettempdir
from uw_utilities import get_UW_velocities

_tempdir = gettempdir()

from mpi4py import MPI

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

class SPM(object):

    def __init__(self, mesh, velocityField, swarm, materialField, airIndex,
                 sedimentIndex, XML, resolution, checkpoint_interval, surfElevation=0.):
        
        # AutoScaling
        try:
            import sys
            sca = sys.modules["unsupported.scaling"]
            self.sca = sca
            self.scaleDIM = 1.0 / sca.scaling["[length]"].magnitude
            self.scaleTIME = 1.0 / sca.scaling["[time]"].magnitude
        
        except KeyError:
       
            self.scaleDIM = 1.0
            self.scaleTIME = 1.0

        self.mesh = mesh
        self.velocityField = velocityField
        self.swarm = swarm
        self.material_index = materialField
        self.airIndex = airIndex
        self.sedimentIndex = sedimentIndex
        self.resolution = resolution
        self.surfElevation = surfElevation
        self.checkpoint_interval = checkpoint_interval / self.scaleTIME

        self.XML = XML
        self.badlands_model = BadlandsModel()
        self.badlands_model.load_xml(self.XML)

        self.minCoord = self.mesh.minCoord
        self.maxCoord = self.mesh.maxCoord
        self.time_years = 0.

        self._tmp = _tempdir
        self.sca = None


        # Create Initial Flat DEM
        self.dem = self._generate_flat_dem(self.minCoord,
                                           self.maxCoord,
                                           self.resolution,
                                           self.surfElevation,
                                           self.scaleDIM)

        self._demfile = self._tmp+"/dem.csv"
        
        if rank == 0:
            np.savetxt(self._demfile, self.dem)
        comm.Barrier()

        # Build Mesh
        self.badlands_model.build_mesh(self._demfile, verbose=False)

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

    def solve(self, dt):
            
        if self.sca:
            dt_years = sca.Dimensionalize(dt, sca.unitRegistry.years).magnitude
        else:
            dt_years = dt

        rg = self.badlands_model.recGrid

        if self.mesh.dim == 2:
            zVals = rg.regZ.mean(axis = 1)
            np_surface = np.column_stack((rg.regX, zVals)) * self.scaleDIM

        if self.mesh.dim == 3:
            np_surface = np.column_stack((rg.rectX, rg.rectY, rg.rectZ))*self.scaleDIM

        # Get Velocity Field at the surface
        tracer_velocity_mps = get_UW_velocities(np_surface, self.velocity_field) * self.scaleTIME / self.scaleDIM

        # Use the tracer vertical velocities to deform the Badlands TIN
        # convert from meters per second to meters displacement over the whole iteration
        tracer_disp = tracer_velocity_mps * self.SECONDS_PER_YEAR * dt_years
        self._inject_badlands_displacement(self.time_years, dt_years, tracer_disp, sigma)

        # Run the Badlands model to the same time point
        self.badlands_model.run_to_time(self.time_years+dt_years)
        
        self.time_years += dt_years

        # TODO: Improve the performance of this function
        self._update_material_types()

        return
    
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
    
    def _update_material_types(self):

        # What do the materials (in air/sediment terms) look like now?
        if self.mesh.dim == 3:
            material_flags = self._determine_particle_state()
        if self.mesh.dim == 2:
            material_flags = self._determine_particle_state_2D()

        # If any materials changed state, update the Underworld material types
        mi = self.material_index.data

        # convert air to sediment
        sedimented_mask = np.logical_and(np.in1d(mi, self.airIndex), material_flags)
        mi[sedimented_mask] = self.sedimentIndex

        # convert sediment to air
        eroded_mask = np.logical_and(np.in1d(mi, self.sedimentIndex), ~material_flags)
        mi[eroded_mask] = self.airIndex
    
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
    
    def _generate_flat_dem(self, minCoord, maxCoord, resolution, elevation, scale=1.):
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

        if self.mesh.dim == 2:
            minCoord = (minCoord[0], minCoord[0])
            maxCoord = (maxCoord[0], maxCoord[0])
            ny = nx

        items = []
        # FIXME: there should be a fast numpy way to do this
        for y in np.linspace(minCoord[1]/scale, maxCoord[1]/scale, ny):
            for x in np.linspace(minCoord[0]/scale, maxCoord[0]/scale, nx):
                items.append([x, y, elevation/scale])

        # NOTE: Badlands uses the difference in X coord of the first two points to determine the resolution.
        # This is something we should fix.
        # This is why we loop in y/x order instead of x/y order.
        return np.array(items)
