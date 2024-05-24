/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Geometry_Simplex_h__
#define __StgDomain_Geometry_Simplex_h__

#include <StgDomain/Mesh/src/Mesh.h>

Bool Simplex_Search3D( Mesh* mesh, unsigned* inc,
               unsigned nSimplices, unsigned** inds, 
               double* point, double* bc, unsigned* inside );
Bool Simplex_Search2D( Mesh* mesh, unsigned* inc,
               unsigned nSimplices, unsigned** inds, 
               double* point, double* bc, unsigned* inside );

void Simplex_Barycenter3D( Mesh* mesh, unsigned* inc, unsigned* inds, double* point, double* bc );
void Simplex_Barycenter2D( Mesh* mesh, unsigned* inc, unsigned* inds, double* point, double* bc );

double Simplex_Volume( Mesh* mesh, unsigned* inc, unsigned* inds );
double Simplex_Area( Mesh* mesh, unsigned* inc, unsigned* inds );

#endif /* __StgDomain_Geometry_Simplex_h__ */
