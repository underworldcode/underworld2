/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "Discretisation.h"


#ifdef WRITE_HDF5
#include <hdf5.h>
#endif

/* Textual name of this class */
const Type FeMesh_Type = "FeMesh";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

FeMesh* FeMesh_New( Name name ) {
   FeMesh* self = _FeMesh_DefaultNew( name );
   _Mesh_Init( (Mesh*)self );
	/* FeMesh info */
	_FeMesh_Init( self, NULL, NULL, False ); /* this is a useless Init() */

   return self;
}

FeMesh* _FeMesh_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(FeMesh);
	Type                                                      type = FeMesh_Type;
	Stg_Class_DeleteFunction*                              _delete = _FeMesh_Delete;
	Stg_Class_PrintFunction*                                _print = _FeMesh_Print;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (void* (*)(Name))_FeMesh_New;
	Stg_Component_ConstructFunction*                    _construct = _FeMesh_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _FeMesh_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _FeMesh_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _FeMesh_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _FeMesh_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	/* The following terms are parameters that have been passed into or defined in this function but are being set before being passed onto the parent */
	Stg_Class_CopyFunction*        _copy = NULL;

   return _FeMesh_New(  FEMESH_PASSARGS  );
}

FeMesh* _FeMesh_New(  FEMESH_DEFARGS  ) {
	FeMesh*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(FeMesh) );
	self = (FeMesh*)_Mesh_New(  MESH_PASSARGS  );

	return self;
}

void _FeMesh_Init( FeMesh* self, ElementType* elType, const char* family, Bool elementMesh ) {
	Stream*	stream;

	assert( self && Stg_CheckType( self, FeMesh ) );

	stream = Journal_Register( Info_Type, (Name)self->type  );
	Stream_SetPrintingRank( stream, 0 );

	self->feElType = elType;
	self->feElFamily = family;
	self->elementMesh = elementMesh;

   /* checkpoint non-constant meshes */
   if ( self->feElFamily && strcmp( self->feElFamily, "constant" ) ){
      self->isCheckpointedAndReloaded = True;
   }
	
	self->inc = IArray_New();
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _FeMesh_Delete( void* feMesh ) {
	FeMesh*	self = (FeMesh*)feMesh;
	/* Delete the parent. */
	_Mesh_Delete( self );
}

void _FeMesh_Print( void* feMesh, Stream* stream ) {
	FeMesh*	self = (FeMesh*)feMesh;
	
	/* Print parent */
	Journal_Printf( stream, "FeMesh (ptr): (%p)\n", self );
	_Mesh_Print( self, stream );
}

void _FeMesh_AssignFromXML( void* feMesh, Stg_ComponentFactory* cf, void* data ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

	_Mesh_AssignFromXML( self, cf, data );

        self->useFeAlgorithms = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"UseFeAlgorithms", True );

	_FeMesh_Init( self, NULL, Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"elementType", "linear"  ), 
		Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"isElementMesh", False )  );
}

void _FeMesh_Build( void* feMesh, void* data ) {
	FeMesh*		self = (FeMesh*)feMesh;
	Stream*		stream;
	ElementType*	elType;

	assert( self );

	stream = Journal_Register( Info_Type, (Name)self->type  );

	_Mesh_Build( self, data );

    /* add this check incase the FeMesh hasn't been provided with a generator yet, in which case there's nothing to do */
	if( !Mesh_GetDimSize( self ) ){
        self->isBuilt = False;
        return;
    }

	Stream_Indent( stream );
	Journal_Printf( stream, "Assigning FeMesh element types...\n" );
	Stream_Indent( stream );

	if( !strcmp( self->feElFamily, "Q2" ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( self );
		if( nDims == 3 )
			elType = (ElementType*)Triquadratic_New( "" );
		else if( nDims == 2 )
			elType = (ElementType*)Biquadratic_New( "" );
		else
			abort();
	}
	else if( !strcmp( self->feElFamily, "Q1" ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( self );
		if( nDims == 3 )
			elType = (ElementType*)TrilinearElementType_New( "" );
		else if( nDims == 2 )
			elType = (ElementType*)BilinearElementType_New( "" );
		else if( nDims == 1 )
                    elType = (ElementType*)LinearElementType_New( "" );
                else
			abort();
	}
	else if( !strcmp( self->feElFamily, "DQ1" ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( self );
		if( nDims == 3 )
			elType = (ElementType*)dQ13DElType_New( "" );
		else if( nDims == 2 )
			elType = (ElementType*)dQ12DElType_New( "" );
        else
			abort();
	}
	else if( !strcmp( self->feElFamily, "DPC1" ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( self );
		if( nDims == 3 )
			elType = (ElementType*)TrilinearInnerElType_New( "" );
		else if( nDims == 2 )
			elType = (ElementType*)BilinearInnerElType_New( "" );
		else
			abort();
	}
	else if( !strcmp( self->feElFamily, "DQ0" ) ) {
		elType = (ElementType*)ConstantElementType_New( "" );
	}
	else
		abort();
	FeMesh_SetElementType( self, elType );
	if( self->feElType )
		Stg_Component_Build( self->feElType, data, False );

    if( !self->elementMesh && self->useFeAlgorithms ) {
        /* We need to swap to the FeMesh element type because the
          geometric versions do not produce the same results. */
        Stg_Class_Delete( self->elTypes[0] );
        self->elTypes[0] = 
           (FeMesh_ElementType*)FeMesh_ElementType_New();
        Mesh_ElementType_SetMesh( self->elTypes[0], self );
        Mesh_ElementType_Update( self->elTypes[0] );
    }

	Journal_Printf( stream, "... FE element types are '%s',\n", elType->type );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
	Stream_UnIndent( stream );
}

void _FeMesh_Initialise( void* feMesh, void* data ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

	_Mesh_Initialise( self, data );

	if( self->feElType )
		Stg_Component_Initialise( self->feElType, data, False );
}

void _FeMesh_Execute( void* feMesh, void* data ) {
}

void _FeMesh_Destroy( void* feMesh, void* data ) {
	FeMesh*	self = (FeMesh*)feMesh;
   
	FeMesh_Destruct( self );
	Stg_Class_Delete( self->inc );
   _Mesh_Destroy( self, data );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void FeMesh_SetElementType( void* feMesh, ElementType* elType ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

   if( self->feElType ) Stg_Class_Delete( self->feElType );
	self->feElType = elType;
}

void FeMesh_SetElementFamily( void* feMesh, const char* family ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

	self->feElFamily = (char*)family;
}

ElementType* FeMesh_GetElementType( void* feMesh, unsigned element ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

	return self->feElType;
}

unsigned FeMesh_GetNodeLocalSize( void* feMesh ) {
	return Mesh_GetLocalSize( feMesh, MT_VERTEX );
}

unsigned FeMesh_GetNodeRemoteSize( void* feMesh ) {
	return Mesh_GetRemoteSize( feMesh, MT_VERTEX );
}

unsigned FeMesh_GetNodeDomainSize( void* feMesh ) {
	return Mesh_GetDomainSize( feMesh, MT_VERTEX );
}

unsigned FeMesh_GetNodeGlobalSize( void* feMesh ) {
	return Mesh_GetGlobalSize( feMesh, MT_VERTEX );
}

unsigned FeMesh_GetElementLocalSize( void* feMesh ) {
	return Mesh_GetLocalSize( feMesh, Mesh_GetDimSize( feMesh ) );
}

unsigned FeMesh_GetElementRemoteSize( void* feMesh ) {
	return Mesh_GetRemoteSize( feMesh, Mesh_GetDimSize( feMesh ) );
}

unsigned FeMesh_GetElementDomainSize( void* feMesh ) {
	return Mesh_GetDomainSize( feMesh, Mesh_GetDimSize( feMesh ) );
}

unsigned FeMesh_GetElementGlobalSize( void* feMesh ) {
	return Mesh_GetGlobalSize( feMesh, Mesh_GetDimSize( feMesh ) );
}

unsigned FeMesh_GetElementNodeSize( void* feMesh, unsigned element ) {
	return Mesh_GetIncidenceSize( feMesh, Mesh_GetDimSize( feMesh ), element, MT_VERTEX );
}

unsigned FeMesh_GetNodeElementSize( void* feMesh, unsigned node ) {
	return Mesh_GetIncidenceSize( feMesh, MT_VERTEX, node, Mesh_GetDimSize( feMesh ) );
}

void FeMesh_GetElementNodes( void* feMesh, unsigned element, IArray* inc ) {
	Mesh_GetIncidence( feMesh, Mesh_GetDimSize( feMesh ), element, MT_VERTEX, inc );
}

void FeMesh_GetNodeElements( void* feMesh, unsigned node, IArray* inc ) {
	Mesh_GetIncidence( feMesh, MT_VERTEX, node, Mesh_GetDimSize( feMesh ), inc );
}

unsigned FeMesh_ElementDomainToGlobal( void* feMesh, unsigned domain ) {
	return Mesh_DomainToGlobal( feMesh, Mesh_GetDimSize( feMesh ), domain );
}

Bool FeMesh_ElementGlobalToDomain( void* feMesh, unsigned global, unsigned* domain ) {
	return Mesh_GlobalToDomain( feMesh, Mesh_GetDimSize( feMesh ), global, domain );
}

unsigned FeMesh_NodeDomainToGlobal( void* feMesh, unsigned domain ) {
	return Mesh_DomainToGlobal( feMesh, MT_VERTEX, domain );
}

Bool FeMesh_NodeGlobalToDomain( void* feMesh, unsigned global, unsigned* domain ) {
	return Mesh_GlobalToDomain( feMesh, MT_VERTEX, global, domain );
}

void FeMesh_CoordGlobalToLocal( void* feMesh, unsigned element, const double* global, double* local ) {
	FeMesh*		self = (FeMesh*)feMesh;
	ElementType*	elType;

	assert( self );
	assert( element < FeMesh_GetElementDomainSize( self ) );
	assert( global );
	assert( local );

	elType = FeMesh_GetElementType( self, element );
	ElementType_ConvertGlobalCoordToElLocal( elType, self, element, global, local );
}

void FeMesh_CoordLocalToGlobal( void* feMesh, unsigned element, const double* local, double* global ) {
	FeMesh*		self = (FeMesh*)feMesh;
	unsigned	nDims;
	ElementType*	elType;
	double*		basis;
	unsigned	nElNodes, *elNodes;
	double		dimBasis;
	double*		vert;
	unsigned	n_i, d_i;

	assert( self );
	assert( element < FeMesh_GetElementDomainSize( self ) );
	assert( global );
	assert( local );

	nDims = Mesh_GetDimSize( self );
	elType = FeMesh_GetElementType( self, element );
	FeMesh_GetElementNodes( self, element, self->inc );
	nElNodes = IArray_GetSize( self->inc );
	elNodes = IArray_GetPtr( self->inc );
	basis = AllocArray( double, nElNodes );
	ElementType_EvaluateShapeFunctionsAt( elType, local, basis );

	memset( global, 0, nDims * sizeof(double) );
	for( n_i = 0; n_i < nElNodes; n_i++ ) {
		dimBasis = basis[n_i];
		vert = Mesh_GetVertex( self, elNodes[n_i] );
		for( d_i = 0; d_i < nDims; d_i++ )
			global[d_i] += dimBasis * vert[d_i];
	}

	FreeArray( basis );
}

void FeMesh_EvalBasis( void* feMesh, unsigned element, double* localCoord, double* basis ) {
	FeMesh*		self = (FeMesh*)feMesh;
	ElementType*	elType;

	assert( self );
	assert( localCoord );

	elType = FeMesh_GetElementType( self, element );
	ElementType_EvaluateShapeFunctionsAt( elType, localCoord, basis );
}

void FeMesh_EvalLocalDerivs( void* feMesh, unsigned element, double* localCoord, double** derivs ) {
	FeMesh*		self = (FeMesh*)feMesh;
	ElementType*	elType;

	assert( self );
	assert( localCoord );
	assert( derivs );

	elType = FeMesh_GetElementType( self, element );
	ElementType_EvaluateShapeFunctionLocalDerivsAt( elType, localCoord, derivs );
}

void FeMesh_EvalGlobalDerivs( void* feMesh, unsigned element, double* localCoord, double** derivs, double* jacDet ) {
	FeMesh*		self = (FeMesh*)feMesh;
	unsigned	nDims;
	ElementType*	elType;
	double		jd;

	assert( self );
	assert( localCoord );
	assert( derivs );

	nDims = Mesh_GetDimSize( self );
	elType = FeMesh_GetElementType( self, element );
	ElementType_ShapeFunctionsGlobalDerivs( elType, self, element, localCoord, nDims, 
						&jd, derivs );
	if( jacDet )
		*jacDet = jd;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void FeMesh_Destruct( FeMesh* self ) {
   Stg_Class_Delete( self->feElType );
	self->feElFamily = NULL;
	/* Disabling the killing of this object from within this
	component as this will be destroyed by the LiveComponentRegister_DestroyAll function 101109 */
	/*KillObject( self->feElType );*/ 
}

#define FINISHED_WRITING_TAG 31
#ifdef WRITE_HDF5
void _FeMesh_DumpMeshHDF5( FeMesh* mesh, const char* filename  ) {
   int                   rank, nRanks;
   unsigned              nDims;
   hid_t                 file, fileSpace, fileData, fileSpace2, fileData2;
   hid_t                 memSpace;
   hsize_t               start[2], count[2], size[2];
   unsigned              totalVerts;
   Node_LocalIndex       lNode_I = 0;
   Node_GlobalIndex      gNode_I = 0;
   double*               coord;
   MPI_Status            status;
   int                   confirmation = 0;
   Element_LocalIndex    lElement_I;
   Element_GlobalIndex   gElement_I;
   Index                 maxNodes;
   IArray*               iarray = IArray_New( );
   int                   err;

   int dim = Mesh_GetDimSize( mesh );
   Comm* stg_comm = Mesh_GetCommTopology( mesh, dim );
   MPI_Comm comm = stg_comm->mpiComm;

   MPI_Comm_rank( comm, &rank);
   MPI_Comm_size( comm, &nRanks );   
         
   nDims  = Mesh_GetDimSize( mesh );

   /* wait for go-ahead from process ranked lower than me, to avoid competition writing to file */
   if ( rank != 0 ) {
      err=MPI_Recv( &confirmation, 1, MPI_INT, rank - 1, FINISHED_WRITING_TAG, comm, &status );
   }   

   if ( rank == 0 ) {
      hid_t     attribData_id, attrib_id, group_id;
      hsize_t   a_dims;
      int       attribData;
      Grid**    grid;
      unsigned* sizes;

      /* Open the HDF5 output file. */
      file = H5Fcreate( filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
      assert( file );

      /** create file attribute */
      /** first store the checkpointing version */
      a_dims = 1;
      attribData = MeshCHECKPOINT_V2;
      attribData_id = H5Screate_simple(1, &a_dims, NULL);
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Gopen_vers == 1
         group_id  = H5Gopen(file, "/");
         attrib_id = H5Acreate(group_id, "checkpoint file version", H5T_STD_I32BE, attribData_id, H5P_DEFAULT);
      #else
         group_id  = H5Gopen(file, "/", H5P_DEFAULT);
         attrib_id = H5Acreate(group_id, "checkpoint file version", H5T_STD_I32BE, attribData_id, H5P_DEFAULT, H5P_DEFAULT);
      #endif
      H5Awrite(attrib_id, H5T_NATIVE_INT, &attribData);
      H5Aclose(attrib_id);
      H5Gclose(group_id);
      H5Sclose(attribData_id);

      /** store the mesh dimensionality */
      a_dims = 1;
      attribData = dim;
      attribData_id = H5Screate_simple(1, &a_dims, NULL);
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Gopen_vers == 1
         group_id  = H5Gopen(file, "/");
         attrib_id = H5Acreate(group_id, "dimensions", H5T_STD_I32BE, attribData_id, H5P_DEFAULT);
      #else
         group_id  = H5Gopen(file, "/", H5P_DEFAULT);
         attrib_id = H5Acreate(group_id, "dimensions", H5T_STD_I32BE, attribData_id, H5P_DEFAULT, H5P_DEFAULT);
      #endif
      H5Awrite(attrib_id, H5T_NATIVE_INT, &attribData);
      H5Aclose(attrib_id);
      H5Gclose(group_id);
      H5Sclose(attribData_id);
       
      /** store the mesh resolution */
      a_dims = dim;
      grid   = (Grid**) Mesh_GetExtension( mesh, Grid*,  mesh->elGridId );   
      sizes  = Grid_GetSizes( *grid ); /** global no. of elements in each dim */
      
      attribData_id = H5Screate_simple(1, &a_dims, NULL);
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Gopen_vers == 1
         group_id  = H5Gopen(file, "/");
         attrib_id = H5Acreate(group_id, "mesh resolution", H5T_STD_I32BE, attribData_id, H5P_DEFAULT);
      #else
         group_id  = H5Gopen(file, "/", H5P_DEFAULT);
         attrib_id = H5Acreate(group_id, "mesh resolution", H5T_STD_I32BE, attribData_id, H5P_DEFAULT, H5P_DEFAULT);
      #endif
      H5Awrite(attrib_id, H5T_NATIVE_INT, sizes);
      H5Aclose(attrib_id);
      H5Gclose(group_id);
      H5Sclose(attribData_id);
   
      /* Dump the min and max coords, and number of processes. */
      count[0] = (hsize_t)nDims;
      fileSpace = H5Screate_simple( 1, count, NULL );         
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
      fileData = H5Dcreate( file, "/min", H5T_NATIVE_DOUBLE, fileSpace, H5P_DEFAULT );
      #else
      fileData = H5Dcreate( file, "/min", H5T_NATIVE_DOUBLE, fileSpace,
         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      #endif
                  
      H5Dwrite( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, mesh->minGlobalCrd );
      H5Dclose( fileData );
      H5Sclose( fileSpace );
         
      fileSpace = H5Screate_simple( 1, count, NULL );       
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
      fileData = H5Dcreate( file, "/max", H5T_NATIVE_DOUBLE, fileSpace, H5P_DEFAULT );
      #else
      fileData = H5Dcreate( file, "/max", H5T_NATIVE_DOUBLE, fileSpace,
         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      #endif
            
      H5Dwrite( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, mesh->maxGlobalCrd );
      H5Dclose( fileData );
      H5Sclose( fileSpace );
       
      /* Write vertex coords to file */   
      /* Create our output space and data objects. */
      totalVerts = Mesh_GetGlobalSize( mesh, 0 );
      size[0] = (hsize_t)totalVerts;
      size[1] = (hsize_t)nDims;
      
      fileSpace = H5Screate_simple( 2, size, NULL );
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
      fileData = H5Dcreate( file, "/vertices", H5T_NATIVE_DOUBLE, fileSpace, H5P_DEFAULT );
      #else
      fileData = H5Dcreate( file, "/vertices", H5T_NATIVE_DOUBLE, fileSpace,
         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      #endif

      /* setup element connectivity dataspaces */
      if (mesh->nElTypes == 1)
         maxNodes = FeMesh_GetElementNodeSize( mesh, 0);
      else {
         /* determine the maximum number of nodes each element has */
         maxNodes = 0;
         for ( gElement_I = 0; gElement_I < FeMesh_GetElementGlobalSize(mesh); gElement_I++ ) {
            unsigned numNodes;
            numNodes = FeMesh_GetElementNodeSize( mesh, gElement_I);
            if( maxNodes < numNodes ) maxNodes = numNodes;
         }
      }
      
      size[0] = (hsize_t)FeMesh_GetElementGlobalSize(mesh);
      size[1] = (hsize_t)maxNodes;
      /* Create our output space and data objects. */         
      fileSpace2 = H5Screate_simple( 2, size, NULL );
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
      fileData2 = H5Dcreate( file, "/connectivity", H5T_NATIVE_INT, fileSpace2, H5P_DEFAULT );
      #else
      fileData2 = H5Dcreate( file, "/connectivity", H5T_NATIVE_INT, fileSpace2,
         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      #endif

   }
   else {
      /* Open the HDF5 output file. */
      file = H5Fopen( filename, H5F_ACC_RDWR, H5P_DEFAULT );
      Journal_Firewall( file >= 0, Journal_Register( Error_Type, (Name)mesh->type  ), 
              "Error in %s for %s '%s' - Cannot open file %s.\n", 
         __func__, FeMesh_Type, mesh->name, filename );

      /* get the node filespace */   
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
      fileData = H5Dopen( file, "/vertices" );
      #else
      fileData = H5Dopen( file, "/vertices", H5P_DEFAULT );
      #endif
      /* get the filespace handle */
      fileSpace = H5Dget_space(fileData);

      /* get the connectivity */   
      #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
      fileData2 = H5Dopen( file, "/connectivity" );
      #else
      fileData2 = H5Dopen( file, "/connectivity", H5P_DEFAULT );
      #endif
      /* get the filespace handle */
      fileSpace2 = H5Dget_space(fileData2);
   }   
      
   count[0] = 1;
   count[1] = nDims;
   memSpace = H5Screate_simple( 2, count, NULL );
   H5Sselect_all( memSpace );
                     
   for( lNode_I = 0; lNode_I < FeMesh_GetNodeLocalSize( mesh ); lNode_I++ ) {
      gNode_I = FeMesh_NodeDomainToGlobal( mesh, lNode_I );
      
      coord = Mesh_GetVertex( mesh, lNode_I );
      
      /* select the region of dataspace to write to  */
      start[1] = 0;
      start[0] = gNode_I;
      H5Sselect_hyperslab( fileSpace, H5S_SELECT_SET, start, NULL, count, NULL );
      
      H5Dwrite( fileData, H5T_NATIVE_DOUBLE, memSpace, fileSpace, H5P_DEFAULT, coord );
   }
   
   /* Close off all our handles. */
   H5Sclose( memSpace );
   H5Dclose( fileData );
   H5Sclose( fileSpace );

   H5Sget_simple_extent_dims( fileSpace2, size, NULL );
   count[0] = 1;
   count[1] = size[1];
   memSpace = H5Screate_simple( 2, count, NULL );
   H5Sselect_all( memSpace );
                     
   for( lElement_I = 0; lElement_I < FeMesh_GetElementLocalSize(mesh); lElement_I++ ) {
      unsigned     *buf_int, *buf_int_temp;
      int*         nodeList, nodesPerEl;
      int          node_I;
      ElementType* elType   = FeMesh_GetElementType( mesh, lElement_I );
      
      gElement_I = FeMesh_ElementDomainToGlobal( mesh,lElement_I );

      /* get element nodes */
      FeMesh_GetElementNodes( mesh, lElement_I, iarray );
      nodesPerEl = IArray_GetSize( iarray );
      nodeList = IArray_GetPtr( iarray );

      buf_int      = Memory_Alloc_Array( unsigned, nodesPerEl, "FEMContext::ElementNodes" );
      buf_int_temp = Memory_Alloc_Array( unsigned, nodesPerEl, "FEMContext::ElementNodes" );
      
      for( node_I = 0; node_I < nodesPerEl; node_I++ )
         buf_int[node_I] = FeMesh_NodeDomainToGlobal( mesh, nodeList[node_I] );
      
      /* copy array before rearranging */
      memcpy( buf_int_temp, buf_int, nodesPerEl*sizeof(unsigned) );

      /*

         Underworld's internal element node ordering is 

     6 __________ 7
    / :         / \
   4 ________ 5    \
   |  :       |     \
   |  2 ------\-----3
   | /         \   /
    /           \ /
   0 ___________ 1


   but vtk's ordering for a HEXAHEDRA is 
      vtu order for hexhedra i.e VTK_HEXAHEDRA = 12

     7 __________ 6
    / :         / \
   4 ________ 5    \
   |  :       |     \
   |  3 ------\-----2
   | /         \   /
    /           \ /
   0 ___________ 1


   */


      
      /* some reordering is required to account for standard node ordering (if we're using linear quads or hexes */ 
      if( !strcmp( elType->type, BilinearElementType_Type ) || !strcmp( elType->type, TrilinearElementType_Type ) ) {
         buf_int[3] = buf_int_temp[2];
         buf_int[2] = buf_int_temp[3];
         if( nDims == 3 ) {
            buf_int[7] = buf_int_temp[6];
            buf_int[6] = buf_int_temp[7];
         }
      }
      /* select the region of dataspace to write to  */
      start[1] = 0;
      start[0] = gElement_I;
      H5Sselect_hyperslab( fileSpace2, H5S_SELECT_SET, start, NULL, count, NULL );
      
      H5Dwrite( fileData2, H5T_NATIVE_INT, memSpace, fileSpace2, H5P_DEFAULT, buf_int );
      
      Memory_Free( buf_int );
      Memory_Free( buf_int_temp );
      
   }
   
   /* Close off all our handles. */
   H5Sclose( memSpace );
   H5Dclose( fileData2 );
   H5Sclose( fileSpace2 );
   H5Fclose( file );

   /* send go-ahead from process ranked lower than me, to avoid competition writing to file */
   if( rank != nRanks - 1 ) 
      MPI_Ssend( &confirmation, 1, MPI_INT, rank + 1, FINISHED_WRITING_TAG, comm );
      
   
   Stg_Class_Delete( iarray );
}

#endif
