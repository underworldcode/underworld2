
import numpy as np
import underworld as uw
from underworld import function as fn

from scipy.spatial import cKDTree as kdTree

class markerLine2D:
    """
    All the bits and pieces needed to define a marker surface (in 2D) from a string of points
    """


    def __init__(self, mesh, velocityField, pointsX, pointsY, fthickness, fmu, fC, fID, insidePt=(0.0,0.0)):

        
        # Marker swarms are probably sparse, and on most procs will have nothing to do
        # if there are no particles (or not enough to compute what we need)
        # then set this flag and return appropriately. This can be checked once the swarm is
        # populated. 
        
        self.empty = False
           
        # Should do some checking first
        
        self.mesh = mesh
        self.velocity = velocityField
        self.thickness = fthickness
#        self.mu = fmu
#        self.C  = fC
        self.ID = fID
        self.insidePt = insidePt
        self.director = None
        
        # Set up the swarm and variables on all procs

        self.swarm = uw.swarm.Swarm( mesh=self.mesh, particleEscape=True )
        self.director = self.swarm.add_variable( dataType="double", count=2)     
        self._swarm_advector = uw.systems.SwarmAdvector( swarm=self.swarm, 
                                                         velocityField=self.velocity, order=2 )

        self.swarm.add_particles_with_coordinates(np.stack((pointsX, pointsY)).T)        
        self.director.data[...] = 0.0       
        
        self._update_kdtree()
        self._update_surface_normals()
 
        return



    def add_points(self, pointsX, pointsY):
        
        self.swarm.add_particles_with_coordinates(np.stack((pointsX, pointsY)).T)        

        self.rebuild()
        

    def rebuild(self):
            
        self._update_kdtree()
        self._update_surface_normals()
 
        return



    def _update_kdtree(self):
        
        self.empty = False
        self.swarm.shadow_particles_fetch()        

        all_particle_coords = np.concatenate((self.swarm.particleCoordinates.data, 
                                              self.swarm.particleCoordinates.data_shadow))
 
        if len(all_particle_coords) < 3:
            self.empty = True
            self.kdtree = kdTree(np.array([(1.0e99,1.0e99), (1.0e99,1.2e99)] ))
        else:    
            self.kdtree = kdTree(all_particle_coords) 

        uw.barrier()

      

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
        
        fdirector = np.concatenate((self.director.data, 
                                    self.director.data_shadow))
              
        director[fpts] = fdirector[p[fpts]]
                        
        return director, fpts
                                  
    
    def compute_signed_distance(self, coords):
  
        # make sure this is called by all procs including those
        # which have an empty self 
        
        self.swarm.shadow_particles_fetch()  
  

        if self.empty:
            return np.empty((0,1)), np.empty(0, dtype="int")
        
        fdirector = np.concatenate((self.director.data, 
                                    self.director.data_shadow))
 


        # Need p, but it hasn't been stored even though d is the proximityVar
        d, p  = self.kdtree.query( coords, distance_upper_bound=self.thickness )

        fpts = np.where( np.isinf(d) == False )[0]

        director = np.zeros_like(coords)  

        vector = coords[fpts] - self.kdtree.data[p[fpts]]
        director = fdirector[p[fpts]]
        
        signed_distance = np.zeros((coords.shape[0],1))  
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

            for i, xy in enumerate(particle_coords):    
                r, neighbours = self.kdtree.query(particle_coords[i], k=3)

                # neighbour points are neighbours[1] and neighbours[2]

                XY1 = self.kdtree.data[neighbours[1]]
                XY2 = self.kdtree.data[neighbours[2]]

                dXY = XY2 - XY1

                Nx[i] =  dXY[1]
                Ny[i] = -dXY[0] 

                if (self.insidePt):
                    sign = np.sign((self.insidePt[0] - xy[0]) * Nx[i] + 
                                   (self.insidePt[1] - xy[1]) * Ny[i])
                    Nx[i] *= sign
                    Ny[i] *= sign 


            for i in range(0, self.swarm.particleLocalCount):
                scale = 1.0 / np.sqrt(Nx[i]**2 + Ny[i]**2)
                Nx[i] *= scale
                Ny[i] *= scale


            self.director.data[:,0] = Nx[:]
            self.director.data[:,1] = Ny[:]

        return 

