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

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>

#include "types.h"
#include "FieldVariable_Register.h"

#include <assert.h>


const Type FieldVariable_Register_Type = "FieldVariable_Register_Type";

FieldVariable_Register*	FieldVariable_Register_New( void ) {
	/* Variables set in this function */
	SizeT                     _sizeOfSelf = sizeof(FieldVariable_Register);
	Type                             type = FieldVariable_Register_Type;
	Stg_Class_DeleteFunction*     _delete = _NamedObject_Register_Delete;
	Stg_Class_PrintFunction*       _print = _NamedObject_Register_Print;
	Stg_Class_CopyFunction*         _copy = NULL;

	return (FieldVariable_Register*) _NamedObject_Register_New( NAMEDOBJECT_REGISTER_PASSARGS );
}



