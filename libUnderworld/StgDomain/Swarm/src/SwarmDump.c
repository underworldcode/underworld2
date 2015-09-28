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
#include <StgDomain/Utils/Utils.h>

#include "Swarm.h"

#include <assert.h>
#include <string.h>

#ifdef WRITE_HDF5
#include <hdf5.h>
#endif

const Type SwarmDump_Type = "SwarmDump";


SwarmDump* SwarmDump_New(               
	Name		name,
	void*		context,
	Swarm**	swarmList,
	Index		swarmCount,
	Bool		newFileEachTime,
	Bool     saveEntryPoint,
	Bool     dataSaveEntryPoint)
{
	SwarmDump* self = _SwarmDump_DefaultNew( name );

	_SwarmDump_Init( self, context, swarmList, swarmCount, newFileEachTime, saveEntryPoint, dataSaveEntryPoint );
	return self;
}

SwarmDump* _SwarmDump_New(  SWARMDUMP_DEFARGS  ) {
        SwarmDump*              self;
        
        /* Allocate memory */
        assert( _sizeOfSelf >= sizeof(SwarmDump) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

        self = (SwarmDump*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
        
        /* Virtual functions */

        return self;
}

void _SwarmDump_Init(                
                SwarmDump*           self,
                void*                context,
                Swarm**              swarmList,
                Index                swarmCount,
                Bool                 newFileEachTime,
                Bool                 saveEntryPoint,
                Bool                 dataSaveEntryPoint )
{
        self->isConstructed = True;

        self->swarmList = Memory_Alloc_Array( Swarm*, swarmCount, "swarmList" );
        memcpy( self->swarmList, swarmList, swarmCount * sizeof(Swarm*) );
        self->swarmCount = swarmCount;

        self->newFileEachTime = newFileEachTime;
                
        /* Only append hook to context's save EP if context is given */
        if ( context ) {
           if(     saveEntryPoint) EP_AppendClassHook( Context_GetEntryPoint( context, AbstractContext_EP_SaveClass     ), SwarmDump_Execute, self );
           if( dataSaveEntryPoint) EP_AppendClassHook( Context_GetEntryPoint( context, AbstractContext_EP_DataSaveClass ), SwarmDump_Execute, self );
        }
}


void _SwarmDump_Delete( void* swarmDump ) {
        SwarmDump* self = (SwarmDump*) swarmDump;
        
        Memory_Free( self->swarmList );
        _Stg_Component_Delete( self );
}

void _SwarmDump_Print( void* _swarmDump, Stream* stream ) {
        SwarmDump* self = (SwarmDump*) _swarmDump;
        Index      swarm_I;

        Journal_Printf( stream, "SwarmDump - '%s'\n", self->name );
        Stream_Indent( stream );
        _Stg_Component_Print( self, stream );

        for ( swarm_I = 0 ; swarm_I < self->swarmCount ; swarm_I++ ) {
                Journal_Printf( stream, "Swarm - '%s'\n", self->swarmList[ swarm_I ]->name );
        }

        Stream_UnIndent( stream );
}

void* _SwarmDump_Copy( void* swarmDump, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
        SwarmDump*      self = (SwarmDump*)swarmDump;
        SwarmDump*      newSwarmDump;
        PtrMap*                 map = ptrMap;
        Bool                    ownMap = False;
        
        if( !map ) {
                map = PtrMap_New( 10 );
                ownMap = True;
        }
        
        newSwarmDump = _Stg_Component_Copy( self, dest, deep, nameExt, map );
        memcpy( newSwarmDump->swarmList, self->swarmList, self->swarmCount * sizeof(Swarm*) );
        newSwarmDump->swarmCount = self->swarmCount;

        if( ownMap ) {
                Stg_Class_Delete( map );
        }
                                
        return (void*)newSwarmDump;
}


void* _SwarmDump_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(SwarmDump);
	Type                                                      type = SwarmDump_Type;
	Stg_Class_DeleteFunction*                              _delete = _SwarmDump_Delete;
	Stg_Class_PrintFunction*                                _print = _SwarmDump_Print;
	Stg_Class_CopyFunction*                                  _copy = _SwarmDump_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _SwarmDump_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _SwarmDump_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _SwarmDump_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _SwarmDump_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _SwarmDump_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _SwarmDump_Destroy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

                return (void*) _SwarmDump_New(  SWARMDUMP_PASSARGS  );
}
void _SwarmDump_AssignFromXML( void* swarmDump, Stg_ComponentFactory* cf, void* data ) {
        SwarmDump*                  self         = (SwarmDump*)swarmDump;
        Swarm**                 swarmList;
        Bool                    newFileEachTime;
        Bool                    saveEntryPoint;
        Bool                    dataSaveEntryPoint;
        Index                   swarmCount;

	self->context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
	if( !self->context  )
		self->context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, True, data  );

        swarmList          = Stg_ComponentFactory_ConstructByList( cf, self->name, (Dictionary_Entry_Key)"Swarm", Stg_ComponentFactory_Unlimited, Swarm, True, &swarmCount, data  ) ;
        newFileEachTime    = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"newFileEachTime", True  );
        saveEntryPoint     = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"saveEntryPoint", True  );
        dataSaveEntryPoint = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"dataSaveEntryPoint", False  );

        _SwarmDump_Init( 
                        self,
                        (AbstractContext*)self->context,
                        swarmList,
                        swarmCount,
                        newFileEachTime,
                        saveEntryPoint,
                        dataSaveEntryPoint);

        Memory_Free( swarmList );
}

void _SwarmDump_Build( void* swarmDump, void* data ) {
        SwarmDump*       self                = (SwarmDump*)     swarmDump;
        Index        swarm_I;

        for ( swarm_I = 0 ; swarm_I < self->swarmCount ; swarm_I++ ) {
                Stg_Component_Build( self->swarmList[ swarm_I ], data, False );
        }
}

void _SwarmDump_Initialise( void* swarmDump, void* data ) {
        SwarmDump*       self                = (SwarmDump*)     swarmDump;
        Index        swarm_I;

        for ( swarm_I = 0 ; swarm_I < self->swarmCount ; swarm_I++ ) {
                Stg_Component_Initialise( self->swarmList[ swarm_I ], data, False );
        }
}

void _SwarmDump_Execute( void* swarmDump, void* data ) {
        SwarmDump*            self                = (SwarmDump*)     swarmDump;
        AbstractContext*  context             = Stg_CheckType( data, AbstractContext );
        Particle_Index    particleLocalCount;
        SizeT             particleSize;
        Index             swarm_I;
        Swarm*            swarm;
        Stream*           info = Journal_Register( Info_Type, (Name)self->type  );
        Processor_Index   rank_I;

        Journal_DPrintf( info, "Proc %d: beginning Swarm binary checkpoint in %s():\n", self->swarmList[0]->myRank, __func__ );
        Stream_Indent( info );
        
        for ( swarm_I = 0 ; swarm_I < self->swarmCount ; swarm_I++ ) {
                char*			swarmSaveFileName      = NULL;
                char*			swarmSaveFileNamePart1 = NULL;
                char*			swarmSaveFileNamePart2 = NULL;

                swarmSaveFileNamePart1 = Context_GetCheckPointWritePrefixString( context );

                swarm = self->swarmList[ swarm_I ];
                particleLocalCount = swarm->particleLocalCount;
                particleSize = (SizeT) swarm->particleExtensionMgr->finalSize;

                if ( self->newFileEachTime )
                    Stg_asprintf( &swarmSaveFileNamePart2, "%s%s.%05d", swarmSaveFileNamePart1, swarm->name, context->timeStep );
                else
                    Stg_asprintf( &swarmSaveFileNamePart2, "%s%s"     , swarmSaveFileNamePart1, swarm->name );

                #ifdef WRITE_HDF5
                   if(context->nproc == 1)
                           Stg_asprintf( &swarmSaveFileName, "%s.h5", swarmSaveFileNamePart2 );
                   else
                           Stg_asprintf( &swarmSaveFileName, "%s.%dof%d.h5", swarmSaveFileNamePart2, (context->rank + 1), context->nproc );
                #else
                   Stg_asprintf( &swarmSaveFileName, "%s.dat", swarmSaveFileNamePart2 );
                #endif
                 
                #ifdef DEBUG
                   for ( rank_I = 0; rank_I < context->nproc; rank_I++ ) {
                           if ( context->rank == rank_I ) {
                                   Journal_DPrintf( info, "Proc %d: for swarm \"%s\", dumping its %u particles of size %u bytes "
                                           "each (= %g bytes total) to file %s\n", context->rank, swarm->name, particleLocalCount,
                                           particleSize, (float)(particleLocalCount * particleSize), swarmSaveFileName );
                           }       
                           MPI_Barrier( context->communicator );
                   }
                #endif

                #ifdef WRITE_HDF5
                   SwarmDump_DumpToHDF5( self, swarm, swarmSaveFileName );
                #else
                   BinaryStream_WriteAllProcessors( swarmSaveFileName, swarm->particles, particleSize, (SizeT) particleLocalCount, context->communicator );
                #endif
                Memory_Free( swarmSaveFileName );
                Memory_Free( swarmSaveFileNamePart1 );
                Memory_Free( swarmSaveFileNamePart2 );

        }
        Stream_UnIndent( info );
        Journal_DPrintf( info, "Proc %d: finished Swarm binary checkpoint.\n", self->swarmList[0]->myRank );
}

void _SwarmDump_Destroy( void* swarmDump, void* data ) {
}

/** Virtual Function Wrappers */
void SwarmDump_Execute( void* swarmDump, void* context ) {
        SwarmDump*        self                = (SwarmDump*)     swarmDump;

        self->_execute( self, context );
}

#ifdef WRITE_HDF5
void SwarmDump_DumpToHDF5( SwarmDump* self, Swarm* swarm, const char* filename ) {
   hid_t                   file, fileSpace, fileData;
   hid_t                   memSpace;
   hid_t                   props;
   hid_t                   attribData_id, attrib_id, group_id;
   //herr_t                  status;
   hsize_t                 size[2];
   //hsize_t                 cdims[2];
   hsize_t                 a_dims;
   int                     attribData;
   hsize_t                 count[2];
   Particle_Index          lParticle_I = 0;
   Stream*                 errorStr = Journal_Register( Error_Type, (Name)self->type  );
   SwarmVariable*          swarmVar;
   Index                   swarmVar_I;
   char                    dataSpaceName[1024];
                
   /* Open the HDF5 output file. */
   file = H5Fcreate( filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
   Journal_Firewall( 
                file >= 0, 
                errorStr,
                "Error in %s for %s '%s' - Cannot create file %s.\n", 
                __func__, 
                self->type, 
                self->name, 
                filename );

   /* create file attribute to indicate whether file is empty, as HDF5 does not allow empty datasets */
   attribData = swarm->particleLocalCount;
   a_dims = 1;
   attribData_id = H5Screate_simple(1, &a_dims, NULL);
   #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Gopen_vers == 1
      group_id  = H5Gopen(file, "/");
      attrib_id = H5Acreate(group_id, "Swarm Particle Count", H5T_STD_I32BE, attribData_id, H5P_DEFAULT);
   #else
      group_id  = H5Gopen(file, "/", H5P_DEFAULT);
      attrib_id = H5Acreate(group_id, "Swarm Particle Count", H5T_STD_I32BE, attribData_id, H5P_DEFAULT, H5P_DEFAULT);
   #endif
   H5Awrite(attrib_id, H5T_NATIVE_INT, &attribData);
   H5Aclose(attrib_id);
   H5Gclose(group_id);
   H5Sclose(attribData_id);

   if(swarm->particleLocalCount > 0){
      /* Loop through the swarmVariable_Register */
      for( swarmVar_I = 0; swarmVar_I < swarm->swarmVariable_Register->objects->count; swarmVar_I++ ) {
         swarmVar = SwarmVariable_Register_GetByIndex( swarm->swarmVariable_Register, swarmVar_I );
   
         /* check that the swarmVariable should be stored */
         if( swarmVar->isCheckpointedAndReloaded ) {
            
            /* Create our file space. */
            size[0]  = swarm->particleLocalCount;
            size[1]  = swarmVar->dofCount;   
            fileSpace = H5Screate_simple( 2, size, NULL );
            
            /* Create our memory space. */
            count[0] = swarm->particleLocalCount;
            count[1] = swarmVar->dofCount;
            memSpace = H5Screate_simple( 2, count, NULL );
            H5Sselect_all( memSpace );
           
            /* set data chunking size.  as we are not opening and closing
               dataset frequently, a large chunk size (the largest!) seems
               appropriate, and gives good compression */
            //cdims[0] = swarm->particleLocalCount;
            //cdims[1] = swarmVar->dofCount;
            
            props  = H5Pcreate( H5P_DATASET_CREATE );
            /* turn on hdf chunking.. as it is required for compression */
            //status = H5Pset_chunk(props, 2, cdims);
            /* turn on compression */
            //status = H5Pset_deflate( props, 6);
            /* turn on data checksum */ 
            //status = H5Pset_fletcher32(props);
   
            /* Create a new dataspace */
            sprintf( dataSpaceName, "/%s", swarmVar->name + strlen(swarm->name)+1 );
            if( swarmVar->variable->dataTypes[0] == Variable_DataType_Int ) {
               /* Allocate space for the values to be written to file */
               int** value = Memory_Alloc_2DArray( int, swarm->particleLocalCount, swarmVar->dofCount, (Name)"swarmVariableValue"  );
               
               #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
               fileData = H5Dcreate( file, dataSpaceName, H5T_NATIVE_INT, fileSpace, props );
               #else
               fileData = H5Dcreate( file, dataSpaceName, H5T_NATIVE_INT, fileSpace,
                                      H5P_DEFAULT, props, H5P_DEFAULT );
               #endif
               
               /* Loop through local particles */      
               for( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
                  /* Write the value of the current swarmVariable at the current particle to the temp array */
                  Variable_GetValue( swarmVar->variable, lParticle_I, value[lParticle_I] );
               }
               /* Write array to dataspace */
               H5Dwrite( fileData, H5T_NATIVE_INT, memSpace, fileSpace, H5P_DEFAULT, *value );
               Memory_Free( value );
            }
            else if( swarmVar->variable->dataTypes[0] == Variable_DataType_Char) {
               char** value;
               /* Allocate space for the values to be written to file */
               value = Memory_Alloc_2DArray( char, swarm->particleLocalCount, swarmVar->dofCount, (Name)"swarmVariableValue"  );
               
               #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
               fileData = H5Dcreate( file, dataSpaceName, H5T_NATIVE_CHAR, fileSpace, props );
               #else
               fileData = H5Dcreate( file, dataSpaceName, H5T_NATIVE_CHAR, fileSpace,
                                      H5P_DEFAULT, props, H5P_DEFAULT );
               #endif
               
               /* Loop through local particles */      
               for( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
                  /* Write the value of the current swarmVariable at the current particle to the temp array */
                  Variable_GetValue( swarmVar->variable, lParticle_I, value[lParticle_I] );
               }
               /* Write array to dataspace */
               H5Dwrite( fileData, H5T_NATIVE_CHAR, memSpace, fileSpace, H5P_DEFAULT, *value );
               Memory_Free( value );
            }
            else if( swarmVar->variable->dataTypes[0] == Variable_DataType_Float ) {
               float** value;
               /* Allocate space for the values to be written to file */
               value = Memory_Alloc_2DArray( float, swarm->particleLocalCount, swarmVar->dofCount, (Name)"swarmVariableValue"  );
               
               #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
               fileData = H5Dcreate( file, dataSpaceName, H5T_NATIVE_FLOAT, fileSpace, props );
               #else
               fileData = H5Dcreate( file, dataSpaceName, H5T_NATIVE_FLOAT, fileSpace,
                                      H5P_DEFAULT, props, H5P_DEFAULT );
               #endif
               
               /* Loop through local particles */      
               for( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {       
                  /* Write the value of the current swarmVariable at the current particle to the temp array */
                  Variable_GetValue( swarmVar->variable, lParticle_I, value[lParticle_I] );
               }
               /* Write array to dataspace */
               H5Dwrite( fileData, H5T_NATIVE_FLOAT, memSpace, fileSpace, H5P_DEFAULT, *value );
               Memory_Free( value );
            }
            else {
               double** value;
               /* Allocate space for the values to be written to file */
               value = Memory_Alloc_2DArray( double, swarm->particleLocalCount, swarmVar->dofCount, (Name)"swarmVariableValue"  );
               
               #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
               fileData = H5Dcreate( file, dataSpaceName, H5T_NATIVE_DOUBLE, fileSpace, props );
               #else
               fileData = H5Dcreate( file, dataSpaceName, H5T_NATIVE_DOUBLE, fileSpace,
                                      H5P_DEFAULT, props, H5P_DEFAULT );
               #endif          
               
               /* Loop through local particles */      
               for( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
                  /* Write the value of the current swarmVariable at the current particle to the temp array */
                  Variable_GetValue( swarmVar->variable, lParticle_I, value[lParticle_I] );
               }
               /* Write array to dataspace */
               H5Dwrite( fileData, H5T_NATIVE_DOUBLE, memSpace, fileSpace, H5P_DEFAULT, *value );
               Memory_Free( value );
            }
            /* Close the dataspace */
            H5Dclose( fileData );
            H5Sclose( memSpace );
            H5Sclose( fileSpace );
            H5Pclose( props );
         }
      }
      /* now save the element the particle belongs to, if available */
      if( IsChild( ((Stg_Component*)swarm->cellLayout)->type, ElementCellLayout_Type )){
         Mesh* cellLayoutMesh = ((ElementCellLayout*)swarm->cellLayout)->mesh;
            
         /* Create our file space. */
         size[0]  = swarm->particleLocalCount;
         fileSpace = H5Screate_simple( 1, size, NULL );
         
         /* Create our memory space. */
         count[0] = swarm->particleLocalCount;
         memSpace = H5Screate_simple( 1, count, NULL );
         H5Sselect_all( memSpace );
           
         /* set data chunking size.  as we are not opening and closing
            dataset frequently, a large chunk size (the largest!) seems
            appropriate, and gives good compression */
         //cdims[0] = swarm->particleLocalCount;
         
         props  = H5Pcreate( H5P_DATASET_CREATE );
         /* turn on hdf chunking.. as it is required for compression */
         //status = H5Pset_chunk(props, 1, cdims);
         /* turn on compression */
         //status = H5Pset_deflate( props, 6);
         /* turn on data checksum */ 
         //status = H5Pset_fletcher32(props);

         /* Allocate space for the values to be written to file */
         int* value = Memory_Alloc_Array( int, swarm->particleLocalCount, (Name)"swarmVariableValue"  );
         
         #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
         fileData = H5Dcreate( file, "OwningElement", H5T_NATIVE_INT, fileSpace, props );
         #else
         fileData = H5Dcreate( file, "OwningElement", H5T_NATIVE_INT, fileSpace,
                                H5P_DEFAULT, props, H5P_DEFAULT );
         #endif
         
         /* Loop through local particles */      
         for( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
            int particleOwningCell;
            /** get owning cell */
            Variable_GetValue( swarm->owningCellVariable->variable, lParticle_I, &particleOwningCell );
            /** convert from domain to global, and save to array */
            value[lParticle_I] = Mesh_DomainToGlobal( cellLayoutMesh, Mesh_GetDimSize( cellLayoutMesh ), particleOwningCell ); 
         }
         /* Write array to dataspace */
         H5Dwrite( fileData, H5T_NATIVE_INT, memSpace, fileSpace, H5P_DEFAULT, value );
         Memory_Free( value );
         /* Close the dataspace */
         H5Dclose( fileData );
         H5Sclose( memSpace );
         H5Sclose( fileSpace );
         H5Pclose( props );
      }
   }
   /* Close off all our handles. */
   H5Fclose( file );
   
}
#endif


