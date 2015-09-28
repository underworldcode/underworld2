/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StgDomain_Mesh_Remesher_h__
#define __StgDomain_Mesh_Remesher_h__

	/* Textual name of this class. */
	extern const Type Remesher_Type;

	/* Virtual function types. */
	typedef void (Remesher_RemeshFunc)( void* _self );

	/* Class contents. */
	#define __Remesher \
		__Stg_Component \
		AbstractContext*		context; \
		Remesher_RemeshFunc*	remeshFunc; \
		Mesh*						mesh;

	struct Remesher { __Remesher };



	/* Constructors */

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define REMESHER_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                Remesher_RemeshFunc*  remeshFunc

	#define REMESHER_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        remeshFunc

	Remesher* _Remesher_New(  REMESHER_DEFARGS  );

	void _Remesher_Init( void* remeshser, AbstractContext* context, Mesh* mesh );

	/* Virtual functions */

	void _Remesher_Delete( void* remesher );

	void _Remesher_Print( void* remesher, Stream* stream );

	Remesher* _Remesher_DefaultNew( Name name );

	void _Remesher_AssignFromXML( void* remesher, Stg_ComponentFactory* cf, void* data );

	void _Remesher_Build( void* remesher, void* data );

	void _Remesher_Initialise( void* remesher, void* data );

	void _Remesher_Execute( void* remesher, void* data );

	void _Remesher_Destroy( void* remesher, void* data );

	/* Public functions */

	#define Remesher_Remesh( self ) \
		(self)->remeshFunc( self )

#endif

