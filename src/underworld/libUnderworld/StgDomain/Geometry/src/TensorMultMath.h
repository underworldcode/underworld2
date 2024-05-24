/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StgDomain_Geometry_TensorMultMath_h__
#define __StgDomain_Geometry_TensorMultMath_h__

/* Added to enhance Tensor math for Solid Mechanics applications */

/* TODO These should be in TensorMath  */
void TensorArray_Identity(Dimension_Index dim, TensorArray tensorArray );
void SymmetricTensor_Identity(Dimension_Index dim, SymmetricTensor symmetricTensor) ;

/* Basic operations */
void TensorArray_Transpose(TensorArray tensorArray, Dimension_Index dim, TensorArray result);
void TensorArray_Add(	TensorArray tensorArrayA, TensorArray tensorArrayB, 
						Dimension_Index dim, TensorArray result);
void TensorArray_Subtract(	TensorArray tensorArrayA, TensorArray tensorArrayB, Dimension_Index dim,
							TensorArray result);

/* Multiplication Operations */

void TensorArray_MultiplyByTensorArray(	TensorArray tensorArrayA, TensorArray tensorArrayB, 
										Dimension_Index dim, TensorArray result);
void TensorArray_MultiplyByRightTranspose(TensorArray tensorArray, Dimension_Index dim, SymmetricTensor result);
void TensorArray_MultiplyByLeftTranspose(TensorArray tensorArray, Dimension_Index dim, SymmetricTensor result);
void TensorArray_MultiplyBySymmetricTensor(	TensorArray tensorArray, SymmetricTensor symmetricTensor,
											Dimension_Index dim, TensorArray result);
void TensorArray_MultiplyByLeftVector(	TensorArray tensorArray, double* vector, 
										Dimension_Index dim, double* result);
void TensorArray_MultiplyByRightVector(	TensorArray tensorArray, double* vector, 
										Dimension_Index dim, double* result); 

/* Other useful operations */
double TensorArray_CalcDeterminant(TensorArray tensorArray, Dimension_Index dim);
void TensorArray_CalcInverse( TensorArray tensorA, Dimension_Index dim, TensorArray result);
void TensorArray_CalcInverseWithDeterminant(TensorArray tensor, double determinant, Dimension_Index dim, TensorArray result);


/* Useful operations in Solid Mechanics */
double TensorArray_DoubleContraction(TensorArray tensorArrayA,TensorArray tensorArrayB, Dimension_Index dim);
double SymmetricTensor_DoubleContraction(SymmetricTensor tensorA, SymmetricTensor tensorB, Dimension_Index dim);

/* Non-square matrix operations */
void NonSquareMatrix_Transpose( double** originalMatrix, Dimension_Index rowDimOrig, 
	Dimension_Index colDimOrig, double** newMatrix );

/** This function multiplies 2 non square matrices and returns the in the resultMatrix. 
 * It requires the columns in MatrixA = rows in BMatrix
 * resultMatrix_ik = AMatrix_ij x BMatrix_jk */
void NonSquareMatrix_MultiplicationByNonSquareMatrix( double **AMatrix, int rowsInA, int colsInA,
					 double **BMatrix, int rowsInB, int colsInB, 
					 double** resultMatrix );	

/** This function multiplies 2 non square matrices and adds the result to the passed in resultMatrix. 
 * It requires the columns in MatrixA = rows in BMatrix
 * resultMatrix_ik += AMatrix_ij x BMatrix_jk */
void NonSquareMatrix_CumulativeMultiplicationByNonSquareMatrix( double **AMatrix, int rowDimA, int colDimA,
					 double **BMatrix, int rowDimB, int colDimB, 
					 double** resultMatrix );

/** This function multiplies a M x N matrices by a N vector.. It requires the columns in AMatrix = rows in BVec 
 * resultVector_i = AMatrix_ij x BVec_j */
void NonSquareMatrix_MatrixVectorMultiplication( double** AMatrix, int rowsInA, int colsInA,
		                               double* BVec, int rowsInB, double* resultVector );

/** This function multiplies a M x N matrices by a N vector, and then adds this result
 * to the passed in 'solution Vector'. It requires the columns in MatrixA = rows in BVec
 * resultVector_i += AMatrix_ij x BVec_j */
void NonSquareMatrix_CumulativeMatrixVectorMultiplication( double** AMatrix, int rowsInA, int colsInA,
		                               double* BVec, int rowsInB, double* resultVector ); 						   

/** Print a named NonSquareMatrix */
#define Journal_PrintNonSquareMatrix(stream, matrix, rowDim, colDim) \
	do {	\
		Journal_Printf( stream, #matrix " - \n" ); \
		Journal_PrintNonSquareMatrix_Unnamed( stream, matrix, rowDim, colDim ); \
	} while(0) 
	
void Journal_PrintNonSquareMatrix_Unnamed( Stream* stream, double** NonSquareMatrix, 
	Dimension_Index rowDim, Dimension_Index colDim ) ;




#endif /* __StgDomain_Geometry_TensorMultMath_h__ */
