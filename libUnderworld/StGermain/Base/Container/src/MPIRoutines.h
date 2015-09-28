/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_MPIRoutines_h__
#define __StGermain_Base_Container_MPIRoutines_h__

void MPIArray_Bcast( unsigned* arraySize, void** array, size_t itemSize, 
		     unsigned root, MPI_Comm comm );

void MPIArray_Gather( unsigned arraySize, void* array, 
		      unsigned** dstSizes, void*** dstArrays, 
		      size_t itemSize, unsigned root, MPI_Comm comm );

void MPIArray_Allgather( unsigned arraySize, void* array, 
			 unsigned** dstSizes, void*** dstArrays, 
			 size_t itemSize, MPI_Comm comm );

void MPIArray_Alltoall( unsigned* arraySizes, void** arrays, 
			unsigned** dstSizes, void*** dstArrays, 
			size_t itemSize, MPI_Comm comm );

void Array_1DTo2D( unsigned nBlocks, unsigned* sizes, void* srcArray, 
		   void*** dstArrays, size_t itemSize );

void Array_2DTo1D( unsigned nBlocks, unsigned* sizes, void** srcArrays, 
		   void** dstArray, size_t itemSize, unsigned** disps );

#endif /* __StGermain_Base_Container_MPIRoutines_h__ */
