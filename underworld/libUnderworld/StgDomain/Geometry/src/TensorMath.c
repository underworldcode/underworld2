/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include "StGermain/libStGermain/src/StGermain.h"

#include "units.h"
#include "types.h"
#include "TensorMath.h"
#include "VectorMath.h"
#include "TrigMath.h"

#include <math.h>
#include <string.h>

/** Mappings from enumerated types to arrays
This gives the user the option to use:
References instead of enumerated types.
TensorMapFT2D[0][0] = FT2D_00
etc.
*/
const unsigned int TensorMapFT2D[2][2] = {{FT2D_00, FT2D_01},{FT2D_10, FT2D_11}};

/** See explanation for TensorMapFT2D */
const unsigned int TensorMapST2D[2][2] = {{ST2D_00, ST2D_01},{ST2D_01, ST2D_11}};

/** See explanation for TensorMapFT2D */
const unsigned int TensorMapFT3D[3][3] ={{FT3D_00, FT3D_01, FT3D_02},{FT3D_10, FT3D_11, FT3D_12},{FT3D_20, FT3D_21, FT3D_22}};

/** See explanation for TensorMapFT2D */
const unsigned int TensorMapST3D[3][3] ={{ST3D_00, ST3D_01, ST3D_02},{ST3D_01, ST3D_11, ST3D_12},{ST3D_02, ST3D_12, ST3D_22}};

/** This is a wrapper that converts a row/col index and a dimension
into the correct number to reference the correct address of the value
in the tensorArray function.
*/
int TensorArray_TensorMap(Dimension_Index row_I, Dimension_Index col_I, Dimension_Index dim) {
	switch (dim) {
		case 3: {
			return TensorMapFT3D[ row_I ][ col_I ];
		}
		case 2: {
			return TensorMapFT2D[ row_I ][ col_I ];
		}
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( dim, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
		
	}
	return 0;
}

/** This is a wrapper that converts a row/col index and a dimension
into the correct number to reference the correct address of the value
in the symmetricTensor function.
*/
int SymmetricTensor_TensorMap(Dimension_Index row_I, Dimension_Index col_I, Dimension_Index dim) {
	switch (dim) {
		case 3: {
			return TensorMapST3D[ row_I ][ col_I ];
		}
		case 2: {
			return TensorMapST2D[ row_I ][ col_I ];
		}
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( dim, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
		
	}
	return 0;
}

/** This sets the values from a nxn array into a TensorArray */ 
void TensorArray_SetFromDoubleArray( TensorArray tensor, double** array, Dimension_Index dim ) {
	Dimension_Index row_I, col_I;

	for ( row_I = 0 ; row_I < dim ; row_I++ ) {
		for ( col_I = 0 ; col_I < dim ; col_I++ ) {
			tensor[ MAP_TENSOR( row_I, col_I, dim ) ] = array[ row_I ][ col_I ];
		}
	}
}

/** Converts a symmetric tensor to a full tensor */
void StGermain_SymmetricTensor_ToTensorArray(SymmetricTensor symTensor, Dimension_Index dim, TensorArray fullTensor) {
	switch (dim) {
		case 3:
			StGermain_SymmetricTensor_ToTensorArray3D(symTensor, fullTensor);
			return;
		case 2:
			StGermain_SymmetricTensor_ToTensorArray2D(symTensor, fullTensor);
			return;
		default: {
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
}
/** This function uses enumerated types to convert symmetric tensors to full tensors */
void StGermain_SymmetricTensor_ToTensorArray2D(SymmetricTensor symTensor, TensorArray fullTensor) {

	fullTensor[FT2D_00] = symTensor[ST2D_00];
	fullTensor[FT2D_01] = symTensor[ST2D_01];
	fullTensor[FT2D_10] = symTensor[ST2D_01];
	fullTensor[FT2D_11] = symTensor[ST2D_11];
	

}

/** This function uses enumerated types to convert symmetric tensors to full tensors */
void StGermain_SymmetricTensor_ToTensorArray3D(SymmetricTensor symTensor, TensorArray fullTensor) {
	/*Using enumerated types to convert symmetric tensors to full tensors */
	fullTensor[FT3D_00] = symTensor[ST3D_00];
	fullTensor[FT3D_01] = symTensor[ST3D_01];
	fullTensor[FT3D_02] = symTensor[ST3D_02];
	fullTensor[FT3D_10] = symTensor[ST3D_01];
	fullTensor[FT3D_11] = symTensor[ST3D_11];
	fullTensor[FT3D_12] = symTensor[ST3D_12];
	fullTensor[FT3D_20] = symTensor[ST3D_02];
	fullTensor[FT3D_21] = symTensor[ST3D_12];
	fullTensor[FT3D_22] = symTensor[ST3D_22];
	

}

/** This function converts TensorArray's to square Matrixes */
void TensorArray_ToMatrix( TensorArray tensor, Dimension_Index dim, double** matrix ) {
	if (dim == 2) {
		matrix[0][0] = tensor[FT2D_00] ; matrix[0][1] = tensor[FT2D_01] ;
		matrix[1][0] = tensor[FT2D_10] ; matrix[1][1] = tensor[FT2D_11] ;
	}
	else if (dim == 3) {
		matrix[0][0] = tensor[FT3D_00];	matrix[0][1] = tensor[FT3D_01];	matrix[0][2] = tensor[FT3D_02];
		matrix[1][0] = tensor[FT3D_10];	matrix[1][1] = tensor[FT3D_11];	matrix[1][2] = tensor[FT3D_12];
		matrix[2][0] = tensor[FT3D_20];	matrix[2][1] = tensor[FT3D_21];	matrix[2][2] = tensor[FT3D_22];
	}
	else {
		Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
	}
}


void ZeroMatrix( double** matrix, Index rows, Index cols ) {
	Index count_I;
	for( count_I = 0 ; count_I < rows ; count_I++ ) {
		memset( matrix[ count_I ], 0, (cols*sizeof(double)) );
	}
}


/** This function extracts out the antiSymmetric part of a tensor Array
v_{ij} = 0.5 * ( u_{ij} - u_{ji} )
See http://mathworld.wolfram.com/AntisymmetricPart.html */
void TensorArray_GetAntisymmetricPart( const TensorArray tensor, Dimension_Index dim, TensorArray antiSymmetric ) {
	switch (dim) {
		case 3:
			/* v_{xz} = 0.5*( u_{xz} - u_{zx} ) */
			antiSymmetric[ TensorMapFT3D[0][2] ] = 0.5 * (tensor[ TensorMapFT3D[0][2] ] - 
							tensor[ TensorMapFT3D[2][0] ] );
			
			/* v_{yz} = 0.5*( u_{yz} - u_{zy} ) */
			antiSymmetric[ TensorMapFT3D[1][2] ] = 0.5 * (tensor[ TensorMapFT3D[1][2] ] - 
							tensor[ TensorMapFT3D[2][1] ] );
			
			/* v_{zx} = 0.5*( u_{zx} - u_{xz} ) */
			antiSymmetric[ TensorMapFT3D[2][0] ] = - antiSymmetric[ TensorMapFT3D[0][2] ];

			/* v_{zy} = 0.5*( u_{zy} - u_{yz} ) */	
			antiSymmetric[ TensorMapFT3D[2][1] ] = - antiSymmetric[ TensorMapFT3D[1][2] ];

			/* v_{zz} = 0.5*( u_{zz} - u_{zz} ) */
			antiSymmetric[ TensorMapFT3D[2][2] ] = 0.0;

			/* v_{xy} = 0.5*( u_{xy} - u_{yx} ) */
			antiSymmetric[ TensorMapFT3D[0][1] ] = 0.5 * (tensor[ TensorMapFT3D[0][1] ] - 
						tensor[ TensorMapFT3D[1][0] ] );

			/* v_{yx} = 0.5*( u_{yx} - u_{xy} ) */
			antiSymmetric[ TensorMapFT3D[1][0] ] = - antiSymmetric[ TensorMapFT3D[0][1] ];

			/* v_{yy} = 0.5*( u_{yy} - u_{yy} ) */
			antiSymmetric[ TensorMapFT3D[1][1] ] = 0.0;
			
			/* v_{xx} = 0.5*( u_{xx} - u_{xx} ) */
			antiSymmetric[ TensorMapFT3D[0][0] ] = 0.0;
			return;
		case 2:		
			/* v_{xy} = 0.5*( u_{xy} - u_{yx} ) */
			antiSymmetric[ TensorMapFT2D[0][1] ] = 0.5 * (tensor[ TensorMapFT2D[0][1] ] - 
						tensor[ TensorMapFT2D[1][0] ] );

			/* v_{yx} = 0.5*( u_{yx} - u_{xy} ) */
			antiSymmetric[ TensorMapFT2D[1][0] ] = - antiSymmetric[ TensorMapFT2D[0][1] ];

			/* v_{yy} = 0.5*( u_{yy} - u_{yy} ) */
			antiSymmetric[ TensorMapFT2D[1][1] ] = 0.0;
			
			/* v_{xx} = 0.5*( u_{xx} - u_{xx} ) */
			antiSymmetric[ TensorMapFT2D[0][0] ] = 0.0;
			return;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot store tensor for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
}

/** This function calculates the symmetric part of a TensorArray and returns
it in a SymmetricTensor: 
v_{ij} = 0.5 * ( u_{ij} + u_{ji} ) 
see http://mathworld.wolfram.com/SymmetricPart.html 
It can also be used to convert a symmetric TensorArray to a SymmetricTensor
if the TensorArray is guaranteed to be Symmetric. (It has no type checking)*/
void TensorArray_GetSymmetricPart( const TensorArray tensor, Dimension_Index dim, SymmetricTensor symmetricTensor ) {
	switch (dim) {
		case 2:
			/* Diagonal Terms */
			/* v_{xx} = 0.5*( u_{xx} + u_{xx} ) */
			/* v_{yy} = 0.5*( u_{yy} + u_{yy} ) */
			symmetricTensor[ TensorMapST2D[0][0] ] = tensor[ TensorMapFT2D[0][0] ]; 
			symmetricTensor[ TensorMapST2D[1][1] ] = tensor[ TensorMapFT2D[1][1] ]; 
			
			/* Off-diagonal Term */
			/* v_{xy} = 0.5*( u_{xy} + u_{yx} ) */
			symmetricTensor[ TensorMapST2D[0][1] ] = 
				0.5 * (tensor[ TensorMapFT2D[0][1] ] + tensor[ TensorMapFT2D[1][0] ]); 
			return;
		case 3:
			/* Diagonal Terms */
			/* v_{xx} = 0.5*( u_{xx} + u_{xx} ) */
			/* v_{yy} = 0.5*( u_{yy} + u_{yy} ) */
			/* v_{zz} = 0.5*( u_{zz} + u_{zz} ) */
			symmetricTensor[ TensorMapST3D[0][0] ] = tensor[ TensorMapFT3D[0][0] ]; 
			symmetricTensor[ TensorMapST3D[1][1] ] = tensor[ TensorMapFT3D[1][1] ]; 
			symmetricTensor[ TensorMapST3D[2][2] ] = tensor[ TensorMapFT3D[2][2] ]; 
			
			/* Off-diagonal Terms */
			/* v_{xy} = 0.5*( u_{xy} + u_{yx} ) */
			/* v_{xz} = 0.5*( u_{xz} + u_{zx} ) */
			/* v_{yz} = 0.5*( u_{yz} + u_{zy} ) */
			symmetricTensor[ TensorMapST3D[0][1] ] =
				0.5 * (tensor[ TensorMapFT3D[0][1] ] + tensor[ TensorMapFT3D[1][0] ]); 
		
			symmetricTensor[ TensorMapST3D[0][2] ] =
				0.5 * (tensor[ TensorMapFT3D[0][2] ] + tensor[ TensorMapFT3D[2][0] ]); 
		
			symmetricTensor[ TensorMapST3D[1][2] ] =
				0.5 * (tensor[ TensorMapFT3D[1][2] ] + tensor[ TensorMapFT3D[2][1] ]); 
			return;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot store tensor for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
}


/** This function calculates the trace of a tenorArray 
trace = u_{ii} */
void TensorArray_GetTrace( TensorArray tensor, Dimension_Index dim, double *trace ) {
	switch (dim) {
	  case 1:
		/* Sum the diagonal terms */
		*trace = 	tensor[0];
		break;
	  case 2:
		/* Sum the diagonal terms */
		/*
		*trace = 	tensor[ MAP_2D_TENSOR( 0, 0 ) ]
		+ 	tensor[ MAP_2D_TENSOR( 1, 1 ) ];
		*/
		*trace = tensor[FT2D_00] + tensor[FT2D_11];
		break;
	  case 3:
		/* Sum the diagonal terms */
		/*
		*trace = 	tensor[ MAP_3D_TENSOR( 0, 0 ) ]
				+	tensor[ MAP_3D_TENSOR( 1, 1 ) ]
				+	tensor[ MAP_3D_TENSOR( 2, 2 ) ];
		*/
		*trace = tensor[FT3D_00] + tensor[FT3D_11] + tensor[FT3D_22];
		break;
		  default:{
		Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot compute trace for tensor in dimension %d (in %s) since dim < 1.\n", dim, __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
			break;
		}
	}
}

/** This function will calculate the trace of a symmetric tensor type */
void SymmetricTensor_GetTrace( const SymmetricTensor symmTensor, Dimension_Index dim, double *trace ) {
	switch (dim) {
	  case 1:
		*trace = symmTensor[0];
		break;
	  case 2:
		*trace = symmTensor[ST2D_00] + symmTensor[ST2D_11];
		break;
	  case 3:
		*trace = symmTensor[ST3D_00] + symmTensor[ST3D_11] + symmTensor[ST3D_22];
		break;
	  default:{
		Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
		Journal_Printf( error, "Cannot compute trace for symmetric tensor in dimension %d (in %s) since dim is not in the range [1, 3].\n", dim, __func__);
		Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
			"In func '%s' don't understand dim = %u\n", __func__, dim );
		break;
		  }
	}

}

/** This function prints an unnamed tensorArray */
void Journal_PrintTensorArray_Unnamed( Stream* stream, TensorArray tensor, Dimension_Index dim ) {
	Dimension_Index row_I, col_I;

	/* For efficency - Check if stream is enabled */
	if (!Stream_IsEnable(stream)) return;

	for ( row_I = 0 ; row_I < dim ; row_I++ ) {
		for ( col_I = 0 ; col_I < dim ; col_I++ ) {
			Journal_Printf( stream, "%7.5g     ", tensor[ MAP_TENSOR( row_I, col_I, dim ) ] );
		}
		Journal_Printf( stream, "\n" );
	}
}

/** This function prints an unnamed square 2-D Array */
void Journal_PrintSquareArray_Unnamed( Stream* stream, double** array, Dimension_Index dim ) {
	Dimension_Index row_I, col_I;

	/* For efficency - Check if stream is enabled */
	if (!Stream_IsEnable(stream)) return;

	for ( row_I = 0 ; row_I < dim ; row_I++ ) {
		for ( col_I = 0 ; col_I < dim ; col_I++ ) {
			Journal_Printf( stream, "%7.5g     ", array[row_I][col_I] );
		}
		Journal_Printf( stream, "\n" );
	}
}

/** This function prints an unnamed SymmetricTensor */
void Journal_PrintSymmetricTensor_Unnamed( Stream* stream, SymmetricTensor tensor, Dimension_Index dim ) {
	Dimension_Index row_I, col_I;
	/* For efficency - Check if stream is enabled */
	if (!Stream_IsEnable(stream)) return;

	for ( row_I = 0 ; row_I < dim ; row_I++ ) {
		for ( col_I = 0 ; col_I < dim ; col_I++ ) {
			Journal_Printf( stream, "%7.5g     ", tensor[ MAP_SYMM_TENSOR( row_I, col_I, dim ) ] );
		}
		Journal_Printf( stream, "\n" );
	}
}

/** This function calculates the second Invariant of a TensorArray: 
u = \sqrt{ 0.5 u_{ij} u_{ij} } */
double TensorArray_2ndInvariant( const TensorArray tensor, Dimension_Index dim ) {
	double invariant = 0.0;
	switch( dim ) {
		case 3:
			invariant += 
				tensor[4] * tensor[4] +
				tensor[5] * tensor[5] +
				tensor[6] * tensor[6] +
				tensor[7] * tensor[7] +
				tensor[8] * tensor[8] ;
		case 2:
			invariant += 
				tensor[1] * tensor[1] +
				tensor[2] * tensor[2] +
				tensor[3] * tensor[3] ;
		case 1:
			invariant += tensor[0] * tensor[0];
			break;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot read tensor for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
	return sqrt( 0.5 * invariant );
}

/** This function calculates the second Invariant of a SymmetricTensor:  
u = \sqrt{ 0.5 u_{ij} u_{ij} } */
double SymmetricTensor_2ndInvariant( const SymmetricTensor tensor, Dimension_Index dim ) {
	double invariant = 0.0;
	
	switch( dim ) {
		case 3:
			invariant = 
				0.5 * ( tensor[ TensorMapST3D[0][0] ] * tensor[ TensorMapST3D[0][0] ]   +
				        tensor[ TensorMapST3D[1][1] ] * tensor[ TensorMapST3D[1][1] ]   +
				        tensor[ TensorMapST3D[2][2] ] * tensor[ TensorMapST3D[2][2] ] ) +
				
				tensor[ TensorMapST3D[0][1] ] * tensor[ TensorMapST3D[1][0] ] +
				tensor[ TensorMapST3D[0][2] ] * tensor[ TensorMapST3D[2][0] ] +
				tensor[ TensorMapST3D[1][2] ] * tensor[ TensorMapST3D[2][1] ] ;
			break;
		case 2:
			invariant = 
				0.5 * ( tensor[ TensorMapST2D[0][0] ] * tensor[ TensorMapST2D[0][0] ]   +
				        tensor[ TensorMapST2D[1][1] ] * tensor[ TensorMapST2D[1][1] ] ) +
				tensor[ TensorMapST2D[0][1] ] * tensor[ TensorMapST2D[1][0] ] ;
			break;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot read tensor for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
	return sqrt( invariant );
}


/** a_i * u_{ij} * b_j */
double TensorArray_MultiplyByVectors( TensorArray tensor, double* a, double* b, Dimension_Index dim ) {
	double result = 0.0;

	switch( dim ) {
		case 3:
			result = 
				a[0] * tensor[ FT3D_00 ] * b[0] +
				a[0] * tensor[ FT3D_01 ] * b[1] +
				a[0] * tensor[ FT3D_02 ] * b[2] +

				a[1] * tensor[ FT3D_10 ] * b[0] +
				a[1] * tensor[ FT3D_11 ] * b[1] +
				a[1] * tensor[ FT3D_12 ] * b[2] +

				a[2] * tensor[ FT3D_20 ] * b[0] +
				a[2] * tensor[ FT3D_21 ] * b[1] +
				a[2] * tensor[ FT3D_22 ] * b[2] ;
			break;
		case 2:
			result = 
				a[0] * tensor[ FT2D_00 ] * b[0] +
				a[0] * tensor[ FT2D_01 ] * b[1] +

				a[1] * tensor[ FT2D_10 ] * b[0] +
				a[1] * tensor[ FT2D_11 ] * b[1] ;
			break;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot read tensor for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
	return result;
}

/** returns u_{ij} * a_j */
void SymmetricTensor_ApplyOnVector( SymmetricTensor tensor, double* vector, Dimension_Index dim, XYZ result ) {
	switch( dim ) {
		case 3:
			result[0] = 
				tensor[ TensorMapST3D[0][0] ] * vector[0] +
				tensor[ TensorMapST3D[0][1] ] * vector[1] +
				tensor[ TensorMapST3D[0][2] ] * vector[2] ;

			result[1] = 	
				tensor[ TensorMapST3D[1][0] ] * vector[0] +
				tensor[ TensorMapST3D[1][1] ] * vector[1] +
				tensor[ TensorMapST3D[1][2] ] * vector[2] ;

			result[2] = 	
				tensor[ TensorMapST3D[2][0] ] * vector[0] +
				tensor[ TensorMapST3D[2][1] ] * vector[1] +
				tensor[ TensorMapST3D[2][2] ] * vector[2] ;
			break;
		case 2:
			result[0] = 
				tensor[ TensorMapST2D[0][0] ] * vector[0] +
				tensor[ TensorMapST2D[0][1] ] * vector[1] ;

			result[1] = 
				tensor[ TensorMapST2D[1][0] ] * vector[0] +
				tensor[ TensorMapST2D[1][1] ] * vector[1] ;
			break;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot read tensor for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
}


/** returns a_i * u_{ij} * b_j */
double SymmetricTensor_MultiplyByVectors( SymmetricTensor tensor, double* a, double* b, Dimension_Index dim ) {
	double result = 0.0;

	switch( dim ) {
		case 3:
			result = 
				a[0] * tensor[ TensorMapST3D[0][0] ] * b[0] +
				a[0] * tensor[ TensorMapST3D[0][1] ] * b[1] +
				a[0] * tensor[ TensorMapST3D[0][2] ] * b[2] +

				a[1] * tensor[ TensorMapST3D[1][0] ] * b[0] +
				a[1] * tensor[ TensorMapST3D[1][1] ] * b[1] +
				a[1] * tensor[ TensorMapST3D[1][2] ] * b[2] +

				a[2] * tensor[ TensorMapST3D[2][0] ] * b[0] +
				a[2] * tensor[ TensorMapST3D[2][1] ] * b[1] +
				a[2] * tensor[ TensorMapST3D[2][2] ] * b[2] ;
			break;
		case 2:
			result = 
				a[0] * tensor[ TensorMapST2D[0][0] ] * b[0] +
				a[0] * tensor[ TensorMapST2D[0][1] ] * b[1] +

				a[1] * tensor[ TensorMapST2D[1][0] ] * b[0] +
				a[1] * tensor[ TensorMapST2D[1][1] ] * b[1] ;
			break;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMath"  );
			Journal_Printf( error, "Cannot read tensor for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
	return result;
}

void SymmetricTensor_ToMatrix( SymmetricTensor tensor, Dimension_Index dim, double** matrix ) {
	if (dim == 2) {
		matrix[0][0] = tensor[ST2D_00] ; matrix[0][1] = tensor[ST2D_01] ;
		matrix[1][0] = tensor[ST2D_01] ; matrix[1][1] = tensor[ST2D_11] ;
	}
	else if (dim == 3) {
		matrix[0][0] = tensor[ST3D_00];	matrix[0][1] = tensor[ST3D_01];	matrix[0][2] = tensor[ST3D_02];
		matrix[1][0] = tensor[ST3D_01];	matrix[1][1] = tensor[ST3D_11];	matrix[1][2] = tensor[ST3D_12];
		matrix[2][0] = tensor[ST3D_02];	matrix[2][1] = tensor[ST3D_12];	matrix[2][2] = tensor[ST3D_22];
	}
	else {
		Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
	}
}
		
void TensorArray_Zero( TensorArray tensor ) {
	memset( tensor, 0, sizeof(TensorArray) );
}
void SymmetricTensor_Zero( SymmetricTensor tensor ) {
	memset( tensor, 0, sizeof(SymmetricTensor) );
}


int _QsortEigenvalue( const void* _a, const void* _b ) {
	Eigenvector* a = (Eigenvector*) _a;
	Eigenvector* b = (Eigenvector*) _b;

	if ( a->eigenvalue > b->eigenvalue )
		return 1;
	else
		return -1;
}


void SymmetricTensor_CalcAllEigenvalues( SymmetricTensor tensor, Dimension_Index dim, Eigenvector* eigenvectorList ) {
	if ( dim == 2 ) 
		SymmetricTensor_CalcAllEigenvalues2D( tensor, eigenvectorList );
	else 
		SymmetricTensor_CalcAllEigenvalues3D( tensor, eigenvectorList );
}

/* 
For a symmetric 2D Matrix, Eigenvalues are given by: 
\lambda = \frac{u_{00} + u_{11}}{2} \pm \sqrt{ \left(\frac{u_{00} - u_{11}}{2}\right)^2 + u_{01} }
*/
void SymmetricTensor_CalcAllEigenvalues2D( SymmetricTensor tensor, Eigenvector* eigenvectorList ) {
	double descriminantRoot, average;
	
	descriminantRoot = sqrt( 0.25 * (tensor[ST2D_00] - tensor[ST2D_11]) * 
				(tensor[ST2D_00] - tensor[ST2D_11]) + tensor[ST2D_01] * tensor[ST2D_01] );

	average = 0.5 * (tensor[ST2D_00] + tensor[ST2D_11]);

	eigenvectorList[0].eigenvalue = average - descriminantRoot;
	eigenvectorList[1].eigenvalue = average + descriminantRoot;
}

#define EQL(A,B)    (fabs( (A) - (B) ) < 1.0e-8) 
/**
Calculates the roots of the characteristic polynomial given by
det( [tensor] - \lambda [I] ) = 0
1.0 \lambda^3 + a2 \lambda^2 + a1 \lambda + a0 = 0
Results compared with applet located at http://www.math.ubc.ca/~israel/applet/mcalc/matcalc.html */
void SymmetricTensor_CalcAllEigenvalues3D( SymmetricTensor tensor, Eigenvector* eigenvectorList ) {
	double a2, a1, a0;
	double rootList[3];

	/*
	a3 = 1.0
	a2 = - tensor_xx - tensor_yy - tensor_zz
	a1 = tensor_xx * tensor_yy  +  tensor_xx * tensor_zz  +  
		tensor_yy * tensor_zz  -  tensor_xy^2  -  tensor_xz^2  - tensor_yz^2 
	a0 = -tensor_xx * tensor_yy * tensor_zz  - 
		2 * tensor_xy * tensor_xz * tensor_yz  + 
		tensor_xx * tensor_yz^2  +  tensor_yy * tensor_xz^2  +  tensor_zz * tensor_xy^2
	*/

	a2 = - tensor[ST3D_00] - tensor[ST3D_11] - tensor[ST3D_22];
	a1 = (tensor[ST3D_00] * tensor[ST3D_11])  +  
		 (tensor[ST3D_00] * tensor[ST3D_22])  +  
		 (tensor[ST3D_11] * tensor[ST3D_22])  -  
		 (tensor[ST3D_01] * tensor[ST3D_01])  -  
		 (tensor[ST3D_02] * tensor[ST3D_02])  -  
		 (tensor[ST3D_12] * tensor[ST3D_12]);
	a0 = - 	  (tensor[ST3D_00] * tensor[ST3D_11] * tensor[ST3D_22])  -  
		2.0 * (tensor[ST3D_01] * tensor[ST3D_02] * tensor[ST3D_12]) +  
			  (tensor[ST3D_00] * tensor[ST3D_12] * tensor[ST3D_12])  +  
			  (tensor[ST3D_11] * tensor[ST3D_02] * tensor[ST3D_02])  +  
			  (tensor[ST3D_22] * tensor[ST3D_01] * tensor[ST3D_01]);
	
	CubicSolver_OnlyRealRoots( a2, a1, a0, rootList );

	eigenvectorList[0].eigenvalue = rootList[0];
	eigenvectorList[1].eigenvalue = rootList[1];
	eigenvectorList[2].eigenvalue = rootList[2];
}
/** Wrapper to calculate 2 and 3D eigenvectors */
void SymmetricTensor_CalcAllEigenvectors( SymmetricTensor tensor, Dimension_Index dim, Eigenvector* eigenvectorList ) {
	if ( dim == 2 ) 
		SymmetricTensor_CalcAllEigenvectors2D( tensor, eigenvectorList );
	else 
		SymmetricTensor_CalcAllEigenvectors3D( tensor, eigenvectorList );
}
/** Calculates an eigenvector for a given 2D SymmetricTensor */
Bool SymmetricTensor_CalcEigenvector2D( SymmetricTensor tensor, Eigenvector* eigenvector ) {
	if ( fabs(tensor[ST2D_01]) > fabs(eigenvector->eigenvalue - tensor[ST2D_00]) ) {
		eigenvector->vector[0] = 1.0;
		eigenvector->vector[1] = (eigenvector->eigenvalue - tensor[ST2D_00])/tensor[ST2D_01];
	}
	else {
		eigenvector->vector[0] = tensor[ST2D_01]/(eigenvector->eigenvalue - tensor[ST2D_00]);
		eigenvector->vector[1] = 1.0;
	}
	StGermain_VectorNormalise( eigenvector->vector, 2 );

	return True;
}

/** Calculates eigenvectors for 2D SymmetricTensor's only. This is guaranteed to
return real eigenvectors and eigenvalues */
void SymmetricTensor_CalcAllEigenvectors2D( SymmetricTensor tensor, Eigenvector* eigenvectorList ) {
	SymmetricTensor_CalcAllEigenvalues2D( tensor, eigenvectorList );
	
	if ( EQL(tensor[ST2D_01],0.0) ) {
		/* [ a 0 ] */
		/* [ 0 b ] */
		if ( EQL(eigenvectorList[0].eigenvalue, tensor[ST2D_00]) ) {
			eigenvectorList[0].vector[0] = 1.0;
			eigenvectorList[0].vector[1] = 0.0;

			eigenvectorList[1].vector[0] = 0.0;
			eigenvectorList[1].vector[1] = 1.0;
			return;
		}
		else {
			eigenvectorList[0].vector[0] = 0.0;
			eigenvectorList[0].vector[1] = 1.0;

			eigenvectorList[1].vector[0] = 1.0;
			eigenvectorList[1].vector[1] = 0.0;
			return;
		}
	}

	/* First Eigenvector */
	SymmetricTensor_CalcEigenvector2D( tensor, &eigenvectorList[0] );
	SymmetricTensor_CalcEigenvector2D( tensor, &eigenvectorList[1] );

	/* Don't need to sort here because SymmetricTensor_CalcEigenvalues2D already has them sorted */
}

/** Calculates an eigenvector for a given 3D SymmetricTensor */
Bool SymmetricTensor_CalcEigenvector3D( SymmetricTensor tensor, Eigenvector* eigenvector ) {
	double A, B, C, d, e, f;

	A = tensor[ST3D_00] - eigenvector->eigenvalue;
	B = tensor[ST3D_11] - eigenvector->eigenvalue;
	C = tensor[ST3D_22] - eigenvector->eigenvalue;
	d = tensor[ST3D_01];
	e = tensor[ST3D_02];
	f = tensor[ST3D_12];

	if ( ! EQL(B*e, f*d) && ! EQL( e, 0.0 ) ) {
		eigenvector->vector[0] = 1.0;
		eigenvector->vector[1] = (f*A - d*e)/(B*e - f*d);
		eigenvector->vector[2] = (-A - d * eigenvector->vector[1] ) / e ;
	}	
	else if ( ! EQL(f*A, d*e) && ! EQL( e, 0.0 ) ) {
		eigenvector->vector[0] = (B*e - f*d)/(f*A - d*e);
		eigenvector->vector[1] = 1.0;
		eigenvector->vector[2] = (-d - A*eigenvector->vector[0])/e;
	}
	else if ( ! EQL(d*d, B*A) && ! EQL( d, 0.0 ) ) {
		eigenvector->vector[0] = (B*e - f*d)/(d*d - B*A);
		eigenvector->vector[1] = (-e - A*eigenvector->vector[0])/d;
		eigenvector->vector[2] = 1.0;
	}
	else {
		return False;
	}

	StGermain_VectorNormalise( eigenvector->vector, 3 );

	return True;
}

/** Calculates eigenvectors for 3D SymmetricTensor's only. This is guaranteed to
return real eigenvectors and eigenvalues */
void SymmetricTensor_CalcAllEigenvectors3D( SymmetricTensor tensor, Eigenvector* eigenvectorList ) {
	Dimension_Index dim_I;
	Bool            result;

	SymmetricTensor_CalcAllEigenvalues3D( tensor, eigenvectorList );

	for ( dim_I = 0 ; dim_I < 3 ; dim_I++ ) {
		result = SymmetricTensor_CalcEigenvector3D( tensor, &eigenvectorList[dim_I] );

		if ( ! result ) {
			SymmetricTensor_CalcAllEigenvectorsJacobi( tensor, 3, eigenvectorList );
			return;
		}
	}

	EigenvectorList_Sort( eigenvectorList, 3 );
}


void SymmetricTensor_CalcAllEigenvectorsJacobi( SymmetricTensor tensor, Dimension_Index dim, Eigenvector* eigenvectorList ){
	double** matrix = Memory_Alloc_2DArray( double, dim, dim, (Name)"Matrix"  );
		
	SymmetricTensor_ToMatrix( tensor, dim, matrix);
	Matrix_CalcAllEigenvectorsJacobi( matrix, dim, eigenvectorList );
	Memory_Free( matrix );
}

/** Modified code from Numerical Recipies */
/**
Numerical Recipies in C
Second Edition, 1992
pp. 463-469
*/
/* Works for symmetric matrix */
/* Order N^3 !! */
/**
Compared results with applet located at
http://www.math.ubc.ca/~israel/applet/mcalc/matcalc.html
*/
#define ROTATE(a,i,j,k,l)\
	g=a[i][j];\
	h=a[k][l];\
	a[i][j]=g-s*(h+g*tau);\
	a[k][l]=h+s*(g-h*tau);

#define ROTATE_EIGENVECTOR_LIST(eigenvectorList,i,j,k,l)\
	g=eigenvectorList[i].vector[j];\
	h=eigenvectorList[k].vector[l];\
	eigenvectorList[i].vector[j]=g-s*(h+g*tau);\
	eigenvectorList[k].vector[l]=h+s*(g-h*tau); 

/** Calculate all Eigenvectors for a Symmetric Tensor converted to a Matrix 
using the Jacobi Method. This method will ONLY work with symmetric real tensors.
See: Numerical Recipies in C
Second Edition, 1992
pp. 463-469,

*/
void Matrix_CalcAllEigenvectorsJacobi(double **matrix, Index count, Eigenvector* eigenvectorList ) {
	int j,iq,ip,i;
	double tresh,theta,tau,t,sum,s,h,g,c,*b,*z;

	b = Memory_Alloc_Array( double, count, "b" );
	z = Memory_Alloc_Array( double, count, "z" );

	for ( ip = 0 ; ip < count ; ip++ ) {
		/* Initialise Identity Matrix */
		for ( iq = 0 ; iq < count ; iq++ ) eigenvectorList[ip].vector[iq]=0.0;
		eigenvectorList[ip].vector[ip]=1.0;
		
		/* Initialise b and eigenvalues to diagonal of matrix */
		b[ip] = eigenvectorList[ip].eigenvalue = matrix[ip][ip];
		/* This vector will accumulate terms of the form ta_{pq} as in eq 11.1.14 of Numerical Recipies */
		z[ip]=0.0;
	}

	for ( i = 1 ; i <= 50 ; i++ ) {

		/* Sum the off-diagonal elements */
		sum = 0.0;
		for ( ip = 0 ; ip < count-1 ; ip++ ) {
			for ( iq = ip+1 ; iq < count ; iq++ )
				sum += fabs(matrix[ip][iq]);
		}
		/* The normal return, which relies on quadratic convergence to machine underflow */
		if (sum == 0.0) 
			break;

		if (i < 4)
			tresh = 0.2 * sum / (count*count);
		else
			tresh=0.0;

		for ( ip = 0; ip < count-1 ; ip++ ) {
			for ( iq = ip+1 ; iq < count ; iq++) {
				g=100.0*fabs(matrix[ip][iq]);

				/* After the first four sweeps, skip the rotation if the off-diagonal element is small */
				if (i > 4 && (fabs(eigenvectorList[ip].eigenvalue)+g) == fabs(eigenvectorList[ip].eigenvalue)
								&& (fabs(eigenvectorList[iq].eigenvalue)+g) == fabs(eigenvectorList[iq].eigenvalue))
					matrix[ip][iq]=0.0;
				else if (fabs(matrix[ip][iq]) > tresh) {
					h=eigenvectorList[iq].eigenvalue - eigenvectorList[ip].eigenvalue;
					if ((double)(fabs(h)+g) == (double)fabs(h))
						t=(matrix[ip][iq])/h;	/* t = 1/(2theta) */
					else {
						theta=0.5*h/(matrix[ip][iq]);
						t=1.0/(fabs(theta)+sqrt(1.0+theta*theta));
						if (theta < 0.0) t = -t;
					}
					c=1.0/sqrt(1+t*t);
					s=t*c;
					tau=s/(1.0+c);
					h=t*matrix[ip][iq];
					z[ip] -= h;
					z[iq] += h;
					eigenvectorList[ip].eigenvalue -= h;
					eigenvectorList[iq].eigenvalue += h;
					matrix[ip][iq]=0.0;
					for ( j = 0 ; j <= ip-1 ; j++ ) {
						ROTATE(matrix,j,ip,j,iq)
					}
					for ( j = ip+1 ; j <= iq-1 ; j++ ) {
						ROTATE(matrix,ip,j,j,iq)
					}
					for ( j = iq+1 ; j < count ; j++ ) {
						ROTATE(matrix,ip,j,iq,j)
					}
					for ( j = 0 ; j < count ; j++ ) {
						ROTATE_EIGENVECTOR_LIST(eigenvectorList,ip,j,iq,j)
					}
				}
			}
		}
		for ( ip = 0 ; ip < count ; ip++ ) {
			b[ip] += z[ip];
			eigenvectorList[ip].eigenvalue =b[ip];
			z[ip]=0.0;
		}
	}
	
	Memory_Free(z);
	Memory_Free(b);
	
	EigenvectorList_Sort( eigenvectorList, count );
}


/** Sorts the eigenvectors according to the value of the eigenvalue - from smallest to greatest */
void EigenvectorList_Sort( Eigenvector* eigenvectorList, Index count ) {
	qsort( eigenvectorList, count, sizeof( Eigenvector ), _QsortEigenvalue );
}
/** Calculates the determinant of a matrix, independent of the ordering of the axis.*/
double StGermain_MatrixDeterminant_AxisIndependent( double** matrix, Dimension_Index dim, Coord_Index A_axis, Coord_Index B_axis, Coord_Index C_axis ) {
	switch (dim) {
		case 1:
			return matrix[A_axis][A_axis];
		case 2:
			return matrix[A_axis][A_axis]*matrix[B_axis][B_axis] - matrix[A_axis][B_axis]*matrix[B_axis][A_axis];
		case 3:
			return 
				matrix[A_axis][A_axis] *
					( matrix[B_axis][B_axis]*matrix[C_axis][C_axis] - matrix[B_axis][C_axis]*matrix[C_axis][B_axis] ) 
				- matrix[A_axis][B_axis] *
					( matrix[B_axis][A_axis]*matrix[C_axis][C_axis] - matrix[B_axis][C_axis]*matrix[C_axis][A_axis] )
				+ matrix[A_axis][C_axis] *
					( matrix[B_axis][A_axis]*matrix[C_axis][B_axis] - matrix[B_axis][B_axis]*matrix[C_axis][A_axis] );
		default: {
			Stream* error = Journal_Register( Error_Type , (Name)CURR_MODULE_NAME  );
			Journal_Printf(error, "Function %s doesn't support dimension %d.\n", __func__, dim);
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)"TensorMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
	return 0; /* Silly, but TAU-PDT complains otherwise */
}

/** Solves a cubic. See:  
Eric W. Weisstein. "Cubic Formula." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/CubicFormula.html */
void CubicSolver_OnlyRealRoots( double a2, double a1, double a0, double* rootList ) {
	double Q, R;
	double D;
	double theta;
	double factor;

	Q = (3.0 * a1 - a2 * a2)/9.0;                                   /* Equation 20 */
	R = (9.0*a2*a1 - 27.0 * a0 - 2.0 * a2 * a2 * a2)/54.0;          /* Equation 21 */

	/* Get Polynomial Discrimanent  (Equation 47) */
	D = Q*Q*Q + R*R;
		
	if ( EQL( D, 0.0 ) ) {
		double halfB;

		if ( R < 0.0 ) 
			halfB = - pow( -R, 1.0/3.0 );
		else
			halfB = pow( R, 1.0/3.0 );

		rootList[0] = -a2/3.0 + 2.0 * halfB;
		rootList[1] = rootList[2] = -a2/3.0 - halfB;
		return;
	}
	Journal_Firewall( D <= 0.0, Journal_Register( Error_Type, (Name)"CubicSolver"  ),
			"In func %s - Polynomial discrimanent %g is positive which means there are complex solutions.\nCannot solve equation x^3 + %.4f x^2 + %.4f x + %.4f = 0\n", __func__ , D, a2, a1, a0 );


	theta = acos( R/sqrt( -Q*Q*Q ) )/3.0;
	factor = 2.0 * sqrt( -Q );

	rootList[0] = factor * cos( theta ) - a2/3.0;
	rootList[1] = factor * cos( theta + 2.0*M_PI/3.0 ) - a2/3.0;
	rootList[2] = factor * cos( theta + 4.0*M_PI/3.0 ) - a2/3.0;
}


/** Uses Cramer's rule to solve a system of linear equations 
 * see http://mathworld.wolfram.com/CramersRule.html */
void TensorArray_SolveSystem( TensorArray tensorArray, double* solution, double* rightHandSide, Dimension_Index dim ) {
	double determinant;
	switch ( dim ) {
		case 3: {
			determinant = + tensorArray[ FT3D_00 ] *
								( tensorArray[ FT3D_11 ] * tensorArray[ FT3D_22 ] -
								  tensorArray[ FT3D_21 ] * tensorArray[ FT3D_12 ] )
							- tensorArray[ FT3D_01 ] *
								( tensorArray[ FT3D_10 ] * tensorArray[ FT3D_22 ] -
								  tensorArray[ FT3D_12 ] * tensorArray[ FT3D_20 ] )
							+ tensorArray[ FT3D_02 ] *
								( tensorArray[ FT3D_10 ] * tensorArray[ FT3D_21 ] -
								  tensorArray[ FT3D_11 ] * tensorArray[ FT3D_20 ] );

			solution[ 0 ] = ( rightHandSide[0] * 
								( tensorArray[ FT3D_11 ] * tensorArray[ FT3D_22 ] -
								  tensorArray[ FT3D_21 ] * tensorArray[ FT3D_12 ] )
							- tensorArray[ FT3D_01 ] *
								( rightHandSide[1] * tensorArray[ FT3D_22 ] -
								  tensorArray[ FT3D_12 ] * rightHandSide[2] )
							+ tensorArray[ FT3D_02 ] *
								( rightHandSide[1] * tensorArray[ FT3D_21 ] -
								  tensorArray[ FT3D_11 ] * rightHandSide[2] ))/determinant;

			solution[ 1 ] = ( tensorArray[ FT3D_00 ] *
								( rightHandSide[1] * tensorArray[ FT3D_22 ] -
								  rightHandSide[2] * tensorArray[ FT3D_12 ] )
							- rightHandSide[0] * 
								( tensorArray[ FT3D_10 ] * tensorArray[ FT3D_22 ] -
								  tensorArray[ FT3D_12 ] * tensorArray[ FT3D_20 ] )
							+ tensorArray[ FT3D_02 ] *
								( tensorArray[ FT3D_10 ] * rightHandSide[2] -
								  rightHandSide[1] * tensorArray[ FT3D_20 ] ))/determinant;

			solution[ 2 ] = ( tensorArray[ FT3D_00 ] *
								( tensorArray[ FT3D_11 ] * rightHandSide[2] -
								  tensorArray[ FT3D_21 ] * rightHandSide[1] )
							- tensorArray[ FT3D_01 ] *
								( tensorArray[ FT3D_10 ] * rightHandSide[2] -
								  rightHandSide[1] * tensorArray[ FT3D_20 ] )
							+ rightHandSide[0] *
								( tensorArray[ FT3D_10 ] * tensorArray[ FT3D_21 ] -
								  tensorArray[ FT3D_11 ] * tensorArray[ FT3D_20 ] ))/determinant;
			return;
		}

		case 2:
			determinant = tensorArray[ FT2D_00 ] * tensorArray[ FT2D_11 ] 
			            - tensorArray[ FT2D_10 ] * tensorArray[ FT2D_01 ];
			
			solution[ 0 ] = (rightHandSide[0] * tensorArray[ FT2D_11 ] - 
				tensorArray[ FT2D_01 ] * rightHandSide[1])/determinant;
			
			solution[ 1 ] = (rightHandSide[1] * tensorArray[ FT2D_00 ] - 
				tensorArray[ FT2D_10 ] * rightHandSide[0])/determinant;

			return;
		default: 
			Journal_Firewall( 
					False, 
					Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  ),
					"Function %s - doesn't understand dim = %u\n", __func__, dim );
	}
}


