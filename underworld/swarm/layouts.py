##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
'''
This module contains classes for populating swarms with particles across
the domain.
'''
import underworld._stgermain as _stgermain
from . import _swarm
import abc as _abc

class _ParticleLayoutAbstract(_stgermain.StgCompoundComponent, metaclass = _abc.ABCMeta):
    """
    Abstract class. Children classes are responsible for populating 
    swarms with particles, generally across the entire domain.

    Parameters
    ----------
    swarm : underworld.swarm.Swarm
        The swarm this layout will act upon
    
    """
    _objectsDict = {  "_layout": None }
    _selfObjectName = "_layout"

    def __init__(self, swarm, **kwargs ):


        if not isinstance(swarm, _swarm.Swarm):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = swarm

        # build parent
        super(_ParticleLayoutAbstract,self).__init__(**kwargs)

    @property
    def swarm(self):
        """
        Returns
        -------
        underworld.swarm.Swarm
            Swarm this layout will act to fill with particlces.
        """
        return self._swarm
    


class PerCellGaussLayout(_ParticleLayoutAbstract):
    """
    This layout populates the domain with particles located at gauss locations 
    within each element of the swarm's associated finite element mesh.

    Parameters
    ----------
    swarm : underworld.swarm.Swarm
        The swarm this layout will act upon
    gaussPointCount : int
        Per cell, the number of gauss points in each dimensional direction.
        Must take an int value between 1 and 5 inclusive.
    

    Example
    -------
    >>> import underworld as uw
    >>> # choose mesh to coincide with global element
    >>> mesh = uw.mesh.FeMesh_Cartesian('Q1/dQ0', (1,1), (-1.,-1.), (1.,1.))
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> layout = uw.swarm.layouts.PerCellGaussLayout(swarm,gaussPointCount=2)
    >>> swarm.populate_using_layout(layout)
    >>> swarm.particleLocalCount
    4
    >>> swarm.particleCoordinates.data
    array([[-0.57735027, -0.57735027],
           [ 0.57735027, -0.57735027],
           [-0.57735027,  0.57735027],
           [ 0.57735027,  0.57735027]])
    >>> import math
    >>> # lets check one of these gauss points
    >>> ( swarm.particleCoordinates.data[3][0] - math.sqrt(1./3.) ) < 1.e-10
    True

    """
    _objectsDict = {  "_layout": "IrregularMeshGaussLayout" }

    def __init__(self, swarm, gaussPointCount, **kwargs ):
        
        if not isinstance(gaussPointCount, int):
            raise TypeError("'gaussPointCount' object passed in must be of type 'int'")
        if gaussPointCount not in [1,2,3,4,5]:
            raise ValueError("'gaussPointCount' object passed in must take a value between \n between 1 and 5 inclusive.")
        self._gaussPointCount = gaussPointCount

        # build parent
        super(PerCellGaussLayout,self).__init__(swarm=swarm, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(PerCellGaussLayout,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._layout.name ][           "dim"] = self.swarm.mesh.dim
        componentDictionary[ self._layout.name ][        "FeMesh"] = self.swarm.mesh._cself.name
        componentDictionary[ self._layout.name ]["gaussParticles"] = self._gaussPointCount


class GlobalSpaceFillerLayout(_ParticleLayoutAbstract):
    """
    This layout fills the domain with particles in a quasi-random pattern. It utilises
    sobol sequences to generate global particle locations which are more uniform than that
    achieved by a purely random generator. This layout is mostly useful where populating 
    particles across a rectangular domain.

    Parameters
    ----------
    swarm : underworld.swarm.Swarm
        The swarm this layout will act upon
    particlesPerCell : float
        The average number of particles per element that this layout will generate.


    Example
    -------
    >>> import underworld as uw
    >>> mesh = uw.mesh.FeMesh_Cartesian('Q1/dQ0', (1,1), (0.,0.), (1.,1.))
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> layout = uw.swarm.layouts.GlobalSpaceFillerLayout(swarm,particlesPerCell=4)
    >>> swarm.populate_using_layout(layout)
    >>> swarm.particleLocalCount
    4
    >>> swarm.particleCoordinates.data
    array([[ 0.5  ,  0.5  ],
           [ 0.25 ,  0.75 ],
           [ 0.75 ,  0.25 ],
           [ 0.375,  0.625]])
    
    """
    _objectsDict = {  "_layout": "SpaceFillerParticleLayout" }

    def __init__(self, swarm, particlesPerCell, **kwargs ):
        import underworld as uw
        if uw.mpi.rank==0:
        # TODO: Deprecate
            import warnings
            warnings.warn("Note that the 'GlobalSpaceFillerLayout' will be deprecated in future releases of Underworld. "
                          "The `PerCellSpaceFillerLayout` provides similar functionality.")
        if not isinstance(particlesPerCell, (int,float)):
            raise TypeError("'particlesPerCell' object passed in must be of type 'float' or 'int'.")
        if particlesPerCell<=0:
            raise ValueError("'particlesPerCell' object passed in must take a value greater than zero.")
        self._particlesPerCell = float(particlesPerCell)

        # build parent
        super(GlobalSpaceFillerLayout,self).__init__(swarm=swarm, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(GlobalSpaceFillerLayout,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._layout.name ][                           "dim"] = self.swarm.mesh.dim
        componentDictionary[ self._layout.name ]["averageInitialParticlesPerCell"] = self._particlesPerCell


class _PerCellMeshParticleLayout(_ParticleLayoutAbstract):
    """
    This layout fills the domain with particles on a per cell basis. It should not
    be directly invoked with instead one of its child classes being used.
    
    Parameters
    ----------
    swarm : underworld.swarm.Swarm
        The swarm this layout will act upon
    particlesPerCell : int
        The number of particles per element/cell that this layout will generate.

    """
    _objectsDict = {  "_layout": "MeshParticleLayout" }

    def __init__(self, swarm, particlesPerCell, **kwargs ):
        if not isinstance(particlesPerCell, int):
            raise TypeError("'particlesPerCell' object passed in must be of type 'int'.")
        if particlesPerCell<1:
            raise ValueError("'particlesPerCell' object passed in must take a value greater than zero.")
        self._particlesPerCell = particlesPerCell

        # build parent
        super(_PerCellMeshParticleLayout,self).__init__(swarm=swarm, **kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(_PerCellMeshParticleLayout,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._layout.name ]["cellParticleCount"] = self._particlesPerCell
        componentDictionary[ self._layout.name ][           "FeMesh"] = self.swarm.mesh._cself.name
        componentDictionary[ self._layout.name ][         "filltype"] = self._filltype



class PerCellSpaceFillerLayout(_PerCellMeshParticleLayout):
    """
    This layout fills the domain with particles in a quasi-random pattern. It utilises
    sobol sequences to generate per element particle locations which are more uniform than that
    achieved by a purely random generator.

    Parameters
    ----------
    swarm : underworld.swarm.Swarm
        The swarm this layout will act upon
    particlesPerCell : int
        The number of particles per element that this layout will generate.
    
    
    Example
    -------
    >>> import underworld as uw
    >>> mesh = uw.mesh.FeMesh_Cartesian('Q1/dQ0', (1,1), (0.,0.), (1.,1.))
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> layout = uw.swarm.layouts.PerCellSpaceFillerLayout(swarm,particlesPerCell=4)
    >>> swarm.populate_using_layout(layout)
    >>> swarm.particleLocalCount
    4
    >>> swarm.particleCoordinates.data
    array([[ 0.5  ,  0.5  ],
           [ 0.25 ,  0.75 ],
           [ 0.75 ,  0.25 ],
           [ 0.375,  0.625]])


    """

    def __init__(self, swarm, particlesPerCell, **kwargs ):
        
        self._filltype = 0  # this sets sobol

        # build parent
        super(PerCellSpaceFillerLayout,self).__init__(swarm=swarm, particlesPerCell=particlesPerCell, **kwargs)


class PerCellRandomLayout(_PerCellMeshParticleLayout):
    """
    This layout fills the domain with particles in a random (per element) pattern.

    Parameters
    ----------
    swarm : underworld.swarm.Swarm
        The swarm this layout will act upon
    particlesPerCell : int
        The number of particles per element that this layout will generate.
    seed : int
        Seed for random generator. Default is 13.


    Example
    -------
    >>> import underworld as uw
    >>> mesh = uw.mesh.FeMesh_Cartesian('Q1/dQ0', (1,1), (0.,0.), (1.,1.))
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> layout = uw.swarm.layouts.PerCellRandomLayout(swarm,particlesPerCell=4)
    >>> swarm.populate_using_layout(layout)
    >>> swarm.particleLocalCount
    4
    >>> swarm.particleCoordinates.data
    array([[ 0.24261743,  0.67115852],
           [ 0.16116546,  0.70790335],
           [ 0.73160516,  0.08792286],
           [ 0.71953113,  0.15966135]])
           
    """

    def __init__(self, swarm, particlesPerCell, seed=13, **kwargs ):
        
        self._filltype = 1  # this sets random
        
        self._seed = seed

        # build parent
        super(PerCellRandomLayout,self).__init__(swarm=swarm, particlesPerCell=particlesPerCell, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(PerCellRandomLayout,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._layout.name ]["seed"] = self._seed

