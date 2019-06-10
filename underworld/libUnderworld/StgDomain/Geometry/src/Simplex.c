/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "Geometry.h"

#include "Simplex.h"

Bool Simplex_Search3D( Mesh* mesh, unsigned* inc,
		       unsigned nSimplices, unsigned** inds, 
		       double* point, double* bc, unsigned* inside )
{
	unsigned	s_i;

	assert( inds );

	for( s_i = 0; s_i < nSimplices; s_i++ ) {
		unsigned	ind_i;

		Simplex_Barycenter3D( mesh, inc, inds[s_i], point, bc );
		for( ind_i = 0; ind_i < 4; ind_i++ ) {
			if( bc[ind_i] < 0.0 || bc[ind_i] > 1.0 )
				break;
		}
		if( ind_i == 4 ) {
			*inside = s_i;
			return True;
		}
	}

	return False;
}

Bool Simplex_Search2D( Mesh* mesh, unsigned* inc,
		       unsigned nSimplices, unsigned** inds, 
		       double* point, double* bc, unsigned* inside )
{
	unsigned	s_i;

	assert( inds );

	for( s_i = 0; s_i < nSimplices; s_i++ ) {
		unsigned	ind_i;

		Simplex_Barycenter2D( mesh, inc, inds[s_i], point, bc );
		for( ind_i = 0; ind_i < 3; ind_i++ ) {
			if( bc[ind_i] < 0.0 || bc[ind_i] > 1.0 )
				break;
		}
		if( ind_i == 3 ) {
			*inside = s_i;
			return True;
		}
	}

	return False;
}

void Simplex_Barycenter3D( Mesh* mesh, unsigned* inc, unsigned* inds, double* point, double* bc ) {
	double*	tet[4];
	double	x0, y0, z0;
	double	x1, y1, z1;
	double	x2, y2, z2;
	double	x3, y3, z3;
	double	px, py, pz;
	double	den;

	assert( mesh );
	assert( inc );
	assert( inds );
	assert( point );
	assert( bc );
	assert( Mesh_GetVertex( mesh, inc[inds[0]]) );
	assert( Mesh_GetVertex( mesh, inc[inds[1]]) );
	assert( Mesh_GetVertex( mesh, inc[inds[2]]) );
	assert( Mesh_GetVertex( mesh, inc[inds[3]]) );

	tet[0] = Mesh_GetVertex( mesh, inc[inds[0]]);
	tet[1] = Mesh_GetVertex( mesh, inc[inds[1]]);
	tet[2] = Mesh_GetVertex( mesh, inc[inds[2]]);
	tet[3] = Mesh_GetVertex( mesh, inc[inds[3]]);
	x0 = tet[0][0]; x1 = tet[1][0]; x2 = tet[2][0]; x3 = tet[3][0];
	y0 = tet[0][1]; y1 = tet[1][1]; y2 = tet[2][1]; y3 = tet[3][1];
	z0 = tet[0][2]; z1 = tet[1][2]; z2 = tet[2][2]; z3 = tet[3][2];
	px = point[0]; py = point[1]; pz = point[2];
	den = 1.0 / (x1*(y0*(z3 - z2) + y2*(z0 - z3) + y3*(z2 - z0)) + 
			     x0*(y2*(z3 - z1) + y1*(z2 - z3) + y3*(z1 - z2)) + 
			     x2*(y1*(z3 - z0) + y0*(z1 - z3) + y3*(z0 - z1)) + 
			     x3*(y0*(z2 - z1) + y1*(z0 - z2) + y2*(z1 - z0)));

	bc[1] = -(x0*(py*(z3 - z2) + y2*(pz - z3) + y3*(z2 - pz)) + 
		   px*(y2*(z3 - z0) + y0*(z2 - z3) + y3*(z0 - z2)) + 
		   x2*(y0*(z3 - pz) + py*(z0 - z3) + y3*(pz - z0)) + 
		   x3*(py*(z2 - z0) + y0*(pz - z2) + y2*(z0 - pz))) * den;
	if( Num_Approx( bc[1], 0.0 ) ) bc[1] = 0.0;
	else if( Num_Approx( bc[1], 1.0 ) ) bc[1] = 1.0;

	bc[2] = (x0*(py*(z3 - z1) + y1*(pz - z3) + y3*(z1 - pz)) + 
		  px*(y1*(z3 - z0) + y0*(z1 - z3) + y3*(z0 - z1)) + 
		  x1*(y0*(z3 - pz) + py*(z0 - z3) + y3*(pz - z0)) + 
		  x3*(py*(z1 - z0) + y0*(pz - z1) + y1*(z0 - pz))) * den;
	if( Num_Approx( bc[2], 0.0 ) ) bc[2] = 0.0;
	else if( Num_Approx( bc[2], 1.0 ) ) bc[2] = 1.0;

	bc[3] = -(x0*(py*(z2 - z1) + y1*(pz - z2) + y2*(z1 - pz)) + 
		   px*(y1*(z2 - z0) + y0*(z1 - z2) + y2*(z0 - z1)) + 
		   x1*(y0*(z2 - pz) + py*(z0 - z2) + y2*(pz - z0)) + 
		   x2*(py*(z1 - z0) + y0*(pz - z1) + y1*(z0 - pz))) * den;
	if( Num_Approx( bc[3], 0.0 ) ) bc[3] = 0.0;
	else if( Num_Approx( bc[3], 1.0 ) ) bc[3] = 1.0;

	bc[0] = 1.0 - bc[1] - bc[2] - bc[3];
	if( Num_Approx( bc[0], 0.0 ) ) bc[0] = 0.0;
	else if( Num_Approx( bc[0], 1.0 ) ) bc[0] = 1.0;
}

void Simplex_Barycenter2D( Mesh* mesh, unsigned* inc, unsigned* inds, double* point, double* bc ) {
	double*	tri[3];
	double	a;
	double	b;
	double	c;
	double	d;
	double	e;
	double	f;

	assert( mesh );
	assert( inc );
	assert( inds );
	assert( point );
	assert( bc );
	assert( Mesh_GetVertex( mesh, inc[inds[0]]) );
	assert( Mesh_GetVertex( mesh, inc[inds[1]]) );
	assert( Mesh_GetVertex( mesh, inc[inds[2]]) );

	tri[0] = Mesh_GetVertex( mesh, inc[inds[0]]);
	tri[1] = Mesh_GetVertex( mesh, inc[inds[1]]);
	tri[2] = Mesh_GetVertex( mesh, inc[inds[2]]);

	a = tri[0][0] - tri[2][0];
	b = tri[1][0] - tri[2][0];
	c = tri[2][0] - point[0];
	d = tri[0][1] - tri[2][1];
	e = tri[1][1] - tri[2][1];
	f = tri[2][1] - point[1];

	bc[0] = (b * f - c * e) / (a * e - b * d);
	if( Num_Approx( bc[0], 0.0 ) ) bc[0] = 0.0;
	else if( Num_Approx( bc[0], 1.0 ) ) bc[0] = 1.0;
	bc[1] = (a * f - c * d) / (b * d - a * e);
	if( Num_Approx( bc[1], 0.0 ) ) bc[1] = 0.0;
	else if( Num_Approx( bc[1], 1.0 ) ) bc[1] = 1.0;
	bc[2] = 1.0 - bc[0] - bc[1];
	if( Num_Approx( bc[2], 0.0 ) ) bc[2] = 0.0;
	else if( Num_Approx( bc[2], 1.0 ) ) bc[2] = 1.0;
}

double Simplex_Volume( Mesh* mesh, unsigned* inc, unsigned* inds ) {
	const double	fac = 1.0 / 6.0;
	double			da[3], db[3], dc[3];

	assert( mesh );
	assert( inc );
	assert( inds );

	Vec_Sub3D( da, Mesh_GetVertex( mesh, inc[inds[2]] ), Mesh_GetVertex( mesh, inc[inds[0]] ) );
	Vec_Sub3D( db, Mesh_GetVertex( mesh, inc[inds[2]] ), Mesh_GetVertex( mesh, inc[inds[1]] ) );
	Vec_Sub3D( dc, Mesh_GetVertex( mesh, inc[inds[2]] ), Mesh_GetVertex( mesh, inc[inds[2]] ) );
	Vec_Cross3D( db, db, dc );

	return fac * fabs( Vec_Dot3D( da, db ) );
}

double Simplex_Area( Mesh* mesh, unsigned* inc, unsigned* inds ) {
	assert( mesh );
	assert( inc );
	assert( inds );
	double*	verta = Mesh_GetVertex( mesh, inc[inds[0]] );
	double*	vertb = Mesh_GetVertex( mesh, inc[inds[1]] );
	double*	vertc = Mesh_GetVertex( mesh, inc[inds[2]] );


	return 0.5 * fabs( verta[0] * (vertc[1] - vertb[1]) +
			   vertb[0] * (verta[1] - vertc[1]) +
			   vertc[0] * (vertb[1] - verta[1]) );
}


