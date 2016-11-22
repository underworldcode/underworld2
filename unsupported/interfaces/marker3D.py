
import numpy as np
import underworld as uw

from scipy.spatial import cKDTree as kdTree

class markerSurface3D(object):
    """
    All the bits and pieces needed to define a marker surface (in 3D) from a string of points
    """


    def __init__(self, mesh, velocityField, pointsX, pointsY, pointsZ, fthickness, insidePt=(0.0,0.0,0.0)):


        # Marker swarms are probably sparse, and on most procs will have nothing to do
        # if there are no particles (or not enough to compute what we need)
        # then set this flag and return appropriately. This can be checked once the swarm is
        # populated.

        self.empty = False

        # Should do some checking first

        self.mesh = mesh
        self.velocity = velocityField
        self.thickness = fthickness
        self.ID = fID
        self.insidePt = insidePt
        self.director = None

        # Set up the swarm and variables on all procs

        self.swarm = uw.swarm.Swarm( mesh=self.mesh, particleEscape=True )
        self.director = self.swarm.add_variable( dataType="double", count=3)
        self._swarm_advector = uw.systems.SwarmAdvector( swarm=self.swarm,
                                                         velocityField=self.velocity, order=2 )

        self.swarm.add_particles_with_coordinates(np.stack((pointsX, pointsY, pointsZ)).T)
        self.director.data[...] = 0.0

        self._update_kdtree()
        self._update_surface_normals()

        return



    def add_points(self, pointsX, pointsY, pointsZ):

        self.swarm.add_particles_with_coordinates(np.stack((pointsX, pointsY, pointsZ)).T)

        self.rebuild()


    def rebuild(self):

        self._update_kdtree()
        self._update_surface_normals()

        return


    def _update_kdtree(self):

        self.empty = False
        self.swarm.shadow_particles_fetch()

        dims = self.swarm.particleCoordinates.data.shape[1]

        pc = np.append(self.swarm.particleCoordinates.data,
                       self.swarm.particleCoordinates.data_shadow)

        all_particle_coords = pc.reshape(-1,dims)

        if len(all_particle_coords) < 4:
            self.empty = True
            self.kdtree = lambda x: float('inf')
        else:
            self.kdtree = kdTree(all_particle_coords)

        return


    def advection(self, dt):
        """
        Update marker swarm particles as material points and rebuild data structures
        """
        self._swarm_advector.integrate( dt, update_owners=True)
        self.swarm.shadow_particles_fetch()

        self._update_kdtree()
        self._update_surface_normals()

        uw.barrier()

        return



    def compute_marker_proximity(self, coords):
        """
        Build a mask of values for points within the influence zone.
        """

        if self.empty:
            return np.empty((0,1)), np.empty(0, dtype="int")

        d, p   = self.kdtree.query( coords, distance_upper_bound=self.thickness )

        fpts = np.where( np.isinf(d) == False )[0]

        proximity = np.zeros((coords.shape[0],1))
        proximity[fpts] = self.ID

        return proximity, fpts


    def compute_normals(self, coords):

        # make sure this is called by all procs including those
        # which have an empty self

        self.swarm.shadow_particles_fetch()

        # Nx, Ny = _points_to_normals(self)

        if self.empty:
            return np.empty((0,2)), np.empty(0, dtype="int")

        d, p   = self.kdtree.query( coords, distance_upper_bound=self.thickness )

        fpts = np.where( np.isinf(d) == False )[0]
        director = np.zeros_like(coords)

        dims = self.swarm.particleCoordinates.data.shape[1]
        fdirector = np.append(self.director.data,
                              self.director.data_shadow).reshape(-1,dims)

        director[fpts] = fdirector[p[fpts]]

        return director, fpts


    def compute_signed_distance(self, coords, distance=None):

        # make sure this is called by all procs including those
        # which have an empty self

        if not distance:
            distance = self.thickness

        if self.empty:
            return np.empty((0,1)), np.empty(0, dtype="int")

        self.swarm.shadow_particles_fetch()
        dims = self.swarm.particleCoordinates.data.shape[1]
        fdirector = np.append(self.director.data,
                              self.director.data_shadow).reshape(-1,dims)

        d, p  = self.kdtree.query( coords, distance_upper_bound=distance )

        fpts = np.where( np.isinf(d) == False )[0]

        director = np.zeros_like(coords)  # Let it be zero outside the region of interest
        director = fdirector[p[fpts]]

        vector = coords[fpts] - self.kdtree.data[p[fpts]]

        signed_distance = np.empty((coords.shape[0],1))
        signed_distance[...] = np.inf

        sd = np.einsum('ij,ij->i', vector, director)
        signed_distance[fpts,0] = sd[:]

        return signed_distance, fpts


    def _update_surface_normals(self):
        """
        Rebuilds the normals for the string of points
        """

        # This is the case if there are too few points to
        # compute normals so there can be values to remove

        if self.empty:
            self.director.data[...] = 0.0
        else:

            particle_coords = self.swarm.particleCoordinates.data

            Nx = np.empty(self.swarm.particleLocalCount)
            Ny = np.empty(self.swarm.particleLocalCount)
            Nz = np.empty(self.swarm.particleLocalCount)

            for i, xyz in enumerate(particle_coords):
                r, neighbours = self.kdtree.query(particle_coords[i], k=3)

                # this point is neighbour[0] and neighbour points are neighbours[(1,2,3)]
                XYZ1 = self.kdtree.data[neighbours[1]]
                XYZ2 = self.kdtree.data[neighbours[2]]
                XYZ3 = self.kdtree.data[neighbours[3]]

                dXYZ1 = XYZ2 - XYZ1
                dXYZ2 = XYZ3 - XYZ1

                # Cross product of those 2 vectors can be use as the local normal (perhaps)

                Nx, Ny, Nz = np.cross(dXYZ1, dXYZ2)

                if (self.insidePt):
                    sign = np.sign((self.insidePt[0] - xyz[0]) * Nx[i] +
                                   (self.insidePt[1] - xyz[1]) * Ny[i] +
                                   (self.insidePt[1] - xyz[2]) * Nz[i] )
                    Nx[i] *= sign
                    Ny[i] *= sign
                    Nz[i] *= sign


            for i in range(0, self.swarm.particleLocalCount):
                scale = 1.0 / np.sqrt(Nx[i]**2 + Ny[i]**2 + Nz[i]**2)
                Nx[i] *= scale
                Ny[i] *= scale
                Nz[i] *= scale


            self.director.data[:,0] = Nx[:]
            self.director.data[:,1] = Ny[:]
            self.director.data[:,2] = Nz[:]

        return
