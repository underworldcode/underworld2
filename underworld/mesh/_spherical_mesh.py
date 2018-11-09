##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
#
#

"""
This module contains FeMesh_CubedSphere classes, and similar.
"""

import underworld as uw
import underworld._stgermain as _stgermain
import underworld.function as fn
import weakref
import libUnderworld
from underworld.mesh import _specialSets_Cartesian
import underworld.function as function
import contextlib
import abc
import h5py
from mpi4py import MPI
import numpy as np

from ._mesh import FeMesh_Cartesian

class FeMesh_SRegion(FeMesh_Cartesian):
    def __init__(self, elementRes=(16,16,10), radialLengths=(3.0,6.0), latExtent=90.0, longExtent=90.0, centroid=[0.0,0.0,0.0], **kwargs):
        """
        Cre Cubed-sphere sixth, centered on the 'centroid'.


        MinI_VertexSet / MaxI_VertexSet -> longitudinal walls : [min/max] = [west/east]
        MinJ_VertexSet / MaxJ_VertexSet -> latitudinal walls  : [min/max] = [south/north]
        MinK_VertexSet / MaxK_VertexSet -> radial walls       : [min/max] = [inner/outer]

        Refer to parent classes for parameters beyond those below.

        Parameter
        ---------
        elementRes : tuple
            Tuple determining number of elements (longitudinally, latitudinally, radially).
        radialLengths : tuple
            Tuple determining the (inner radialLengths, outer radialLengths).
        longExtent : float
            The angular extent of the domain between great circles of longitude.
        latExtent : float
            The angular extent of the domain between great circles of latitude.


        Example
        -------

        >>> (radMin, radMax) = (4.0,8.0)
        >>> mesh = uw.mesh.FeMesh_SRegion( elementRes=(20,20,14), radialLengths=(radMin, radMax) )
        >>> integral = uw.utils.Integral( 1.0, mesh).evaluate()[0]
        >>> exact = (4./3.)*np.pi*(radMax**3 - radMin**3) / 6.0
        >>> np.isclose(integral, exact, rtol=0.02)
        True
        """

        if not isinstance( latExtent, (float,int) ):
            raise TypeError("Provided 'latExtent' must be a float or integer")
        self._latExtent = latExtent
        if not isinstance( longExtent, (float,int) ):
            raise TypeError("Provided 'longExtent' must be a float or integer")
        self._longExtent = longExtent
        if not isinstance( radialLengths, (tuple,list)):
            raise TypeError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        if len(radialLengths) != 2:
            raise ValueError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        for el in radialLengths:
            if not isinstance( el, (float,int)) :
                raise TypeError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        self._radialLengths = radialLengths

        lat_half = latExtent/2.0
        long_half = longExtent/2.0

        # build 3D mesh cartesian mesh centred on (0.0,0.0,0.0) - in _setup() we deform the mesh
        # elementType="Q1/dQ0",
        super(FeMesh_SRegion,self).__init__(elementRes=elementRes,
                    minCoord=(radialLengths[0],-long_half,-lat_half), maxCoord=(radialLengths[1],long_half,lat_half), periodic=None, **kwargs)

        self.specialSets["innerWall_VertexSet"] = _specialSets_Cartesian.MinI_VertexSet
        self.specialSets["outerWall_VertexSet"] = _specialSets_Cartesian.MaxI_VertexSet
        self.specialSets["northWall_VertexSet"] = _specialSets_Cartesian.MaxK_VertexSet
        self.specialSets["southWall_VertexSet"] = _specialSets_Cartesian.MinK_VertexSet
        self.specialSets["eastWall_VertexSet"]  = _specialSets_Cartesian.MaxJ_VertexSet
        self.specialSets["westWall_VertexSet"]  = _specialSets_Cartesian.MinJ_VertexSet

        self._centroid = centroid

    def _setup(self):
        import underworld.function as fn

        with self.deform_mesh():
            # perform Cubed-sphere projection on coordinates
            # fac = np.pi/180.0
            old = self.data
            (x,y) = (np.tan(self.data[:,1]*np.pi/180.0), np.tan(self.data[:,2]*np.pi/180.0))
            d = self.data[:,0] / np.sqrt( x**2 + y**2 + 1)
            self.data[:,0] = self._centroid[0] + d*x
            self.data[:,1] = self._centroid[1] + d*y
            self.data[:,2] = self._centroid[2] + d

        # add a boundary MeshVariable - 1 if nodes is on the boundary(ie 'AllWalls_VertexSet'), 0 if node is internal
        self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)
        self.bndMeshVariable.data[:] = 0.
        self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0


        # ASSUME the parent class builds the _boundaryNodeFn
        # self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)
        # self.bndMeshVariable.data[:] = 0.
        # # set a value 1.0 on provided vertices
        # self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0
        # # note we use this condition to only capture border swarm particles
        # # on the surface itself. for those directly adjacent, the deltaMeshVariable will evaluate
        # # to non-zero (but less than 1.), so we need to remove those from the integration as well.
        self._boundaryNodeFn = fn.branching.conditional(
                                        [  ( self.bndMeshVariable > 0.999, 1. ),
                                        (                    True, 0. )   ] )

        """
        Rotation documentation.
        We will create 3 basis vectors that will rotate the (x,y,z) problem to be a
        (r,n,t) [radial, normal to cut, tangential to cut] problem.

        The rotations are performed on the local element level using the existing machinery
        provided by UW2. As such only elements on the domain boundary need rotation, all internal
        elements can continue with the (x,y,z) representation.

        This is implemented with rotations on all dofs such that:
         1. If on the domain boundary - rotated to (r,n,t) and
         2. If not on the domain boundary - rotated by identity matrix i.e. (NO rotation).
        """

        # initialiase bases vectors as meshVariables
        self._e1 = self.add_variable(nodeDofCount=3)
        self._e2 = self.add_variable(nodeDofCount=3)
        self._e3 = self.add_variable(nodeDofCount=3)

        # _x_or_radial, y_or_east, z_or_north functions
        self._fn_x_or_radial = fn.branching.conditional(
                                    [ ( self.bndMeshVariable > 0.9, self.fn_unitvec_radial() ),
                                      (               True, fn.misc.constant(1.0)*(1.,0.,0.) ) ] )
        self._fn_y_or_east   = fn.branching.conditional(
                                    [ ( self.bndMeshVariable > 0.9, self._getEWFn() ),
                                      (               True, fn.misc.constant(1.0)*(0.,1.,0.) ) ] )
        self._fn_z_or_north  = fn.branching.conditional(
                                    [ ( self.bndMeshVariable > 0.9, self._getNSFn() ),
                                      (               True, fn.misc.constant(1.0)*(0.,0.,1.) ) ] )

        # shorthand variables for the walls
        inner = self.specialSets["innerWall_VertexSet"]
        outer = self.specialSets["outerWall_VertexSet"]
        W     = self.specialSets["westWall_VertexSet"]
        E     = self.specialSets["eastWall_VertexSet"]
        S     = self.specialSets["southWall_VertexSet"]
        N     = self.specialSets["northWall_VertexSet"]

        # evaluate the new bases
        self._e1.data[:] = self._fn_x_or_radial.evaluate(self)
        self._e2.data[:] = self._fn_y_or_east.evaluate(self) # only good on EW walls
        self._e3.data[:] = self._fn_z_or_north.evaluate(self) # only good on NS walls

        # build the correct e3 on EW, with e3 = e1 cross e2
        walls = E+W
        a = self._e1.data[walls.data]
        b = self._e2.data[walls.data]
        self._e3.data[walls.data] = np.cross(a,b)

        # build the correct e2 on NS-EW
        # note, at the side edges of the sixth a choice for the basis must be made
        # as two non orthogonal side walls meet. We let the basis of the EW walls
        # define the rotations required and don't correct them below.
        walls = N+S - walls
        a = self._e3.data[walls.data]
        b = self._e1.data[walls.data]
        self._e2.data[walls.data] = np.cross(a,b)

    def fn_unitvec_radial(self):

        pos = fn.coord()
        centre = self._centroid
        r_vec = pos - centre
        mag = fn.math.sqrt(fn.math.dot( r_vec, r_vec ))
        r_vec = r_vec / mag
        return r_vec

    def _getEWFn(self):
        pos = fn.coord() - self._centroid
        xi = fn.math.atan(pos[0]/pos[2])
        # vec = [ cos(xi), 0.0, -sin(xi) ]
        vec =       fn.math.cos(xi) * (1.,0.,0.)
        vec = vec + fn.math.sin(xi) * (0.,0.,-1.)
        return vec

    def _getNSFn(self):
        pos = fn.coord() - self._centroid
        xi = fn.math.atan(pos[1]/pos[2])
        # vec = [ 0.0, cos(xi), -sin(xi) ]
        vec =       fn.math.cos(xi) * (0.,1.,0.)
        vec = vec + fn.math.sin(xi) * (0.,0.,-1.)
        return vec



class FeMesh_Cylinder(FeMesh_Cartesian):
# LNM
    def __init__(self, elementRes=(6,12,12), cyl_size=(0.0,1.0,1.0), elementType="Q1/dQ0", **kwargs):
        """

        Refer to parent classes for parameters beyond those below.

        Parameter
        ---------
        elementRes : tuple
            Tuple determining number of elements (longitudinally, latitudinally, radially).
        cyl_size : tuple
            Tuple determining the (base height, lid height and radius).


        """

        if not isinstance( cyl_size, (tuple,list)):
            raise TypeError("Provided 'cyl_size' must be a tuple/list of 3 floats")
        if len(cyl_size) != 3:
            raise ValueError("Provided 'cyl_size' must be a tuple/list of 3 floats")
        for el in cyl_size:
            if not isinstance( el, (float,int)) :
                raise TypeError("Provided 'cyl_size' must be a tuple/list of 3 floats")

        self._cyl_size = cyl_size

        # build 3D mesh cartesian mesh centred on (0.0,0.0,0.0) - in _setup() we deform the mesh
        # elementType="Q1/dQ0",
        super(FeMesh_Cylinder,self).__init__(elementRes=elementRes,
                    minCoord=(0.0,-1.0,-1.0),
                    maxCoord=(1.0, 1.0, 1.0),
                    periodic=None, **kwargs)

        self.specialSets["base_VertexSet"] = _specialSets_Cartesian.MinI_VertexSet
        self.specialSets["lid_VertexSet"]  = _specialSets_Cartesian.MaxI_VertexSet
        self.specialSets["verticalWall_VertexSet"] = _specialSets_Cartesian.AllJKWalls



    def _setup(self):
        import underworld.function as fn

        radius = self._cyl_size[2]
        base   = self._cyl_size[0]
        top    = self._cyl_size[1]

        W = self.specialSets["verticalWall_VertexSet"]

        with self.deform_mesh():
            # old = self.data
            (x,y) = (self.data[:,1], self.data[:,2] )

            # Move nodes away from the axis
            r = np.hypot(x,y)
            x[ r == 0.0 ] += 0.000001 * self._cyl_size[2]

            theta = np.arctan2(y,x)
            scale  = np.maximum(np.cos(theta%(np.pi*0.5)), np.sin(theta%(np.pi*0.5)))

            self.data[:,0] = self.data[:,0] * (top - base) + base
            self.data[:,1] *= scale
            self.data[:,2] *= scale

        # add a boundary MeshVariable - 1 if nodes is on the boundary(ie 'AllWalls_VertexSet'), 0 if node is internal
        self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)
        self.bndMeshVariable.data[:] = 0.
        self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0


        self._boundaryNodeFn = fn.branching.conditional(
                                        [  ( self.bndMeshVariable != 0.0 , 1. ),
                                        (                            True, 0. )   ] )

        """
        Rotation documentation.
        We will create 3 basis vectors that will rotate the (x,y,z) problem to be a
        (r, theta, z) natural coordinate system for this mesh

        The rotations are performed on the local element level using the existing machinery
        provided by UW2. As such only elements on the domain boundary need rotation, all internal
        elements can continue with the (x,y,z) representation.

        This is implemented with rotations on all dofs such that:
         1. If on the domain boundary - rotated to (r,n,t) and
         2. If not on the domain boundary - rotated by identity matrix i.e. (NO rotation).
        """

        # initialiase bases vectors as meshVariables
        self._e1 = self.add_variable(nodeDofCount=3)
        self._e2 = self.add_variable(nodeDofCount=3)
        self._e3 = self.add_variable(nodeDofCount=3)

        posFn = fn.coord()
        radiusFn =  fn.math.sqrt(posFn[1]**2 + posFn[2]**2)


        self._fn_x_or_rotated  = fn.misc.constant(1.0)*(1.,0.,0.)

        self._fn_y_or_rotated  = fn.branching.conditional(
                                    [ ( self.bndMeshVariable != 0.0, self._get_tangential_Fn() ),
                                      (                       True, fn.misc.constant(1.0)*(0.,1.,0.) ) ] )
        self._fn_z_or_rotated  = fn.branching.conditional(
                                    [ ( self.bndMeshVariable != 0.0 , self._get_normal_Fn() ),
                                      (                       True, fn.misc.constant(1.0)*(0.,0.,1.) ) ] )

        # evaluate the new bases

        self._e1.data[:] = self._fn_x_or_rotated.evaluate(self) # should be unchanged in this mesh
        self._e2.data[:] = self._fn_y_or_rotated.evaluate(self) #
        self._e3.data[:] = self._fn_z_or_rotated.evaluate(self) #
        #
        # self._e1.data[:] = (1.,0.,0.)
        # self._e2.data[:] = self._get_tangential_Fn().evaluate(self)
        # self._e3.data[:] = self._get_normal_Fn().evaluate(self)

        return


    def fn_r_theta_z(self):
        pos = fn.coord()
        r = fn.math.sqrt(pos[1]**2 + pos[2]**2)
        theta = fn.math.atan2(pos[2],pos[1])
        z = pos[0]
        return r, theta, z


    ## This one is y or other direction  (normal)
    def _get_tangential_Fn(self):
        pos = fn.coord()
        xi  = fn.math.atan2(pos[2],pos[1])
        vec =       fn.math.cos(xi) * (0.0,1.0, 0.0)
        vec = vec + fn.math.sin(xi) * (0.0,0.0, 1.0)
        return vec

    ## This one is z or other direction (tangential)
    def _get_normal_Fn(self):
        pos = fn.coord()
        xi = fn.math.atan2(pos[2],pos[1])
        vec =       fn.math.cos(xi) * (0.0, 0.0, 1.0)
        vec = vec + fn.math.sin(xi) * (0.0,-1.0, 0.0)
        return vec


class FeMesh_SphericalCap(FeMesh_Cartesian):
# LNM
    def __init__(self, resolution=(6,12), cap_size=(0.5,1.0, 90.0), elementType="Q1/dQ0", **kwargs):
        """
        Refer to parent classes for parameters beyond those below.

        Parameter
        ---------
        elementRes : tuple
            Tuple determining number of elements (longitudinally, latitudinally, radially).
        cap_size : tuple
            Tuple determining the (inner radius, outer radius and total cap angle (180 -> full hemisphere)).

        """


        if not isinstance( resolution, (tuple,list)):
            raise TypeError("Provided 'resolution' must be a tuple/list of 2 ints")
        if len(resolution) != 2:
            raise ValueError("Provided 'resolution' must be a tuple/list of 2 ints")
        for el in resolution:
            if not isinstance( el, int) :
                raise TypeError("Provided 'resolution' must be a tuple/list of 2 ints")


        if not isinstance( cap_size, (tuple,list)):
            raise TypeError("Provided 'cap_size' must be a tuple/list of 3 floats")
        if len(cap_size) != 3:
            raise ValueError("Provided 'cap_size' must be a tuple/list of 3 floats")
        for el in cap_size:
            if not isinstance( el, (float,int)) :
                raise TypeError("Provided 'cap_size' must be a tuple/list of 3 floats")

        self._cap_size = cap_size
        self._resolution = resolution

        # build 3D mesh cartesian mesh centred on (0.0,0.0,0.0) - in _setup() we deform the mesh
        # elementType="Q1/dQ0"

        super(FeMesh_SphericalCap,self).__init__(elementRes=(resolution[0], resolution[1], resolution[1]),
                    minCoord=(0.0,-1.0,-1.0),
                    maxCoord=(1.0, 1.0, 1.0),
                    periodic=None, **kwargs)

        self.specialSets["core_VertexSet"]    = _specialSets_Cartesian.MinI_VertexSet
        self.specialSets["surface_VertexSet"] = _specialSets_Cartesian.MaxI_VertexSet
        self.specialSets["wall_VertexSet"]    = _specialSets_Cartesian.AllJKWalls


    def _setup(self):
        import underworld.function as fn

        self.xyz     = self.data
        self.rthetaphi = self.data.copy()

        cap_size = self._cap_size

        W = self.specialSets["wall_VertexSet"]

        with self.deform_mesh():

            (y,z) = (self.data[:,1], self.data[:,2] )
            theta = np.arctan2(z,y)
            scale  = np.maximum(np.cos(theta%(np.pi*0.5)), np.sin(theta%(np.pi*0.5)))
            self.data[:,1] *= scale
            self.data[:,2] *= scale

            ## OK, so now we have a cylinder and we can
            ## deform this to the hemispherical shell

            # 1. Distance from axis maps to co-latitude

            phi = np.hypot(y,z) * (0.5 * np.pi * cap_size[2] / 180.0)
            r   = (cap_size[1] - cap_size[0]) * self.data[:,0] + cap_size[0]

            # 2. We have r, theta, phi

            self.data[:,0] = r * np.cos(phi)
            self.data[:,1] = r * np.sin(phi) * np.cos(theta)
            self.data[:,2] = r * np.sin(phi) * np.sin(theta)

            self.rthetaphi[:,0] = r
            self.rthetaphi[:,1] = theta
            self.rthetaphi[:,2] = phi




        """
        Rotation documentation.
        We will create 3 basis vectors that will rotate the (x,y,z) problem to be a
        (r,n,t) [radial, normal to cut, tangential to cut] problem.

        The rotations are performed on the local element level using the existing machinery
        provided by UW2. As such only elements on the domain boundary need rotation, all internal
        elements can continue with the (x,y,z) representation.

        This is implemented with rotations on all dofs such that:
         1. If on the domain boundary - rotated to (r,n,t) and
         2. If not on the domain boundary - rotated by identity matrix i.e. (NO rotation).
        """

        # initialiase bases vectors as meshVariables
        self._e1 = self.add_variable(nodeDofCount=3)
        self._e2 = self.add_variable(nodeDofCount=3)
        self._e3 = self.add_variable(nodeDofCount=3)

        self._e1.data[:] = self.fn_unitvec_r().evaluate(self)
        self._e2.data[:] = self.fn_unitvec_lon().evaluate(self)
        self._e3.data[:] = self.fn_unitvec_lat().evaluate(self)


    def fn_r_theta_phi(self):
        pos = fn.coord()
        r = fn.math.sqrt(pos[0]**2 + pos[1]**2 + pos[2]**2)
        theta = fn.math.acos(pos[0]/r)
        phi   = fn.math.atan2(pos[2],pos[1])
        return r, theta, phi


    def fn_unitvec_r(self):
        pos = fn.coord()
        mag = fn.math.sqrt(fn.math.dot( pos, pos ))
        r_vec = pos / mag
        return r_vec

    ## This one is theta (colatitude) unit vector
    def fn_unitvec_theta(self):
        pos = fn.coord()
        r = fn.math.sqrt(pos[0]**2 + pos[1]**2 + pos[2]**2)
        theta = fn.math.acos(pos[0]/r)
        phi   = fn.math.atan2(pos[2],pos[1])

        vec   = fn.math.cos(theta) * fn.math.cos(phi) * (0.0, 1.0, 0.0)
        vec +=  fn.math.cos(theta) * fn.math.sin(phi) * (0.0, 0.0, 1.0)
        vec += -fn.math.sin(theta) * (1.0, 0.0, 0.0)
        return vec

    ## This one is phi (longitudinal) unit vector
    def fn_unitvec_phi(self):
        pos = fn.coord()
        r = fn.math.sqrt(pos[0]**2 + pos[1]**2 + pos[2]**2)
        theta = fn.math.acos(pos[0]/r)
        phi   = fn.math.atan2(pos[2],pos[1])

        vec   = -fn.math.sin(phi) * (0.0, 1.0, 0.0)
        vec  +=  fn.math.cos(phi) * (0.0, 0.0, 1.0)
        return vec





class FeMesh_CubedSphere(FeMesh_Cartesian):
# LNM
# A big blob with the inner nodes masked out / set to boundary nodes

    def __init__(self, radial_resolution=6, radii=(0.5,1.0), core_stretch=1.0, elementType="Q1/dQ0", process_kites=True, **kwargs):
        """
        Refer to parent classes for parameters beyond those below.

        Parameter
        ---------
        resolution : int
            Int determining number of *elements* radially (implicity sets lateral resolution)
        radii : tuple
            Tuple determining the (inner radius, outer radius ).
        core_stretch : float
            Float that determines the relative node spacing in the core cf. outer shell

        """

        if not isinstance(radial_resolution, int):
            raise TypeError("CubedSphere: radial_resolution expects an integer value")

        if not isinstance(core_stretch, float):
            raise TypeError("CubedSphere: core_stretch expects an float value")

        if not isinstance( radii, (tuple,list)):
            raise TypeError("Provided 'radii' must be a tuple/list of 2 floats")
        if len(radii) != 2:
            raise ValueError("Provided 'radii' must be a tuple/list of 2 floats")
        for el in radii:
            if not isinstance( el, (float,int)) :
                raise TypeError("Provided 'radii' must be a tuple/list of 2 floats")


        self._radial_resolution = radial_resolution
        self._core_stretch = core_stretch
        self._radii = radii
        selt._kites=process_kites

        # build 3D mesh cartesian mesh centred on (0.0,0.0,0.0) - in _setup() we deform the mesh
        # elementType="Q1/dQ0", otherwise we have crashes ... sigh

        res = radial_resolution * 2

        super(FeMesh_CubedSphere,self).__init__(elementRes=(res, res, res),
                    minCoord=(-1.0,-1.0,-1.0),
                    maxCoord=( 1.0, 1.0, 1.0),
                    periodic=None, **kwargs)

        self.specialSets["surface_VertexSet"]    = _specialSets_Cartesian.AllWalls



    def _setup(self):
        import underworld.function as fn

        # This is an alias for the mesh points in Cartesian

        self.xyz = self.data
        self.rlonlat = self.data.copy()

        inner_radius = self._radii[0] / self._radii[1]
        outer_radius = 1.0

        # We need a mesh with a node that aligns on the CMB

        core_elts  = np.round( self._radial_resolution * inner_radius / self._core_stretch)
        scale = self._radial_resolution * inner_radius / core_elts
        max_puff1 = outer_radius * scale
        scale_back = (outer_radius-inner_radius) / (max_puff1-inner_radius)

        with self.deform_mesh():

            ## Radial levels before a

            (x,y,z) = (self.data[:,0], self.data[:,1], self.data[:,2] )

            dd = 0.0000001 * inner_radius
            x[ x < dd ] += dd


            # This is the standard cubed sphere which is the gentlest
            # move away from the Cartesian mesh as possible. The problem
            # is that there are no concentric spheres in the mesh and the
            # inner boundary (and any internal phase boundaries) do not lie
            # on element edges.

            # xr0 = x
            # yr0 = y
            # zr0 = z
            #
            # xp0 = x * np.sqrt(1.0 - 0.5*yr0**2 - 0.5*zr0**2 + yr0**2 * zr0**2 / 3.0)
            # yp0 = y * np.sqrt(1.0 - 0.5*xr0**2 - 0.5*zr0**2 + xr0**2 * zr0**2 / 3.0)
            # zp0 = z * np.sqrt(1.0 - 0.5*xr0**2 - 0.5*yr0**2 + xr0**2 * yr0**2 / 3.0)

            # This is a modification which applies the same transformation at each
            # level of the mesh and hence creates nested spheres. The issue here
            # is the element geometry along various symmetry planes / axes

            rr = np.maximum(np.abs(x),np.maximum(np.abs(y),np.abs(z)))

            xr = x  / rr
            yr = y  / rr
            zr = z  / rr

            xp = x * np.sqrt(1.0 - 0.5*yr**2 - 0.5*zr**2 + yr**2 * zr**2 / 3.0)
            yp = y * np.sqrt(1.0 - 0.5*xr**2 - 0.5*zr**2 + xr**2 * zr**2 / 3.0)
            zp = z * np.sqrt(1.0 - 0.5*xr**2 - 0.5*yr**2 + xr**2 * yr**2 / 3.0)

            ur = (rr - inner_radius) / (outer_radius-inner_radius)

            # w = 0.2 * np.sin(ur * np.pi)

            # xp = w * xp0 + (1.0 - w) * xp
            # yp = w * yp0 + (1.0 - w) * yp
            # zp = w * zp0 + (1.0 - w) * zp

            # cases with kites ... nodes where abs(x) = abs(y) = abs(z)

            nodes1 = np.logical_and(np.isclose(np.abs(x), np.abs(y)), np.abs(x) >= np.abs(z))
            nodes2 = np.logical_and(np.isclose(np.abs(x), np.abs(z)), np.abs(x) >= np.abs(y))
            nodes3 = np.logical_and(np.isclose(np.abs(z), np.abs(y)), np.abs(z) >= np.abs(x))

            is_a_half_kite = np.logical_or(np.logical_or(nodes1, nodes2), nodes3)

            # nodesA = np.isclose(np.abs(y), np.abs(z))
            # nodesB = np.isclose(np.abs(x), np.abs(z))
            #
            # is_a_full_kite = np.logical_and(nodesA, nodesB)

            # print("Kites: {}".format(np.where(is_a_kite)))


            r   = np.sqrt(xp**2 + yp**2 + zp**2)
            rscale = self._radii[1] * np.minimum(rr * scale, (rr * scale - inner_radius) * scale_back + inner_radius)

            if self._kites:
                inodes = np.isclose(rscale, self._radii[0], rtol=1.0e-3)
                onodes = np.isclose(rscale, self._radii[1], rtol=1.0e-3)
                ionodes = np.logical_or(inodes, onodes)
                bodynodes = np.logical_not(ionodes)

                # kites are not i/o nodes
                is_moveable_half_kite = np.logical_and(bodynodes, is_a_half_kite)

                rscale[is_moveable_half_kite] += 0.5 * (self._radii[1] - self._radii[0]) / self._radial_resolution


            lon   = np.arctan2(zp,yp)
            lat   = np.arcsin( xp/r)

            self.rlonlat[:,0] = rscale
            self.rlonlat[:,1] = lon
            self.rlonlat[:,2] = lat

            self.data[:,0] = rscale * np.sin(lat)
            self.data[:,1] = rscale * np.cos(lat) * np.cos(lon)
            self.data[:,2] = rscale * np.cos(lat) * np.sin(lon)
            #

        # ASSUME the parent class builds the _boundaryNodeFn
        # self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)
        # self.bndMeshVariable.data[:] = 0.
        # # set a value 1.0 on provided vertices
        # self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0
        # # note we use this condition to only capture border swarm particles
        # # on the surface itself. for those directly adjacent, the deltaMeshVariable will evaluate
        # # to non-zero (but less than 1.), so we need to remove those from the integration as well.

        surface_vertexSet = self.specialSets["surface_VertexSet"]

        lower_surface_vertexSet = self.specialSets["Empty"]
        lower_surface_vertexSet += np.where(np.isclose(self.rlonlat[:,0], self._radii[0], rtol=1.0e-3))[0]
        self.specialSets["lower_surface_vertexSet"]  = lambda selfobject: lower_surface_vertexSet

        excluded_vertexSet = self.specialSets["Empty"]
        excluded_vertexSet += np.where(self.rlonlat[:,0] < self._radii[0]*0.999)[0]  # Need to be careful about the resolution.
        self.specialSets["excluded_vertexSet"] = lambda selfobject: excluded_vertexSet

        dead_centre_vertexSet = self.specialSets["Empty"]
        dead_centre_vertexSet += np.where(self.rlonlat[:,0] < 0.00001 * self._radii[1])[0]
        self.specialSets["dead_centre_vertexSet"] = lambda selfobject: dead_centre_vertexSet


        """
        Rotation documentation.
        We will create 3 basis vectors that will rotate the (x,y,z) problem to be a
        (r,n,t) [radial, normal to cut, tangential to cut] problem.

        The rotations are performed on the local element level using the existing machinery
        provided by UW2. As such only elements on the domain boundary need rotation, all internal
        elements can continue with the (x,y,z) representation.

        This is implemented with rotations on all dofs such that:
         1. If on the domain boundary - rotated to (r,n,t) and
         2. If not on the domain boundary - rotated by identity matrix i.e. (NO rotation).
        """

        # initialiase bases vectors as meshVariables

        self._e1 = self.add_variable(nodeDofCount=3)
        self._e2 = self.add_variable(nodeDofCount=3)
        self._e3 = self.add_variable(nodeDofCount=3)

        self._e1.data[:] = self.fn_unitvec_r().evaluate(self)
        self._e2.data[:] = self.fn_unitvec_lon().evaluate(self)
        self._e3.data[:] = self.fn_unitvec_lat().evaluate(self)


    def fn_r_lon_lat(self):
        pos = fn.coord()
        rFn = fn.math.sqrt(pos[0]**2 + pos[1]**2 + pos[2]**2)
        lonFn = fn.math.atan2(pos[2],pos[1])
        latFn = fn.math.asin(pos[0]/rFn)
        return rFn, lonFn, latFn


    def vec_rlonlat2xyz(self, vField, index_set=None):
        """ Takes a vector mesh variable and optional index set
            This is not a UW function though perhaps it should be
        """

        if index_set is None:
            nodes = slice(None)
        else:
            nodes = index_set.data

        tv = vField.data[nodes,:].copy() # be careful not to over-write
        uv = vField.data[nodes,:]

        e1 = self.fn_unitvec_r().evaluate(self.data[nodes])
        e2 = self.fn_unitvec_lon().evaluate(self.data[nodes])
        e3 = self.fn_unitvec_lat().evaluate(self.data[nodes])

        tv[:,0] = uv[:,0] * e1[:,0] + uv[:,1] * e2[:,0] + uv[:,2] * e3[:,0]
        tv[:,1] = uv[:,0] * e1[:,1] + uv[:,1] * e2[:,1] + uv[:,2] * e3[:,1]
        tv[:,2] = uv[:,0] * e1[:,2] + uv[:,1] * e2[:,2] + uv[:,2] * e3[:,2]

        return tv

    def vec_xyz2rlonlat(self, vField, index_set=None):
        """ Takes a vector mesh variable and optional index set
            This is not a UW function though perhaps it should be
        """

        if index_set is None:
            nodes = slice(None)
        else:
            nodes = index_set.data

        tv = vField.data[nodes,:].copy() # be careful not to over-write
        uv = vField.data[nodes,:]

        e1 = self.fn_unitvec_r().evaluate(self.data[nodes])
        e2 = self.fn_unitvec_lon().evaluate(self.data[nodes])
        e3 = self.fn_unitvec_lat().evaluate(self.data[nodes])

        tv[:,0] = uv[:,0] * e1[:,0] + uv[:,1] * e1[:,1] + uv[:,2] * e1[:,2]
        tv[:,1] = uv[:,0] * e2[:,0] + uv[:,1] * e2[:,1] + uv[:,2] * e2[:,2]
        tv[:,2] = uv[:,0] * e3[:,0] + uv[:,1] * e3[:,1] + uv[:,2] * e3[:,2]

        return tv


    def fn_unitvec_r(self):
        pos = fn.coord()
        mag = fn.math.sqrt(fn.math.dot( pos, pos ))
        r_vec = pos / mag
        return r_vec

    # The unit vector is defined along the direction of colatitude towards North
    def fn_unitvec_lat(self):
        pos = fn.coord()
        # r = fn.math.sqrt(pos[0]**2 + pos[1]**2 + pos[2]**2)
        r = fn.math.sqrt(fn.math.dot( pos, pos ))
        colat = fn.math.acos(pos[0]/r)
        lon   = fn.math.atan2(pos[2],pos[1])

        vec  = -fn.math.cos(colat) * fn.math.cos(lon) * (0.0, 1.0, 0.0)
        vec += -fn.math.cos(colat) * fn.math.sin(lon) * (0.0, 0.0, 1.0)
        vec +=  fn.math.sin(colat) * (1.0, 0.0, 0.0)
        return vec

    ## This one is longitudinal unit vector
    def fn_unitvec_lon(self):
        pos = fn.coord()
        # r = fn.math.sqrt(pos[0]**2 + pos[1]**2 + pos[2]**2)
        # colat = fn.math.acos(pos[0]/r)
        lon   = fn.math.atan2(pos[2],pos[1])
        vec   = -fn.math.sin(lon) * (0.0, 1.0, 0.0)
        vec  +=  fn.math.cos(lon) * (0.0, 0.0, 1.0)
        return vec


class FeMesh_SquaredCircle(FeMesh_Cartesian):
# LNM
# A 2D cubed sphere with the inner nodes masked out / set to boundary nodes

    def __init__(self, radial_resolution=6, radii=(0.5,1.0), core_stretch=1.0, process_kites=True, elementType="Q1/dQ0", **kwargs):
        """
        Refer to parent classes for parameters beyond those below.

        Parameter
        ---------
        resolution : int
            Int determining number of *elements* radially (implicity sets lateral resolution)
        radii : tuple
            Tuple determining the (inner radius, outer radius ).
        core_stretch : float
            Float that determines the relative node spacing in the core cf. outer shell

        """

        if not isinstance(radial_resolution, int):
            raise TypeError("CubedSphere: radial_resolution expects an integer value")

        if not isinstance(core_stretch, float):
            raise TypeError("CubedSphere: core_stretch expects an float value")

        if not isinstance( radii, (tuple,list)):
            raise TypeError("Provided 'radii' must be a tuple/list of 2 floats")
        if len(radii) != 2:
            raise ValueError("Provided 'radii' must be a tuple/list of 2 floats")
        for el in radii:
            if not isinstance( el, (float,int)) :
                raise TypeError("Provided 'radii' must be a tuple/list of 2 floats")


        self._radial_resolution = radial_resolution
        self._core_stretch = core_stretch
        self._radii = radii
        self._kites=process_kites


        # build 3D mesh cartesian mesh centred on (0.0,0.0,0.0) - in _setup() we deform the mesh
        # elementType="Q1/dQ0", otherwise we have crashes ... sigh

        res = radial_resolution * 2

        import time
        walltime=time.clock()

        super(FeMesh_SquaredCircle,self).__init__(elementRes=(res, res),
                    minCoord=(-1.0,-1.0),
                    maxCoord=( 1.0, 1.0),
                    periodic=None, **kwargs)

        self.specialSets["surface_VertexSet"]    = _specialSets_Cartesian.AllWalls


        ## The following can be defined at this stage since they don't
        ## depend on the mesh resolution or coordinates.

        (radiusFn, thetaFn) = self.fn_r_theta()

        dR = (radii[1]-radii[0])
        iR = (radii[0])

        # A uw function that scales from 0.0 at the inner surface to 1.0 at the outer
        self.unit_radiusFn = fn.branching.conditional([ (radiusFn < iR, 0.0),
                                                      (True, (radiusFn-iR) / dR) ])

        # A uw function that is 1.0 between the inner and outer shell and 0.0
        # in the excluded inner region
        self.shellMaskFn  = fn.branching.conditional([ (radiusFn < iR, 0.0),
                                                       (True, 1.0 )])



    def _setup(self):
        import underworld.function as fn

        # This is an alias for the mesh points in Cartesian

        self.xy = self.data
        self.rtheta = self.data.copy()

        inner_radius = self._radii[0] / self._radii[1]
        outer_radius = 1.0

        # We need a mesh with a node that aligns on the CMB

        core_elts  = np.round( self._radial_resolution * inner_radius / self._core_stretch)
        scale = self._radial_resolution * inner_radius / core_elts
        max_puff1 = outer_radius * scale
        scale_back = (outer_radius-inner_radius) / (max_puff1-inner_radius)

        self._core_elt_resolution = core_elts

        with self.deform_mesh():

            ## Radial levels before a

            (x,y) = (self.data[:,0], self.data[:,1])


            dd = 0.0000001 * inner_radius
            x[ x < dd ] += dd


            # This is a modification which applies the same transformation at each
            # level of the mesh and hence creates nested spheres. The issue here
            # is the element geometry along various symmetry planes / axes

            rr = np.maximum(np.abs(x),np.abs(y))

            xr = x  / rr
            yr = y  / rr

            xp = x * np.sqrt(1.0 - 0.5*yr**2)
            yp = y * np.sqrt(1.0 - 0.5*xr**2)

            ur = (rr - inner_radius) / (outer_radius-inner_radius)


            # cases with kites ... nodes where abs(x) = abs(y) = abs(z)

            is_a_kite = np.isclose(np.abs(x), np.abs(y))

            # print("Kites: {}".format(np.where(is_a_kite)))


            r   = np.sqrt(xp**2 + yp**2)
            rscale = self._radii[1] * np.minimum(rr * scale, (rr * scale - inner_radius) * scale_back + inner_radius)

            if self._kites:
                inodes = np.isclose(rscale, self._radii[0], rtol=1.0e-3)
                onodes = np.isclose(rscale, self._radii[1], rtol=1.0e-3)
                ionodes = np.logical_or(inodes, onodes)
                bodynodes = np.logical_not(ionodes)

                # kites are not i/o nodes
                is_moveable_kite = np.logical_and(bodynodes, is_a_kite)

                rscale[is_moveable_kite] += 0.5 * (self._radii[1] - self._radii[0]) / self._radial_resolution


            theta   = np.arctan2(yp,xp)

            self.rtheta[:,0] = rscale
            self.rtheta[:,1] = theta

            self.data[:,0] = rscale * np.cos(theta)
            self.data[:,1] = rscale * np.sin(theta)
            #

        ## Vertex sets for boundaries, excluded regions and the central node

        surface_vertexSet = self.specialSets["surface_VertexSet"]

        lower_surface_vertexSet = self.specialSets["Empty"]
        lower_surface_vertexSet += np.where(np.isclose(self.rtheta[:,0], self._radii[0], rtol=1.0e-3))[0]
        self.specialSets["lower_surface_vertexSet"]  = lambda selfobject: lower_surface_vertexSet

        excluded_vertexSet = self.specialSets["Empty"]
        excluded_vertexSet += np.where(self.rtheta[:,0] < self._radii[0]*0.999)[0]  # Need to be careful about the resolution.
        self.specialSets["excluded_vertexSet"] = lambda selfobject: excluded_vertexSet

        dead_centre_vertexSet = self.specialSets["Empty"]
        dead_centre_vertexSet += np.where(self.rtheta[:,0] < 0.001 * self._radii[1])[0]
        self.specialSets["dead_centre_vertexSet"] = lambda selfobject: dead_centre_vertexSet

        ## mesh geometry information
        self.area = uw.utils.Integral(self.shellMaskFn, self).evaluate()[0]
        self.full_area = uw.utils.Integral(1.0, self).evaluate()[0]

        ## moments of weight functions used to compute mean / radial gradients in the shell
        ## calculate this once at setup time.
        self._c0 = uw.utils.Integral(self.unit_radiusFn, self).evaluate()[0] / self.area
        self._c1 = uw.utils.Integral(self.shellMaskFn*(self.unit_radiusFn-self._c0)**2, self).evaluate()[0]



        """
        Rotation documentation.
        We will create 3 basis vectors that will rotate the (x,y,z) problem to be a
        (r,n,t) [radial, normal to cut, tangential to cut] problem.

        The rotations are performed on the local element level using the existing machinery
        provided by UW2. As such only elements on the domain boundary need rotation, all internal
        elements can continue with the (x,y,z) representation.

        This is implemented with rotations on all dofs such that:
         1. If on the domain boundary - rotated to (r,n,t) and
         2. If not on the domain boundary - rotated by identity matrix i.e. (NO rotation).
        """

        # initialiase bases vectors as meshVariables
        self._e1 = self.add_variable(nodeDofCount=2)
        self._e2 = self.add_variable(nodeDofCount=2)

        self._e1.data[:] = self.fn_unitvec_r().evaluate(self)
        self._e2.data[:] = self.fn_unitvec_theta().evaluate(self)


    def mean_value(self, fn=None):
        """Returns mean value on the shell of scalar uw function"""

        ## Need to check here that the supplied function is
        ## valid and is a scalar.

        mean_value = uw.utils.Integral(fn * self.shellMaskFn, self).evaluate()[0] / self.area

        return mean_value

    def radial_gradient_value(self, fn=None):
        """Returns vertical gradient within the shell of scalar uw function"""

        ## Need to check here that the supplied function is
        ## valid and is a scalar. Mask function is to eliminate the
        ## effect of the values in the core.

        rGrad = uw.utils.Integral(fn * (self.unit_radiusFn-self._c0)*self.shellMaskFn, self).evaluate()[0] / self._c1

        return rGrad

    def fn_r_theta(self):
        pos = fn.coord()
        rFn = fn.math.sqrt(pos[0]**2 + pos[1]**2)
        thetaFn = fn.math.atan2(pos[1],pos[0])
        return rFn, thetaFn


    def vec_rtheta2xyz(self, vField, index_set=None):
        """ Takes a vector mesh variable and optional index set
            This is not a UW function though perhaps it should be
        """

        if index_set is None:
            nodes = slice(None)
        else:
            nodes = index_set.data

        tv = vField.data[nodes,:].copy() # be careful not to over-write
        uv = vField.data[nodes,:]

        e1 = self._e1
        e2 = self._e2

        tv[:,0] = uv[:,0] * e1[:,0] + uv[:,1] * e2[:,0]
        tv[:,1] = uv[:,0] * e1[:,1] + uv[:,1] * e2[:,1]

        return tv

    def vec_xyz2rtheta(self, vField, index_set=None):
        """ Takes a vector mesh variable and optional index set
            This is not a UW function though perhaps it should be
        """

        if index_set is None:
            nodes = slice(None)
        else:
            nodes = index_set.data

        tv = vField.data[nodes,:].copy() # be careful not to over-write
        uv = vField.data[nodes,:]

        e1 = self._e1
        e2 = self._e2

        tv[:,0] = uv[:,0] * e1[:,0] + uv[:,1] * e1[:,1]
        tv[:,1] = uv[:,0] * e2[:,0] + uv[:,1] * e2[:,1]

        return tv


    def fn_unitvec_r(self):
        pos = fn.coord()
        mag = fn.math.sqrt(fn.math.dot( pos, pos ))
        r_vec = pos / mag
        return r_vec

    # The unit vector is defined along the direction of colatitude towards North
    def fn_unitvec_theta(self):
        pos = fn.coord()
        # r = fn.math.sqrt(pos[0]**2 + pos[1]**2 + pos[2]**2)

        r = fn.math.sqrt(fn.math.dot( pos, pos ))
        theta   = fn.math.atan2(pos[1],pos[0])

        vec  =  -fn.math.sin(theta) * (1.0, 0.0)
        vec  +=  fn.math.cos(theta) * (0.0, 1.0)

        return vec




class FeMesh_Annulus(FeMesh_Cartesian):
    def __init__(self, elementRes=(10,16), radialLengths=(3.0,6.0), angularExtent=[0.0,360.0], centroid=[0.0,0.0], periodic=[False, True],  **kwargs):
        """
        This class generates a 2D finite element mesh which is topologically cartesian
        and in an annulus geometry. It is possible to directly build a dual mesh by
        passing a pair of element types to the constructor. Warning only 'elementTypes' Q1/dQ0 are tested,
        use other types at your own risk.

        Class initialiser for Annulus mesh, centered on the 'centroid'.

        MinI_VertexSet / MaxI_VertexSet -> radial walls       : [min/max] = [inner/outer]
        MinJ_VertexSet / MaxJ_VertexSet -> angular walls      : [min/max] = [right/left]

        Parameter
        ---------
        elementRes : 3-tuple
            1st element - Number of elements across the radial length of the domain
            2nd element - Number of elements along the circumfrance

        radialLengths : 2-tuple, default (3.0,6.0)
            The radial position of the inner and outer surfaces respectively.
            (inner radialLengths, outer radialLengths)

        angularExtent : 2-tuple, default (0.0,360.0)
            The angular extent of the domain, i.e. [15,75], starts at 15 degrees until 75 degrees.
            0 degrees represents the x-axis, i.e. 3 o'clock.

        radialData : Return the mesh node locations in polar form.
            (radial length, angle in degrees)

        periodic : 2-tuple, default [False,True]
            Sets the periodic boundary conditions along the radial and angular walls, respectively

        See parent classes for further required/optional parameters.

        >>> (radMin, radMax) = (4.0,8.0)
        >>> mesh = uw.mesh.FeMesh_Annulus( elementRes=(14, 36), radialLengths=(radMin, radMax), angularExtent=[0.0,180.0] )
        >>> integral = uw.utils.Integral( 1.0, mesh).evaluate()[0]
        >>> exact = np.pi*(radMax**2 - radMin**2)/2.
        >>> np.fabs(integral-exact)/exact < 1e-1
        True

        """

        errmsg = "Provided 'angularExtent' must be a tuple/list of 2 floats between values [0,360]"
        if not isinstance( angularExtent, (tuple,list)):
            raise TypeError(errmsg)
        if len(angularExtent) != 2:
            raise ValueError(errmsg)
        for el in angularExtent:
            if not isinstance( el, (float,int)) or (el < 0.0 or el > 360.0):
                raise TypeError(errmsg)
        self._angularExtent = angularExtent

        errmsg = "Provided 'radialLengths' must be a tuple/list of 2 floats"
        if not isinstance( radialLengths, (tuple,list)):
            raise TypeError(errmsg)
        if len(radialLengths) != 2:
            raise ValueError(errmsg)
        for el in radialLengths:
            if not isinstance( el, (float,int)) :
                raise TypeError(errmsg)
        self._radialLengths = radialLengths

        # build 3D mesh cartesian mesh centred on (0.0,0.0,0.0) - in _setup() we deform the mesh
        super(FeMesh_Annulus,self).__init__(elementRes=elementRes,
                    minCoord=(radialLengths[0],angularExtent[0]), maxCoord=(radialLengths[1],angularExtent[1]), periodic=periodic, **kwargs)

        # define new specialSets, TODO, remove labels that don't make sense for the annulus
        self.specialSets["inner"] = _specialSets_Cartesian.MinI_VertexSet
        self.specialSets["outer"] = _specialSets_Cartesian.MaxI_VertexSet
        self._centroid = centroid

    @property
    def radialLengths(self):
        """
        Returns:
        Annulus min/max radius
        """
        return self._radialLengths

    @property
    def angularExtent(self):
        """
        Returns:
        Annulus min/max angular extents
        """
        return self._angularExtent

    def fn_unitvec_radial(self):
        # returns the radial position
        pos = fn.coord()
        centre = self._centroid
        r_vec = pos - centre
        mag = fn.math.sqrt(fn.math.dot( r_vec, r_vec ))
        r_vec = r_vec / mag
        return r_vec

    def fn_unitvec_tangent(self):
        # returns the radial position
        pos = fn.coord()
        centre = self._centroid
        r_vec = pos - centre
        theta = [-1.0*r_vec[1], r_vec[0]]
        mag = fn.math.sqrt(fn.math.dot( theta, theta ))
        theta = theta / mag
        return theta

    @property
    def radialData(self):
        # returns data in polar form
        r = np.sqrt((self.data ** 2).sum(1))
        theta = (180/np.pi)*np.arctan2(self.data[:,1],self.data[:,0])
        return np.array([r,theta]).T

    # a function for radial coordinates
    @property
    def fn_radial(self):
        pos = fn.coord()
        centre = self._centroid
        r_vec = pos - centre
        return fn.math.sqrt(fn.math.dot( r_vec, r_vec ))

    def _setup(self):
        from underworld import function as fn

        with self.deform_mesh():
            # basic polar coordinate -> cartesian map, i.e. r,t -> x,y
            r = self.data[:,0]
            t = self.data[:,1] * np.pi/180.0

            offset_x = self._centroid[0]
            offset_y = self._centroid[1]

            (self.data[:,0], self.data[:,1]) = offset_x + r*np.cos(t), offset_y + r*np.sin(t)

        # add a boundary MeshVariable - 1 if nodes is on the boundary(ie 'AllWalls_VertexSet'), 0 if node is internal
        self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)
        self.bndMeshVariable.data[:] = 0.
        self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0


        self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)

        self.bndMeshVariable.data[:] = 0.
        # set a value 1.0 on provided vertices
        self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0
        # note we use this condition to only capture border quadrature points
        # on the surface. For points not on the surface the bndMeshVariable will evaluate
        # <1.0, so we need to remove those from the integration as well.
        self.bnd_vec_normal  = fn.branching.conditional(
                             [ ( self.bndMeshVariable > 0.9, self.fn_unitvec_radial() ),
                               (               True, fn.misc.constant(1.0)*(1.0,0.0) ) ] )

        self.bnd_vec_tangent = fn.branching.conditional(
                             [ ( self.bndMeshVariable > 0.9, self.fn_unitvec_tangent() ),
                               (               True, fn.misc.constant(1.0)*(0.0,1.0) ) ] )

         # define solid body rotation function for the annulus
        r = fn.math.sqrt(fn.math.pow(fn.coord()[0],2.) + fn.math.pow(fn.coord()[1],2.))
        self.sbr_fn = r*self.fn_unitvec_tangent() # solid body rotation function

def _FeMesh_Annulus(*args, **kwargs):
    from warnings import warn
    warn("Deprecation warning, '_FeMesh_Annulus' will be deprecated, call 'FeMesh_Annulus' instead")
    return FeMesh_Annulus(*args, **kwargs)

def _FeMesh_Regional(*args, **kwargs):
    from warnings import warn
    warn("Deprecation warning, '_FeMesh_Regional' will be deprecated, call 'FeMesh_SRegion' instead")
    return FeMesh_SRegion(*args, **kwargs)
