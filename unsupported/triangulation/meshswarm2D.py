
import numpy as np
import underworld as uw
from underworld import function as fn

import contextlib

# from scipy.spatial import Delaunay as delaunay
from scipy.interpolate import LinearNDInterpolator
from scipy.interpolate import CloughTocher2DInterpolator



class meshSwarmVariable2D():

    def __init__(self, meshSwarm=None, dataType="int", count=1 ):

        self.meshSwarm = meshSwarm
        self._data = None

        # Create 2 swarm variables on the meshSwarm - one for fixed points, one for Lagrangian

        self.fixedPointData = meshSwarm.fixedSwarm.add_variable( dataType=dataType, count=count )
        self.lagrangianData = meshSwarm.lagrSwarm.add_variable(  dataType=dataType, count=count )

        meshSwarm.variables.append(self)

        return


    def _update_cached_data(self):

        self.meshSwarm.fixedSwarm.shadow_particles_fetch()
        self.meshSwarm.lagrSwarm.shadow_particles_fetch()

        self._data = np.concatenate((self.fixedPointData.data,
                                     self.lagrangianData.data,
                                     self.fixedPointData.data_shadow,
                                     self.lagrangianData.data_shadow))


        return

    # evaluate ONLY works if scalar data here ...

    def evaluate(self, coords):

        self._update_cached_data()
        self.meshSwarm._linear_interpolator.values = self._data.astype('double')

        return self.meshSwarm._linear_interpolator(coords)


    def fn_gradient(self, coords):

        self._update_cached_data()

        dFdx, dFdy = self.meshSwarm.triangulation_compute_gradient(self._data.astype('double'))

        self.meshSwarm._linear_interpolator.values = dFdx.astype('double')
        dFdx_at_coords = self.meshSwarm._linear_interpolator(coords)

        self.meshSwarm._linear_interpolator.values = dFdy.astype('double')
        dFdy_at_coords = self.meshSwarm._linear_interpolator(coords)

        return dFdx_at_coords, dFdy_at_coords


class meshSwarm2D:
    """
    All the bits and pieces needed to triangulate a given swarm and track the triangulation
    during deformation in a parallel environment

    This runs as an add-on to an existing swarm, but would better be a child class of the swarm !
    """

    def __init__(self, mesh, velocityField):
        """
        meshSwarm2D: this ties a swarm variable and boundary points of a mesh variable together,
        keeps track of shadow information and provides an interpolator.

        To update the variables, there is an buid/update function.
        """

        # Make 2 swarms - one advected, one not

        self.fixedSwarm = uw.swarm.Swarm( mesh=mesh, particleEscape=True )
        self.lagrSwarm  = uw.swarm.Swarm( mesh=mesh, particleEscape=True )
        self._advector  = uw.systems.SwarmAdvector( swarm=self.lagrSwarm, velocityField=velocityField, order=2 )

        self.variables = []

        # Can only build / update the triangulation once points are added

        return


    @contextlib.contextmanager
    def deform_swarm(self, update_owners=True):
        """
            Updating positions of either of the fixed or lagrangian swarms
            must be done with this context manager to ensure the triangulation
            information is properly populated
        """
        self.fixedSwarm._locked = True
        self.fixedSwarm._particleCoordinates.data.flags.writeable = True
        self.lagrSwarm._locked = True
        self.lagrSwarm._particleCoordinates.data.flags.writeable = True

        try:
            yield
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "An exception was encountered during meshSwarm deformation."
                                                     +"Particle locations may not be correctly modified." )

        finally:

            self.fixedSwarm._locked = False
            self.fixedSwarm._particleCoordinates.data.flags.writeable = False
            self.lagrSwarm._locked = False
            self.lagrSwarm._particleCoordinates.data.flags.writeable = False

            self.fixedSwarm.update_particle_owners()
            self.lagrSwarm.update_particle_owners()
            self.update_triangulation()


    def get_max_dt(self):

        dt = self._advector.get_max_dt()
        return dt

    def integrate(self, dt):

        self._advector.integrate(dt)
        self.update_triangulation()

    def update_triangulation(self):

        self.fixedSwarm.shadow_particles_fetch()
        self.lagrSwarm.shadow_particles_fetch()

        # Need to add boundary points for the interpolator / triangulation

        all_particle_coords = np.concatenate((self.fixedSwarm.particleCoordinates.data,
                                              self.lagrSwarm.particleCoordinates.data,
                                              self.lagrSwarm.particleCoordinates.data_shadow,
                                              self.fixedSwarm.particleCoordinates.data_shadow))

        pts = all_particle_coords.shape[0]
        data = np.zeros(pts)

        self.moving_data_start = self.fixedSwarm.particleLocalCount
        self.shadow_data_start = self.fixedSwarm.particleLocalCount + self.lagrSwarm.particleLocalCount

        # The linear interpolator can have its data values reloaded and still works
        # correctly, this behaviour is leveraged by the mesh swarm variable
        # at the moment, so we keep the linear one even if a different interpolator is used
        # in this swarm.
        self._linear_interpolator = LinearNDInterpolator(all_particle_coords, data)
        self.interpolator = self._linear_interpolator
        # self.interpolator = CloughTocher2DInterpolator(all_particle_coords, data)

        self.triangulation = self.interpolator.tri
        self.triangulation_edge_lengths()

        # for var in self.variables:
        #     var.update_data()

        # Things to help compute gradients (could be optional !)

        tri = self.triangulation

        # Triangle encircling vectors

        self.vA = tri.points[tri.simplices[:,1]] - tri.points[tri.simplices[:,0]]
        self.vB = tri.points[tri.simplices[:,2]] - tri.points[tri.simplices[:,1]]
        self.vC = tri.points[tri.simplices[:,0]] - tri.points[tri.simplices[:,2]]

        self.tri_area = 0.5 * (self.vA[:,0] * self.vB[:,1] - self.vA[:,1] * self.vB[:,0])

        w  = np.zeros(tri.npoints)

        for triangle in tri.simplices:
            w[triangle[0]]    += 1.0
            w[triangle[1]]    += 1.0
            w[triangle[2]]    += 1.0

        self.simplex2node_weight = w

        uw.barrier()

        return

    def _triangulation_compute_gradient_irregular(self, values):

        tri = self.triangulation

        vA = self.vA
        vB = self.vB
        vC = self.vC

        F = values.reshape(-1,1)

        FA = (F[tri.simplices[:,0]] + F[tri.simplices[:,1]]) * 0.5
        FB = (F[tri.simplices[:,1]] + F[tri.simplices[:,2]]) * 0.5
        FC = (F[tri.simplices[:,2]] + F[tri.simplices[:,0]]) * 0.5

        dFdyx = (FA*vA + FB*vB + FC*vC)
        dFdyx /= self.tri_area.reshape(-1,1)

        dFdx   = np.zeros(tri.npoints)
        dFdy   = np.zeros(tri.npoints)

        for idx, triangle in enumerate(tri.simplices):
            dFdx[triangle[0]] += dFdyx[idx,1]
            dFdx[triangle[1]] += dFdyx[idx,1]
            dFdx[triangle[2]] += dFdyx[idx,1]
            dFdy[triangle[0]] -= dFdyx[idx,0]
            dFdy[triangle[1]] -= dFdyx[idx,0]
            dFdy[triangle[2]] -= dFdyx[idx,0]

        dFdx /= self.simplex2node_weight
        dFdy /= self.simplex2node_weight

        return dFdx, dFdy


    def _triangulation_compute_gradient_tri_centres(self, values):

        tri = self.triangulation

        vA = self.vA
        vB = self.vB
        vC = self.vC

        F = values.reshape(-1,1)

        FA = (F[tri.simplices[:,0]] + F[tri.simplices[:,1]]) * 0.5
        FB = (F[tri.simplices[:,1]] + F[tri.simplices[:,2]]) * 0.5
        FC = (F[tri.simplices[:,2]] + F[tri.simplices[:,0]]) * 0.5

        dFdyx = (FA*vA + FB*vB + FC*vC)
        dFdyx /= self.tri_area.reshape(-1,1)

        return dFdyx[:,1], dFdyx[:,0]


    def triangulation_smooth(self, values):

        tri = self.triangulation

        vA = self.vA
        vB = self.vB
        vC = self.vC

        # F = values.reshape(-1,1)
        Ft = np.einsum("ij->i",values[tri.simplices]) / 3

        Fn   = np.zeros(tri.npoints)

        for idx, triangle in enumerate(tri.simplices):
            Fn[triangle[0]] += Ft[idx]
            Fn[triangle[1]] += Ft[idx]
            Fn[triangle[2]] += Ft[idx]

        Fn /= self.simplex2node_weight

        return Fn


    def triangulation_compute_gradient(self, values, res=None, second=False):

        from scipy.interpolate import griddata
        from scipy.interpolate import interpn

        if res == None:
            res = int(1.0 / self.edge_lengths.max())

        resX=res
        resY=res

        space = 1.0 / (res-1.0)

        minX, minY = self.lagrSwarm.mesh.data.min(axis=0)
        maxX, maxY = self.lagrSwarm.mesh.data.max(axis=0)

        xs = np.linspace(minX, maxX, resX)
        ys = np.linspace(minY, maxY, resY)

        gradmesh = np.array(np.meshgrid(xs, ys)).transpose(2,1,0)
        gradvals = griddata( self.triangulation.points, values, gradmesh, method="cubic" ) # Cubic not available in 3D ...

        dFdx, dFdy      = np.gradient(gradvals.reshape(resX,resY), space, edge_order=1)

        if second:
            d2Fdx2, d2Fdxdy = np.gradient(dFdx.reshape(resX,resY), space, edge_order=1)
            d2Fdydx, d2Fdy2 = np.gradient(dFdy.reshape(resX,resY), space, edge_order=1)

        # Now map these back to the node points

        dFdxN   = interpn((xs,ys),   dFdx, self.triangulation.points,   method="linear")
        dFdyN   = interpn((xs,ys),  -dFdy, self.triangulation.points,   method="linear")

        if second:
            dF2dx2N = interpn((xs,ys),   d2Fdx2, self.triangulation.points, method="linear")
            dF2dy2N = interpn((xs,ys),   d2Fdy2, self.triangulation.points, method="linear")
            dF2dyxN = interpn((xs,ys),   d2Fdydx+d2Fdxdy, self.triangulation.points, method="linear")

        if not second:
            return dFdxN, dFdyN
        else:
            return dFdxN, dFdyN, dF2dx2N, dF2dyxN, dF2dy2N


    def triangulation_edge_lengths(self):
        """
        Given a delaunay (qhull) triangulation object, identify the
        edges (by vertex pair), and their lengths. This can be used for
        mesh refinement
        """

        def edge_index(a,b):
            return min((a,b), (b,a) )

        tri = self.triangulation

        edges_d = {}

        for t in tri.simplices:
            e1 = edge_index(t[0],t[1])
            e2 = edge_index(t[0],t[2])
            e3 = edge_index(t[1],t[2])

            edges_d[e1] = None
            edges_d[e2] = None
            edges_d[e3] = None


        # This is (essentially) free but the array might be useful later
        edges_l = list(edges_d.keys())
        edges = np.array(edges_d.keys())

        points = tri.points

        edge_lengths = np.sqrt( (points[edges[:,0],0] - points[edges[:,1],0])**2 +
                                (points[edges[:,0],1] - points[edges[:,1],1])**2 )


        self.edges = edges
        self.edge_lengths = edge_lengths
        self.edge_list = edges_l

        return

    def refine_swarm_max_length(self, maxl):

        pts = self.triangulation.points
        edges = self.edges

        sniparray = np.logical_and((self.edge_lengths > maxl),
                                    np.logical_and((self.edges[:,1] < self.shadow_data_start),
                                                   (self.edges[:,0] < self.shadow_data_start)))

        snipsnip = np.where(sniparray)

        add_points = 0.5 * (pts[edges[snipsnip][:,0]] +
                            pts[edges[snipsnip][:,1]] )

        return add_points

    def refine_swarm_too_narrow(self, max_aspect):

        tri = self.triangulation
        pts = self.triangulation.points

        # Find "aspect ratio" of triangles and suggest centroid as new point

        lA = np.sqrt(np.einsum("ij->i", self.vA**2))
        lB = np.sqrt(np.einsum("ij->i", self.vB**2))
        lC = np.sqrt(np.einsum("ij->i", self.vC**2))

        lS = np.vstack((lA, lB, lC)).T

        aspect = np.max(lS, axis=1 ) / np.min(lS, axis=1 )

        # Any triangle (even partly) in the shadow, the aspect ratio should be ignored
        aspect[np.where(tri.simplices[:].max(axis=1) > self.shadow_data_start)] = 0.0

        divide = np.where(aspect > max_aspect)

        # if uw.rank()==0 and len(divide) != 0:
        #     print uw.rank(), "PT: ", divide[0][0], aspect[divide[0][0]]
        #     print self.shadow_data_start, tri.simplices[divide[0][0]]
        #     print pts[tri.simplices[divide[0][0]]]

        # print "min/max aspect ratio = ", aspect.min(), aspect.max()

        add_points = pts[tri.simplices[divide]].mean(axis=1)

        return add_points

    def prune_swarm_min_length(self, minl):
        """
        Suggest points in a triangulation which should be deleted based on
        a minimum edge length criterion (minl). Any point with two connecting
        edges of length < minl should be deleted, and any point with one
        connecting edge < minl / 5.0 should be deleted.
        """

        pts = self.triangulation.points
        edges = self.edges
        del_targets  = np.zeros(self.triangulation.points.shape[0])
        del_targets2 = np.zeros(self.triangulation.points.shape[0])

        snipsnip = np.where(self.edge_lengths < minl)
        snipsnip2 = np.where(self.edge_lengths < minl/2.0)
        snippoints = edges[snipsnip].reshape(-1)
        snippoints2 = edges[snipsnip2].reshape(-1)

        for pt in snippoints:
            del_targets[pt] += 1

        for pt in snippoints2:
            del_targets2[pt] += 1

        # The shadow data and the fixed points are not candidates for deletion !

        del_targets[0:self.moving_data_start]  = 0
        del_targets[self.shadow_data_start::] = 0

        del_targets2[0:self.moving_data_start]  = 0
        del_targets2[self.shadow_data_start::] = 0

        del_points = np.where(np.logical_or(del_targets > 1, del_targets2 > 0))[0]


#        print uw.rank(), "min edge - ", self.edge_lengths.min(), np.count_nonzero(del_targets), np.count_nonzero(del_targets2)

        return del_points


"""
def swarm_gradient_fn(self, values):

    delta = self.edge_lengths.mean() * 0.5
    points = self.interpolator.points.copy()

    c2D = CloughTocher2DInterpolator(self.triangulation.points, values.reshape(-1,1))

    stencil_offsets  = np.array([[-delta, -delta ],
                                 [-delta,  0.0   ],
                                 [-delta,  delta ],
                                 [   0.0, -delta ],
                                 [   0.0,  delta ],
                                 [ delta, -delta ],
                                 [ delta,  0.0   ],
                                 [ delta,  delta ]] )

    stencil_weights  =  np.array([[ -0.25,  0.25 ],
                                  [ -0.5 ,   0.0 ],
                                  [ -0.25, -0.25 ],
                                  [  0.0 ,  0.5  ],
                                  [  0.0,  -0.5  ],
                                  [  0.25 , 0.25 ],
                                  [  0.5,   0.0  ],
                                  [  0.25 ,-0.25 ]] )

    ## Find points off the edges

    minusX     = c2D(points-(delta,0.0))
    plusX      = c2D(points+(delta,0.0))
    minusY     = c2D(points-(0.0,delta))
    plusY      = c2D(points+(0.0,delta))

    points[np.where(np.isnan(minusX[:,0])),0] += delta*1.001
    points[np.where(np.isnan(plusX[:,0])), 0] -= delta*1.001
    points[np.where(np.isnan(minusY[:,0])),1] += delta*1.001
    points[np.where(np.isnan(plusY[:,0])), 1] -= delta*1.001

    ## Now everything should be inside the box

    dX = np.zeros_like(values)
    dY = np.zeros_like(values)

    for i in range(0,8):
        vals = c2D(points + stencil_offsets[i])
        dX +=  vals[:,0] * stencil_weights[i,0]
        dY +=  vals[:,0] * stencil_weights[i,1]


    dX /= (2.0*delta)
    dY /= (2.0*delta)

    return dX, dY
"""
