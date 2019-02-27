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
import libUnderworld

class TimeIntegration(_stgermain.StgCompoundComponent):
    """
    Abstract class for integrating numerical objects (fields, swarms, etc.) in time.

    The integration algorithm is a modified Runge Kutta method that only evaluates
    midpoint information varying in space - using only the present timestep solution.
    The order of the integration used can be 1,2,4

    Parameters
    ----------
    order: int {1,2,4}
        Defines the numerical order 'in space' of the Runge Kutta like integration scheme.

    """
    _objectsDict = {     "_system" : "TimeIntegrator",
                      "_integrand" : None }
    _selfObjectName = "_system"

    def __init__(self, order, **kwargs):
        if not isinstance( order, int):
            raise ValueError( "Provided 'order' must be of 'int' class." )
        self._order = order

        if not self._order in [1,2,4]:
            raise ValueError( "Provided 'order' must take value 1, 2 or 4." )

        super(TimeIntegration, self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        super(TimeIntegration,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[     self._cself.name ][         "order"] = self._order
        # set self as the integrand's integrator
        componentDictionary[ self._integrand.name ]["TimeIntegrator"] = self._cself.name

    def integrate(self,dt):
        self.dt = dt
        libUnderworld.StgDomain._TimeIntegrator_Execute(self._cself, None)

    def get_max_dt(self):
        # child should override
        raise RuntimeError("Child class should override this method. Please contact developers.")

    @property
    def dt(self):
        """
        Time integrator timestep size.
        """
        return self._cself.dt
    @dt.setter
    def dt(self, value):
        self._cself.dt = value

    @property
    def time(self):
        """
        Time integrator time value.
        """
        return self._cself.time
    @time.setter
    def time(self, value):
        self._cself.time = value


class SwarmAdvector(TimeIntegration):
    """
    Objects of this class advect a swarm through time using
    the provided velocity field.

    Parameters
    ----------
    velocityField : underworld.mesh.MeshVariable
        The MeshVariable field used for evaluating the velocity field that advects the swarm particles

    swarm : underworld.swarm.Swarm
        Particle swarm that will be advected by the given velocity field

    """
    _objectsDict = { "_integrand" : "SwarmAdvector" }

    def __init__(self, velocityField, swarm, order=2, **kwargs):
        if not isinstance( velocityField, uw.mesh.MeshVariable):
            raise ValueError( "Provided 'velocityField' must be of 'MeshVariable' class." )
        self._velocityField = velocityField

        if swarm and not isinstance(swarm, uw.swarm.Swarm):
            raise ValueError( "Provided 'swarm' must be of 'Swarm' class." )
        self._swarm = swarm

        super(SwarmAdvector, self).__init__(order=order, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        super(SwarmAdvector,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._integrand.name ]["VelocityField"] = self._velocityField._cself.name
        componentDictionary[ self._integrand.name ][        "Swarm"] = self._swarm._cself.name
        componentDictionary[ self._integrand.name ]["allowFallbackToFirstOrder"] = True

    def get_max_dt(self):
        return libUnderworld.PICellerator.SwarmAdvector_MaxDt(self._integrand)

    def integrate(self, dt, update_owners=True):
        """
        Integrate the associated swarm in time, by dt, using the velocityfield that is associated with this class

        Parameters
        ----------
        dt: double
            The timestep to use in the intergration
        update_owners: bool
            If this is set to False, particle ownership (which element owns a
            particular particle) is not updated after advection. This is often
            necessary when both the mesh and particles are advecting
            simultaneously.

        Example
        -------

        >>> import underworld as uw
        >>> import numpy as np
        >>> from underworld import function as fn
        >>> dim=2;
        >>> elementMesh = uw.mesh.FeMesh_Cartesian(elementType="Q1/dQ0", elementRes=(9,9), minCoord=(-1.,-1.), maxCoord=(1.,1.))
        >>> velocityField = uw.mesh.MeshVariable( mesh=elementMesh, nodeDofCount=dim )
        >>> swarm = uw.swarm.Swarm(mesh=elementMesh)
        >>> particle = np.zeros((1,2))
        >>> particle[0] = [0.2,-0.2]
        >>> swarm.add_particles_with_coordinates(particle)
        array([0], dtype=int32)
        >>> velocityField.data[:]=[1.0,1.0]
        >>> swarmAdvector = uw.systems.SwarmAdvector(velocityField=velocityField, swarm=swarm, order=2)
        >>> dt=swarmAdvector.get_max_dt()
        >>> swarmAdvector.integrate(dt)
        >>> np.allclose(swarm.particleCoordinates.data[0], [ 0.27856742, -0.12143258], rtol=1e-4)
        True

        """
        libUnderworld.StgFEM._FeVariable_SyncShadowValues( self._integrand.velocityField )
        super(SwarmAdvector,self).integrate(dt)
        # this check isn't necessary, but good. possibly get rid.
        libUnderworld.StgDomain.Swarm_CheckCoordsAreFinite( self._integrand.swarm );
        # Move particles across processors because they've just been advected
        
        if update_owners:
            self._swarm.update_particle_owners()
