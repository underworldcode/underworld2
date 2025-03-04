/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Automation_HierarchyTable_h__
#define __StGermain_Base_Automation_HierarchyTable_h__
	
	/* Textual name of this class */
	extern const Type HierarchyTable_Type;

	extern HierarchyTable *stgHierarchyTable;
	
	/* HierarchyTable information */
	#define __HierarchyTable \
		/* General info */ \
		__HashTable \
		\
		/* Virtual info */ \
		\
		/* HierarchyTable info */ \
	
	struct HierarchyTable { __HierarchyTable };
	
	/** Constructor Implementation */
	HierarchyTable* HierarchyTable_New(  );
	
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/** This function returns the pointer to the singleton "stgHierarchyTable" */
	#define HierarchyTable_GetHierarchyTable() \
		((HierarchyTable*) stgHierarchyTable )
	
	/** Function to be called once per class and registers it's parent in the Hierarchy hash table 
	 * the pointer to the child's type string can then be used as a key to find the pointer the 
	 * parent's type string */
	int HierarchyTable_RegisterParent( void* hierarchyTable, Type childType, Type parentType ) ;

	/** Shortcut to above function - it grabs the pointer to the HierarchyTable singleton for you */
	#define RegisterParent( childType, parentType ) \
		HierarchyTable_RegisterParent( HierarchyTable_GetHierarchyTable(), (childType), (parentType) )

	/** Macro to get immediate parent for a type */
	#define HierarchyTable_GetParent( hierarchyTable, type ) \
		(Type) HashTable_FindEntryFunction( (HashTable*) hierarchyTable, type, sizeof(const Type) )

	/** Shortcut to above macro - Gets the HierarchyTable pointer for you */
	#define GetParent( type ) \
		HierarchyTable_GetParent( HierarchyTable_GetHierarchyTable(), type )
	
	/** Function to keep going up the hierarchy to test whether a certain type is a child of a another type */
	Bool HierarchyTable_IsChild( void* hierarchyTable, Type childType, Type possibleParentType ) ;

	/** Shortcut to above function - it grabs the pointer to the HierarchyTable singleton for you */
	#define IsChild( childType, possibleParentType ) \
		HierarchyTable_IsChild( HierarchyTable_GetHierarchyTable(), (childType), (possibleParentType) )

	/** Macro which checks to see if a specific pointer to an object is an instance of a class -
	 * i.e. it's type is a child of this 'possibleParentType' */
	#define Stg_Class_IsInstance( classPtr, possibleParentType )\
			IsChild( ((Stg_Class*) classPtr)->type, (possibleParentType) )

	#define Stg_CheckType( classPtr, possibleParentTypedef ) \
		(possibleParentTypedef*) Stg_Class_CheckType( (classPtr), possibleParentTypedef ## _Type )

	#define Stg_CompareType( classPtr, possibleParentTypedef ) \
		(possibleParentTypedef*) Stg_Class_CompareType( (classPtr), possibleParentTypedef ## _Type )

	#ifdef DEBUG
		#define Stg_DCheckType Stg_CheckType
	#else
		#define Stg_DCheckType( classPtr, possibleParentTypedef ) \
			(possibleParentTypedef*) (classPtr)
	#endif

	Stg_Class* Stg_Class_CheckType( const void* classPtr, Type possibleParentType );
	Bool Stg_Class_CompareType( const void* classPtr, Type possibleParentType );

	void HierarchyTable_PrintParents( void* hierarchyTable, Type childType, Stream* stream ) ;
	void HierarchyTable_PrintChildren( void* hierarchyTable, Type parentType, Stream* stream ) ;

	#define PrintParents( childType, stream ) \
		HierarchyTable_PrintParents( HierarchyTable_GetHierarchyTable(), (childType), (stream) )
	#define PrintChildren( parentType, stream ) \
		HierarchyTable_PrintChildren( HierarchyTable_GetHierarchyTable(), (parentType), (stream) )

	Type HierarchyTable_GetTypePtrFromName( void* hierarchyTable, Name typeName ) ;

#endif /* __StGermain_Base_Automation_HierarchyTable_h__ */
