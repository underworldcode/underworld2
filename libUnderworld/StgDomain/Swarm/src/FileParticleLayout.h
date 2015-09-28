/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_FileParticleLayout_h__
#define __StgDomain_Swarm_FileParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type FileParticleLayout_Type;
	
	/* FileParticleLayout information */
#ifdef READ_HDF5
	#define __FileParticleLayout \
		__GlobalParticleLayout \
		\
		Name                                             filename;    \
		Stream*                                          errorStream; \
		hid_t** fileData; \
		hid_t** fileSpace; \
		Index* lastParticleIndex; \
		hsize_t start[2]; \
		hsize_t count[2]; \
		/** number of files previous checkpoint stored across */ \
		Index                           checkpointfiles;		
#else
	#define __FileParticleLayout \
		__GlobalParticleLayout \
		\
		Name       filename;    \
		FILE*      file;        \
		Stream*    errorStream; \
		/** number of files previous checkpoint stored across */ \
		Index      checkpointfiles;
#endif

	struct FileParticleLayout { __FileParticleLayout };
	
	/* Create a new FileParticleLayout and initialise */
   FileParticleLayout* FileParticleLayout_New( Name name,
      AbstractContext* context, 
      CoordSystem      coordSystem,
      Bool             weightsInitialisedAtStartup,
      unsigned int     totalInitialParticles, 
      double           averageInitialParticlesPerCell, 
      Name             filename, 
      Index            checkpointfiles );

   /* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define FILEPARTICLELAYOUT_DEFARGS \
                GLOBALPARTICLELAYOUT_DEFARGS, \
                Name          filename, \
                Index  checkpointfiles

	#define FILEPARTICLELAYOUT_PASSARGS \
                GLOBALPARTICLELAYOUT_PASSARGS, \
	        filename,        \
	        checkpointfiles

   FileParticleLayout* _FileParticleLayout_New(  FILEPARTICLELAYOUT_DEFARGS  );
	
	void _FileParticleLayout_Init( void* particleLayout, Name filename, Index checkpointfiles );
	
	/* 'Stg_Class' Stuff */
	void _FileParticleLayout_Delete( void* particleLayout );
	void _FileParticleLayout_Print( void* particleLayout, Stream* stream );
	#define FileParticleLayout_Copy( self ) \
		(FileParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define FileParticleLayout_DeepCopy( self ) \
		(FileParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _FileParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _FileParticleLayout_DefaultNew( Name name ) ;
	void _FileParticleLayout_AssignFromXML( void* particleLayout, Stg_ComponentFactory *cf, void* data );
	void _FileParticleLayout_Build( void* particleLayout, void* data );
	void _FileParticleLayout_Initialise( void* particleLayout, void* data );
	void _FileParticleLayout_Execute( void* particleLayout, void* data );
	void _FileParticleLayout_Destroy( void* particleLayout, void* data );
	
	void _FileParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm ) ;
	void _FileParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm ) ;
	void _FileParticleLayout_InitialiseParticle( void* particleLayout, void* swarm, Particle_Index newParticle_I, void* particle);
	/* small routine to find out number of files fileParticleLayout is stored across, which maybe have been stored in the timeInfo checkpoint file */ 
	Index _FileParticleLayout_GetFileCountFromTimeInfoFile( void* context );

#endif /* __StgDomain_Swarm_FileParticleLayout_h__ */

