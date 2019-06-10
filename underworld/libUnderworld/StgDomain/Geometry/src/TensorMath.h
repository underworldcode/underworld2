/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StgDomain_Geometry_TensorMath_h__
#define __StgDomain_Geometry_TensorMath_h__

	/* TensorArray - Tensor (t_{ij}) here is defined in 2D as
	 * t_{00} = tensor[0] t_{01} = tensor[1]
	 * t_{10} = tensor[2] t_{11} = tensor[3] 
	 *
	 * and in 3D as
	 * t_{00} = tensor[0] t_{01} = tensor[1] t_{02} = tensor[2]
	 * t_{10} = tensor[3] t_{11} = tensor[4] t_{12} = tensor[5]
	 * t_{20} = tensor[6] t_{21} = tensor[7] t_{22} = tensor[8]
	 *
	 * */

	/** SymmetricTensor - stores only unique components 
	 * in 2D
	 * tensor[0] = u_{00}
	 * tensor[1] = u_{11}
	 * tensor[2] = u_{12} = u_{21}
	 *
	 * in 3D
	 * tensor[0] = u_{00}
	 * tensor[1] = u_{11}
	 * tensor[2] = u_{22}
	 * tensor[3] = u_{01} = u_{10}
	 * tensor[4] = u_{02} = u_{20}
	 * tensor[5] = u_{12} = u_{21}
	 */

	typedef struct {
		XYZ    vector;
		double eigenvalue;
	} Eigenvector;

/** TensorIndex creates an enumerated type that can be used as 
Tensor indices for referencing:
These indexes are defined as: 
	ST ~ symmetric tensorArray
	FT ~ full tensorArray \\
	2D ~ in 2 dimensions \\
	3D ~ in 3 dimensions \\
For example, tensorArray[FT2D_00] \\

And the indexes follow this naming scheme (for full tensors):
For 2D:
\left{ 	a_{00},	a_{01} \\
		a_{10},	a_{11} \right}
For 3D:		
\left{ 	a_{00},	a_{01},	a_{02} \\
		a_{10},	a_{11},	a_{12} \\
		a_{20}, a_{21}, a_{22} \right}

Symmetric tensors only use the upper triangle indicee definitions.

*/
 typedef enum TensorIndexST2D { ST2D_00=0, ST2D_11=1, ST2D_01=2 } TensorIndexST2D;
 /** See description for TensorIndexST2D */
 typedef enum TensorIndexFT2D { FT2D_00=0, FT2D_11=3, FT2D_01=1, FT2D_10=2 } TensorIndexFT2D;
 /** See description for TensorIndexST2D */
 typedef enum TensorIndexST3D { ST3D_00=0, ST3D_11=1, ST3D_22=2, ST3D_01=3, ST3D_02=4, ST3D_12=5} TensorIndexST3D;
  /** See description for TensorIndexST2D */
 typedef enum TensorIndexFT3D { FT3D_00=0, FT3D_11=4, FT3D_22=8, FT3D_01=1, FT3D_02=2, FT3D_10=3, FT3D_12=5, FT3D_20=6, FT3D_21=7} TensorIndexFT3D;

/*Define mapping function for enumerated types to arrays */
extern const unsigned int TensorMapFT2D[2][2];
extern const unsigned int TensorMapST2D[2][2];
extern const unsigned int TensorMapFT3D[3][3];
extern const unsigned int TensorMapST3D[3][3];

/*Define TensorArray mapping functions */
 
int TensorArray_TensorMap(Dimension_Index row_I, Dimension_Index col_I, Dimension_Index dim);
int SymmetricTensor_TensorMap(Dimension_Index row_I, Dimension_Index col_I, Dimension_Index dim);	

/** Alternate mapping index for 2D TensorArray */ 
#define MAP_2D_TENSOR( ii, jj )  ( (jj) + 2*(ii) )

 /** Alternate mapping index for 3D TensorArray */ 
#define MAP_3D_TENSOR( ii, jj )  ( (jj) + 3*(ii) )

/** Alternate wrapper for indexing a TensorArray */
#define MAP_TENSOR( ii, jj, dim ) \
	((dim) == 2 ? MAP_2D_TENSOR( ii, jj ) : MAP_3D_TENSOR( ii, jj ))

/** Alternate mapping index for 2D SymmetricTensor */ 
#define MAP_2D_SYMM_TENSOR( ii, jj )  ( (ii) == (jj) ? (ii) : 2 )

/** Alternate mapping index for 3D SymmetricTensor */ 
#define MAP_3D_SYMM_TENSOR( ii, jj )  ( (ii) == (jj) ? (ii) : ((ii) + (jj) + 2) )

/** Alternate wrapper for indexing a SymmetricTensor */
#define MAP_SYMM_TENSOR( ii, jj, dim ) \
	(dim == 2 ? MAP_2D_SYMM_TENSOR( ii, jj ) : MAP_3D_SYMM_TENSOR( ii, jj ))

/** Function calculates component size of symmetric tensor given it's dimension */
#define StGermain_nSymmetricTensorVectorComponents( dim ) (Index) (0.5 * ((dim) * ((dim) + 1)))

/** Prints the tensorArray, and it's name in the code */
#define Journal_PrintTensorArray(stream, tensor, dim) \
	do {	\
		Journal_Printf( stream, #tensor " - \n" ); \
		Journal_PrintTensorArray_Unnamed( stream, tensor, dim ); \
	} while(0) 
	
/** Prints the Symmetric Tensor, and it's name in the code */
#define Journal_PrintSymmetricTensor(stream, tensor, dim) \
	do {	\
		Journal_Printf( stream, #tensor " - \n" ); \
		Journal_PrintSymmetricTensor_Unnamed( stream, tensor, dim ); \
	} while (0)
	
/** Prints a square matrix, and it's name in the code */
#define Journal_PrintSquareArray(stream, array, dim) \
	do {	\
		Journal_Printf( stream, #array " - \n" ); \
		Journal_PrintSquareArray_Unnamed( stream, array, dim ); \
	} while (0)
	
/* Define Tensor conversion functions */ 
void StGermain_SymmetricTensor_ToTensorArray(SymmetricTensor symTensor, Dimension_Index dim, TensorArray fullTensor);
	
void StGermain_SymmetricTensor_ToTensorArray2D(SymmetricTensor symTensor, TensorArray fullTensor);
void StGermain_SymmetricTensor_ToTensorArray3D(SymmetricTensor symTensor, TensorArray fullTensor);

void TensorArray_ToMatrix( TensorArray tensor, Dimension_Index dim, double** matrix );	
void SymmetricTensor_ToMatrix( SymmetricTensor tensor, Dimension_Index dim, double** matrix ) ;
	
/* Define Tensor conversion / extraction functions */	
void TensorArray_SetFromDoubleArray( TensorArray tensor, double** array, Dimension_Index dim ) ;
void TensorArray_GetAntisymmetricPart( const TensorArray tensor, Dimension_Index dim, TensorArray antiSymmetric ) ;
void TensorArray_GetSymmetricPart( const TensorArray tensor, Dimension_Index dim, SymmetricTensor symmetricTensor ) ;

/* Define Print functions */	
void Journal_PrintTensorArray_Unnamed( Stream* stream, TensorArray tensor, Dimension_Index dim ) ;
void Journal_PrintSymmetricTensor_Unnamed( Stream* stream, SymmetricTensor tensor, Dimension_Index dim ) ;
void Journal_PrintSquareArray_Unnamed( Stream* stream, double** array, Dimension_Index dim ) ;

/* Define Full Tensor and Symmetric Tensor, 'trace' functions */ 
void TensorArray_GetTrace( TensorArray tensor, Dimension_Index dim, double *trace );
void SymmetricTensor_GetTrace( const SymmetricTensor tensor, Dimension_Index dim, double *trace );
	
double TensorArray_2ndInvariant( const TensorArray tensor, Dimension_Index dim ) ;
double SymmetricTensor_2ndInvariant( const SymmetricTensor tensor, Dimension_Index dim ) ;

/* Define Null / Zero matrices */
void TensorArray_Zero( TensorArray tensor ) ;
void SymmetricTensor_Zero( SymmetricTensor tensor ) ;
void ZeroMatrix( double** matrix, Index rows, Index cols ) ;

/* Define tensorArray Vector functions */
double TensorArray_MultiplyByVectors( TensorArray tensor, double* a, double* b, Dimension_Index dim ) ;

void SymmetricTensor_ApplyOnVector( SymmetricTensor tensor, double* vector, Dimension_Index dim, XYZ result ) ;
double SymmetricTensor_MultiplyByVectors( SymmetricTensor tensor, double* a, double* b, Dimension_Index dim ) ;


/* Eigenvector Stuff - All Eigenvector calculations sort eigenvectors from smallest to largest */
void SymmetricTensor_CalcAllEigenvalues( SymmetricTensor tensor, Dimension_Index dim, Eigenvector* eigenvectorList ) ;
void SymmetricTensor_CalcAllEigenvalues2D( SymmetricTensor tensor, Eigenvector* eigenvectorList ) ;
void SymmetricTensor_CalcAllEigenvalues3D( SymmetricTensor tensor, Eigenvector* eigenvectorList ) ;

void SymmetricTensor_CalcAllEigenvectors( SymmetricTensor tensor, Dimension_Index dim, Eigenvector* eigenvectorList ) ;

Bool SymmetricTensor_CalcEigenvector2D( SymmetricTensor tensor, Eigenvector* eigenvector ) ;
void SymmetricTensor_CalcAllEigenvectors2D( SymmetricTensor tensor, Eigenvector* eigenvectorList ) ;

Bool SymmetricTensor_CalcEigenvector3D( SymmetricTensor tensor, Eigenvector* eigenvector ) ;
void SymmetricTensor_CalcAllEigenvectors3D( SymmetricTensor tensor, Eigenvector* eigenvectorList ) ;

void SymmetricTensor_CalcAllEigenvectorsJacobi( SymmetricTensor tensor, Dimension_Index dim, Eigenvector* eigenvectorList );
void Matrix_CalcAllEigenvectorsJacobi(double **matrix, Index count, Eigenvector* eigenvectorList ) ;

/* Sorts the eigenvectors according to the value of the eigenvalue - from smallest to greatest */
void EigenvectorList_Sort( Eigenvector* eigenvectorList, Index count ) ;

/* Define other useful tensor and matrix functions */
#define StGermain_MatrixDeterminant( matrix, dim ) StGermain_MatrixDeterminant_AxisIndependent( matrix, dim, I_AXIS, J_AXIS, K_AXIS )
double StGermain_MatrixDeterminant_AxisIndependent( double** matrix, Dimension_Index dim, Coord_Index A_axis, Coord_Index B_axis, Coord_Index C_axis ) ;

void CubicSolver_OnlyRealRoots( double a2, double a1, double a0, double* rootList ) ;

void TensorArray_SolveSystem( TensorArray tensorArray, double* solution, double* rightHandSide, Dimension_Index dim ) ;

#endif /* __StgDomain_Geometry_TensorMath_h__ */
