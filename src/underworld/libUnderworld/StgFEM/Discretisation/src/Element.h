/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_Element_h__
#define __StgFEM_Discretisation_Element_h__
	
	/* Element Information */
	#define __FiniteElement_Element \
		ElementType_Index	elementType_I; \
		Cell_Index		cell_I;
	struct FiniteElement_Element { __FiniteElement_Element };
	
	/* Print the contents of an Element */
	void FiniteElement_Element_Print( void* element, Stream* stream );
	
#endif /* __StgFEM_Discretisation_Element_h__ */
