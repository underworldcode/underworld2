/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <mpi.h>
#include "StGermain/StGermain.h"

#include "units.h"
#include "types.h"
#include "TensorMath.h"
#include "VectorMath.h"
#include "TensorMultMath.h"
#include <math.h>
#include <string.h>

#define STG_TENSORMULT_ERROR 1.0e-15;

/** Create Identity Tensor */
void TensorArray_Identity(Dimension_Index dim, TensorArray tensorArray){

	Dimension_Index index;
	/* Check dimension */
	if ( (dim != 2)&&(dim != 3) ) {		
		Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
		Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
		Journal_Firewall( dim, error,
			"In func '%s' don't understand dim = %u\n", __func__, dim );
	}
	
	/* Calculate indentity matrix */
	for (index = 0; index < (dim * dim); index++){
		tensorArray[index] = 0.0;	
	}
	for (index = 0; index < dim; index++ ){
		tensorArray[TensorArray_TensorMap(index, index, dim)] = 1.0;
	}			
	return;
}

/** Create Identity SymmetricTensor */
void SymmetricTensor_Identity(Dimension_Index dim, SymmetricTensor symmetricTensor) {

	Dimension_Index index;
	/* Check dimension */
	if ( (dim != 2)&&(dim != 3) ) {		
		Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
		Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
		Journal_Firewall( dim, error,
			"In func '%s' don't understand dim = %u\n", __func__, dim );
	}
	
	/* Calculate indentity matrix: zero max = (0.5 * ((dim) * ((dim) + 1)) [Triangular number]*/
	for (index = 0; index < (0.5 * (dim * (dim + 1 )) ); index++){
		symmetricTensor[index] = 0.0;	
	}
	for (index = 0; index < dim; index++ ){
		symmetricTensor[SymmetricTensor_TensorMap(index, index, dim)] = 1.0;
	}			
	return;
}

/** Calculates the transpose of a given tensor array */
void TensorArray_Transpose(TensorArray tensor, Dimension_Index dim, TensorArray result){
switch (dim) {
        case 3:
            result[FT3D_00] = tensor[FT3D_00];
            result[FT3D_01] = tensor[FT3D_10];
            result[FT3D_02] = tensor[FT3D_20];
            result[FT3D_10] = tensor[FT3D_01];
            result[FT3D_11] = tensor[FT3D_11];
            result[FT3D_12] = tensor[FT3D_21];
            result[FT3D_20] = tensor[FT3D_02];
            result[FT3D_21] = tensor[FT3D_12];
            result[FT3D_22] = tensor[FT3D_22];
            return;
        case 2:
            result[FT2D_00] = tensor[FT2D_00];
            result[FT2D_01] = tensor[FT2D_10];
            result[FT2D_10] = tensor[FT2D_01];
            result[FT2D_11] = tensor[FT2D_11];
            break;
        default: {
           	Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
           	Journal_Printf( error, "Cannot read tensor for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( dim, error,
				"In func '%s' don't understand dim = %u\n", __func__, dim );;
            }
        }
}

/** Adds tensorA to tensorB and returns answer in TensorArray result */
void TensorArray_Add(TensorArray tensorA, TensorArray tensorB, Dimension_Index dim, TensorArray result) {
    Dimension_Index index;
	
	if ( (dim != 2)&&(dim != 3) ) {		
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
			Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( dim, error,
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
    for (index = 0; index < (dim * dim); index++){
        result[index] = tensorA[index] + tensorB[index];
    }
}

/** Subtracts tensorB from tensorA and returns answer in TensorArray result*/
void TensorArray_Subtract(	TensorArray tensorArrayA, TensorArray tensorArrayB, Dimension_Index dim, 
							TensorArray result) 
{
	Dimension_Index index;
	
		if ( (dim != 2)&&(dim != 3) ) {		
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
			Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( dim, error,
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	    for (index = 0; index < (dim * dim); index++){
        result[index] = tensorArrayA[index] - tensorArrayB[index];
    }
}

/** Multiplies two TensorArray's */ 
void TensorArray_MultiplyByTensorArray(	TensorArray tensorA, TensorArray tensorB, Dimension_Index dim, TensorArray result) {
	
	switch (dim) {
        case 3:
            result[FT3D_00] = 	tensorA[FT3D_00] * tensorB[FT3D_00] 
							+ 	tensorA[FT3D_01] * tensorB[FT3D_10]
							+ 	tensorA[FT3D_02] * tensorB[FT3D_20];
		
            result[FT3D_01] =	tensorA[FT3D_00] * tensorB[FT3D_01]
							+	tensorA[FT3D_01] * tensorB[FT3D_11]
							+	tensorA[FT3D_02] * tensorB[FT3D_21];
        
			result[FT3D_02] =	tensorA[FT3D_00] * tensorB[FT3D_02]
							+	tensorA[FT3D_01] * tensorB[FT3D_12]
							+ 	tensorA[FT3D_02] * tensorB[FT3D_22];

            result[FT3D_10] =	tensorA[FT3D_10] * tensorB[FT3D_00]
							+	tensorA[FT3D_11] * tensorB[FT3D_10]
							+	tensorA[FT3D_12] * tensorB[FT3D_20];
		
            result[FT3D_11] =	tensorA[FT3D_10] * tensorB[FT3D_01]
							+ 	tensorA[FT3D_11] * tensorB[FT3D_11]
							+	tensorA[FT3D_12] * tensorB[FT3D_21];
        
			result[FT3D_12] =	tensorA[FT3D_10] * tensorB[FT3D_02]
							+ 	tensorA[FT3D_11] * tensorB[FT3D_12]
							+	tensorA[FT3D_12] * tensorB[FT3D_22];

            result[FT3D_20] =	tensorA[FT3D_20] * tensorB[FT3D_00]
							+	tensorA[FT3D_21] * tensorB[FT3D_10]
							+	tensorA[FT3D_22] * tensorB[FT3D_20];
							
            result[FT3D_21]	=	tensorA[FT3D_20] * tensorB[FT3D_01]
							+	tensorA[FT3D_21] * tensorB[FT3D_11]
							+	tensorA[FT3D_22] * tensorB[FT3D_21];
							
            result[FT3D_22] =	tensorA[FT3D_20] * tensorB[FT3D_02]
							+	tensorA[FT3D_21] * tensorB[FT3D_12]
							+	tensorA[FT3D_22] * tensorB[FT3D_22];
            return;
        case 2:
            result[FT2D_00]	=	tensorA[FT2D_00] * tensorB[FT2D_00] 
							+ 	tensorA[FT2D_01] * tensorB[FT2D_10];
		
            result[FT2D_01]	=	tensorA[FT2D_00] * tensorB[FT2D_01]
							+ 	tensorA[FT2D_01] * tensorB[FT2D_11];

            result[FT2D_10]	=	tensorA[FT2D_10] * tensorB[FT2D_00]
							+	tensorA[FT2D_11] * tensorB[FT2D_10];
		
            result[FT2D_11]	=	tensorA[FT2D_10] * tensorB[FT2D_01]
							+	tensorA[FT2D_11] * tensorB[FT2D_11];
            return;
        default: {
           Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
           Journal_Printf( error, "Cannot read tensor for dimension %d in %s.\n", dim, __func__);
           	Journal_Firewall( dim, Journal_Register( Error_Type, (Name)"TensorMultMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );;
            }
    }

}



/** Multiplies tensor A, on the right by it's transpose, A^T to give A*A^T 
And returns the answer in a symmetric tensor*/
void TensorArray_MultiplyByRightTranspose(TensorArray tensor, Dimension_Index dim, SymmetricTensor result) {

	TensorArray tensorTranspose;
	TensorArray fullTensorResult;


	TensorArray_Transpose(tensor, dim, tensorTranspose);
	TensorArray_MultiplyByTensorArray(tensor, tensorTranspose, dim, fullTensorResult);
	
	/** Answer is automatically a symmetric tensor by definition */	
	TensorArray_GetSymmetricPart( fullTensorResult, dim, result ) ;

	return;
}

/** Multiplies tensor A, on the left by it's transpose, A^T to give A^T * A 
And returns the answer in a symmetric tensor*/
void TensorArray_MultiplyByLeftTranspose(TensorArray tensor, Dimension_Index dim, SymmetricTensor result) {

	TensorArray tensorTranspose;
	TensorArray fullTensorResult;

	TensorArray_Transpose( tensor, dim, tensorTranspose);
	TensorArray_MultiplyByTensorArray( tensorTranspose, tensor, dim, fullTensorResult );
	
	/** Answer is automatically a symmetric tensor by definition */	
	TensorArray_GetSymmetricPart( fullTensorResult, dim, result);
	return;
}

/**	Multiplies TensorArray by SymmetricTensor, and gives answer in a TensorArray:
	A * symB */
void TensorArray_MultiplyBySymmetricTensor(	TensorArray tensorArray, SymmetricTensor symmetricTensor,
											Dimension_Index dim, TensorArray result)
{
switch (dim) {
        case 3:
            result[FT3D_00] = 	tensorArray[FT3D_00] * symmetricTensor[ST3D_00] 
							+ 	tensorArray[FT3D_01] * symmetricTensor[ST3D_01]
							+ 	tensorArray[FT3D_02] * symmetricTensor[ST3D_02];
		
            result[FT3D_01] =	tensorArray[FT3D_00] * symmetricTensor[ST3D_01]
							+	tensorArray[FT3D_01] * symmetricTensor[ST3D_11]
							+	tensorArray[FT3D_02] * symmetricTensor[ST3D_12];
        
			result[FT3D_02] =	tensorArray[FT3D_00] * symmetricTensor[ST3D_02]
							+	tensorArray[FT3D_01] * symmetricTensor[ST3D_12]
							+ 	tensorArray[FT3D_02] * symmetricTensor[ST3D_22];

            result[FT3D_10] =	tensorArray[FT3D_10] * symmetricTensor[ST3D_00]
							+	tensorArray[FT3D_11] * symmetricTensor[ST3D_01]
							+	tensorArray[FT3D_12] * symmetricTensor[ST3D_02];
		
            result[FT3D_11] =	tensorArray[FT3D_10] * symmetricTensor[ST3D_01]
							+ 	tensorArray[FT3D_11] * symmetricTensor[ST3D_11]
							+	tensorArray[FT3D_12] * symmetricTensor[ST3D_12];
        
			result[FT3D_12] =	tensorArray[FT3D_10] * symmetricTensor[ST3D_02]
							+ 	tensorArray[FT3D_11] * symmetricTensor[ST3D_12]
							+	tensorArray[FT3D_12] * symmetricTensor[ST3D_22];

            result[FT3D_20] =	tensorArray[FT3D_20] * symmetricTensor[ST3D_00]
							+	tensorArray[FT3D_21] * symmetricTensor[ST3D_01]
							+	tensorArray[FT3D_22] * symmetricTensor[ST3D_02];
							
            result[FT3D_21]	=	tensorArray[FT3D_20] * symmetricTensor[ST3D_01]
							+	tensorArray[FT3D_21] * symmetricTensor[ST3D_11]
							+	tensorArray[FT3D_22] * symmetricTensor[ST3D_12];
							
            result[FT3D_22] =	tensorArray[FT3D_20] * symmetricTensor[ST3D_02]
							+	tensorArray[FT3D_21] * symmetricTensor[ST3D_12]
							+	tensorArray[FT3D_22] * symmetricTensor[ST3D_22];
            return;
        case 2:
            result[FT2D_00]	=	tensorArray[FT2D_00] * symmetricTensor[ST2D_00] 
							+ 	tensorArray[FT2D_01] * symmetricTensor[ST2D_01];
		
            result[FT2D_01]	=	tensorArray[FT2D_00] * symmetricTensor[ST2D_01]
							+ 	tensorArray[FT2D_01] * symmetricTensor[ST2D_11];

            result[FT2D_10]	=	tensorArray[FT2D_10] * symmetricTensor[ST2D_00]
							+	tensorArray[FT2D_11] * symmetricTensor[ST2D_01];
		
            result[FT2D_11]	=	tensorArray[FT2D_10] * symmetricTensor[ST2D_01]
							+	tensorArray[FT2D_11] * symmetricTensor[ST2D_11];
            return;
        default: {
           Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
           Journal_Printf( error, "Cannot read tensor for dimension %d in %s.\n", dim, __func__);
           	Journal_Firewall( dim, Journal_Register( Error_Type, (Name)"TensorMultMath"  ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );;
            }
		}
}

/**	Multiplies SymmetricTensor by TensorArray and gives answer in a TensorArray:
	symA * B */
void SymmetricTensor_MultiplyByTensorArray(	TensorArray tensorArray, SymmetricTensor symmetricTensor,
											Dimension_Index dim, TensorArray result)
{
	TensorArray fullTensor;
	StGermain_SymmetricTensor_ToTensorArray(symmetricTensor, dim, fullTensor);
	TensorArray_MultiplyByTensorArray(fullTensor, tensorArray, dim, result);
	return;
}

/** Multiplies a tensorArray by vector on the left: v * A */
void TensorArray_MultiplyByLeftVector(	TensorArray tensorArray, double* vector, 
										Dimension_Index dim, double* result) 
{
	switch (dim) {
		case 3:
			result[0] = vector[0] * tensorArray[FT3D_00] +
						vector[1] * tensorArray[FT3D_10] +
						vector[2] * tensorArray[FT3D_20];
		
			result[1] = vector[0] * tensorArray[FT3D_01] +
						vector[1] * tensorArray[FT3D_11] +
						vector[2] * tensorArray[FT3D_21];
		
			result[2] = vector[0] * tensorArray[FT3D_02] +
						vector[1] * tensorArray[FT3D_12] +
						vector[2] * tensorArray[FT3D_22];		
			return;
		case 2:
			result[0] = vector[0] * tensorArray[FT2D_00] 
					  + vector[1] * tensorArray[FT2D_10];
		
			result[1] = vector[0] * tensorArray[FT2D_01] 
					  + vector[1] * tensorArray[FT2D_11];		
			return;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
			Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( dim, error,
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}		
		return;
}

/** Multiplies a tensorArray by vector on the right:  A * v */
void TensorArray_MultiplyByRightVector(	TensorArray tensorArray, double* vector, 
										Dimension_Index dim, double* result)
{
	switch (dim) {
		case 3:
			result[0] = tensorArray[FT3D_00] * vector[0] +
						tensorArray[FT3D_01] * vector[1] +
						tensorArray[FT3D_02] * vector[2];
		
			result[1] = tensorArray[FT3D_10] * vector[0] +
						tensorArray[FT3D_11] * vector[1] +
						tensorArray[FT3D_12] * vector[2];
		
			result[2] = tensorArray[FT3D_20] * vector[0] +
						tensorArray[FT3D_21] * vector[1] +
						tensorArray[FT3D_22] * vector[2];		
			return;
		case 2:
			result[0] = tensorArray[FT2D_00] * vector[0] + 
						tensorArray[FT2D_01] * vector[1];
		
			result[1] = tensorArray[FT2D_10] * vector[0] +
					  	tensorArray[FT2D_11] * vector[1];		
			return;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
			Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( dim, error,
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}		
	return;	
	
}

/** Calculates the determinant of a TensorArray*/
double TensorArray_CalcDeterminant(TensorArray tensorArray, Dimension_Index dim) {

    double determinant;
    switch ( dim ) {
        case 3:{
            determinant =   tensorArray[ FT3D_00] *
                                ( tensorArray[ FT3D_11 ] * tensorArray[ FT3D_22 ] -
                                  tensorArray[ FT3D_21 ] * tensorArray[ FT3D_12 ] )
                            - tensorArray[ FT3D_01 ] *
                                ( tensorArray[ FT3D_10 ] * tensorArray[ FT3D_22 ] -
                                  tensorArray[ FT3D_12 ] * tensorArray[ FT3D_20 ] )
                            + tensorArray[ FT3D_02 ] *
                                ( tensorArray[ FT3D_10 ] * tensorArray[ FT3D_21 ] -
                                  tensorArray[ FT3D_11 ] * tensorArray[ FT3D_20 ] );
            return determinant;
            }
        case 2:{
            determinant = tensorArray[ FT2D_00 ] * tensorArray[ FT2D_11 ]
                        - tensorArray[ FT2D_10 ] * tensorArray[ FT2D_01 ];
            return determinant;
            }
        default: {
 			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
			Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
			Journal_Firewall( dim, error,
				"In func '%s' don't understand dim = %u\n", __func__, dim );
        }
    }
	return 0;
}

/** Calculates the inverse of a tensorArray for non-zero determinants.  */
void TensorArray_CalcInverse(TensorArray tensor, Dimension_Index dim, TensorArray result) {
	
	double determinant;
	/* Calculate determinant */
	determinant = TensorArray_CalcDeterminant(tensor, dim);	
	TensorArray_CalcInverseWithDeterminant(tensor, determinant, dim, result);
	return;
}

/** Calculates inverse of tensorArray for non-zero determinant when given a value 
for the determinant. This allows the use of different determinants 
if such calculation is needed.*/
void TensorArray_CalcInverseWithDeterminant(TensorArray tensor, double determinant, Dimension_Index dim, TensorArray result) {
	double  errorValue;

	/* Check if determinant is zero or close to zero*/
	errorValue = STG_TENSORMULT_ERROR;
	if (fabs(determinant) <= errorValue) {
			Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
			Journal_Printf( error, "Error in '%s', Cannot calculate inverse of singular tensorArray:\n", 
							__func__); 
			Journal_PrintTensorArray( error, tensor, dim);
			Journal_Printf( error, "Determinant, %g is zero or near zero. \n", determinant);
		Journal_Firewall( False, Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  ),
				"In func '%s',TensorArray is singular, cannot divide by zero determinant, %g\n", __func__, determinant );
		return;		
	}
	
	/** Uses formula : 
	A^{-1} \= \frac{(-1)^i + j(M_{ij})}{detA} 
	*/ 
	switch (dim) {
			case 3:
				result[FT3D_00] = ( tensor[FT3D_11] * tensor[FT3D_22] - 
									tensor[FT3D_21] * tensor[FT3D_12] ) / determinant;
				result[FT3D_10] = ( tensor[FT3D_20] * tensor[FT3D_12] - 
									tensor[FT3D_10] * tensor[FT3D_22] ) / determinant;
				result[FT3D_20] = ( tensor[FT3D_10] * tensor[FT3D_21] - 
									tensor[FT3D_20] * tensor[FT3D_11] ) / determinant;
	
				result[FT3D_01] = ( tensor[FT3D_21] * tensor[FT3D_02] - 
									tensor[FT3D_01] * tensor[FT3D_22] ) / determinant;
				result[FT3D_11] = ( tensor[FT3D_00] * tensor[FT3D_22] - 
									tensor[FT3D_20] * tensor[FT3D_02] ) / determinant;
				result[FT3D_21] = ( tensor[FT3D_20] * tensor[FT3D_01] - 
									tensor[FT3D_00] * tensor[FT3D_21] ) / determinant;
	
				result[FT3D_02] = ( tensor[FT3D_01] * tensor[FT3D_12] - 
									tensor[FT3D_11] * tensor[FT3D_02] ) / determinant;
				result[FT3D_12] = ( tensor[FT3D_10] * tensor[FT3D_02] - 
									tensor[FT3D_00] * tensor[FT3D_12] ) / determinant;
				result[FT3D_22] = ( tensor[FT3D_00] * tensor[FT3D_11] - 
									tensor[FT3D_10] * tensor[FT3D_01] ) / determinant;
				break;
			case 2:
				result[FT2D_00] = tensor[FT2D_11] / determinant;
				result[FT2D_01] = ( -1.0 * tensor[FT2D_01] ) / determinant;
				result[FT2D_10] = ( -1.0 * tensor[FT2D_10] ) / determinant;
				result[FT2D_11] = tensor[FT2D_00] / determinant;
				break;
			default: {
 				Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
				Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
				Journal_Firewall( dim, error,
				"In func '%s' don't understand dim = %u\n", __func__, dim );
				}
		}
}


/** Calculate double dot product of two tensors */
double TensorArray_DoubleContraction(TensorArray tensorA,TensorArray tensorB, Dimension_Index dim){
    double contraction;
    Dimension_Index i, j;
	/** \[\sigma:\epsilon=\sum_{i=1}^{n}\sum_{i=1}^{n}\sigma_{ij}\epsilon_{ij}\]  */
	/* Check dimension */
	if ( (dim != 2)&&(dim != 3) ) {		
		Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
		Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
		Journal_Firewall( dim, error,
			"In func '%s' don't understand dim = %u\n", __func__, dim );
	}
	
	/* Calculate contraction */
	contraction = 0.0;
	for ( i = 0; i < dim; i++) {
		for (j = 0; j < dim; j++) {
			contraction = 	contraction + 
							tensorA[ TensorArray_TensorMap(i, j, dim) ] * 
							tensorB[ TensorArray_TensorMap(i, j, dim) ];       
		}        
	}

    return contraction;
}

/** Calculate double dot product of two symmteric tensors */
double SymmetricTensor_DoubleContraction(SymmetricTensor tensorA, SymmetricTensor tensorB, Dimension_Index dim)
{
    double contraction;
    Dimension_Index i, j;
	
	/* Check dimension */
	if ( (dim != 2)&&(dim != 3) ) {		
		Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );
		Journal_Printf( error, "Cannot get tensor value for dimension %d in %s.\n", dim, __func__);
		Journal_Firewall( dim, error,
			"In func '%s' don't understand dim = %u\n", __func__, dim );
	}
	
	/* Calculate contraction */
	contraction = 0.0;
	for ( i = 0; i < dim; i++) {
		for (j = 0; j < dim; j++) {
			contraction = 	contraction + 
							tensorA[ SymmetricTensor_TensorMap(i, j, dim) ] * 
							tensorB[ SymmetricTensor_TensorMap(i, j, dim) ];       
		}        
	}

    return contraction;
}

/************ All NonSquareMatrix Functions assume the output Matrix/Vector data structure is already initialised *******/
/** This function calculates the transpose of a non-square nxm 2D matrix
	It requires the row and column dimensions, and assumes an answer matrix
	that is the correct size, ie mxn */
void NonSquareMatrix_Transpose( double** originalMatrix, Dimension_Index rowDimOrig, 
	Dimension_Index colDimOrig, double** newMatrix ) {
	Dimension_Index row_I, col_I;
	if ((rowDimOrig <=0) || (colDimOrig <=0)) {
        Stream* error = Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  );

		Journal_Firewall( False, error,
				"In func '%s' don't understand rows = %u or cols = %u\n", 
			__func__, rowDimOrig, colDimOrig );
	}
	for( row_I = 0 ; row_I < rowDimOrig ; row_I++ ) {
		for( col_I = 0 ; col_I < colDimOrig ; col_I++ ) {
			newMatrix[ col_I ][ row_I ] = originalMatrix[ row_I ][ col_I ];
		}
	}
	return;
}

/** This function multiplies 2 non square matrices and returns the in the resultMatrix. 
 * It requires the columns in MatrixA = rows in BMatrix
 * resultMatrix_ik = AMatrix_ij x BMatrix_jk */
void NonSquareMatrix_MultiplicationByNonSquareMatrix( double **AMatrix, int rowDimA, int colDimA,
					 double **BMatrix, int rowDimB, int colDimB, 
					 double** resultMatrix ) {
	int counter;
	for( counter = 0 ; counter < rowDimA ; counter++ ) {
		memset( resultMatrix[counter], 0, sizeof( double ) * colDimB );
	}

	NonSquareMatrix_CumulativeMultiplicationByNonSquareMatrix(AMatrix,rowDimA,colDimA,
		 	BMatrix,rowDimB, colDimB, 
			resultMatrix );	
	
}

/** This function multiplies 2 non square matrices and adds the result to the passed in resultMatrix. 
 * It requires the columns in MatrixA = rows in BMatrix
 * resultMatrix_ik += AMatrix_ij x BMatrix_jk */
void NonSquareMatrix_CumulativeMultiplicationByNonSquareMatrix( double **AMatrix, 
					 int rowDimA, int colDimA,
					 double **BMatrix, int rowDimB, int colDimB, 
					 double** resultMatrix ) {
			
  int row_I, col_I; /* location with resultMatrix  */
  int counter;      /* counter which facilitates the multiplication of AMatrix and BMatrix */
	/** Error Checking Code */				 
	Journal_Firewall( (colDimA == rowDimB), Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  ),
			"In func '%s'  row dimension B, %u != column dimension A, %u\n", 
		__func__, rowDimB, colDimA );
	Journal_Firewall( (AMatrix && BMatrix && resultMatrix), Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  ),
			"In func '%s', Input matrices: %s %s, or Output matrix: %s is NULL \n", 
		__func__, AMatrix, BMatrix, resultMatrix);

	/** Calculate result matrix */
	for( row_I = 0 ; row_I < rowDimA ; row_I++ ) {
		for( col_I = 0 ; col_I < colDimB ; col_I++ ) {
			/** resultMatrix[ row_I ][ col_I] = AMatrix_ij x BMatrix_jk */
		       	for( counter = 0 ; counter < colDimA ; counter++ ) {
				resultMatrix[ row_I ][ col_I ] += ( AMatrix[row_I][counter] * BMatrix[counter][col_I] );
			}
		}
	}
}

/** This function multiplies a M x N matrices by a N vector.. It requires the columns in AMatrix = rows in BVec
	resultVector_i = AMatrix_ij x BVec_j */
void NonSquareMatrix_MatrixVectorMultiplication( double** AMatrix, int rowsInA, int colsInA,
		     double* BVec, int rowsInB,double* resultVector ) {
  /*This assumes the resultVector is of the correct length. */
	memset(resultVector,0,sizeof(double)* rowsInA);		 
	NonSquareMatrix_CumulativeMatrixVectorMultiplication( AMatrix, rowsInA, colsInA,
		                     BVec, rowsInB, resultVector);						   
}

/** This function multiplies a M x N matrices by a N vector, and then adds this result
 * to the passed in 'solution Vector'. It requires the columns in MatrixA = rows in BVec
 * resultVector_i = AMatrix_ij x BVec_j */
void NonSquareMatrix_CumulativeMatrixVectorMultiplication( double** AMatrix, int rowsInA, int colsInA,
		                               double* BVec, int rowsInB, double* resultVector ) {
  int row_I, col_I; /* counters through matrix rows and columns respectively */
	/** Error Checking Code */  
	Journal_Firewall( ( colsInA == rowsInB ), Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  ),
		"In func '%s' column dimensions of A_Matrix = %d is not equal to the row dimensions of B_Vec = %d\n",
		__func__, colsInA, rowsInB );
	Journal_Firewall( (resultVector && AMatrix && BVec) , Journal_Register( ErrorStream_Type, (Name)"TensorMultMath"  ),
		"In func '%s', Input matrices: %p %p, or Output matrix: %p is NULL \n", 
		__func__, AMatrix, BVec, resultVector);

	/* calculate the result Vector */
	for( row_I = 0 ; row_I < rowsInA ; row_I++ ) {
		for( col_I = 0 ; col_I < colsInA ; col_I++ ) {
			resultVector[ row_I ] += AMatrix[ row_I ][ col_I ] * BVec[ col_I ];
		}
	}
}

/** Prints out a non square matrix if given the row and col dimension */
void Journal_PrintNonSquareMatrix_Unnamed( Stream* stream, double** NonSquareMatrix, 
	Dimension_Index rowDim, Dimension_Index colDim ) 
{
	Dimension_Index row_I, col_I;

	/* For efficency - Check if stream is enabled */
	if (!Stream_IsEnable(stream)) return;
	
	for ( row_I = 0 ; row_I < rowDim ; row_I++ ) {
		for ( col_I = 0 ; col_I < colDim ; col_I++ ) {
			Journal_Printf( stream, "%7.5g	", NonSquareMatrix[row_I][col_I] );
			}
		Journal_Printf( stream, "\n" );
	}
		
}


