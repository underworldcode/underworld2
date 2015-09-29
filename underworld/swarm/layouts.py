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
import _swarm

class _ParticleLayout(_stgermain.StgCompoundComponent):
    """
    Abstract class. Children classes are responsible for populating 
    swarms with particles, generally across the entire domain.
    """
    _objectsDict = {  "_layout": None }
    _selfObjectName = "_layout"

    def __init__(self, swarm, **kwargs ):
        """
        Parameters
        ----------
        swarm : uw.swarm.Swarm
            The swarm this layout will act upon
        
        """

        if not isinstance(swarm, _swarm.Swarm):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = swarm

        # build parent
        super(_ParticleLayout,self).__init__(**kwargs)

    @property
    def swarm(self):
        """    
        swarm (Swarm): swarm this layout will act to fill with particlces.
        """
        return self._swarm
    


class PerCellGaussLayout(_ParticleLayout):
    """
    This layout populates the domain with particles located at gauss locations 
    within each element of the swarm's associated finite element mesh.
    
    """
    _objectsDict = {  "_layout": "IrregularMeshGaussLayout" }

    def __init__(self, swarm, gaussPointCount, **kwargs ):
        """
        Parameters
        ----------
        swarm : uw.swarm.Swarm
            The swarm this layout will act upon
        gaussPointCount : int
            Per cell, the number of gauss points in each dimensional direction.
            Must take an int value between 1 and 5 inclusive.
        
        >>> import underworld as uw
        >>> # choose mesh to coincide with global element
        >>> mesh = uw.mesh.FeMesh_Cartesian()
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

        if not isinstance(gaussPointCount, int):
            raise TypeError("'gaussPointCount' object passed in must be of type 'int'")
        if gaussPointCount not in [1,2,3,4,5]:
            raise ValueError("'gaussPointCount' object passed in must take a value between \n between 1 and 5 inclusive.")
        self._gaussPointCount = gaussPointCount

        # build parent
        super(PerCellGaussLayout,self).__init__(swarm=swarm, **kwargs)

    @property
    def gaussPointCount(self):
        """    
        gaussPointCount (int): Number of gauss points in each direction.
        """
        return self._gaussPointCount


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(PerCellGaussLayout,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._layout.name ][           "dim"] = self.swarm.feMesh.dim
        componentDictionary[ self._layout.name ][        "FeMesh"] = self.swarm.feMesh._cself.name
        componentDictionary[ self._layout.name ]["gaussParticles"] = self.gaussPointCount


class GlobalSpaceFillerLayout(_ParticleLayout):
    """
    This layout fills the domain with particles in a quasi-random pattern. It utilises
    sobol sequences to generate global particle locations which are more uniform than that
    achieved by a purely random generator. This layout is mostly useful where populating 
    particles across a rectangular domain.
    
    """
    _objectsDict = {  "_layout": "SpaceFillerParticleLayout" }

    def __init__(self, swarm, particlesPerCell, **kwargs ):
        """
        Parameters
        ----------
        swarm : uw.swarm.Swarm
            The swarm this layout will act upon
        particlesPerCell : float
            The average number of particles per element that this layout will generate.
        
        >>> import underworld as uw
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> layout = uw.swarm.layouts.GlobalSpaceFillerLayout(swarm,particlesPerCell=4)
        >>> uw.libUnderworld.StgDomain._ZeroSobolGeneratorGeneratorCount()  # this step isn't necessary, but we include it to ensure deterministic results for this doctest
        >>> swarm.populate_using_layout(layout)
        >>> swarm.particleLocalCount
        4
        >>> swarm.particleCoordinates.data
        array([[ 0.5  ,  0.5  ],
               [ 0.25 ,  0.75 ],
               [ 0.75 ,  0.25 ],
               [ 0.375,  0.625]])

        """


        if not isinstance(particlesPerCell, (int,float)):
            raise TypeError("'particlesPerCell' object passed in must be of type 'float' or 'int'.")
        if particlesPerCell<=0:
            raise ValueError("'particlesPerCell' object passed in must take a value greater than zero.")
        self._particlesPerCell = float(particlesPerCell)

        # build parent
        super(GlobalSpaceFillerLayout,self).__init__(swarm=swarm, **kwargs)

    @property
    def particlesPerCell(self):
        """    
        particlesPerCell (float): The average number of particles per cell.
        """
        return self._particlesPerCell


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(GlobalSpaceFillerLayout,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._layout.name ][                           "dim"] = self.swarm.feMesh.dim
        componentDictionary[ self._layout.name ]["averageInitialParticlesPerCell"] = self.particlesPerCell


class _PerCellMeshParticleLayout(_ParticleLayout):
    """
    This layout fills the domain with particles on a per cell basis. It should not
    be directly invoked, with instead one of its child classes being used.
    
    """
    _objectsDict = {  "_layout": "MeshParticleLayout" }

    def __init__(self, swarm, particlesPerCell, **kwargs ):
        """
        Parameters
        ----------
        swarm : uw.swarm.Swarm
            The swarm this layout will act upon
        particlesPerCell : int
            The number of particles per element that this layout will generate.
        
        """

        if not isinstance(particlesPerCell, int):
            raise TypeError("'particlesPerCell' object passed in must be of type 'int'.")
        if particlesPerCell<1:
            raise ValueError("'particlesPerCell' object passed in must take a value greater than zero.")
        self._particlesPerCell = particlesPerCell

        # build parent
        super(_PerCellMeshParticleLayout,self).__init__(swarm=swarm, **kwargs)

    @property
    def particlesPerCell(self):
        """    
        particlesPerCell (float): The number of particles per cell.
        """
        return self._particlesPerCell


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(_PerCellMeshParticleLayout,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._layout.name ]["cellParticleCount"] = self.particlesPerCell
        componentDictionary[ self._layout.name ][           "FeMesh"] = self.swarm.feMesh._cself.name
        componentDictionary[ self._layout.name ][         "filltype"] = self._filltype



class PerCellSpaceFillerLayout(_PerCellMeshParticleLayout):
    """
    This layout fills the domain with particles in a quasi-random pattern. It utilises
    sobol sequences to generate per element particle locations which are more uniform than that
    achieved by a purely random generator.
    
    """

    def __init__(self, swarm, particlesPerCell, **kwargs ):
        """
        Parameters
        ----------
        swarm : uw.swarm.Swarm
            The swarm this layout will act upon
        particlesPerCell : int
            The number of particles per element that this layout will generate.
        
        >>> import underworld as uw
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> layout = uw.swarm.layouts.PerCellSpaceFillerLayout(swarm,particlesPerCell=4)
        >>> uw.libUnderworld.StgDomain._ZeroSobolGeneratorGeneratorCount()  # this isn't usually necessary, but we include it to ensure deterministic results for this doctest
        >>> swarm.populate_using_layout(layout)
        >>> swarm.particleLocalCount
        4
        >>> swarm.particleCoordinates.data
        array([[ 0.5  ,  0.5  ],
               [ 0.25 ,  0.75 ],
               [ 0.75 ,  0.25 ],
               [ 0.375,  0.625]])


        """

        self._filltype = 0  # this sets sobol

        # build parent
        super(PerCellSpaceFillerLayout,self).__init__(swarm=swarm, particlesPerCell=particlesPerCell, **kwargs)


class PerCellRandomLayout(_PerCellMeshParticleLayout):
    """
    This layout fills the domain with particles in a random (per element) pattern.
    """

    def __init__(self, swarm, particlesPerCell, seed=13, **kwargs ):
        """
        Parameters
        ----------
        swarm : uw.swarm.Swarm
            The swarm this layout will act upon
        particlesPerCell : int
            The number of particles per element that this layout will generate.
        seed : int
            Seed for random generator. Default is 13.
        

        >>> import underworld as uw
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> layout = uw.swarm.layouts.PerCellRandomLayout(swarm,particlesPerCell=4)
        >>> uw.libUnderworld.StgDomain._ZeroSobolGeneratorGeneratorCount()  # this isn't usually necessary, but we include it to ensure deterministic results for this doctest
        >>> swarm.populate_using_layout(layout)
        >>> swarm.particleLocalCount
        4
        >>> swarm.particleCoordinates.data
        array([[ 0.24261743,  0.67115852],
               [ 0.16116546,  0.70790335],
               [ 0.73160516,  0.08792286],
               [ 0.71953113,  0.15966135]])
               
        """

        self._filltype = 1  # this sets random
        
        self._seed = seed

        # build parent
        super(PerCellRandomLayout,self).__init__(swarm=swarm, particlesPerCell=particlesPerCell, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(PerCellRandomLayout,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._layout.name ]["seed"] = self._seed

