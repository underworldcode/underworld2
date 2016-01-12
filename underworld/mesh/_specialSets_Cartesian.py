##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module contains functions which generate special IndexSet objects for a Cartesian mesh.

The mesh object must be cartesian, with its size described by the tuple (i,j,k) for a 3d
mesh taking i,j,k vertices in each direction.  Note that this should not be confused with the
element count used to generate the mesh, with the vertex count depending on the order of the
mesh.


>>> import underworld as uw
>>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(2,2), minCoord=(0.,0.), maxCoord=(1.,1.) )
>>> uw.mesh._specialSets_Cartesian.MinI_VertexSet( linearMesh )
FeMesh_IndexSet([0, 3, 6])

"""

import underworld as _uw
import libUnderworld as _libUnderworld
from functools import wraps as _wraps

def _meshCheck(func):
    """
    This is a simple decorator function to check that the correct mesh is being passed in.
    """
    @_wraps(func)  # this nested decorator imports the docstring from the actual function
    def _checkmesh(mesh):
        if mesh is None:
            raise TypeError( "Mesh provided is of type 'None'. A valid mesh must be provided." )
        if not isinstance( mesh, _uw.mesh.FeMesh_Cartesian ):
            raise TypeError( "The provided mesh must be of FeMesh_Cartesian class.")
        return func(mesh)
    return _checkmesh

@_meshCheck
def MaxI_VertexSet( mesh ):
    """
    Returns the set of local indices which are within the global 3d index set (MaxI,*,*).
    Here MaxI is the global mesh maximum index in the I direction.
    
    Parameter
    ---------
    mesh : FeMesh
        Mesh from which the IndexSet set is required.
    
    Returns
    -------
    set : IndexSet
        The MaxI IndexSet.

    """
    return _uw.mesh.FeMesh_IndexSet( object           = mesh,
                                     topologicalIndex = 0,
                                     size             = _libUnderworld.StgDomain.Mesh_GetDomainSize( mesh._mesh, _libUnderworld.StgDomain.MT_VERTEX ),
                                     fromObject       = _libUnderworld.StgDomain.RegularMeshUtils_CreateGlobalMaxISet( mesh._mesh ) )


@_meshCheck
def MinI_VertexSet( mesh ):
    """
    Returns the set of local indices which are within the global 3d index set (MinI,*,*).
    Here MinI is the global mesh minimum index in the I direction.
    
    Parameter
    ---------
    mesh : FeMesh
        Mesh from which the IndexSet set is required.
    
    Returns
    -------
    set : IndexSet
        The MinI IndexSet.

    """
    return _uw.mesh.FeMesh_IndexSet( object           = mesh,
                                     topologicalIndex = 0,
                                     size             = _libUnderworld.StgDomain.Mesh_GetDomainSize( mesh._mesh, _libUnderworld.StgDomain.MT_VERTEX ),
                                     fromObject       = _libUnderworld.StgDomain.RegularMeshUtils_CreateGlobalMinISet( mesh._mesh ) )

@_meshCheck
def MaxJ_VertexSet( mesh ):
    """
    Returns the set of local indices which are within the global 3d index set (*,MaxJ,*).
    Here MaxJ is the global mesh maximum index in the J direction.
    
    Parameter
    ---------
    mesh : FeMesh
        Mesh from which the IndexSet set is required.
    
    Returns
    -------
    set : IndexSet
        The MaxJ IndexSet.

    """
    return _uw.mesh.FeMesh_IndexSet( object           = mesh,
                                     topologicalIndex = 0,
                                     size             = _libUnderworld.StgDomain.Mesh_GetDomainSize( mesh._mesh, _libUnderworld.StgDomain.MT_VERTEX ),
                                     fromObject       = _libUnderworld.StgDomain.RegularMeshUtils_CreateGlobalMaxJSet( mesh._mesh ) )

@_meshCheck
def MinJ_VertexSet( mesh ):
    """
    Returns the set of local indices which are within the global 3d index set (*,MinJ,*).
    Here MinJ is the global mesh minimum index in the J direction.
    
    Parameter
    ---------
    mesh : FeMesh
        Mesh from which the IndexSet set is required.
    
    Returns
    -------
    set : IndexSet
        The MinJ IndexSet.

    """
    return _uw.mesh.FeMesh_IndexSet( object           = mesh,
                                     topologicalIndex = 0,
                                     size             = _libUnderworld.StgDomain.Mesh_GetDomainSize( mesh._mesh, _libUnderworld.StgDomain.MT_VERTEX ),
                                     fromObject       = _libUnderworld.StgDomain.RegularMeshUtils_CreateGlobalMinJSet( mesh._mesh ) )


@_meshCheck
def MaxK_VertexSet( mesh ):
    """
    Returns the set of local indices which are within the global 3d index set (*,*,MaxK).
    Here MaxK is the global mesh maximum index in the K direction.
    
    Parameter
    ---------
    mesh : FeMesh
        Mesh from which the IndexSet set is required.
    
    Returns
    -------
    set : IndexSet
        The MaxK IndexSet.

    """
    if mesh.dim < 3:
        raise ValueError( "Mesh provided must be 3-Dimensional to use this function.")
    
    return _uw.mesh.FeMesh_IndexSet( object           = mesh,
                                     topologicalIndex = 0,
                                     size             = _libUnderworld.StgDomain.Mesh_GetDomainSize( mesh._mesh, _libUnderworld.StgDomain.MT_VERTEX ),
                                     fromObject       = _libUnderworld.StgDomain.RegularMeshUtils_CreateGlobalMaxKSet( mesh._mesh ) )


@_meshCheck
def MinK_VertexSet( mesh ):
    """
    Returns the set of local indices which are within the global 3d index set (*,*,MinK).
    Here MinK is the global mesh minimum index in the K direction.
    
    Parameter
    ---------
    mesh : FeMesh
        Mesh from which the IndexSet set is required.
    
    Returns
    -------
    set : IndexSet
        The MinK IndexSet.

    """
    return _uw.mesh.FeMesh_IndexSet( object           = mesh,
                                     topologicalIndex = 0,
                                     size             = _libUnderworld.StgDomain.Mesh_GetDomainSize( mesh._mesh, _libUnderworld.StgDomain.MT_VERTEX ),
                                     fromObject       = _libUnderworld.StgDomain.RegularMeshUtils_CreateGlobalMinKSet( mesh._mesh ) )

@_meshCheck
def AllWalls( mesh ):
    """
    Returns the set of local indices which fall along the outer wall of the domain.
    
    Parameter
    ---------
    mesh : FeMesh
        Mesh from which the IndexSet set is required.
    
    Returns
    -------
    set : IndexSet
        The OuterWall IndexSet.

    """
    wallSet  = MaxI_VertexSet( mesh ) # we create the set here
    wallSet += MinI_VertexSet( mesh )
    wallSet += MaxJ_VertexSet( mesh )
    wallSet += MinJ_VertexSet( mesh )
    if mesh.dim == 3:
        wallSet += MaxK_VertexSet( mesh )
        wallSet += MinK_VertexSet( mesh )

    return wallSet
