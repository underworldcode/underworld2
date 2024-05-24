/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Automation_offsets_h__
#define __StGermain_Base_Automation_offsets_h__

	/** Given a struct object and the name of a member, return the member */
	#define GetMember( structVar, member ) \
		( (structVar).member )

	/** Given an array object and an index into the array, return the indexed item */
	#define GetIndex( arrayVar, index ) \
		( (arrayVar)[(index)] )

	/** Given a struct object and the name of a member, work out the offset of that member in the struct. */
	#define GetOffsetOfMember( structVar, member ) \
		( (ArithPointer)&( GetMember( (structVar), member ) ) - (ArithPointer)&(structVar) )

	/** Given an array object and an index into the array, work out the offset of that index in the array */
	#define GetOffsetOfIndex( arrayVar, index ) \
		( (ArithPointer)&( GetIndex( (arrayVar), (index) ) ) - (ArithPointer)&(arrayVar) )

	/** Given a struct object, the name of an array member and an index into the array, work out the offset of that index in
	    the struct */
	#define GetOffsetOfMemberArrayIndex( structVar, arrayMember, index ) \
		( GetOffsetOfMember( (structVar), arrayMember ) + GetOffsetOfIndex( (structVar).arrayMember, (index) ) )

	/** Given an array object, the index of a struct item and a name of a member, work out the offset of that member in the
	    array */
	#define GetOffsetOfIndexStructMember( arrayVar, structIndex, member ) \
		( GetOffsetOfIndex( (arrayVar), (structIndex) ) + \
		  GetOffsetOfMember( GetIndex( (arrayVar), (structIndex) ), (index) ) )

#endif /* __StGermain_Base_Automation_offsets_h__ */
