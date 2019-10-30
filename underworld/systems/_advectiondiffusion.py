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
from . import sle
import underworld.libUnderworld as libUnderworld
from mpi4py import MPI


class AdvectionDiffusion(object):
    """
    This class provides functionality for a discrete representation
    of an advection-diffusion equation.

    .. math::
        \\frac{\\partial\\phi}{\\partial t}  + {\\bf u } \\cdot \\nabla \\phi= \\nabla { ( k  \\nabla \\phi ) } + H

    Two methods are available to integrate the scalar :math:`\phi` through time:
    
    1. SUPG - The Streamline Upwind Petrov Galerkin method. [1]_
    2. SLCN - The Semi-Lagrangian Crank-Nicholson method. [2]_

    SLCN is the preferred method for Q1 elements on a orthogonal cartesian meshes. It is
    quicker, less diffusive and unconditionally stable. SUPG, the legacy method, is more robust 
    for arbitrarily deformed meshes. Both methods are considered EXPERIMENTAL for non Q1 element meshes.

    Parameters
    ----------
    phiField : underworld.mesh.MeshVariable
        The concentration field, typically the temperature field
    velocityField : underworld.mesh.MeshVariable
        The velocity field.
    fn_diffusivity : underworld.function.Function
        A function that defines the diffusivity within the domain.
    fn_sourceTerm : underworld.function.Function
        A function that defines the heating within the domain. Optional.
    conditions : underworld.conditions.SystemCondition
        Numerical conditions to impose on the system. This should be supplied as
        the condition itself, or a list object containing the conditions.
    phiDotField : underworld.mesh.MeshVariable
        Only used for SUPG. 
        A MeshVariable that defines the initial time derivative of the phiField.
        Typically 0 at the beginning of a model, e.g. phiDotField.data[:]=0
        When using a phiField loaded from disk one should also load the phiDotField to ensure
        the solving method has the time derivative information for a smooth restart.
        No Dirichlet conditions are required for this field as the phiField degrees of freedom
        map exactly to this field's Dirichlet conditions, the value of which ought to be 0
        for constant values of phi.
    allow_non_q1 : Bool (default False)
        Allow the integration to perform over a non Q1 element mesh. (Under Q2
        elements instabilities have been observed as the implementation is only
        for Q1 elements)


    Notes
    -----
    Constructor must be called by collectively all processes.

    .. [1] Brooks, A. N. and Hughes, T. J. R., "Streamline Upwind/Petrov-Galerkin Formulations for 
       Convection Dominated Flows with Particular Emphasis on the Incompressible Navier-Stokes 
       Equations", Comput. Methods Appl. Mech. Eng., Aug 1990, 199-259.
    .. [2] Spiegelman, M. and Katz, R.F., "A semi-Lagrangian Crank-Nicolson algorithm for the 
       numerical solution of advection-diffusion problems", Geochemistry, Geophysics, Geosystems, 
       7(4), 2006.
    """

    def __init__(self, phiField=None, velocityField=None, fn_diffusivity=None,
                 fn_sourceTerm=None, method="SUPG", conditions=[],
                 phiDotField=None, allow_non_q1=False, **kwargs):

        if not isinstance(method, str) or method.upper() not in ("SUPG","SLCN"):
            raise ValueError("'method' parameter must be 'SUPG' or 'SLCN'")
        self.method = method.upper()

        if self.method == "SLCN" and phiDotField:
            import warnings
            warnings.warn("'phiDotField' doesn't influence the 'SLCN' method."+
                          " It's only required for the 'SUPG' method")
        if self.method == "SUPG" and not phiDotField:
            raise ValueError("'phiDotField' is required for the 'SUPG' method")

        # check phiField, velocity, diff, source, conditions
        if not isinstance( phiField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'phiField' must be of 'MeshVariable' class." )
        if phiField.data.shape[1] != 1:
            raise TypeError( "Provided 'phiField' must be a scalar" )

        if velocityField.data.shape[1] != phiField.mesh.dim:
            raise TypeError( "Provided 'velocityField' must be the same dimensionality as the phiField's mesh" )

        if phiField.mesh.elementType != 'Q1':
            if allow_non_q1 == False:
                raise ValueError("The 'phiField' is discretised on a {} element mesh. The current 'uw.system.AdvectionDiffusion' "
                                 "implementation is only stable for a phiField discretised with Q1 elements. "
                                 "For non Q1 elements this implementation is EXPERIMENTAL, instability and implementation problems"
                                 "have been observed." 
                                 "Either create a Q1 mesh for the 'phiField' or, if you know what you're doing, override "
                                 "this error with the argument 'allow_non_q1=True' in the constructor.".format(phiField.mesh.elementType))

        if self.method == "SUPG":
            self.system = _SUPG_AdvectionDiffusion(
                                phiField, phiDotField, velocityField, 
                                fn_diffusivity, fn_sourceTerm, conditions)
        elif self.method == "SLCN":
            self.system = _SLCN_AdvectionDiffusion(
                                phiField, velocityField, fn_diffusivity, 
                                fn_sourceTerm, conditions)

    @property
    def velocityField(self):
        return self.system.velocityField

    @property
    def phiField(self):
        return self.system.phiField

    def integrate(self, dt=0.0, **kwargs):
        """
        Integrates the advection diffusion system through time, dt
        Must be called collectively by all processes.

        Parameters
        ----------
        dt : float
            The timestep interval to use
        """
        self.system.integrate(dt, **kwargs)

    def get_max_dt(self):
        """
        Returns a timestep size for the current system.

        Returns
        -------
        float
         The timestep size.
        """
        return self.system.get_max_dt()


class _SLCN_AdvectionDiffusion(object):
    def __init__(self, phiField, velocityField, fn_diffusivity, fn_sourceTerm=None, conditions=[]):
        """Implements the Spiegelman / Katz   Semi-lagrangian Advection / Crank Nicholson Diffusion algorithm"""

        mesh = velocityField.mesh

        # unknown field and velocity field
        self.phiField = phiField
        self.vField   = velocityField

        # uw.functions for diffusivity and a source term
        self.fn_diffusivity = uw.function.Function.convert(fn_diffusivity)
        self.fn_sourceTerm  = uw.function.Function.convert(fn_sourceTerm)
        self.fn_dt          = uw.function.misc.constant(1.0)  # dummy value

        # build a grid field, phiStar, for the information at departure points
        self._phiStar = phiField.copy()

        # placeholder for swarm-based _mesh_interpolator_stripy
        self._mswarm = None
        self._mswarm_advector = None

        # a data storage for the local node indices for cubic interpolation
        # stencil.
        self._stencilField = mesh.add_variable(nodeDofCount=3)

        # check input 'conditions' list is valid
        if not isinstance(conditions, (list, tuple)):
            conditionslist = []
            conditionslist.append(conditions)
            conditions = conditionslist

        nbc = None  # storage for neumann conditions

        for cond in conditions:
            if not isinstance( cond, uw.conditions.SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list 'SystemCondition' objects." )
            # check type of condition
            if type(cond) == uw.conditions.NeumannCondition:
                if nbc != None:
                    # check only one nbc condition is given in 'conditions' list
                    RuntimeError( "Provided 'conditions' can only accept one NeumannConditions condition object.")
                nbc = cond
            elif type(cond) == uw.conditions.DirichletCondition:
                if cond.variable == self.phiField:
                    libUnderworld.StgFEM.FeVariable_SetBC( self.phiField._cself, cond._cself )
            else:
                raise RuntimeError("Input condition type not recognised.")
        self._conditions = conditions

        # build matrices and vectors
        phi_eqnums        = uw.systems.sle.EqNumber(phiField)
        solv = self._solv = uw.systems.sle.SolutionVector(phiField, phi_eqnums)
        f    = self._f    = uw.systems.sle.AssembledVector(phiField, phi_eqnums)
        K    = self._K    = uw.systems.sle.AssembledMatrix(solv, solv, f)

        # create quadrature swarm
        intSwarm = uw.swarm.GaussIntegrationSwarm(mesh, particleCount=5)

        fn_dt = self.fn_dt

        # take sourceTerm into account - implementation doesn't track from departure points so is less accurate in time
        if fn_sourceTerm is not None:
            rhs_term = self._phiStar + fn_dt * self.fn_sourceTerm
        else:
            rhs_term = self._phiStar

        self._mv_term = uw.systems.sle.VectorAssemblyTerm_NA__Fn( 
                            integrationSwarm = intSwarm,
                            assembledObject    = f,
                            mesh = mesh,
                            fn   = 1. * rhs_term )

        self._kv_term = uw.systems.sle.VectorAssemblyTerm_NA_i__Fn_i( 
                            integrationSwarm = intSwarm,
                            assembledObject  = f,
                            mesh = mesh,
                            fn   = -0.5 * fn_dt * self.fn_diffusivity * self._phiStar.fn_gradient )

        if nbc is not None:
            # -VE flux because of the FEM discretisation method of the initial equation
            negativeCond = uw.conditions.NeumannCondition( 
                            fn_flux  = fn_dt * nbc.fn_flux,
                            variable = nbc.variable,
                            indexSetsPerDof = nbc.indexSetsPerDof )

            # NOTE many NeumannConditions can be used but the _sufaceFluxTerm only records the last
            self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                        assembledObject    = f,
                                        surfaceGaussPoints = 2,
                                        nbc         = negativeCond )

        self._k_term = uw.systems.sle.MatrixAssemblyTerm_NA_i__NB_i__Fn(
                        assembledObject  = K,
                        integrationSwarm = intSwarm,
                        fn = 0.5 * fn_dt * self.fn_diffusivity)

        self._m_term = uw.systems.sle.MatrixAssemblyTerm_NA__NB__Fn(
                        assembledObject  = K,
                        integrationSwarm = intSwarm,
                        fn   = 1.,
                        mesh = mesh)

        # functions used to calculate the timestep, see function get_max_dt()

        self._maxVsq  = uw.function.view.min_max(velocityField, fn_norm = uw.function.math.dot(velocityField, velocityField) )
        self._maxDiff = uw.function.view.min_max(self.fn_diffusivity)

        # Note that the c level minSep on the mesh is for the local domain
        sepFn = uw.function.misc.constant( velocityField.mesh._cself.minSep)
        minmaxSep  = uw.function.view.min_max(sepFn)
        minmaxSep.evaluate(mesh)

        self._minDx = minmaxSep.min_global()

        # the required for the solve
        self.sle = uw.utils.SolveLinearSystem(AMat=K, bVec=f, xVec=solv)

        # Check available interpolation packages
        self._mesh_interpolator_stripy = None
        self._mesh_interpolator_rbf = None
        self._cKDTree = None

        try:
            import stripy
            self._have_stripy = True
        except ImportError:
            self._have_stripy = False

        try:
            from scipy.interpolate import Rbf
            self._have_rbf = True
        except ImportError:
            self._have_rbf = False


    def _integrate_original_version(self, dt, solve=True):
        # use the given timestep
        self.fn_dt.value = dt

        # apply conditions
        uw.libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._solv._cself )

        # update T* - temperature at departure points

        uw.libUnderworld.StgFEM.SemiLagrangianIntegrator_SolveNew(
            self.phiField._cself,
            dt,
            self.vField._cself,
            self._phiStar._cself,
            self._stencilField )

        # solve T

        if solve:
            self.sle.solve()

        return

    def _phiStar_stripy_old(self, dt):

        import stripy
        import numpy as np
        from scipy.spatial import cKDTree

        mesh = self.phiField.mesh
        phiStar = mesh.add_variable(dataType="double", nodeDofCount=1)
        phiNorm = mesh.add_variable(dataType="double", nodeDofCount=1)

        if self._mesh_interpolator_stripy == None:
            self._mesh_interpolator_stripy = stripy.Triangulation(mesh.data[:, 0], mesh.data[:, 1], permute=True)

        mesh_interpolator = self._mesh_interpolator_stripy

        # The swarm info can also be cached !
        mswarm = uw.swarm.Swarm(mesh, particleEscape=True)
        mswarm_map = mswarm.add_variable(dataType="int", count=1)
        mswarm_home_pts = mswarm.add_variable(dataType="double", count=mesh.dim)
        mcoords = mesh.data.copy()

        # layout = uw.swarm.layouts.PerCellGaussLayout(mswarm, gaussPointCount=5)
        # mswarm.populate_using_layout(layout)

        local_nId = -1 * np.ones(mesh.nodesGlobal, dtype=np.int)
        for i, gId in enumerate(mesh.data_nodegId):
            local_nId[gId] = i

        element_centroids = mesh.data[local_nId[mesh.data_elementNodes]].mean(axis=1)
        element_centroids2 = element_centroids.reshape(tuple((*element_centroids.shape, 1)))
        element_coords = mesh.data[local_nId[mesh.data_elementNodes]].transpose(0, 2, 1)
        swarm_coords   = (element_coords - element_centroids2) * 0.8 + element_centroids2
        swarm_coords2  = swarm_coords.transpose(0, 2, 1).reshape(-1, 2)
        localID = mswarm.add_particles_with_coordinates(swarm_coords2)

        accepted = np.where(localID != -1)
        mswarm_map.data[:] = mesh.data_elementNodes.reshape(-1, 1)[accepted]
        mswarm_home_pts.data[:] = mswarm.particleCoordinates.data[accepted]

        #surface = mesh.specialSets["surface_VertexSet"]
        # mcoords[surface,:] *= 0.9999
        # localID = mswarm.add_particles_with_coordinates(mcoords)
        # not_accepted = np.where(localID == -1)
        print("A{}: mswarm has {} particles ({} local)".format(uw.mpi.rank, mswarm.particleGlobalCount, mswarm.particleLocalCount))

        morig_coords = mswarm.add_variable("double", mesh.dim)
        morig_coords.data[...] = mswarm.particleCoordinates.data[...]

        mswarm_Tstar = mswarm.add_variable(dataType="float", count=1)

        madvector = uw.systems.SwarmAdvector(velocityField=self.vField, swarm=mswarm)
        madvector.integrate(-dt, update_owners=True)
        # madvector.integrate(-dt*0.5, update_owners=True)

        print("B{}: mswarm has {} particles ({} local)".format(uw.mpi.rank, mswarm.particleGlobalCount, mswarm.particleLocalCount))

        # mswarm_Tstar.data[:,0], err = mesh_interpolator.interpolate_cubic(mswarm.particleCoordinates.data[:,0],
        #                                 mswarm.particleCoordinates.data[:,1],
        #                                 self.phiField.data)
        #

        mswarm_Tstar.data[:] = self.phiField.evaluate(mswarm)
        ## mswarm_Tstar.data[:,0] = mswarm.particleCoordinates.data[:,0]

        # Restore
        with mswarm.deform_swarm():
            mswarm.particleCoordinates.data[:] = mswarm_home_pts.data[:]

        phiStar.data[:] = 0.0
        phiNorm.data[:] = 0.0

        # Surely this can be optimised (maybe the kdTree (cached) would be quicker / less storage ?)
        for i, gnode in enumerate(mswarm_map.data[:, 0]):
            node = np.where(mesh.data_nodegId == gnode)[0]
            phiStar.data[node] += mswarm_Tstar.data[i]
            phiNorm.data[node] += 1.0

        if uw.mpi.size > 1:
            mswarm.shadow_particles_fetch()
            for i, gnode in enumerate(mswarm_map.data_shadow[:, 0]):
                node = np.where(mesh.data_nodegId == gnode)[0]
                phiStar.data[node] += mswarm_Tstar.data_shadow[i, 0]
                phiNorm.data[node] += 1.0

        phiStar.data[np.where(phiNorm.data > 0.0)] /= phiNorm.data[np.where(phiNorm.data > 0.0)]

        return phiStar

    def _build_phiStar_swarm(self, ratio=0.9):

        import numpy as np

        mesh = self.phiField.mesh

        if self._mswarm == None:
            mswarm = uw.swarm.Swarm(mesh, particleEscape=True)
            mswarm_map = mswarm.add_variable(dataType="int", count=1)
            mswarm_home_pts = mswarm.add_variable(dataType="double", count=mesh.dim)
            mswarm_phiStar = mswarm.add_variable(dataType="float", count=1)

            local_nId = -1 * np.ones(mesh.nodesGlobal, dtype=np.int)
            for i, gId in enumerate(mesh.data_nodegId):
                local_nId[gId] = i

            # print("{} - building mswarm".format(uw.mpi.rank), flush=True )

            layout = uw.swarm.layouts.PerCellRandomLayout(mswarm, particlesPerCell=mesh.data_elementNodes[0].shape[0])
            mswarm.populate_using_layout(layout)

            # element_centroids = mesh.data[local_nId[mesh.data_elementNodes]].mean(axis=1)
            # element_centroids2 = element_centroids.reshape(tuple((*element_centroids.shape, 1)))
            # element_coords = mesh.data[local_nId[mesh.data_elementNodes]].transpose(0,2,1)
            # swarm_coords   = (element_coords - element_centroids2) * ratio + element_centroids2
            # swarm_coords2  = swarm_coords.transpose(0,2,1).reshape(-1, mesh.dim)

            # This is not optimised for the element loop
            # But there eliminates the initial search issues
            # associated with adding points to an empty swarm.

            ## print("{} - adding {} particles".format(uw.mpi.rank, swarm_coords2.shape[0]), flush=True )

            with mswarm.deform_swarm(update_owners=True):
                for el in range(0, mesh.elementsLocal):
                    element_centroid = mesh.data[local_nId[mesh.data_elementNodes[el]]].mean(axis=0)
                    node_rel_coords  = mesh.data[local_nId[mesh.data_elementNodes[el]]] - element_centroid
                    particle_coords  = node_rel_coords * ratio + element_centroid

                    particles = np.where(mswarm.owningCell.data == el)[0]
                    mswarm.particleCoordinates.data[particles] = particle_coords
                    mswarm_map.data[particles, 0] = mesh.data_elementNodes[el]

            #
            # localID = mswarm.add_particles_with_coordinates(swarm_coords2)
            # accepted = np.where(localID != -1)

            # mswarm_map.data[:] = mesh.data_elementNodes.reshape(-1,1)[accepted]
            # mswarm_home_pts.data[:] = swarm_coords2[accepted]

            mswarm_home_pts.data[:] = mswarm.particleCoordinates.data[:]

            # if np.any(localID == -1):
            #     print("{} - particles missing: {}".format(uw.mpi.rank, np.where(localID == -1).shape[0]), flush=True )

            # Make these variables accessible

            self._mswarm = mswarm
            self._mswarm_global_particles = mswarm.particleGlobalCount
            self._mswarm_map = mswarm_map
            self._mswarm_home_pts = mswarm_home_pts
            self._mswarm_phiStar = mswarm_phiStar

        if self._mswarm_advector == None:
            madvector = uw.systems.SwarmAdvector(velocityField=self.vField, swarm=self._mswarm)
            self._mswarm_advector = madvector

        return

    def _reset_phiStar_swarm(self):

        if self._mswarm == None or self._mswarm_advector == None:
            self._build_phiStar_swarm()
        else:
            # Original point locations are carried by the particles
            # and therefore it can snap back
            import warnings

            with self._mswarm.deform_swarm():
                self._mswarm.particleCoordinates.data[:] = self._mswarm_home_pts.data[:]
                if self._mswarm.particleGlobalCount != self._mswarm_global_particles:
                    warnings.warn("Some particles were lost during advection step - smaller dt may be needed")

        return

    def _phiStar_stripy(self, dt, smooth=0.9):

        import stripy
        import numpy as np
        from scipy.spatial import cKDTree
        import time

        if self._mswarm == None:
            self._build_phiStar_swarm(ratio=smooth)

        mesh = self.phiField.mesh
        phiStar = mesh.add_variable(dataType="double", nodeDofCount=1)
        phiNorm = mesh.add_variable(dataType="double", nodeDofCount=1)
        mswarm_phiStar = self._mswarm_phiStar
        mswarm = self._mswarm

        if self._mesh_interpolator_stripy == None:
            self._mesh_interpolator_stripy = stripy.Triangulation(mesh.data[:, 0], mesh.data[:, 1], permute=True)

        mesh_interpolator = self._mesh_interpolator_stripy

        # Consider doing this in 2 half steps ...
        self._mswarm_advector.integrate(-dt, update_owners=True)

        mswarm_phiStar.data[:, 0], err = mesh_interpolator.interpolate_cubic(mswarm.particleCoordinates.data[:, 0],
                                                                             mswarm.particleCoordinates.data[:, 1],
                                                                             self.phiField.data)

        # Restore
        self._reset_phiStar_swarm()

        phiStar.data[:] = 0.0
        phiNorm.data[:] = 0.0

        # Surely this can be optimised (maybe the kdTree (cached) would be quicker / less storage ?)
        for i, gnode in enumerate(self._mswarm_map.data[:, 0]):
            node = np.where(mesh.data_nodegId == gnode)[0]
            phiStar.data[node] += mswarm_phiStar.data[i]
            phiNorm.data[node] += 1.0

        if uw.mpi.size > 1:
            mswarm.shadow_particles_fetch()
            for i, gnode in enumerate(self._mswarm_map.data_shadow[:, 0]):
                node = np.where(mesh.data_nodegId == gnode)[0]
                phiStar.data[node] += mswarm_phiStar.data_shadow[i, 0]
                phiNorm.data[node] += 1.0

        phiStar.data[np.where(phiNorm.data > 0.0)] /= phiNorm.data[np.where(phiNorm.data > 0.0)]

        self._phiStar_dirichlet_conditions(phiStar)

        return phiStar

    def _phiStar_dirichlet_conditions(self, phiStar):

        for cond in self._conditions:
            if type(cond) == uw.conditions.DirichletCondition:
                if cond.variable == self.phiField:
                    indexSet = cond.indexSetsPerDof[0]
                    phiStar.data[indexSet] = self.phiField.data[indexSet]

        return

    def _phiStar_rbf(self, dt, smooth=0.9):

        import numpy as np
        from scipy.spatial import cKDTree
        from scipy.interpolate import Rbf
        import time

        if self._mswarm == None:
            self._build_phiStar_swarm(ratio=smooth)

        walltime = time.process_time()

        mesh = self.phiField.mesh
        phiStar = mesh.add_variable(dataType="double", nodeDofCount=1)
        phiNorm = mesh.add_variable(dataType="double", nodeDofCount=1)
        mswarm_phiStar = self._mswarm_phiStar
        mswarm = self._mswarm

        # ## This can't be cached
        # if mesh.dim == 2:
        #     mesh_interpolator = Rbf(mesh.data[:,0],mesh.data[:,1], self.phiField.data, smooth=0.0, function='thin_plate' )
        # else:
        #     mesh_interpolator = Rbf(mesh.data[:,0],mesh.data[:,1], mesh.data[:,2], self.phiField.data, smooth=0.0, function='thin_plate' )

        # This really only needs to be built if the mesh changes
        mesh_tree = cKDTree( mesh.data )

        self._mswarm_advector.integrate(-dt, update_owners=True)

        # if mesh.dim == 2:
        #     mswarm_phiStar.data[:,0] = mesh_interpolator(mswarm.particleCoordinates.data[:,0],
        #                                                  mswarm.particleCoordinates.data[:,1])
        # else:
        #     mswarm_phiStar.data[:,0] = mesh_interpolator(mswarm.particleCoordinates.data[:,0],
        #                                                  mswarm.particleCoordinates.data[:,1],
        #                                                  mswarm.particleCoordinates.data[:,2] )
        #

        # EBE version - global RBF is impractical in nearly every case
        # We need to know the element size and mesh dimension to do this interpolation
        # correctly ... first, the 3D, Q1 version ...

        if "Q1" in mesh.elementType:
            stencil_size = 6**mesh.dim
        elif "Q2" in mesh.elementType:
            stencil_size = 7**mesh.dim
        else:  # No idea
            stencil_size = 7**mesh.dim

        # I think this can be eliminated at some stage ...
        local_nId = -1 * np.ones(mesh.nodesGlobal, dtype=np.int)
        for i, gId in enumerate(mesh.data_nodegId):
            local_nId[gId] = i

        for el in range(0, mesh.elementsLocal):
            # if el%1000 == 0:
            #     print("{}: Element: {}".format(uw.mpi.rank, el), flush=True)
            element_centroid = mesh.data[local_nId[mesh.data_elementNodes[el]]].mean(axis=0)

            d, local_nodes = mesh_tree.query(element_centroid, k=stencil_size)
            particles = np.where(mswarm.owningCell.data == el)[0]

            if mesh.dim == 2:
                mesh_interpolator = Rbf(mesh.data[local_nodes, 0],
                                        mesh.data[local_nodes, 1],
                                        self.phiField.data[local_nodes], smooth=0.0, function='thin_plate' )
                locations_x, locations_y = mswarm.particleCoordinates.data[particles].T
                mswarm_phiStar.data[particles, 0] = mesh_interpolator(locations_x, locations_y)

            else:
                mesh_interpolator = Rbf(mesh.data[local_nodes, 0],
                                        mesh.data[local_nodes, 1],
                                        mesh.data[local_nodes, 2],
                                        self.phiField.data[local_nodes], smooth=0.0, function='thin_plate' )

                locations_x, locations_y, locations_z = mswarm.particleCoordinates.data[particles].T
                mswarm_phiStar.data[particles, 0] = mesh_interpolator(locations_x, locations_y, locations_z)

        # Restore
        self._reset_phiStar_swarm()

        phiStar.data[:] = 0.0
        phiNorm.data[:] = 0.0

        # Surely this can be optimised (maybe the kdTree (cached) would be quicker / less storage ?)
        for i, gnode in enumerate(self._mswarm_map.data[:, 0]):
            node = np.where(mesh.data_nodegId == gnode)[0]
            phiStar.data[node] += mswarm_phiStar.data[i]
            phiNorm.data[node] += 1.0

        if uw.mpi.size > 1:
            mswarm.shadow_particles_fetch()
            for i, gnode in enumerate(self._mswarm_map.data_shadow[:, 0]):
                node = np.where(mesh.data_nodegId == gnode)[0]
                phiStar.data[node] += mswarm_phiStar.data_shadow[i, 0]
                phiNorm.data[node] += 1.0

        phiStar.data[np.where(phiNorm.data > 0.0)] /= phiNorm.data[np.where(phiNorm.data > 0.0)]

        self._phiStar_dirichlet_conditions(phiStar)
        #
        # print("{} - RBF interpolation ... {}s".format(uw.mpi.rank, time.process_time()-walltime), flush=True )
        #

        return phiStar

    def _phiStar_fe(self, dt, smooth=0.9):

        import numpy as np
        import time
        from scipy.spatial import cKDTree

        if self._mswarm == None:
            self._build_phiStar_swarm(ratio=smooth)

        mesh = self.phiField.mesh
        phiStar = mesh.add_variable(dataType="double", nodeDofCount=1)
        phiNorm = mesh.add_variable(dataType="double", nodeDofCount=1)
        mswarm_phiStar = self._mswarm_phiStar
        mswarm = self._mswarm

        self._mswarm_advector.integrate(-dt, update_owners=True)

        # FE variable based interpolator
        mswarm_phiStar.data[:] = self.phiField.evaluate(mswarm)

        # Restore
        walltime = time.process_time()
        self._reset_phiStar_swarm()

        phiStar.data[:] = 0.0
        phiNorm.data[:] = 0.0

        # Surely this can be optimised (maybe the kdTree (cached) would be quicker / less storage ?)
        for i, gnode in enumerate(self._mswarm_map.data[:, 0]):
            node = np.where(mesh.data_nodegId == gnode)[0]
            phiStar.data[node] += mswarm_phiStar.data[i]
            phiNorm.data[node] += 1.0

        if uw.mpi.size > 1:
            mswarm.shadow_particles_fetch()
            for i, gnode in enumerate(self._mswarm_map.data_shadow[:, 0]):
                node = np.where(mesh.data_nodegId == gnode)[0]
                phiStar.data[node] += mswarm_phiStar.data_shadow[i, 0]
                phiNorm.data[node] += 1.0

        phiStar.data[np.where(phiNorm.data > 0.0)] /= phiNorm.data[np.where(phiNorm.data > 0.0)]

        self._phiStar_dirichlet_conditions(phiStar)

        return phiStar

    def integrate(self, dt=0.0, phiStar=None, interpolator="", solve=True, phiStarCopy=None, smooth=0.9, substeps=1):
        """SLCN integration in time. In a regular mesh, the update
        of the field can be calculated directly, but in an irregular
        mesh, it is necessary to supply phiStar (the T at launch points)"""

        import warnings

        dts = dt / float(substeps)
        for substep in range(0, substeps):

            # use the given timestep
            self.fn_dt.value = dts

            # apply conditions
            uw.libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._solv._cself )

            # update T* - temperature at departure points

            if "stripy" in interpolator.lower():
                if self._have_stripy:
                    if self.phiField.mesh.dim == 2:
                        phiStar = self._phiStar_stripy(dts, smooth=smooth)
                    else:
                        warnings.warn("stripy is only suitable for 2D meshes", category=RuntimeWarning)
                else:
                    warnings.warn("stripy is not installed", category=RuntimeWarning)

            if "rbf" in interpolator.lower():
                if self._have_rbf:
                    phiStar = self._phiStar_rbf(dts, smooth=smooth)
                else:
                    warnings.warn("scipy / rbf is not installed", category=RuntimeWarning)

            if "fe" in interpolator.lower():
                phiStar = self._phiStar_fe(dts, smooth=smooth)
                warnings.warn("fe is a low-order method for debugging use only", category=RuntimeWarning)

            if phiStar is None:
                
                if not hasattr(self, "_built_stencil"):
                    uw.libUnderworld.StgFEM.SemiLagrangianIntegrator_BuildStaticStencils(self._stencilField._cself)
                    self._built_stencil = True


                # Extremely unreliable !!
                uw.libUnderworld.StgFEM.SemiLagrangianIntegrator_SolveNew(
                    self.phiField._cself,
                    dt,
                    self.vField._cself,
                    self._phiStar._cself,
                    self._stencilField._cself )

            else:
                self._phiStar.data[:] = phiStar.data[:]

            if phiStarCopy is not None:
                phiStarCopy.data[:] = self._phiStar.data[:]

            # Solve the update problem
            if solve:
                self.sle.solve()

        self.fn_dt.value = dt
        return

    def launchPts(self, dt=0.0):
        """SLCN integration in time. In a regular mesh, the update
        of the field can be calculated directly, but in an irregular
        mesh, it is necessary to supply phiStar (the T at launch points)"""

        # use the given timestep
        self.fn_dt.value = dt

        # apply conditions
        uw.libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._solv._cself )

        # Find departure points

        # New FeVariable

        depPointsField = self.vField.copy()

        uw.libUnderworld.StgFEM.SemiLagrangianIntegrator_FindLaunchPts(
            dt,
            self.vField._cself,
            depPointsField._cself)

        return depPointsField

    def get_max_dt(self):
        """
        Returns a timestep size for the current system.

        Returns
        -------
        float
         The timestep size.
        """

        mesh = self.phiField.mesh
        vField = self.vField

        # Note: this is important (the velocity one, especially):

        self._maxVsq.reset()
        self._maxDiff.reset()

        # evaluate the global maximum velocity
        ignore = self._maxVsq.evaluate(mesh)
        vmax = uw._np.sqrt(self._maxVsq.max_global())

        # evaluate the global maximum diffusivity

        if type(self.fn_diffusivity) == uw.swarm.SwarmVariable:
            maxDiffusion = self._maxDiff.evaluate(self.fn_diffusivity.swarm)
        else:
            maxDiffusion = self._maxDiff.evaluate(self.phiField.mesh)

        maxDiffusion = self._maxDiff.max_global()

        # the minimum separation of the mesh (globally)
        dx = self._minDx
        #dx = self.vField.mesh._typicalDx # from LM implementation

        # Note, if dx is not constant, these tests are potentially
        # overly pessimistic

        diff_dt = dx * dx / maxDiffusion
        adv_dt  = dx / vmax

        return min(adv_dt, diff_dt)


class _SUPG_AdvectionDiffusion(_stgermain.StgCompoundComponent):
    """
    This class provides functionality for a discrete representation
    of an advection-diffusion equation.

    The class uses the Streamline Upwind Petrov Galerkin SUPG method
    to integrate through time.

    .. math::
        \\frac{\\partial\\phi}{\\partial t}  + {\\bf u } \\cdot \\nabla \\phi= \\nabla { ( k  \\nabla \\phi ) } + H


    Parameters
    ----------
    phiField : underworld.mesh.MeshVariable
        The concentration field, typically the temperature field
    phiDotField : underworld.mesh.MeshVariable
        A MeshVariable that defines the initial time derivative of the phiField.
        Typically 0 at the beginning of a model, e.g. phiDotField.data[:]=0
        When using a phiField loaded from disk one should also load the phiDotField to ensure
        the solving method has the time derivative information for a smooth restart.
        No dirichlet conditions are required for this field as the phiField degrees of freedom
        map exactly to this field's dirichlet conditions, the value of which ought to be 0
        for constant values of phi.
    velocityField : underworld.mesh.MeshVariable
        The velocity field.
    fn_diffusivity : underworld.function.Function
        A function that defines the diffusivity within the domain.
    fn_sourceTerm : underworld.function.Function
        A function that defines the heating within the domain. Optional.
    conditions : underworld.conditions.SystemCondition
        Numerical conditions to impose on the system. This should be supplied as
        the condition itself, or a list object containing the conditions.
        Notes
    -----
    Constructor must be called by collectively all processes.

    """
    _objectsDict = {  "_system": "AdvectionDiffusionSLE",
                      "_solver": "AdvDiffMulticorrector" }
    _selfObjectName = "_system"

    def __init__(self, phiField, phiDotField, velocityField, fn_diffusivity, fn_sourceTerm=None, conditions=[]):

        self._diffusivity   = fn_diffusivity
        self._source        = fn_sourceTerm

        if not isinstance( phiField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'phiField' must be of 'MeshVariable' class." )
        if phiField.data.shape[1] != 1:
            raise TypeError( "Provided 'phiField' must be a scalar" )
        self._phiField = phiField
        if not isinstance( phiDotField, (uw.mesh.MeshVariable, type(None))):
            raise TypeError( "Provided 'phiDotField' must be 'None' or of 'MeshVariable' class." )
        if self._phiField.data.shape != phiDotField.data.shape:
            raise TypeError( "Provided 'phiDotField' is not the same shape as the provided 'phiField'" )
        self._phiDotField = phiDotField

        # check compatibility of phiField and velocityField
        if velocityField.data.shape[1] != self._phiField.mesh.dim:
            raise TypeError( "Provided 'velocityField' must be the same dimensionality as the phiField's mesh" )
        self._velocityField = velocityField

        if not isinstance(conditions, (list, tuple)):
            conditionslist = []
            conditionslist.append(conditions)
            conditions = conditionslist

        # check input 'conditions' list is valid
        nbc = None
        for cond in conditions:
            if not isinstance( cond, uw.conditions.SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list 'SystemCondition' objects." )
            # set the bcs on here
            if type(cond) == uw.conditions.NeumannCondition:
                if nbc != None:
                    # check only one nbc condition is given in 'conditions' list
                    RuntimeError( "Provided 'conditions' can only accept one NeumannConditions condition object.")
            elif type(cond) == uw.conditions.DirichletCondition:
                if cond.variable == self._phiField:
                    libUnderworld.StgFEM.FeVariable_SetBC( self._phiField._cself, cond._cself )
                    libUnderworld.StgFEM.FeVariable_SetBC( self._phiDotField._cself, cond._cself )
            else:
                raise RuntimeError("Input condition type not recognised.")
        self._conditions = conditions

        # force removal of BCs as SUPG cannot handle leaving them in
        self._eqNumPhi    = sle.EqNumber( phiField, removeBCs=True )
        self._eqNumPhiDot = sle.EqNumber( phiDotField, removeBCs=True )

        self._phiSolution    = sle.SolutionVector( phiField, self._eqNumPhi )
        self._phiDotSolution = sle.SolutionVector( phiDotField, self._eqNumPhiDot )

        # create force vectors
        self._residualVector = sle.AssembledVector(phiField, self._eqNumPhi )
        self._massVector     = sle.AssembledVector(phiField, self._eqNumPhi )

        # create swarm
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(self._phiField.mesh)

        super(_SUPG_AdvectionDiffusion, self).__init__()

        self._cself.phiVector = self._phiSolution._cself
        self._cself.phiDotVector = self._phiDotSolution._cself

    def _add_to_stg_dict(self, componentDictionary):
        # call parents method
        super(_SUPG_AdvectionDiffusion, self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][   "SLE_Solver"] = self._solver.name
        componentDictionary[ self._cself.name ][     "PhiField"] = self._phiField._cself.name
        componentDictionary[ self._cself.name ][     "Residual"] = self._residualVector._cself.name
        componentDictionary[ self._cself.name ][   "MassMatrix"] = self._massVector._cself.name
        componentDictionary[ self._cself.name ][  "PhiDotField"] = self._phiDotField._cself.name
        componentDictionary[ self._cself.name ][          "dim"] = self._phiField.mesh.dim
        componentDictionary[ self._cself.name ]["courantFactor"] = 0.50

    def _setup(self):
        # create assembly terms here.
        # in particular, the residualTerm requires and tries to build _system, so if created in __init__
        # this causes a conflict.
        self._lumpedMassTerm = sle.LumpedMassMatrixVectorTerm( integrationSwarm = self._gaussSwarm,
                                                               assembledObject = self._massVector  )
        self._residualTerm   = sle.AdvDiffResidualVectorTerm(     velocityField = self._velocityField,
                                                                  diffusivity = self._diffusivity,
                                                                  sourceTerm = self._source,
                                                                  integrationSwarm = self._gaussSwarm,
                                                                  assembledObject = self._residualVector,
                                                                  extraInfo = self._cself.name )
        for cond in self._conditions:
            if isinstance( cond, uw.conditions.NeumannCondition ):

                # -VE flux because of the FEM discretisation method of the initial equation
                negativeCond = uw.conditions.NeumannCondition( fn_flux=cond.fn_flux,
                                                               variable=cond.variable,
                                                               indexSetsPerDof=cond.indexSetsPerDof )

                # NOTE many NeumannConditions can be used but the _sufaceFluxTerm only records the last
                self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                    assembledObject  = self._residualVector,
                    surfaceGaussPoints = 2,
                    nbc         = negativeCond )

        self._cself.advDiffResidualForceTerm = self._residualTerm._cself
    
    def integrate(self, dt, **kwargs):
        """
        Integrates the advection diffusion system through time, dt
        Must be called collectively by all processes.

        Parameters
        ----------
        dt : float
            The timestep interval to use

        """
        self._cself.currentDt = dt
        libUnderworld.Underworld._AdvectionDiffusionSLE_Execute( self._cself, None )

    def get_max_dt(self):
        """
        Returns a numerically stable timestep size for the current system.
        Note that as a default, this method returns a value one half the
        size of the Courant timestep.

        Returns
        -------
        float
            The timestep size.
        """
        return libUnderworld.Underworld.AdvectionDiffusionSLE_CalculateDt( self._cself, None )
