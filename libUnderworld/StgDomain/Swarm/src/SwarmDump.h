/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_SwarmDump_h__
#define __StgDomain_Swarm_SwarmDump_h__

	/** Textual name of this class */
	extern const Type SwarmDump_Type;
	
	/** SwarmDump contents */
	#define __SwarmDump \
		/* General info */ \
		__Stg_Component \
		\
		AbstractContext*				   context;		  \
		/* Virtual info */ \
		\
		/* Member info */ \
		Swarm**                                            swarmList;             \
		Index                                              swarmCount;            \
		Bool                                               newFileEachTime;

	struct SwarmDump { __SwarmDump };	

	/** Creation implementation */
	SwarmDump* SwarmDump_New(		
		Name                                               name,
		void*                                              context,
		Swarm**                                            swarmList,
		Index                                              swarmCount,
		Bool                                               newFileEachTime,
      Bool                                               saveEntryPoint,
      Bool                                               dataSaveEntryPoint);
	
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SWARMDUMP_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define SWARMDUMP_PASSARGS \
                STG_COMPONENT_PASSARGS

	SwarmDump* _SwarmDump_New(  SWARMDUMP_DEFARGS  );

	/** Member initialisation implementation */
	void _SwarmDump_Init( 
		SwarmDump*                                         self,
		void*                                              context,
		Swarm**                                            swarmList,
		Index                                              swarmCount,
		Bool                                               newFileEachTime,
      Bool                                               saveEntryPoint,
      Bool                                               dataSaveEntryPoint);
	
	void _SwarmDump_Delete( void* swarmDump ) ;
	void _SwarmDump_Print( void* _swarmDump, Stream* stream ) ;
	void* _SwarmDump_Copy( void* swarmDump, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	#define SwarmDump_Copy( self ) \
		(SwarmDump*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	
	void* _SwarmDump_DefaultNew( Name name );
	void _SwarmDump_AssignFromXML( void* swarmDump, Stg_ComponentFactory* cf, void* data ) ;
	void _SwarmDump_Build( void* swarmDump, void* data ) ;
	void _SwarmDump_Initialise( void* swarmDump, void* data ) ;
	void _SwarmDump_Execute( void* swarmDump, void* data ) ;
	void _SwarmDump_Destroy( void* swarmDump, void* data ) ;

	/** Virtual Function Wrappers */
	void SwarmDump_Execute( void* swarmDump, void* context ) ;
#ifdef WRITE_HDF5
	void SwarmDump_DumpToHDF5( SwarmDump* self, Swarm* swarm, const char* filename );
#endif

#endif /* __StgDomain_Swarm_SwarmDump_h__ */

