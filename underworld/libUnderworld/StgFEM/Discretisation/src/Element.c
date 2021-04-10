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

#include "Element.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void FiniteElement_Element_Print( void* element, Stream* stream ) {
	FiniteElement_Element*	self = (FiniteElement_Element*)element;
	
	Journal_Printf( stream, "FiniteElement_Element (ptr): %p\n", self );
	Journal_Printf( stream, "\telementType_I: %u", self->elementType_I );
	Journal_Printf( stream, "\tcell_I: %u", self->cell_I );
}


