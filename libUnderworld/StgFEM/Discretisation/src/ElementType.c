/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "types.h"

#include "FeMesh.h"
#include "ElementType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


const Type ElementType_Type = "ElementType";

ElementType* _ElementType_New(  ELEMENTTYPE_DEFARGS  ) {
	ElementType* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ElementType) );
	self = (ElementType*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	self->_build = _build;
	self->_evaluateShapeFunctionsAt = _evaluateShapeFunctionsAt;
	self->_evaluateShapeFunctionLocalDerivsAt = _evaluateShapeFunctionLocalDerivsAt;
	self->_convertGlobalCoordToElLocal = _convertGlobalCoordToElLocal;
	self->_jacobianDeterminantSurface = _jacobianDeterminantSurface;
	self->_surfaceNormal = _surfaceNormal;
	
	/* ElementType info */
	
	return self;
}

void _ElementType_Init( ElementType* self, Index nodeCount ) {
	/* General and Virtual info should already be set */
	self->dim = 0;
	/* ElementType info */
	self->nodeCount = nodeCount;
	self->debug = Stream_RegisterChild( StgFEM_Discretisation_Debug, ElementType_Type );
	self->inc = IArray_New();
}


void _ElementType_Destroy( void* elementType, void* data ){
	ElementType* self = (ElementType*)elementType;

	Stg_Class_Delete( self->inc );

	Stg_Component_Destroy( self, data, False );
}

void _ElementType_Delete( void* elementType ) {
	ElementType* self = (ElementType*)elementType;

	/* Stg_Class_Delete parent*/
	_Stg_Component_Delete( self );
}

void _ElementType_Print( void* elementType, Stream* stream ) {
	ElementType* self = (ElementType*)elementType;
	
	/* Set the Journal for printing informations */
	Stream* elementTypeStream = stream;
	
	/* General info */
	Journal_Printf( elementTypeStream, "ElementType (ptr): %p\n", self );
	
	/* Print parent */
	_Stg_Class_Print( self, elementTypeStream );
	
	/* Virtual info */
	Journal_Printf( elementTypeStream, "\t_build (func ptr): %p\n", self->_build );
	Journal_Printf( elementTypeStream, "\t_evaluateShapeFunctionsAt (func ptr): %p\n", self->_evaluateShapeFunctionsAt );
	Journal_Printf( elementTypeStream,  "\t_evaluateShapeFunctionLocalDerivsAt (func ptr): %p\n", self->_evaluateShapeFunctionLocalDerivsAt );
	
	/* ElementType info */
	Journal_Printf( elementTypeStream, "\tnodeCount: %u\n", self->nodeCount );
}

/* +++ Virtual Function Interfaces +++ */

void ElementType_Build( void* elementType, void *data ) {
	ElementType* self = (ElementType*)elementType;
	
	/* ElementType's are implemented NOT in the standard parent child
	 * manner as the rest of StGermain. Here the parents calls the child's
	 * build function */
	self->_build( self, data);
}

void ElementType_EvaluateShapeFunctionsAt( void* elementType, const double localCoord[], double* const evaluatedValues ) {
	ElementType* self = (ElementType*)elementType;
	
	self->_evaluateShapeFunctionsAt( self, localCoord, evaluatedValues );
}

void ElementType_EvaluateShapeFunctionLocalDerivsAt( void* elementType, const double localCoord[], double** const evaluatedDerivatives ) {
	ElementType* self = (ElementType*)elementType;
	
	self->_evaluateShapeFunctionLocalDerivsAt( self, localCoord, evaluatedDerivatives );
}

double _ElementType_JacobianDeterminantSurface( void* elementType, void* mesh, unsigned element_I, const double localCoord[], 
						unsigned face_I, unsigned norm ) 
{
	ElementType*	self;
	Stream*			error = Journal_Register( ErrorStream_Type, (Name)ElementType_Type );

	self = (ElementType* ) elementType;

	Journal_Printf( error, "Error: the jacobian for this element type cannot be evaluated on the element surface" );
	Journal_Printf( error, "(perhaps because the nodes are defined internally for the element).\n" );
	assert( 0 );

	return -1;
}

double ElementType_JacobianDeterminantSurface( void* elementType, void* mesh, unsigned element_I, 
						const double localCoord[], unsigned face_I, unsigned norm ) {
	ElementType* self = (ElementType*)elementType;

	return self->_jacobianDeterminantSurface( self, mesh, element_I, localCoord, face_I, norm );
}

#define EPS 1.0E-6

int _ElementType_SurfaceNormal( void* elementType, unsigned element_I, unsigned dim, double* xi, double* normal ) {
	ElementType* self;

	self = (ElementType*)elementType;

	memset( normal, 0, sizeof(double) * dim );

	if( xi[J_AXIS] < -1.0 + EPS ) {
		normal[J_AXIS] = -1.0;
		return 0;
	}
	else if( xi[J_AXIS] > +1.0 - EPS ) {
		normal[J_AXIS] = +1.0;
		return 1;
	}
	else if( xi[I_AXIS] < -1.0 + EPS ) {
		normal[I_AXIS] = -1.0;
		return 2;
	}
	else if( xi[I_AXIS] > +1.0 - EPS ) {
		normal[I_AXIS] = +1.0;
		return 3;
	}
	else if( xi[K_AXIS] < -1.0 + EPS ) {
		normal[K_AXIS] = -1.0;
		return 4;
	}
	else if( xi[K_AXIS] > +1.0 - EPS ) {
		normal[K_AXIS] = +1.0;
		return 5;
	}
	return 0;
}

int ElementType_SurfaceNormal( void* elementType, unsigned element_I, unsigned dim, double* xi, double* normal ) {
	ElementType* 	self = (ElementType*)elementType;

	return self->_surfaceNormal( self, element_I, dim, xi, normal );
}

void ElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord )
{
	ElementType*	self = (ElementType*)elementType;

	self->_convertGlobalCoordToElLocal( self, mesh, element, globalCoord, elLocalCoord );
}


/* +++ Virtual Function Implementations +++ */

void _ElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		_mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord )
{		
	ElementType*		self            = (ElementType*)elementType;
	Mesh*			mesh = (Mesh*)_mesh;
	TensorArray         jacobiMatrix;
	double              tolerance       = 0.0001; /* TODO put on class */
	double              maxResidual;
	Iteration_Index     maxIterations   = 100;    /*  TODO put on class */
	Iteration_Index     iteration_I;
	Node_Index          node_I;
	Node_Index          nodeCount       = self->nodeCount;
	double*             evaluatedShapeFuncs = self->evaluatedShapeFunc;
	XYZ                 rightHandSide;
	XYZ                 xiIncrement;
	double              shapeFunc;
	double*       	    nodeCoord;
	double**            GNi = self->GNi;
	unsigned	    nInc, *inc;
	Dimension_Index     dim             = Mesh_GetDimSize( mesh );

	/* This function uses a Newton-Raphson iterative method to find the local coordinate from the global coordinate 
	 * the equations are ( see FEM/BEM nodes p. 9 )
	 *
	 * x = \Sum_n( Ni( \xi, \eta, \zeta ) x_n )
	 * y = \Sum_n( Ni( \xi, \eta, \zeta ) y_n )
	 * z = \Sum_n( Ni( \xi, \eta, \zeta ) z_n )
	 *
	 * which are non-linear.
	 *
	 * This can be formulated into a system of linear equations of the form
	 *
	 * [              ][ \xi_{i + 1}   - \xi_i   ]   [ x - \Sum_n( Ni( \xi_i, \eta_i, \zeta_i ) x_n ]
	 * [   Jacobian   ][ \eta_{i + 1}  - \eta_i  ] = [ y - \Sum_n( Ni( \xi_i, \eta_i, \zeta_i ) y_n ]
	 * [              ][ \zeta_{i + 1} - \zeta_I ]   [ z - \Sum_n( Ni( \xi_i, \eta_i, \zeta_i ) z_n ]
	 *
	 * see http://en.wikipedia.org/wiki/Newton-Raphson_method
	 *
	 *
	 * This iterative method doesn't converge for all irregular element, e.g. kite element for projection
	 * lead to the solution to this equation having complex number. The iterative method is only valid for
	 * REAL numbers, thus the method doesn't converge.
	 *
	 * */

	Mesh_GetIncidence( mesh, Mesh_GetDimSize( mesh ), element, MT_VERTEX, self->inc );
	nInc = IArray_GetSize( self->inc );
	inc = IArray_GetPtr( self->inc );

	/* Initial guess for element local coordinate is in the centre of the element - ( 0.0, 0.0, 0.0 ) */
	memset( elLocalCoord, 0, dim*sizeof(double) );

	/* Do Newton-Raphson Iteration */
	for ( iteration_I = 0 ; iteration_I < maxIterations ; iteration_I++ ) {
		/* Initialise Values */
		TensorArray_Zero( jacobiMatrix );
		memset( rightHandSide, 0, sizeof( XYZ ) );

		/* Evaluate shape functions for rhs */
		ElementType_EvaluateShapeFunctionsAt( self, elLocalCoord, evaluatedShapeFuncs );
		self->_evaluateShapeFunctionLocalDerivsAt( self, elLocalCoord, GNi );


		for ( node_I = 0 ; node_I < nodeCount ; node_I++ ) {
			shapeFunc = evaluatedShapeFuncs[node_I];
			nodeCoord = Mesh_GetVertex( mesh, inc[node_I] );

			/* Form jacobi matrix */
			jacobiMatrix[ MAP_TENSOR( 0, 0, dim ) ] += GNi[0][node_I] * nodeCoord[ I_AXIS ];
			jacobiMatrix[ MAP_TENSOR( 0, 1, dim ) ] += GNi[1][node_I] * nodeCoord[ I_AXIS ];

			jacobiMatrix[ MAP_TENSOR( 1, 0, dim ) ] += GNi[0][node_I] * nodeCoord[ J_AXIS ];
			jacobiMatrix[ MAP_TENSOR( 1, 1, dim ) ] += GNi[1][node_I] * nodeCoord[ J_AXIS ];
			

			/* Form right hand side */
			rightHandSide[ I_AXIS ] -= shapeFunc * nodeCoord[ I_AXIS ];
			rightHandSide[ J_AXIS ] -= shapeFunc * nodeCoord[ J_AXIS ];

			if ( dim == 3 ) {
				jacobiMatrix[ MAP_3D_TENSOR( 0, 2 ) ] += GNi[2][node_I] * nodeCoord[ I_AXIS ];
				jacobiMatrix[ MAP_3D_TENSOR( 1, 2 ) ] += GNi[2][node_I] * nodeCoord[ J_AXIS ];

				jacobiMatrix[ MAP_3D_TENSOR( 2, 0 ) ] += GNi[0][node_I] * nodeCoord[ K_AXIS ];
				jacobiMatrix[ MAP_3D_TENSOR( 2, 1 ) ] += GNi[1][node_I] * nodeCoord[ K_AXIS ];
				jacobiMatrix[ MAP_3D_TENSOR( 2, 2 ) ] += GNi[2][node_I] * nodeCoord[ K_AXIS ];
				
				rightHandSide[ K_AXIS ] -= shapeFunc * nodeCoord[ K_AXIS ];
			}
		}

		/* Finish building right hand side */
		rightHandSide[ I_AXIS ] += globalCoord[ I_AXIS ];
		rightHandSide[ J_AXIS ] += globalCoord[ J_AXIS ];
		if ( dim == 3 )
			rightHandSide[ K_AXIS ] += globalCoord[ K_AXIS ];

		/* Solve for xi increment */
		TensorArray_SolveSystem( jacobiMatrix, xiIncrement, rightHandSide, dim );

		/* Update xi */
		elLocalCoord[ I_AXIS ] += xiIncrement[ I_AXIS ];
		elLocalCoord[ J_AXIS ] += xiIncrement[ J_AXIS ];
		if ( dim == 3 )
			elLocalCoord[ K_AXIS ] += xiIncrement[ K_AXIS ];

		/* Check for convergence */
		maxResidual = fabs( xiIncrement[ I_AXIS ] );
		if ( maxResidual < fabs( xiIncrement[ J_AXIS ] ) )
			maxResidual = fabs( xiIncrement[ J_AXIS ] );
		if ( dim == 3 && maxResidual < fabs( xiIncrement[ K_AXIS ] ) )
			maxResidual = fabs( xiIncrement[ K_AXIS ] );

		if ( maxResidual < tolerance )
			return;
	}
	/* if we are here, it means the iterative method didn't converge.
	   Thus we set the local coord's to be invalid, i.e. greater than 1.0 */
	elLocalCoord[0] = 1.1;
}				


/* +++ Public Functions +++ */
void ElementType_ShapeFunctionsGlobalDerivs( 
		void*			elementType,
		void*			_mesh,
		Element_DomainIndex	elId, 
		double*			xi, 
		int			dim, 
		double*			detJac, 
		double**		GNx )
{
	ElementType*			self = (ElementType*)elementType;
	Mesh*				mesh = (Mesh*)_mesh;
	double				*nodeCoord;
	
	double jac[3][3];
	int rows;		/* max dimensions */
	int cols;		/* max nodes per el */
	double** GNi; 
	int n, i, j;
	double globalSF_DerivVal;
	int dx, dxi;
	double tmp, D = 0.0;
	double cof[3][3];	/* cofactors */
	unsigned nInc, *inc;
	Index nodesPerEl;

	rows=Mesh_GetDimSize( mesh );
	cols=self->nodeCount;	
	
	GNi = self->GNi;

	nodesPerEl = self->nodeCount;

	Mesh_GetIncidence( mesh, Mesh_GetDimSize( mesh ), elId, MT_VERTEX, self->inc );
	nInc = IArray_GetSize( self->inc );
	inc = IArray_GetPtr( self->inc );
	
	/*
	If constant shape function gets passed in here, getLocalDeriv will
	indicate the error and exit code.
	*/
	
	self->_evaluateShapeFunctionLocalDerivsAt( self, xi, GNi );
	
	
	/* build the jacobian matrix */
	/*
	jac = 	\sum_i d/d\xi( N_i ) x_i 		\sum_i d/d\xi( N_i ) y_i
			\sum_i d/d\eta( N_i ) x_i 		\sum_i d/d\eta( N_i ) y_i
	*/
	/* unroll this bugger cause we do it all the time */
	if( dim == 2 ) {
		jac[0][0] = jac[0][1] = jac[1][0] = jac[1][1] = 0.0;
		for( n=0; n<nodesPerEl; n++){	
			nodeCoord = Mesh_GetVertex( mesh, inc[n] );
			jac[0][0] = jac[0][0] + GNi[0][n] * nodeCoord[0];
			jac[0][1] = jac[0][1] + GNi[0][n] * nodeCoord[1];
			
			jac[1][0] = jac[1][0] + GNi[1][n] * nodeCoord[0];
			jac[1][1] = jac[1][1] + GNi[1][n] * nodeCoord[1];
		}
	}
	
	if( dim == 3 ) {
		jac[0][0] = jac[0][1] = jac[0][2] = 0.0;
		jac[1][0] = jac[1][1] = jac[1][2] = 0.0;
		jac[2][0] = jac[2][1] = jac[2][2] = 0.0;
		for( n=0; n<nodesPerEl; n++){	
			nodeCoord = Mesh_GetVertex( mesh, inc[n] );
			jac[0][0] = jac[0][0] + GNi[0][n] * nodeCoord[0];
			jac[0][1] = jac[0][1] + GNi[0][n] * nodeCoord[1];
			jac[0][2] = jac[0][2] + GNi[0][n] * nodeCoord[2];
			
			jac[1][0] = jac[1][0] + GNi[1][n] * nodeCoord[0];
			jac[1][1] = jac[1][1] + GNi[1][n] * nodeCoord[1];
			jac[1][2] = jac[1][2] + GNi[1][n] * nodeCoord[2];
			
			jac[2][0] = jac[2][0] + GNi[2][n] * nodeCoord[0];
			jac[2][1] = jac[2][1] + GNi[2][n] * nodeCoord[1];
			jac[2][2] = jac[2][2] + GNi[2][n] * nodeCoord[2];
		}
	}
	
	/* get determinant of the jacobian matrix */
	if( dim == 2 ) {
		D = jac[0][0]*jac[1][1] - jac[0][1]*jac[1][0]; 
	}		
	if( dim == 3 ) {
		D = jac[0][0]*( jac[1][1]*jac[2][2] - jac[1][2]*jac[2][1] ) 
				  - jac[0][1]*( jac[1][0]*jac[2][2] - jac[1][2]*jac[2][0] ) 
				  + jac[0][2]*( jac[1][0]*jac[2][1] - jac[1][1]*jac[2][0] );
	}
	(*detJac) = D;
	
	
	/* invert the jacobian matrix A^-1 = adj(A)/det(A) */
	if( dim == 2 ) {
		tmp = jac[0][0];
		jac[0][0] = jac[1][1]/D;
		jac[1][1] = tmp/D;
		jac[0][1] = -jac[0][1]/D;
		jac[1][0] = -jac[1][0]/D;		
	}
	if( dim == 3 ) {
		/*
		00 01 02
		10 11 12
		20 21 22		
		*/		
		cof[0][0] = jac[1][1]*jac[2][2] - jac[1][2]*jac[2][1];
		cof[1][0] = -(jac[1][0]*jac[2][2] - jac[1][2]*jac[2][0]);
		cof[2][0] = jac[1][0]*jac[2][1] - jac[1][1]*jac[2][0];
		
		cof[0][1] = -(jac[0][1]*jac[2][2] - jac[0][2]*jac[2][1]);
		cof[1][1] = jac[0][0]*jac[2][2] - jac[0][2]*jac[2][0];
		cof[2][1] = -(jac[0][0]*jac[2][1] - jac[0][1]*jac[2][0]);
		
		cof[0][2] = jac[0][1]*jac[1][2] - jac[0][2]*jac[1][1];
		cof[1][2] = -(jac[0][0]*jac[1][2] - jac[0][2]*jac[1][0]);
		cof[2][2] = jac[0][0]*jac[1][1] - jac[0][1]*jac[1][0];
		
		for( i=0; i<dim; i++ ) {
			for( j=0; j<dim; j++ ) {
				jac[i][j] = cof[i][j]/D;
			}
		}
		
		
	}
	
	/* get global derivs Ni_x, Ni_y and Ni_z if dim == 3 */
	for( dx=0; dx<dim; dx++ ) {
		for( n=0; n<nodesPerEl; n++ ) {
			
			globalSF_DerivVal = 0.0;
			for(dxi=0; dxi<dim; dxi++) {
				globalSF_DerivVal = globalSF_DerivVal + GNi[dxi][n] * jac[dx][dxi];
			}
			
			GNx[dx][n] = globalSF_DerivVal;
		}
	}
}

void ElementType_Jacobian_AxisIndependent( 
		void*               elementType, 
		void*               _mesh, 
		Element_DomainIndex	elId, 
		double*             xi, 
		Dimension_Index     dim, 
		double**            jacobian, 
		double**            _GNi, 
		Coord_Index         A_axis, 
		Coord_Index         B_axis, 
		Coord_Index         C_axis ) 
{
	ElementType* self        = (ElementType*) elementType;
	Mesh*        mesh        = (Mesh*)_mesh;
	double*      nodeCoord;
	double**     GNi;
	Node_Index   nodesPerEl  = self->nodeCount;
	Node_Index   node_I;
	unsigned	nInc, *inc;

	Mesh_GetIncidence( mesh, Mesh_GetDimSize( mesh ), elId, MT_VERTEX, self->inc );
	nInc = IArray_GetSize( self->inc );
	inc = IArray_GetPtr( self->inc );
	
	/* If GNi isn't passed in - then evaluate them for you */
	if (_GNi == NULL) {
		/* Using 3 here instead of dim so that you can pass in dim = 2 and use axes 0 and 2 for your jacobian */
		GNi = Memory_Alloc_2DArray( double, 3, nodesPerEl, (Name)"Temporary GNi"  );
		self->_evaluateShapeFunctionLocalDerivsAt( self, xi, GNi );
	}
	else GNi = _GNi;

	/* build the jacobian matrix */
	/*
	jacobian =  \sum_i d/d\xi( N_i ) x_i        \sum_i d/d\xi( N_i ) y_i
	            \sum_i d/d\eta( N_i ) x_i       \sum_i d/d\eta( N_i ) y_i
	*/
	switch (dim) {
		case 1: 			
			jacobian[A_axis][A_axis] = 0.0;
			for( node_I = 0 ; node_I < nodesPerEl; node_I++){
				nodeCoord = Mesh_GetVertex( mesh, inc[node_I] );
				jacobian[A_axis][A_axis] += GNi[A_axis][node_I] * nodeCoord[A_axis];
			}
			break;
		case 2:
			jacobian[A_axis][A_axis] = jacobian[A_axis][B_axis] = jacobian[B_axis][A_axis] = jacobian[B_axis][B_axis] = 0.0;
			for( node_I = 0 ; node_I < nodesPerEl; node_I++){
				nodeCoord = Mesh_GetVertex( mesh, inc[node_I] );
				jacobian[A_axis][A_axis] += GNi[A_axis][node_I] * nodeCoord[A_axis];
				jacobian[A_axis][B_axis] += GNi[A_axis][node_I] * nodeCoord[B_axis];

				jacobian[B_axis][A_axis] += GNi[B_axis][node_I] * nodeCoord[A_axis];
				jacobian[B_axis][B_axis] += GNi[B_axis][node_I] * nodeCoord[B_axis];
			}
			break;
		case 3:
			jacobian[A_axis][A_axis] = jacobian[A_axis][B_axis] = jacobian[A_axis][C_axis] = 0.0;
			jacobian[B_axis][A_axis] = jacobian[B_axis][B_axis] = jacobian[B_axis][C_axis] = 0.0;
			jacobian[C_axis][A_axis] = jacobian[C_axis][B_axis] = jacobian[C_axis][C_axis] = 0.0;
			for( node_I = 0 ; node_I < nodesPerEl; node_I++){
				nodeCoord = Mesh_GetVertex( mesh, inc[node_I] );

				jacobian[A_axis][A_axis] += GNi[A_axis][node_I] * nodeCoord[A_axis];
				jacobian[A_axis][B_axis] += GNi[A_axis][node_I] * nodeCoord[B_axis];
				jacobian[A_axis][C_axis] += GNi[A_axis][node_I] * nodeCoord[C_axis];

				jacobian[B_axis][A_axis] += GNi[B_axis][node_I] * nodeCoord[A_axis];
				jacobian[B_axis][B_axis] += GNi[B_axis][node_I] * nodeCoord[B_axis];
				jacobian[B_axis][C_axis] += GNi[B_axis][node_I] * nodeCoord[C_axis];

				jacobian[C_axis][A_axis] += GNi[C_axis][node_I] * nodeCoord[A_axis];
				jacobian[C_axis][B_axis] += GNi[C_axis][node_I] * nodeCoord[B_axis];
				jacobian[C_axis][C_axis] += GNi[C_axis][node_I] * nodeCoord[C_axis];
			}
			break;
		/* Mainly here to check unrolled loops above */
		default: {
			Coord_Index row_I, column_I;
			
			for ( row_I = 0 ; row_I < dim ; row_I++ ) {
				for ( column_I = 0 ; column_I < dim ; column_I++ ) {
					/* Initialise */
					jacobian[row_I][column_I] = 0.0;

					/* Calculate */
					for( node_I = 0 ; node_I < nodesPerEl; node_I++){
						nodeCoord = Mesh_GetVertex( mesh, inc[node_I] );
				
						jacobian[row_I][column_I] += GNi[row_I][node_I] * nodeCoord[column_I];
					}
				}
			}
		}
	}

	/* Clean up */
	if (_GNi == NULL) 
		Memory_Free(GNi);
}

double ElementType_JacobianDeterminant_AxisIndependent( 
		void*               elementType, 
		void*               _mesh, 
		Element_DomainIndex	elId, 
		double*             xi, 
		Dimension_Index     dim, 
		Coord_Index         A_axis, 
		Coord_Index         B_axis, 
		Coord_Index         C_axis ) 
{
	double** jacobian;
	double detJac;

	/* Using 3 here instead of dim so that you can pass in dim = 2 and use axes 0 and 2 for your jacobian */
	jacobian = Memory_Alloc_2DArray( double, 3, 3, (Name)"Temporary Jacobian"  );

	ElementType_Jacobian_AxisIndependent( elementType, _mesh, elId, xi, dim, jacobian, NULL, A_axis, B_axis, C_axis );
	detJac = StGermain_MatrixDeterminant_AxisIndependent( jacobian, dim, A_axis, B_axis, C_axis );

	/* Cleaning up */
	Memory_Free( jacobian );

	return detJac;
}

void ElementType_GetFaceNodes( void* elementType, Mesh* mesh, unsigned element_I, unsigned face_I, 
				unsigned nNodes, unsigned* nodes ) {
	ElementType* 	self        = (ElementType*) elementType;
	Index		node_i;
	unsigned*	inc;

	assert( mesh && Stg_CheckType( mesh, FeMesh ) );

	FeMesh_GetElementNodes( mesh, element_I, self->inc );
	inc = IArray_GetPtr( self->inc );

	for( node_i = 0; node_i < nNodes; node_i++ )
		nodes[node_i] = inc[self->faceNodes[face_I][node_i]];
}




