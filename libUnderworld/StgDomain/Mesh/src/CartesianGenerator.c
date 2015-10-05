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

#ifdef READ_HDF5
#include <hdf5.h>
#endif

#include <mpi.h>
#include <StGermain/StGermain.h>

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>

#include "types.h"
#include "shortcuts.h"
#include "Grid.h"
#include "Decomp.h"
#include "Sync.h"
#include "MeshTopology.h"
#include "IGraph.h"
#include "Mesh_ElementType.h"
#include "Mesh_HexType.h"
#include "MeshClass.h"
#include "MeshGenerator.h"
#include "Mesh_Algorithms.h"
#include "Mesh_RegularAlgorithms.h"
#include "CartesianGenerator.h"

#define OFFSET_TAG 6

/* Textual name of this class */
const Type CartesianGenerator_Type = "CartesianGenerator";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

void* _CartesianGenerator_DefaultNew( Name name ) {
   /* Variables set in this function */
	SizeT                                                    _sizeOfSelf = sizeof(CartesianGenerator);
	Type                                                            type = CartesianGenerator_Type;
	Stg_Class_DeleteFunction*                                    _delete = _CartesianGenerator_Delete;
	Stg_Class_PrintFunction*                                      _print = _CartesianGenerator_Print;
	Stg_Class_CopyFunction*                                        _copy = NULL;
	Stg_Component_DefaultConstructorFunction*        _defaultConstructor = (void* (*)(Name))_CartesianGenerator_DefaultNew;
	Stg_Component_ConstructFunction*                          _construct = _CartesianGenerator_AssignFromXML;
	Stg_Component_BuildFunction*                                  _build = _CartesianGenerator_Build;
	Stg_Component_InitialiseFunction*                        _initialise = _CartesianGenerator_Initialise;
	Stg_Component_ExecuteFunction*                              _execute = _CartesianGenerator_Execute;
	Stg_Component_DestroyFunction*                              _destroy = _CartesianGenerator_Destroy;
	AllocationType                                    nameAllocationType = NON_GLOBAL;
	MeshGenerator_SetDimSizeFunc*                         setDimSizeFunc = CartesianGenerator_SetDimSize;
	MeshGenerator_GenerateFunc*                             generateFunc = CartesianGenerator_Generate;
	CartesianGenerator_SetTopologyParamsFunc*      setTopologyParamsFunc = _CartesianGenerator_SetTopologyParams;
	CartesianGenerator_GenElementsFunc*                  genElementsFunc = _CartesianGenerator_GenElements;
	CartesianGenerator_GenFacesFunc*                        genFacesFunc = _CartesianGenerator_GenFaces;
	CartesianGenerator_GenEdgesFunc*                        genEdgesFunc = _CartesianGenerator_GenEdges;
	CartesianGenerator_GenVerticesFunc*                  genVerticesFunc = _CartesianGenerator_GenVertices;
	CartesianGenerator_GenElementVertexIncFunc*  genElementVertexIncFunc = _CartesianGenerator_GenElementVertexInc;
	CartesianGenerator_GenVolumeEdgeIncFunc*        genVolumeEdgeIncFunc = _CartesianGenerator_GenVolumeEdgeInc;
	CartesianGenerator_GenVolumeFaceIncFunc*        genVolumeFaceIncFunc = _CartesianGenerator_GenVolumeFaceInc;
	CartesianGenerator_GenFaceVertexIncFunc*        genFaceVertexIncFunc = _CartesianGenerator_GenFaceVertexInc;
	CartesianGenerator_GenFaceEdgeIncFunc*          genFaceEdgeIncFunc = _CartesianGenerator_GenFaceEdgeInc;
	CartesianGenerator_GenEdgeVertexIncFunc*        genEdgeVertexIncFunc = _CartesianGenerator_GenEdgeVertexInc;
	CartesianGenerator_GenElementTypesFunc*         genElementTypesFunc = _CartesianGenerator_GenElementTypes;
   CartesianGenerator_CalcGeomFunc*                calcGeomFunc = CartesianGenerator_CalcGeom;

	CartesianGenerator* self = _CartesianGenerator_New(  CARTESIANGENERATOR_PASSARGS  );
   return self;
}

CartesianGenerator* CartesianGenerator_New( Name name, AbstractContext* context ) {
	CartesianGenerator* self = _CartesianGenerator_DefaultNew( name );
	/* CartesianGenerator info */
   _MeshGenerator_Init( (MeshGenerator*)self, context );
	_CartesianGenerator_Init( self );
   return self;
}

CartesianGenerator* _CartesianGenerator_New(  CARTESIANGENERATOR_DEFARGS  ) {
	CartesianGenerator* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(CartesianGenerator) );
	self = (CartesianGenerator*)_MeshGenerator_New(  MESHGENERATOR_PASSARGS  );

	/* Virtual info */
	self->setTopologyParamsFunc = setTopologyParamsFunc;
	self->genElementsFunc = genElementsFunc;
	self->genFacesFunc = genFacesFunc;
	self->genEdgesFunc = genEdgesFunc;
	self->genVerticesFunc = genVerticesFunc;
	self->genElementVertexIncFunc = genElementVertexIncFunc;
	self->genVolumeEdgeIncFunc = genVolumeEdgeIncFunc;
	self->genVolumeFaceIncFunc = genVolumeFaceIncFunc;
	self->genFaceVertexIncFunc = genFaceVertexIncFunc;
	self->genFaceEdgeIncFunc = genFaceEdgeIncFunc;
	self->genEdgeVertexIncFunc = genEdgeVertexIncFunc;
	self->genElementTypesFunc = genElementTypesFunc;
  self->calcGeomFunc = calcGeomFunc; 
  self->initVtkFile = NULL;
  self->initMeshFile = NULL;


	return self;
}

void _CartesianGenerator_Init( CartesianGenerator* self ) {
	Stream*	stream;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );

	stream = Journal_Register( Info_Type, (Name)self->type  );
	Stream_SetPrintingRank( stream, 0 );

	self->comm = NULL;
	self->regular = True;
	memset( self->periodic, 0, 3 * sizeof(Bool) );
	self->maxDecompDims = 0;
	self->minDecomp = NULL;
	self->maxDecomp = NULL;
	self->shadowDepth = 1;
	self->crdMin = NULL;
	self->crdMax = NULL;

	self->vertGrid = NULL;
	self->elGrid = NULL;
	self->procGrid = NULL;
	self->origin = NULL;
	self->range = NULL;
	self->vertOrigin = NULL;
	self->vertRange = NULL;
   
   /* set load from checkpoint to false by default */
   self->readFromFile = False;

}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _CartesianGenerator_Delete( void* meshGenerator ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;

	if( self->initVtkFile ) FreeArray( self->initVtkFile );
	if( self->initMeshFile ) FreeArray( self->initMeshFile );

	/* Delete the parent. */
	_MeshGenerator_Delete( self );

}

void _CartesianGenerator_Print( void* meshGenerator, Stream* stream ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	
	/* Set the Journal for printing informations */
	Stream* meshGeneratorStream;
	meshGeneratorStream = Journal_Register( InfoStream_Type, (Name)"CartesianGeneratorStream"  );

	assert( self && Stg_CheckType( self, CartesianGenerator ) );

	/* Print parent */
	Journal_Printf( stream, "CartesianGenerator (ptr): (%p)\n", self );
	_MeshGenerator_Print( self, stream );
}

void _CartesianGenerator_AssignFromXML( void* meshGenerator, Stg_ComponentFactory* cf, void* data ) {
	CartesianGenerator*		self = (CartesianGenerator*)meshGenerator;
	Dictionary*					dict;
	Dictionary_Entry_Value*	tmp;
	char*							rootKey;
	Dictionary_Entry_Value*	sizeList;
	Dictionary_Entry_Value	*minList, *maxList;
	unsigned          *minDecomp, *maxDecomp;
	double						*crdMin, *crdMax;
	unsigned*					size;
	unsigned						shadowDepth;
	Stream*						stream;
	Stream*						errorStream = Journal_Register( Error_Type, (Name)self->type  );
	unsigned						d_i;
   AbstractContext*        context;	
#ifdef READ_HDF5
	hid_t file, fileData;
#endif
	char *_vtk_file = NULL;
	
	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( cf );

	/* Call parent construct. */
	_MeshGenerator_AssignFromXML( self, cf, data );

    context = self->context;

	/* Rip out the components structure as a dictionary. */
	dict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( cf->componentDict, (Dictionary_Entry_Key)self->name )  );

	/* Read the sizes. */
	sizeList = Dictionary_Get( dict, (Dictionary_Entry_Key)"size" );
	assert( sizeList );
	assert( Dictionary_Entry_Value_GetCount( sizeList ) >= self->nDims  );
	size = Memory_Alloc_Array_Unnamed( unsigned, self->nDims );
	for( d_i = 0; d_i < self->nDims; d_i++ ) {
		tmp = Dictionary_Entry_Value_GetElement( sizeList, d_i );
		rootKey = Dictionary_Entry_Value_AsString( tmp );

		if( !Stg_StringIsNumeric( (char *)rootKey )  )
			tmp = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)rootKey );
		size[d_i] = Dictionary_Entry_Value_AsUnsignedInt( tmp  );
        Journal_Firewall( size[d_i]>0, errorStream,
            "Error: in %s: Element resolution of %d invalid.  Must be >0. Check your input.",
            __func__, size[d_i] ); 

	}

	/* Read decomposition restrictions. */
	self->maxDecompDims = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"maxDecomposedDims", 0  );

	minList = Dictionary_Get( dict, (Dictionary_Entry_Key)"minDecompositions" );
	if( minList  ) {
		minDecomp = AllocArray( unsigned, self->nDims );
		for( d_i = 0; d_i < self->nDims; d_i++ ) {
			tmp = Dictionary_Entry_Value_GetElement( minList, d_i );
			rootKey = Dictionary_Entry_Value_AsString( tmp );

			if( !Stg_StringIsNumeric( (char *)rootKey )  )
				tmp = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)rootKey );
			minDecomp[d_i] = Dictionary_Entry_Value_AsUnsignedInt( tmp  );
		}
	}
	else
		minDecomp = NULL;

	if( minDecomp ) memcpy(self->minDecomp, minDecomp, self->nDims*sizeof(unsigned) );

	maxList = Dictionary_Get( dict, (Dictionary_Entry_Key)"maxDecompositions" );
	if( maxList  ) {
		maxDecomp = AllocArray( unsigned, self->nDims );
		for( d_i = 0; d_i < self->nDims; d_i++ ) {
			tmp = Dictionary_Entry_Value_GetElement( maxList, d_i );
			rootKey = Dictionary_Entry_Value_AsString( tmp );

			if( !Stg_StringIsNumeric( (char *)rootKey )  )
				tmp = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)rootKey );
			maxDecomp[d_i] = Dictionary_Entry_Value_AsUnsignedInt( tmp  );
		}
	}
	else
		maxDecomp = NULL;

	if( maxDecomp ) memcpy(self->maxDecomp, maxDecomp, self->nDims*sizeof(unsigned) );

	/* Initial setup. */
	self->elGrid = Grid_New();
	Grid_SetNumDims( self->elGrid, self->nDims );
	Grid_SetSizes( self->elGrid, size );

        /* Contact stuff. */
        self->contactDepth[0][0] = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"contactDepth-left", 0  );
        self->contactDepth[0][1] = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"contactDepth-right", 0  );
        self->contactDepth[1][0] = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"contactDepth-bottom", 0  );
        self->contactDepth[1][1] = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"contactDepth-top", 0  );
        self->contactDepth[2][0] = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"contactDepth-back", 0  );
        self->contactDepth[2][1] = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"contactDepth-front", 0  );
        self->contactGeom[0] = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"contactGeometry-x", 0.0  );
        self->contactGeom[1] = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"contactGeometry-y", 0.0  );
        self->contactGeom[2] = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"contactGeometry-z", 0.0  );

	/* Read geometry. */
	minList = Dictionary_Get( dict, (Dictionary_Entry_Key)"minCoord"  );
	maxList = Dictionary_Get( dict, (Dictionary_Entry_Key)"maxCoord" );
	if( minList && maxList ) {
		assert( Dictionary_Entry_Value_GetCount( minList ) >= self->nDims );
		assert( Dictionary_Entry_Value_GetCount( maxList ) >= self->nDims  );
		crdMin = Memory_Alloc_Array_Unnamed( double, 3 );
		crdMax = Memory_Alloc_Array_Unnamed( double, 3 );
		for( d_i = 0; d_i < self->nDims; d_i++ ) {
		   double maxVal;
			tmp = Dictionary_Entry_Value_GetElement( minList, d_i );
			rootKey = Dictionary_Entry_Value_AsString( tmp );

			if( !Stg_StringIsNumeric( (char *)rootKey )  )
				tmp = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)rootKey );
			crdMin[d_i] = Dictionary_Entry_Value_AsDouble( tmp  );

			tmp = Dictionary_Entry_Value_GetElement( maxList, d_i );
			rootKey = Dictionary_Entry_Value_AsString( tmp );

			if( !Stg_StringIsNumeric( (char *)rootKey )  )
				tmp = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)rootKey );
			crdMax[d_i] = Dictionary_Entry_Value_AsDouble( tmp );
			/* test to ensure provided domain is valid */
			maxVal =  (abs(crdMax[d_i]) > abs(crdMin[d_i])) ? abs(crdMax[d_i]) : abs(crdMin[d_i]);
			if( maxVal == 0  ) maxVal = 1;  /* if maxVal is zero, then both numbers must be zero, set to one as next test will fail */
         Journal_Firewall( ( ( (crdMax[d_i] - crdMin[d_i])/maxVal) > 1E-10 || d_i==J_AXIS), errorStream,
                     "\n\nError in %s for %s '%s'\n\n"
                     "Dimension of domain (min = %f, max = %f) for component number %u is not valid.\n\n",
                     __func__, self->type, self->name,
                     crdMin[d_i], crdMax[d_i], d_i);
		}

		if( context && self->context->loadFromCheckPoint  ) {
         char*   meshReadFileName;
         char*   meshReadFileNamePart;
         self->readFromFile = True;
#ifdef READ_HDF5
         hid_t   attrib_id, group_id;
         herr_t  status;

         meshReadFileNamePart = Context_GetCheckPointReadPrefixString( context );

         /*
            The following is EVIL code. As we only checkpoint deforming meshes every timestep we need to query if the mesh is deforming.
            Unfortunately the only way we can tell at the AssignFromXML phase of the code is by detecting if the Underworld_EulerDeform plugin
            (the plugin responsible for deforming the mesh) is active.
            So the following logic is:
               if the Underworld_EulerDeform is loaded:
                  read the mesh file at appropriate timestep
               else:
                  read the mesh file at timestep 0 
         */
         /** assumption: that the generator only generates one mesh, or that the information in that mesh's checkpoint
         file is valid for all meshes being generated TODO: generalise **/
         if( Stg_ObjectList_Get( context->plugins->modules, "Underworld_EulerDeform" ) ) {
            Stg_asprintf( &meshReadFileName, "%sMesh.%s.%.5u.h5", meshReadFileNamePart, self->meshes[0]->name, self->context->restartTimestep );
         } else {
            Stg_asprintf( &meshReadFileName, "%sMesh.%s.%.5u.h5", meshReadFileNamePart, self->meshes[0]->name, 0 );
         }
	      
	      /** Read in minimum coord. */
	      file = H5Fopen( meshReadFileName, H5F_ACC_RDONLY, H5P_DEFAULT );
	      
	      if( !file ) {
            /** file not found, so don't load from checkpoint */
            self->readFromFile = False;
				Journal_Printf( errorStream, 
					"Warning - Couldn't find checkpoint mesh file with filename \"%s\".\n", 
					meshReadFileName );
			}
			else {
         int res[self->nDims];

         #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
	         fileData = H5Dopen( file, "/min" );
         #else
	         fileData = H5Dopen( file, "/min", H5P_DEFAULT );
         #endif
	         H5Dread( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, crdMin );
	         H5Dclose( fileData );
	         
	         /** Read in maximum coord. */
         #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
	         fileData = H5Dopen( file, "/max" );
         #else
	         fileData = H5Dopen( file, "/max", H5P_DEFAULT );
         #endif
	         H5Dread( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, crdMax );
	         H5Dclose( fileData );

            /** get the file attributes  */
         #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Gopen_vers == 1
            group_id  = H5Gopen(file, "/");
            attrib_id = H5Aopen_name(group_id, "checkpoint file version");
         #else
            group_id  = H5Gopen(file, "/", H5P_DEFAULT);
            attrib_id = H5Aopen(group_id, "checkpoint file version", H5P_DEFAULT);
         #endif

         /** check for correct mesh size */
      
         #if H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8
            attrib_id = H5Aopen_name(group_id, "mesh resolution");
         #else
            attrib_id = H5Aopen(group_id, "mesh resolution", H5P_DEFAULT);
         #endif
         status = H5Aread(attrib_id, H5T_NATIVE_INT, &res);
         H5Aclose(attrib_id);
   
         if(self->nDims == 2){
            if( !( (size[0] == res[0]) && (size[1] == res[1]) )){
               if (context->interpolateRestart)
                  self->readFromFile = False;
               else
                  Journal_Firewall( 
                     NULL, 
                     errorStream,
                     "\n\nError in %s for %s '%s'\n"
                     "Size of mesh (%u,%u) for checkpoint file (%s) does not correspond to simulation mesh size (%u,%u).\n\n"
                     "If you would like to interpolate checkpoint data to simulation mesh size\n"
                     "    please re-launch using '--interpolateRestart=1' flag\n\n", 
                     __func__, self->type, self->name, 
                     (unsigned int) res[0], (unsigned int) res[1],
                     meshReadFileName,
                     (unsigned int) size[0], (unsigned int) size[1]);
            }
         } else {
            if( !( (size[0] == res[0]) && (size[1] == res[1]) && (size[2] == res[2]) )){
               if (context->interpolateRestart)
                  self->readFromFile = False;
               else
                  Journal_Firewall( 
                     NULL, 
                     errorStream,
                     "\n\nError in %s for %s '%s'\n"
                     "Size of mesh (%u,%u,%u) for checkpoint file (%s) does not correspond to simulation mesh size (%u,%u,%u).\n\n"
                     "If you would like to interpolate checkpoint data to simulation mesh size\n"
                     "    please re-launch using '--interpolateRestart=1' flag\n\n", 
                     __func__, self->type, self->name, 
                     (unsigned int) res[0], (unsigned int) res[1], (unsigned int) res[2],
                     meshReadFileName,
                     (unsigned int) size[0], (unsigned int) size[1], (unsigned int) size[2]);
               }
            }

         H5Gclose(group_id);
         H5Fclose( file );

         }

#else
         Journal_Firewall(!context->interpolateRestart,  
                     errorStream,"\n\n Interpolation restart not supported for ASCII checkpoint files \n\n");

         meshReadFileNamePart = Context_GetCheckPointReadPrefixString( context );

         // Evil plugin code to determine which mesh file to load
         if( Stg_ObjectList_Get( context->plugins->modules, "Underworld_EulerDeform" ) ) {
            Stg_asprintf( &meshReadFileName, "%sMesh.%s.%.5u.dat", meshReadFileNamePart, self->meshes[0]->name, self->context->restartTimestep );
         } else {
            Stg_asprintf( &meshReadFileName, "%sMesh.%s.%.5u.dat", meshReadFileNamePart, self->meshes[0]->name, 0 );
         }
	        
			FILE* meshFile = fopen( meshReadFileName, "r" );	
			/*Journal_Firewall( 
				meshFile != 0, 
				errorStream, 
				"Error in %s - Couldn't find checkpoint mesh file with filename \"%s\" - aborting.\n", 
				__func__,  
				meshReadFileName );*/

			if( meshFile == 0 ) {
            /* file not found, so don't load from checkpoint */
            self->readFromFile = False;            
				Journal_Printf( errorStream, 
					"Warning - Couldn't find checkpoint mesh file with filename \"%s\".\n", 
					meshReadFileName );
			}
			else {
				/* Read min and max coords from file */
		      if(self->nDims==2)
                  fscanf( meshFile, "Min: %lg %lg 0\n", &crdMin[0], &crdMin[1] );
            else
                  fscanf( meshFile, "Min: %lg %lg %lg\n", &crdMin[0], &crdMin[1], &crdMin[2] );
		      if(self->nDims==2)
                  fscanf( meshFile, "Max: %lg %lg 0\n", &crdMax[0], &crdMax[1] );
            else
                  fscanf( meshFile, "Max: %lg %lg %lg\n", &crdMax[0], &crdMax[1], &crdMax[2] );
            
				fclose( meshFile );
			}
#endif
				
                        self->initMeshFile=strdup( meshReadFileName );
			Memory_Free( meshReadFileName );
			Memory_Free( meshReadFileNamePart );

		}	
		   
		/* Initial setup. */
		CartesianGenerator_SetGeometryParams( self, crdMin, crdMax );

		/* Free coordinate arrays. */
		FreeArray( crdMin );
		FreeArray( crdMax );
	}

	/* Read and set shadow depth. */
	shadowDepth = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"shadowDepth", 1  );
	CartesianGenerator_SetShadowDepth( self, shadowDepth );

	/* Read regular flag. */
	self->regular = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"regular", True  );

	/* is there an initial vtk file, copy the file name */
	_vtk_file = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"initial_vtk_file", NULL  );
  if( _vtk_file != NULL ) {
    /* doing processing here because the _CartesianGenerator_Init is called BEFORE the AssignFromXML
     * very TODO:Fix the this ordering issue
     */
    int rank;
    int slength = strlen(_vtk_file);
    self->initVtkFile = Memory_Alloc_Array_Unnamed( char, slength+1 );
    strcpy( self->initVtkFile, _vtk_file );
    // test if proc 0 can open the file
    MPI_Comm_rank( self->mpiComm, &rank );
    if( rank == 0 ) {
      FILE *fptr=fopen(self->initVtkFile,"r");
      if( fptr==NULL ) {
        printf("Error in function %s\n"
        "Trying to open the vtk file \'%s\' but can't - does it exist?\n", __func__, self->initVtkFile );
        exit(EXIT_FAILURE);
      }
      fclose(fptr);
    }
	} else {
    self->initVtkFile = NULL;
  }

	/* Read periodic flags. */
	self->periodic[0] = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"periodic_x", False  );
	self->periodic[1] = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"periodic_y", False  );
	self->periodic[2] = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"periodic_z", False  );

	/* Read a general dictionary flag for which processor to watch. */
	stream = Journal_Register( Info_Type, (Name)self->type  );
	Stream_SetPrintingRank( stream, Dictionary_GetUnsignedInt_WithDefault( cf->rootDict, "rankToWatch", 0 ) );

	/* Free stuff. */
	FreeArray( size );
}

void _CartesianGenerator_Build( void* meshGenerator, void* data ) {
  CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;

	_MeshGenerator_Build( meshGenerator, data );

	/* need to make tmp storage here for the CartesianGenerator_SetTopologyParam function
		 the algorithm 1st frees all memory and rebuilds the Topology params, so tmpSizes & min/maxDecomp 
		 stores the initial user input
	 */
	unsigned *tmpSizes = Memory_Alloc_Array( unsigned, self->nDims, "Grid::sizes" );
	unsigned *minDecomp = NULL;
	unsigned *maxDecomp = NULL;
	unsigned maxDecompDims = self->maxDecompDims;

	memcpy( tmpSizes, self->elGrid->sizes, self->nDims*sizeof(unsigned) ); 

	if( self->minDecomp ) {
		minDecomp = Memory_Alloc_Array( unsigned, self->nDims, "Grid::sizes" );
		memcpy( minDecomp, self->minDecomp, self->nDims*sizeof(unsigned) ); 
	}
	if( self->maxDecomp) {
		maxDecomp = Memory_Alloc_Array( unsigned, self->nDims, "Grid::sizes" );
		memcpy( maxDecomp, self->maxDecomp, self->nDims*sizeof(unsigned) ); 
	}

	CartesianGenerator_SetTopologyParams( self, tmpSizes, maxDecompDims, minDecomp, maxDecomp );

	FreeArray( tmpSizes );
	FreeArray( minDecomp );
	FreeArray( maxDecomp );

	
}

void _CartesianGenerator_Initialise( void* meshGenerator, void* data ) {
}

void _CartesianGenerator_Execute( void* meshGenerator, void* data ) {
}

void _CartesianGenerator_Destroy( void* meshGenerator, void* data ) {
   CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
   CartesianGenerator_Destruct( self );

   /* Destroy Parent */
   _MeshGenerator_Destroy( self, data );
}

void CartesianGenerator_SetDimSize( void* meshGenerator, unsigned nDims ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );

	_MeshGenerator_SetDimSize( self, nDims );

	self->minDecomp = MemRearray( self->minDecomp, unsigned, self->nDims, CartesianGenerator_Type );
	memset( self->minDecomp, 0, nDims * sizeof(unsigned) );
	self->maxDecomp = MemRearray( self->maxDecomp, unsigned, self->nDims, CartesianGenerator_Type );
	memset( self->maxDecomp, 0, nDims * sizeof(unsigned) );
}

void CartesianGenerator_Generate( void* meshGenerator, void* _mesh, void* data ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type );
	Mesh*			mesh = (Mesh* )_mesh;
	Grid**			grid;
	unsigned		*localRange, *localOrigin;
	Bool			*periodic;
	
    /* this generator doesn't rely on a parent mesh */
    mesh->parentMesh = mesh;
	/* Sanity check. */
	assert( self );
	assert( !self->elGrid || mesh );

	Journal_Printf( stream, "Cartesian generator: '%s'\n", self->name );
	Stream_Indent( stream );

	/* If we havn't been given anything, don't do anything. */
	if( self->elGrid ) {
		unsigned	d_i;

		Journal_Printf( stream, "Target mesh: '%s'\n", mesh->name );
		Journal_Printf( stream, "Global element size: %d", self->elGrid->sizes[0] );
		for( d_i = 1; d_i < self->elGrid->nDims; d_i++ )
			Journal_Printf( stream, "x%d", self->elGrid->sizes[d_i] );
		Journal_Printf( stream, "\n" );
		Journal_Printf( stream, "Local offset of rank %d: %d", stream->_printingRank, self->origin[0] );
		for( d_i = 1; d_i < self->elGrid->nDims; d_i++ )
			Journal_Printf( stream, "x%d", self->origin[d_i] );
		Journal_Printf( stream, "\n" );
		Journal_Printf( stream, "Local range of rank %d: %d", stream->_printingRank, self->range[0] );
		for( d_i = 1; d_i < self->elGrid->nDims; d_i++ )
			Journal_Printf( stream, "x%d", self->range[d_i] );
		Journal_Printf( stream, "\n" );

		/* Fill topological values. */
		MeshTopology_SetComm( mesh->topo, self->comm );
		MeshTopology_SetNumDims( mesh->topo, self->elGrid->nDims );
		CartesianGenerator_GenTopo( self, (IGraph*)mesh->topo );

		/* Fill geometric values. */
		CartesianGenerator_GenGeom( self, mesh, data );

		/* Fill element types. */
		CartesianGenerator_GenElementTypes( self, mesh );
	}
	else {
		MeshTopology_SetNumDims( mesh->topo, 0 );
	}

	/* Add extensions to the mesh and fill with cartesian information. */
	mesh->vertGridId = ExtensionManager_Add( mesh->info, (Name)"vertexGrid", sizeof(Grid*) );
	grid = (Grid** )Mesh_GetExtension(mesh,Grid**,mesh->vertGridId);
	*grid = Grid_New( );
	Grid_SetNumDims( *grid, self->vertGrid->nDims );
	Grid_SetSizes( *grid, self->vertGrid->sizes );

	mesh->elGridId = ExtensionManager_Add( mesh->info, (Name)"elementGrid", sizeof(Grid*) );
	grid = (Grid** )Mesh_GetExtension(mesh, Grid**, mesh->elGridId );
	*grid = Grid_New( );
	Grid_SetNumDims( *grid, self->elGrid->nDims );
	Grid_SetSizes( *grid, self->elGrid->sizes );

	mesh->localOriginId = ExtensionManager_AddArray( mesh->info, "localOrigin", sizeof(unsigned), Mesh_GetDimSize( mesh ) );
	localOrigin = Mesh_GetExtension(mesh,unsigned*,mesh->localOriginId);
	memcpy( localOrigin, self->origin, Mesh_GetDimSize( mesh ) * sizeof(unsigned) );

	mesh->localRangeId = ExtensionManager_AddArray( mesh->info, "localRange", sizeof(unsigned), Mesh_GetDimSize( mesh ) );
	localRange = Mesh_GetExtension( mesh, unsigned*, mesh->localRangeId );
	memcpy( localRange, self->range, Mesh_GetDimSize( mesh ) * sizeof(unsigned) );

	mesh->periodicId = ExtensionManager_AddArray( mesh->info, "periodic", sizeof(Bool), 3 );
	periodic = Mesh_GetExtension(mesh, Bool*, mesh->periodicId );
	memcpy( periodic, self->periodic, 3 * sizeof(Bool) );
    
    mesh->isRegular = self->regular;

	Stream_UnIndent( stream );
}

void _CartesianGenerator_SetTopologyParams( void* meshGenerator, unsigned* sizes, unsigned maxDecompDims, unsigned* minDecomp, unsigned* maxDecomp ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	unsigned		d_i;

	/* Sanity check. */
	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( !self->nDims || sizes );
	assert( self->nDims <= 3 );

	/* Kill everything we have, topologically. */
	CartesianGenerator_DestructTopology( self );

	/* Don't continue if we have nothing. */
	if( !self->nDims )
		return;

	/* Set the parameters. */
	self->elGrid = Grid_New();
	Grid_SetNumDims( self->elGrid, self->nDims );
	Grid_SetSizes( self->elGrid, sizes );

	self->vertGrid = Grid_New();
	Grid_SetNumDims( self->vertGrid, self->nDims );
	for( d_i = 0; d_i < self->nDims; d_i++ )
		sizes[d_i]++;
	Grid_SetSizes( self->vertGrid, sizes );
	for( d_i = 0; d_i < self->nDims; d_i++ )
		sizes[d_i]--;

	if( minDecomp )
		memcpy( self->minDecomp, minDecomp, self->nDims * sizeof(unsigned) );
	else
		memset( self->minDecomp, 0, self->nDims * sizeof(unsigned) );
	if( maxDecomp )
		memcpy( self->maxDecomp, maxDecomp, self->nDims * sizeof(unsigned) );
	else
		memset( self->maxDecomp, 0, self->nDims * sizeof(unsigned) );
	self->maxDecompDims = maxDecompDims;

	/* As soon as we know the topology, we can decompose. */
	CartesianGenerator_BuildDecomp( self );
}

void _CartesianGenerator_GenElements( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
	Grid*			grid;
	unsigned		nEls;
	unsigned*		els;
	unsigned*		dimInds;
	unsigned		d_i, e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );

	Journal_Printf( stream, "Generating elements...\n" );
	Stream_Indent( stream );

	grid = Grid_New();
	Grid_SetNumDims( grid, self->elGrid->nDims );
	Grid_SetSizes( grid, self->range );

	nEls = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls *= grid->sizes[d_i];
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->elGrid->nDims );

	for( e_i = 0; e_i < nEls; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( self->elGrid, dimInds );
	}

	IGraph_SetLocalElements( topo, grid->nDims, nEls, els );
	FreeArray( els );
	FreeArray( dimInds );
	FreeObject( grid );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenVertices( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	/*@
		Performs parallel initialisation of 'topo' vertex grid

		@*/
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
	unsigned		rank;
	Grid*			globalGrid;
	Grid*			grid;
	unsigned		local_vert_num; /* local number of vertices */
	unsigned		nLocals, *locals;
	unsigned		nRemotes;
	unsigned		*dimInds, *rankInds;
	unsigned		d_i, e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );

	Journal_Printf( stream, "Generating vertices...\n" );
	Stream_Indent( stream );

	MPI_Comm_rank( Comm_GetMPIComm( self->comm ), (int*)&rank );

	globalGrid = self->vertGrid;

	grid = Grid_New();
	Grid_SetNumDims( grid, self->elGrid->nDims );
	Grid_SetSizes( grid, self->vertRange );

	/* compute the local number of vertices on this proc */
	local_vert_num = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		local_vert_num *= grid->sizes[d_i];
	locals = Memory_Alloc_Array_Unnamed( unsigned, local_vert_num );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->elGrid->nDims );
	rankInds = Memory_Alloc_Array_Unnamed( unsigned, self->elGrid->nDims );
	Grid_Lift( self->procGrid, rank, rankInds );

	/* calculate local vertex grid position */
	nLocals = 0;
	nRemotes = 0;
	for( e_i = 0; e_i < local_vert_num; e_i++ ) {
		/* calculate local grid position -> dimInds */
		Grid_Lift( grid, e_i, dimInds );

		/* offset dimInds by the processor's vertex origin */
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->vertOrigin[d_i];

		/* calculate the global grid node Id for given vertex */
		locals[(nLocals)++] = Grid_Project( globalGrid, dimInds );
	}

	/* save locals information on topo */
	IGraph_SetElements( topo, 0, nLocals, locals );
	FreeArray( locals );
	FreeArray( dimInds );
	FreeArray( rankInds );
	FreeObject( grid );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenEdges( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( self->elGrid->nDims >= 2 );
	assert( self->elGrid->nDims <= 3 );

	Journal_Printf( stream, "Generating edges...\n" );
	Stream_Indent( stream );

	if( self->elGrid->nDims == 2 )
		CartesianGenerator_GenEdges2D( self, topo, grids );
	else
		CartesianGenerator_GenEdges3D( self, topo, grids );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenFaces( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
	Grid*			globalGrid;
	unsigned		nGlobalEls[2];
	Grid*			grid;
	unsigned		nEls[3];
	unsigned*		els;
	unsigned*		dimInds;
	unsigned		d_i, e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );
	assert( self->elGrid->nDims == 3 );

	Journal_Printf( stream, "Generating faces...\n" );
	Stream_Indent( stream );

	globalGrid = Grid_New();
	Grid_SetNumDims( globalGrid, self->elGrid->nDims );
	self->elGrid->sizes[2]++;
	Grid_SetSizes( globalGrid, self->elGrid->sizes );
	self->elGrid->sizes[2]--;

	grid = Grid_New();
	Grid_SetNumDims( grid, self->elGrid->nDims );
	self->range[2]++;
	Grid_SetSizes( grid, self->range );
	self->range[2]--;

	nEls[0] = grid->sizes[0];
	nGlobalEls[0] = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[0] *= grid->sizes[d_i];
		nGlobalEls[0] *= globalGrid->sizes[d_i];
	}
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls[0] );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->elGrid->nDims );

	for( e_i = 0; e_i < nEls[0]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( globalGrid, dimInds );
	}

	self->elGrid->sizes[1]++;
	Grid_SetSizes( globalGrid, self->elGrid->sizes );
	self->elGrid->sizes[1]--;

	self->range[1]++;
	Grid_SetSizes( grid, self->range );
	self->range[1]--;

	nEls[1] = grid->sizes[0];
	nGlobalEls[1] = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[1] *= grid->sizes[d_i];
		nGlobalEls[1] *= globalGrid->sizes[d_i];
	}
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] );

	for( e_i = 0; e_i < nEls[1]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + e_i] = nGlobalEls[0] + Grid_Project( globalGrid, dimInds );
	}

	self->elGrid->sizes[0]++;
	Grid_SetSizes( globalGrid, self->elGrid->sizes );
	self->elGrid->sizes[0]--;

	self->range[0]++;
	Grid_SetSizes( grid, self->range );
	self->range[0]--;

	nEls[2] = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls[2] *= grid->sizes[d_i];
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] + nEls[2] );

	for( e_i = 0; e_i < nEls[2]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + nEls[1] + e_i] = nGlobalEls[0] + nGlobalEls[1] + Grid_Project( globalGrid, dimInds );
	}

	IGraph_SetElements( topo, MT_FACE, nEls[0] + nEls[1] + nEls[2], els );

	FreeArray( dimInds );
	FreeArray( els );
	FreeObject( grid );
	FreeObject( globalGrid );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenElementVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
	int			nDomainEls;
	unsigned*		incEls;
	unsigned*		dimInds;
	unsigned		vertsPerEl;
	const Sync*		sync;
	unsigned		e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );

	Journal_Printf( stream, "Generating element-vertex incidence...\n" );
	Stream_Indent( stream );

	vertsPerEl = (topo->nDims == 1) ? 2 : (topo->nDims == 2) ? 4 : 8;

	sync = IGraph_GetDomain( topo, topo->nDims );
	nDomainEls = Sync_GetNumDomains( sync );
	incEls = Memory_Alloc_Array_Unnamed( unsigned, vertsPerEl );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < nDomainEls; e_i++ ) {
		unsigned	gInd = Sync_DomainToGlobal( sync, e_i );
		unsigned	curNode = 0;

		Grid_Lift( grids[topo->nDims][0], gInd, dimInds );

		incEls[curNode++] = Grid_Project( grids[0][0], dimInds );

		dimInds[0]++;
		incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
		dimInds[0]--;

		if( topo->nDims >= 2 ) {
			dimInds[1]++;
			incEls[curNode++] = Grid_Project( grids[0][0], dimInds );

			dimInds[0]++;
			incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]--;
			dimInds[1]--;

			if( topo->nDims >= 3 ) {
				dimInds[2]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );

				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]--;

				dimInds[1]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );

				dimInds[0]++;
				incEls[curNode++] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]--;
				dimInds[1]--;
				dimInds[2]--;
			}
		}

		CartesianGenerator_MapToDomain( self, (Sync*)IGraph_GetDomain( topo, 0 ), 
						vertsPerEl, incEls );
		IGraph_SetIncidence( topo, topo->nDims, e_i, MT_VERTEX, vertsPerEl, incEls );
	}

	FreeArray( incEls );
	FreeArray( dimInds );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenVolumeEdgeInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
	unsigned		nIncEls;
	unsigned*		incEls;
	unsigned*		dimInds;
	unsigned		e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );
	assert( topo->nDims >= 3 );

	Journal_Printf( stream, "Generating volume-edge incidence...\n" );
	Stream_Indent( stream );

	incEls = Memory_Alloc_Array_Unnamed( unsigned, 12 );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < topo->remotes[3]->nDomains; e_i++ ) {
		unsigned	gInd = Sync_DomainToGlobal( topo->remotes[3], e_i );

		nIncEls = 12;
		Grid_Lift( grids[topo->nDims][0], gInd, dimInds );

		incEls[0] = Grid_Project( grids[1][0], dimInds );

		dimInds[1]++;
		incEls[1] = Grid_Project( grids[1][0], dimInds );
		dimInds[1]--;

		incEls[2] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;

		dimInds[0]++;
		incEls[3] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;
		dimInds[0]--;

		dimInds[2]++;
		incEls[4] = Grid_Project( grids[1][0], dimInds );

		dimInds[1]++;
		incEls[5] = Grid_Project( grids[1][0], dimInds );
		dimInds[1]--;

		incEls[6] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;

		dimInds[0]++;
		incEls[7] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;
		dimInds[0]--;
		dimInds[2]--;

		incEls[8] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;

		dimInds[0]++;
		incEls[9] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;
		dimInds[0]--;

		dimInds[1]++;
		incEls[10] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;

		dimInds[0]++;
		incEls[11] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;
		dimInds[0]--;
		dimInds[1]--;

		CartesianGenerator_MapToDomain( self, (Sync*)IGraph_GetDomain( topo, 1 ), 
						nIncEls, incEls );
		IGraph_SetIncidence( topo, topo->nDims, e_i, 1, nIncEls, incEls );
	}

	FreeArray( incEls );
	FreeArray( dimInds );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenVolumeFaceInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
	unsigned		nIncEls;
	unsigned*		incEls;
	unsigned*		dimInds;
	unsigned		e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );
	assert( topo->nDims >= 3 );

	Journal_Printf( stream, "Generating volume-face incidence...\n" );
	Stream_Indent( stream );

	incEls = Memory_Alloc_Array_Unnamed( unsigned, 6 );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < topo->remotes[MT_VOLUME]->nDomains; e_i++ ) {
		unsigned	gInd = Sync_DomainToGlobal( topo->remotes[MT_VOLUME], e_i );

		nIncEls = 6;
		Grid_Lift( grids[topo->nDims][0], gInd, dimInds );

		incEls[0] = Grid_Project( grids[2][0], dimInds );

		dimInds[2]++;
		incEls[1] = Grid_Project( grids[2][0], dimInds );
		dimInds[2]--;

		incEls[2] = Grid_Project( grids[2][1], dimInds ) + grids[2][0]->nPoints;

		dimInds[1]++;
		incEls[3] = Grid_Project( grids[2][1], dimInds ) + grids[2][0]->nPoints;
		dimInds[1]--;

		incEls[4] = Grid_Project( grids[2][2], dimInds ) + grids[2][0]->nPoints + grids[2][1]->nPoints;

		dimInds[0]++;
		incEls[5] = Grid_Project( grids[2][2], dimInds ) + grids[2][0]->nPoints + grids[2][1]->nPoints;
		dimInds[0]--;

		CartesianGenerator_MapToDomain( self, (Sync*)IGraph_GetDomain( topo, 2 ), 
						nIncEls, incEls );
		IGraph_SetIncidence( topo, topo->nDims, e_i, 2, nIncEls, incEls );
	}

	FreeArray( incEls );
	FreeArray( dimInds );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenFaceVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
	unsigned		nIncEls;
	unsigned*		incEls;
	unsigned*		dimInds;
	unsigned		e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );
	assert( topo->nDims >= 2 );

	Journal_Printf( stream, "Generating face-vertex incidence...\n" );
	Stream_Indent( stream );

	incEls = Memory_Alloc_Array_Unnamed( unsigned, 4 );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < topo->remotes[MT_FACE]->nDomains; e_i++ ) {
		unsigned	gInd = Sync_DomainToGlobal( topo->remotes[MT_FACE], e_i );

		nIncEls = 4;

		if( gInd < grids[2][0]->nPoints ) {
			Grid_Lift( grids[2][0], gInd, dimInds );

			incEls[0] = Grid_Project( grids[0][0], dimInds );

			dimInds[0]++;
			incEls[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]--;

			dimInds[1]++;
			incEls[2] = Grid_Project( grids[0][0], dimInds );

			dimInds[0]++;
			incEls[3] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]--;
			dimInds[1]--;
		}
		else if( gInd < grids[2][0]->nPoints + grids[2][1]->nPoints ) {
			assert( topo->nDims >= 3 );

			Grid_Lift( grids[2][1], gInd - grids[2][0]->nPoints, dimInds );

			incEls[0] = Grid_Project( grids[0][0], dimInds );

			dimInds[0]++;
			incEls[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]--;

			dimInds[2]++;
			incEls[2] = Grid_Project( grids[0][0], dimInds );

			dimInds[0]++;
			incEls[3] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]--;
			dimInds[2]--;
		}
		else {
			assert( gInd < grids[2][0]->nPoints + grids[2][1]->nPoints + grids[2][2]->nPoints );
			assert( topo->nDims >= 3 );

			Grid_Lift( grids[2][2], gInd - grids[2][0]->nPoints - grids[2][1]->nPoints, dimInds );

			incEls[0] = Grid_Project( grids[0][0], dimInds );

			dimInds[1]++;
			incEls[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]--;

			dimInds[2]++;
			incEls[2] = Grid_Project( grids[0][0], dimInds );

			dimInds[1]++;
			incEls[3] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]--;
			dimInds[2]--;
		}

		CartesianGenerator_MapToDomain( self, (Sync*)IGraph_GetDomain( topo, 0 ), 
						nIncEls, incEls );
		IGraph_SetIncidence( topo, 2, e_i, 0, nIncEls, incEls );
	}

	FreeArray( incEls );
	FreeArray( dimInds );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenFaceEdgeInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
	unsigned		nIncEls;
	unsigned*		incEls;
	unsigned*		dimInds;
	unsigned		e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );
	assert( topo->nDims >= 2 );

	Journal_Printf( stream, "Generating face-edge incidence...\n" );
	Stream_Indent( stream );

	incEls = Memory_Alloc_Array_Unnamed( unsigned, 4 );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < topo->remotes[MT_FACE]->nDomains; e_i++ ) {
		unsigned	gInd = Sync_DomainToGlobal( topo->remotes[MT_FACE], e_i );

		nIncEls = 4;

		if( gInd < grids[2][0]->nPoints ) {
			Grid_Lift( grids[2][0], gInd, dimInds );

			incEls[0] = Grid_Project( grids[1][0], dimInds );

			dimInds[1]++;
			incEls[1] = Grid_Project( grids[1][0], dimInds );
			dimInds[1]--;

			incEls[2] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;

			dimInds[0]++;
			incEls[3] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;
			dimInds[0]--;
		}
		else if( gInd < grids[2][0]->nPoints + grids[2][1]->nPoints ) {
			assert( topo->nDims >= 3 );

			Grid_Lift( grids[2][1], gInd - grids[2][0]->nPoints, dimInds );

			incEls[0] = Grid_Project( grids[1][0], dimInds );

			dimInds[2]++;
			incEls[1] = Grid_Project( grids[1][0], dimInds );
			dimInds[2]--;

			incEls[2] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;

			dimInds[0]++;
			incEls[3] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;
			dimInds[0]--;
		}
		else {
			assert( gInd < grids[2][0]->nPoints + grids[2][1]->nPoints + grids[2][2]->nPoints );
			assert( topo->nDims >= 3 );

			Grid_Lift( grids[2][2], gInd - grids[2][0]->nPoints - grids[2][1]->nPoints, dimInds );

			incEls[0] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;

			dimInds[2]++;
			incEls[1] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;
			dimInds[2]--;

			incEls[2] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;

			dimInds[1]++;
			incEls[3] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;
			dimInds[1]--;
		}

		CartesianGenerator_MapToDomain( self, (Sync*)IGraph_GetDomain( topo, 1 ), 
						nIncEls, incEls );
		IGraph_SetIncidence( topo, 2, e_i, 1, nIncEls, incEls );
	}

	FreeArray( incEls );
	FreeArray( dimInds );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenEdgeVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids ) {
   CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
   Stream*			stream = Journal_Register( Info_Type, (Name)self->type  );
   unsigned		nIncEls;
   unsigned*		incEls;
   unsigned*		dimInds;
   const Sync* sync;
   unsigned		e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );
	assert( topo );
	assert( grids );

	Journal_Printf( stream, "Generating edge-vertex incidence...\n" );
	Stream_Indent( stream );

	sync = IGraph_GetDomain( topo, 1 );
	incEls = MemArray( unsigned, 2, CartesianGenerator_Type );
	dimInds = MemArray( unsigned, topo->nDims, CartesianGenerator_Type );
	for( e_i = 0; e_i < Sync_GetNumDomains( sync ); e_i++ ) {
		unsigned	gInd = Sync_DomainToGlobal( sync, e_i );

		nIncEls = 2;

		if( gInd < grids[1][0]->nPoints ) {
			Grid_Lift( grids[1][0], gInd, dimInds );

			incEls[0] = Grid_Project( grids[0][0], dimInds );

			dimInds[0]++;
			incEls[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]--;
		}
		else if( gInd < grids[1][0]->nPoints + grids[1][1]->nPoints ) {
			assert( topo->nDims >= 2 );

			Grid_Lift( grids[1][1], gInd - grids[1][0]->nPoints, dimInds );

			incEls[0] = Grid_Project( grids[0][0], dimInds );

			dimInds[1]++;
			incEls[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]--;
		}
		else {
			assert( gInd < grids[1][0]->nPoints + grids[1][1]->nPoints + grids[1][2]->nPoints );
			assert( topo->nDims >= 3 );

			Grid_Lift( grids[1][2], gInd - grids[1][0]->nPoints - grids[1][1]->nPoints, dimInds );

			incEls[0] = Grid_Project( grids[0][0], dimInds );

			dimInds[2]++;
			incEls[1] = Grid_Project( grids[0][0], dimInds );
			dimInds[2]--;
		}

		CartesianGenerator_MapToDomain( self, (Sync*)IGraph_GetDomain( topo, 0 ), 
						nIncEls, incEls );
		IGraph_SetIncidence( topo, MT_EDGE, e_i, MT_VERTEX, nIncEls, incEls );
	}

	FreeArray( incEls );
	FreeArray( dimInds );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void _CartesianGenerator_GenElementTypes( void* meshGenerator, Mesh* mesh ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;
	Stream*			stream;
	unsigned		nDomainEls;
	unsigned		e_i;

	assert( self && Stg_CheckType( self, CartesianGenerator ) );

	stream = Journal_Register( Info_Type, (Name)self->type  );
	Journal_Printf( stream, "Generating element types...\n" );
	Stream_Indent( stream );

	mesh->nElTypes = 1;
	mesh->elTypes = Memory_Alloc_Array( Mesh_ElementType*, mesh->nElTypes, "Mesh::elTypes" );
	mesh->elTypes[0] = (Mesh_ElementType*)Mesh_HexType_New();
	Mesh_ElementType_SetMesh( mesh->elTypes[0], mesh );
	nDomainEls = Mesh_GetDomainSize( mesh, Mesh_GetDimSize( mesh ) );
	mesh->elTypeMap = Memory_Alloc_Array( unsigned, nDomainEls, "Mesh::elTypeMap" );
	for( e_i = 0; e_i < nDomainEls; e_i++ )
		mesh->elTypeMap[e_i] = 0;

	if( self->regular )
		Mesh_SetAlgorithms( mesh, Mesh_RegularAlgorithms_New( "", NULL ) );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... element types are '%s',\n", mesh->elTypes[0]->type );
	Journal_Printf( stream, "... mesh algorithm type is '%s',\n", mesh->algorithms->type );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void CartesianGenerator_SetGeometryParams( void* meshGenerator, double* min, double* max ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;

	/* Sanity check. */
	assert( self );
	assert( !self->elGrid->nDims || (min && max) );

	/* Kill everything we have, geometrically. */
	KillArray( self->crdMin );
	KillArray( self->crdMax );

	/* Set the parameters. */
	if( self->elGrid->nDims ) {
		self->crdMin = Memory_Alloc_Array( double, self->elGrid->nDims, "CartesianGenerator::min" );
		self->crdMax = Memory_Alloc_Array( double, self->elGrid->nDims, "CartesianGenerator::max" );
		memcpy( self->crdMin, min, self->elGrid->nDims * sizeof(double) );
		memcpy( self->crdMax, max, self->elGrid->nDims * sizeof(double) );
	}
}

void CartesianGenerator_SetShadowDepth( void* meshGenerator, unsigned depth ) {
	CartesianGenerator*	self = (CartesianGenerator*)meshGenerator;

	/* Sanity check. */
	assert( self );

	self->shadowDepth = depth;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void CartesianGenerator_BuildDecomp( CartesianGenerator* self ) {
	unsigned	nProcs, rank;
	unsigned	nPos = 0;
	unsigned**	posNSubDomains = NULL;
	unsigned*	tmpSubDomains;
	double		bestRatio;
	unsigned	bestPos;
	unsigned	*myRankInds, *rankInds;
	unsigned	nNbrs, *nbrs;
	unsigned	p_i, d_i, r_i;
   Stream*  errorStream = Journal_Register( Error_Type, (Name)self->type  );

	/* Sanity check. */
	assert( self );

	/* Collect information regarding parallel configuration. */
	MPI_Comm_size( self->mpiComm, (int*)&nProcs );
	MPI_Comm_rank( self->mpiComm, (int*)&rank );

	/* Allocate for possible sub-domains. */
	tmpSubDomains = Memory_Alloc_Array( unsigned, self->elGrid->nDims, "" );

	/* Build a list of all acceptable decompositions. */
	CartesianGenerator_RecurseDecomps( self, 0, nProcs, 
                  tmpSubDomains, &nPos, &posNSubDomains );

   Journal_Firewall( nPos, errorStream,
      "\nError in %s for %s '%s'\n\n"
      "There are no acceptable decompositions for the current mesh resolution and processor count.\n\n",
      __func__, self->type, self->name);

	/* Free unneeded memory. */
	FreeArray( tmpSubDomains );

	/* Now check for the best ratio. */
	bestRatio = HUGE_VAL;
	bestPos = -1;
	for( p_i = 0; p_i < nPos; p_i++ ) {
		double		curRatio = 0.0;
		unsigned	decompDims = 0;

		/* If decomposed in more dimensions than allowed, skip. */
		for( d_i = 0; d_i < self->elGrid->nDims; d_i++ )
			decompDims += (posNSubDomains[p_i][d_i] > 1) ? 1 : 0;
		if( self->maxDecompDims && decompDims > self->maxDecompDims )
			continue;

		/* Evaluate ratios. */
		for( d_i = 0; d_i < self->elGrid->nDims; d_i++ ) {
			double		nI = (double)self->elGrid->sizes[d_i] / (double)posNSubDomains[p_i][d_i];
			unsigned	d_j;

			for( d_j = d_i + 1; d_j < self->elGrid->nDims; d_j++ ) {
				double	nJ = (double)self->elGrid->sizes[d_j] / (double)posNSubDomains[p_i][d_j];

				curRatio += (nI > nJ) ? nI / nJ : nJ / nI;
			}
		}

		/* Better than best? */
		if( curRatio < bestRatio ) {
			bestRatio = curRatio;
			bestPos = p_i;
		}
	}
	assert( bestPos != -1 );

	/* Allocate for results. */
	self->origin = Memory_Alloc_Array( unsigned, self->elGrid->nDims, "CartesianGenerator::origin" );
	self->range = Memory_Alloc_Array( unsigned, self->elGrid->nDims, "CartesianGenerator::range" );

	/* Build a sub-domain grid. */
	self->procGrid = Grid_New();
	Grid_SetNumDims( self->procGrid, self->elGrid->nDims );
	Grid_SetSizes( self->procGrid, posNSubDomains[bestPos] );

	/* Free unneeded space. */
	FreeArray( posNSubDomains );

	/* Lift the rank to a parameterised offset. */
	Grid_Lift( self->procGrid, rank, self->origin );
	for( d_i = 0; d_i < self->elGrid->nDims; d_i++ ) {
		unsigned	base = self->elGrid->sizes[d_i] / self->procGrid->sizes[d_i];
		unsigned	mod = self->elGrid->sizes[d_i] % self->procGrid->sizes[d_i];
		unsigned	origin = self->origin[d_i];

		self->origin[d_i] *= base;
		self->range[d_i] = base;
		if( origin < mod ) {
			self->origin[d_i] += origin;
			self->range[d_i]++;
		}
		else
			self->origin[d_i] += mod;
   }

   /* Copy to the vertex equivalents. */
	self->vertOrigin = AllocArray( unsigned, self->elGrid->nDims );
	self->vertRange = AllocArray( unsigned, self->elGrid->nDims );
	for( d_i = 0; d_i < self->elGrid->nDims; d_i++ ) {
		self->vertOrigin[d_i] = self->origin[d_i];
		self->vertRange[d_i] = self->range[d_i] + 1;
	}

	/* Build the comm topology. */
	myRankInds = AllocArray( unsigned, Grid_GetNumDims( self->procGrid ) );
	rankInds = AllocArray( unsigned, Grid_GetNumDims( self->procGrid ) );
	Grid_Lift( self->procGrid, rank, myRankInds );
	nNbrs = 0;
	nbrs = NULL;
	for( r_i = 0; r_i < Grid_GetNumPoints( self->procGrid ); r_i++ ) {
		if( r_i == rank )
			continue;

		Grid_Lift( self->procGrid, r_i, rankInds );
		for( d_i = 0; d_i < Grid_GetNumDims( self->procGrid ); d_i++ ) {
			if( (myRankInds[d_i] > 0 && rankInds[d_i] < myRankInds[d_i] - 1) || 
			    (myRankInds[d_i] < self->procGrid->sizes[d_i] - 1 && rankInds[d_i] > myRankInds[d_i] + 1) )
			{
				break;
			}
		}
		if( d_i == Grid_GetNumDims( self->procGrid ) ) {
			nbrs = ReallocArray( nbrs, unsigned, nNbrs + 1 );
			nbrs[nNbrs++] = r_i;
		}
	}

	FreeArray( myRankInds );
	FreeArray( rankInds );

	Stg_Class_RemoveRef( self->comm );
	self->comm = Comm_New();
	Stg_Class_AddRef( self->comm );
	Comm_SetMPIComm( self->comm, self->mpiComm );
	Comm_SetNeighbours( self->comm, nNbrs, nbrs );
	FreeArray( nbrs );
}

void CartesianGenerator_RecurseDecomps( CartesianGenerator* self, 
					unsigned dim, unsigned max, 
					unsigned* nSubDomains, 
					unsigned* nPos, unsigned*** posNSubDomains )
{
	unsigned	nProcs;
	unsigned	nSDs = 1;
	unsigned	d_i;

	MPI_Comm_size( self->mpiComm, (int*)&nProcs );

	/* If we're over the limit, return immediately. */
	for( d_i = 0; d_i < dim; d_i++ )
		nSDs *= nSubDomains[d_i];
	if( nSDs > nProcs )
		return;

	/* Where are we up to? */
	if( dim == self->elGrid->nDims ) {
		/* If this covers all processors, store it. */
		if( nSDs == nProcs ) {
			/* If we havn't already allocated do it now. */
			if( !*posNSubDomains ) {
				*nPos = 1;
				*posNSubDomains = Memory_Alloc_2DArray_Unnamed( unsigned, 2, self->elGrid->nDims );
			}
			else {
				/* Reallocate the arrays. */
				(*nPos)++;
				if( *nPos != 2 ) {
					*posNSubDomains = Memory_Realloc_2DArray( *posNSubDomains, unsigned, 
										  *nPos, self->elGrid->nDims );
				}
			}

			/* Store status. */
			memcpy( (*posNSubDomains)[(*nPos) - 1], nSubDomains, self->elGrid->nDims * sizeof(unsigned) );
		}
	}
	else {
		unsigned	p_i;

		/* Loop over all remaining */
		for( p_i = 0; p_i < max; p_i++ ) {
			/* Don't try and decompose more than this dimension allows. */
			if( p_i >= self->elGrid->sizes[dim] || 
			    (self->maxDecomp[dim] && p_i >= self->maxDecomp[dim]) )
			{
				break;
			}

			/* If we have a minimum decomp, skip until we reach it. */
			if( self->minDecomp[dim] && p_i < self->minDecomp[dim] - 1 )
				continue;

			/* Set the number of sub-domains. */
			nSubDomains[dim] = p_i + 1;

			/* Try this combination. */
			CartesianGenerator_RecurseDecomps( self, dim + 1, max - nSDs + 1, nSubDomains, 
							   nPos, posNSubDomains );
		}
	}
}

void CartesianGenerator_GenTopo( CartesianGenerator* self, IGraph* topo ) {
	Grid***		grids;
	const Comm* comm;
	unsigned	d_i, d_j;

	assert( self );
	assert( topo );

	MeshTopology_SetNumDims( topo, self->elGrid->nDims );

	/* Build additional grids for use in numbering. */
	grids = Memory_Alloc_2DArray_Unnamed( Grid*, topo->nTDims, topo->nTDims );
	for( d_i = 0; d_i < topo->nTDims; d_i++ )
		memset( grids[d_i], 0, topo->nTDims * sizeof(Grid*) );

	grids[topo->nDims][0] = self->elGrid;
	grids[0][0] = self->vertGrid;

	if( topo->nDims >= 2 ) {
		grids[1][0] = Grid_New();
		Grid_SetNumDims( grids[1][0], topo->nDims );
		for( d_i = 0; d_i < topo->nDims; d_i++ ) {
			if( d_i == 0 ) continue;
			grids[topo->nDims][0]->sizes[d_i]++;
		}
		Grid_SetSizes( grids[1][0], grids[topo->nDims][0]->sizes );
		for( d_i = 0; d_i < topo->nDims; d_i++ ) {
			if( d_i == 0 ) continue;
			grids[topo->nDims][0]->sizes[d_i]--;
		}

		grids[1][1] = Grid_New();
		Grid_SetNumDims( grids[1][1], topo->nDims );
		for( d_i = 0; d_i < topo->nDims; d_i++ ) {
			if( d_i == 1 ) continue;
			grids[topo->nDims][0]->sizes[d_i]++;
		}
		Grid_SetSizes( grids[1][1], grids[topo->nDims][0]->sizes );
		for( d_i = 0; d_i < topo->nDims; d_i++ ) {
			if( d_i == 1 ) continue;
			grids[topo->nDims][0]->sizes[d_i]--;
		}

		if( topo->nDims >= 3 ) {
			grids[1][2] = Grid_New();
			Grid_SetNumDims( grids[1][2], topo->nDims );
			for( d_i = 0; d_i < topo->nDims; d_i++ ) {
				if( d_i == 2 ) continue;
				grids[topo->nDims][0]->sizes[d_i]++;
			}
			Grid_SetSizes( grids[1][2], grids[topo->nDims][0]->sizes );
			for( d_i = 0; d_i < topo->nDims; d_i++ ) {
				if( d_i == 2 ) continue;
				grids[topo->nDims][0]->sizes[d_i]--;
			}

			grids[2][0] = Grid_New();
			Grid_SetNumDims( grids[2][0], topo->nDims );
			grids[topo->nDims][0]->sizes[2]++;
			Grid_SetSizes( grids[2][0], grids[topo->nDims][0]->sizes );
			grids[topo->nDims][0]->sizes[2]--;

			grids[2][1] = Grid_New();
			Grid_SetNumDims( grids[2][1], topo->nDims );
			grids[topo->nDims][0]->sizes[1]++;
			Grid_SetSizes( grids[2][1], grids[topo->nDims][0]->sizes );
			grids[topo->nDims][0]->sizes[1]--;

			grids[2][2] = Grid_New();
			Grid_SetNumDims( grids[2][2], topo->nDims );
			grids[topo->nDims][0]->sizes[0]++;
			Grid_SetSizes( grids[2][2], grids[topo->nDims][0]->sizes );
			grids[topo->nDims][0]->sizes[0]--;
		}
	}

	/* Generate topological elements. */
	if( self->enabledDims[0] )
		CartesianGenerator_GenVertices( self, topo, grids );
	if( self->enabledDims[self->nDims] )
		CartesianGenerator_GenElements( self, topo, grids );
	if( topo->nDims >= 2 ) {
		if( self->enabledDims[1] )
			CartesianGenerator_GenEdges( self, topo, grids );
		if( topo->nDims >= 3 ) {
			if( self->enabledDims[2] )
				CartesianGenerator_GenFaces( self, topo, grids );
		}
	}

	/* Generate topological incidence. */
	if( self->enabledInc[self->nDims][0] )
		CartesianGenerator_GenElementVertexInc( self, topo, grids );
	if( topo->nDims >= 2 ) {
		if( self->enabledInc[2][1] )
			CartesianGenerator_GenFaceEdgeInc( self, topo, grids );
		if( self->enabledInc[1][0] )
			CartesianGenerator_GenEdgeVertexInc( self, topo, grids );
		if( topo->nDims >= 3 ) {
			if( self->enabledInc[2][0] )
				CartesianGenerator_GenFaceVertexInc( self, topo, grids );
			if( self->enabledInc[3][1] )
				CartesianGenerator_GenVolumeEdgeInc( self, topo, grids );
			if( self->enabledInc[3][2] )
				CartesianGenerator_GenVolumeFaceInc( self, topo, grids );
		}
	}

	/* Set the shadow depth and correct incidence. */
	comm = MeshTopology_GetComm( topo );
	if( self->shadowDepth && Comm_GetNumNeighbours( comm ) > 0 ) {
		/* Build enough incidence to set shadow depth. */
		IGraph_InvertIncidence( topo, MT_VERTEX, topo->nDims );
		IGraph_ExpandIncidence( topo, topo->nDims );

		/* Set the shadow depth. */
		MeshTopology_SetShadowDepth( topo, self->shadowDepth );

		/* Kill up relations and neighbours. */
		IGraph_RemoveIncidence( topo, topo->nDims, topo->nDims );
		IGraph_RemoveIncidence( topo, 0, topo->nDims );
	}

	/* Complete all required relations. */
	for( d_i = 0; d_i < self->nDims; d_i++ ) {
		for( d_j = d_i + 1; d_j <= self->nDims; d_j++ ) {
			if( !self->enabledInc[d_i][d_j] )
				continue;

			IGraph_InvertIncidence( topo, d_i, d_j );
		}
	}
	for( d_i = 0; d_i <= self->nDims; d_i++ ) {
		if( self->enabledInc[d_i][d_i] ) {
			if( d_i == 0 )
				CartesianGenerator_CompleteVertexNeighbours( self, topo, grids );
			else {
				IGraph_ExpandIncidence( topo, d_i );
			}
		}
	}

	/* Generate any boundary elements required. */
	CartesianGenerator_GenBndVerts( self, topo, grids );

	/* Free allocated grids. */
	grids[topo->nDims][0] = NULL;
	for( d_i = 1; d_i < topo->nDims; d_i++ ) {
		unsigned	d_j;

		for( d_j = 0; d_j < topo->nTDims; d_j++ )
			FreeObject( grids[d_i][d_j] );
	}
	FreeArray( grids );
}

void CartesianGenerator_GenEdges2D( CartesianGenerator* self, IGraph* topo, Grid*** grids ) {
	Grid*		globalGrid;
	unsigned	nGlobalEls;
	Grid*		grid;
	unsigned	nEls[2];
	unsigned*	els;
	unsigned*	dimInds;
	unsigned	d_i, e_i;

	assert( self );
	assert( topo );
	assert( grids );
	assert( self->elGrid->nDims == 2 );

	globalGrid = Grid_New();
	Grid_SetNumDims( globalGrid, self->elGrid->nDims );
	self->elGrid->sizes[1]++;
	Grid_SetSizes( globalGrid, self->elGrid->sizes );
	self->elGrid->sizes[1]--;

	grid = Grid_New();
	Grid_SetNumDims( grid, self->elGrid->nDims );
	self->range[1]++;
	Grid_SetSizes( grid, self->range );
	self->range[1]--;

	nEls[0] = grid->sizes[0];
	nGlobalEls = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[0] *= grid->sizes[d_i];
		nGlobalEls *= globalGrid->sizes[d_i];
	}
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls[0] );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->elGrid->nDims );

	for( e_i = 0; e_i < nEls[0]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( globalGrid, dimInds );
	}

	self->elGrid->sizes[0]++;
	Grid_SetSizes( globalGrid, self->elGrid->sizes );
	self->elGrid->sizes[0]--;

	self->range[0]++;
	Grid_SetSizes( grid, self->range );
	self->range[0]--;

	nEls[1] = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls[1] *= grid->sizes[d_i];
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] );

	for( e_i = 0; e_i < nEls[1]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + e_i] = nGlobalEls + Grid_Project( globalGrid, dimInds );
	}

	IGraph_SetElements( topo, MT_EDGE, nEls[0] + nEls[1], els );

	FreeArray( dimInds );
	FreeArray( els );
	FreeObject( grid );
	FreeObject( globalGrid );
}

void CartesianGenerator_GenEdges3D( CartesianGenerator* self, IGraph* topo, Grid*** grids ) {
	Grid*		globalGrid;
	unsigned	nGlobalEls[2];
	Grid*		grid;
	unsigned	nEls[3];
	unsigned*	els;
	unsigned*	dimInds;
	unsigned	d_i, e_i;

	assert( self );
	assert( topo );
	assert( grids );
	assert( self->elGrid->nDims == 3 );

	globalGrid = Grid_New();
	Grid_SetNumDims( globalGrid, self->elGrid->nDims );
	self->elGrid->sizes[1]++;
	self->elGrid->sizes[2]++;
	Grid_SetSizes( globalGrid, self->elGrid->sizes );
	self->elGrid->sizes[1]--;
	self->elGrid->sizes[2]--;

	grid = Grid_New();
	Grid_SetNumDims( grid, self->elGrid->nDims );
	self->range[1]++;
	self->range[2]++;
	Grid_SetSizes( grid, self->range );
	self->range[1]--;
	self->range[2]--;

	nEls[0] = grid->sizes[0];
	nGlobalEls[0] = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[0] *= grid->sizes[d_i];
		nGlobalEls[0] *= globalGrid->sizes[d_i];
	}
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls[0] );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->elGrid->nDims );

	for( e_i = 0; e_i < nEls[0]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( globalGrid, dimInds );
	}

	self->elGrid->sizes[0]++;
	self->elGrid->sizes[2]++;
	Grid_SetSizes( globalGrid, self->elGrid->sizes );
	self->elGrid->sizes[0]--;
	self->elGrid->sizes[2]--;

	self->range[0]++;
	self->range[2]++;
	Grid_SetSizes( grid, self->range );
	self->range[0]--;
	self->range[2]--;

	nEls[1] = grid->sizes[0];
	nGlobalEls[1] = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[1] *= grid->sizes[d_i];
		nGlobalEls[1] *= globalGrid->sizes[d_i];
	}
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] );

	for( e_i = 0; e_i < nEls[1]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + e_i] = nGlobalEls[0] + Grid_Project( globalGrid, dimInds );
	}

	self->elGrid->sizes[0]++;
	self->elGrid->sizes[1]++;
	Grid_SetSizes( globalGrid, self->elGrid->sizes );
	self->elGrid->sizes[0]--;
	self->elGrid->sizes[1]--;

	self->range[0]++;
	self->range[1]++;
	Grid_SetSizes( grid, self->range );
	self->range[0]--;
	self->range[1]--;

	nEls[2] = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls[2] *= grid->sizes[d_i];
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] + nEls[2] );

	for( e_i = 0; e_i < nEls[2]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + nEls[1] + e_i] = nGlobalEls[0] + nGlobalEls[1] + Grid_Project( globalGrid, dimInds );
	}

	IGraph_SetElements( topo, MT_EDGE, nEls[0] + nEls[1] + nEls[2], els );

	FreeArray( dimInds );
	FreeArray( els );
	FreeObject( grid );
	FreeObject( globalGrid );
}

void CartesianGenerator_GenBndVerts( CartesianGenerator* self, IGraph* topo, Grid*** grids ) {
	/*@
		Builds an int array on topo->nBndEls[0]. These integers represent the domain (local+shadow) nodes
		on the processor that lie on the model space boundary

		
		@*/
	IArray bndElsObj, *bndEls = &bndElsObj;
	int nDims, nVerts, nBndEls;
	const int* ptr;
	int global;
	int* inds;
	const Sync* sync;
	int v_i, d_i;

	assert( self );
	assert( topo );
	assert( grids );

	nDims = MeshTopology_GetNumDims( topo );
	inds = AllocArray( int, nDims );
	IArray_Init( bndEls );
	sync = IGraph_GetDomain( topo, 0 );
	nVerts = Sync_GetNumDomains( sync );
	for( v_i = 0; v_i < nVerts; v_i++ ) {
		global = Sync_DomainToGlobal( sync, v_i );
		Grid_Lift( grids[0][0], global, inds );
		for( d_i = 0; d_i < nDims; d_i++ ) {
			if( inds[d_i] == 0 || inds[d_i] == grids[0][0]->sizes[d_i] - 1 )
				break;
		}
		if( d_i < nDims )
			IArray_Append( bndEls, v_i );
	}
	FreeArray( inds );

	IArray_GetArray( bndEls, &nBndEls, &ptr );
	IGraph_SetBoundaryElements( topo, 0, nBndEls, ptr );
	IArray_Destruct( bndEls );
}

void CartesianGenerator_CompleteVertexNeighbours( CartesianGenerator* self, IGraph* topo, Grid*** grids ) {
   Stream*		stream = Journal_Register( Info_Type, (Name)self->type  );
   const Sync* sync;
   unsigned	nDims;
   unsigned	nVerts;
   unsigned*	inds;
   unsigned	*nNbrs, **nbrs;
   unsigned	domain, global;
   unsigned	v_i;

	assert( self );
	assert( topo );
	assert( grids );

	Journal_Printf( stream, "Generating vertex neighbours...\n" );
	Stream_Indent( stream );

	nDims = topo->nDims;
	sync = IGraph_GetDomain( topo, MT_VERTEX );
	nVerts = Sync_GetNumDomains( sync );
	inds = AllocArray( unsigned, nDims );
	nNbrs = AllocArray( unsigned, nVerts );
	nbrs = AllocArray2D( unsigned, nVerts, (nDims == 3) ? 6 : (nDims == 2) ? 4 : 2 );
	for( v_i = 0; v_i < nVerts; v_i++ ) {
		nNbrs[v_i] = 0;
		global = Sync_DomainToGlobal( sync, v_i );
		Grid_Lift( grids[0][0], global, inds );

		if( inds[0] > 0 ) {
			inds[0]--;
			domain = Grid_Project( grids[0][0], inds );
			if( Sync_TryGlobalToDomain( sync, domain, &domain ) ) {
				nbrs[v_i][nNbrs[v_i]] = domain;
				nNbrs[v_i]++;
			}
			inds[0]++;
		}

		if( inds[0] < grids[0][0]->sizes[0] - 1 ) {
			inds[0]++;
			domain = Grid_Project( grids[0][0], inds );
			if( Sync_TryGlobalToDomain( sync, domain, &domain ) ) {
				nbrs[v_i][nNbrs[v_i]] = domain;
				nNbrs[v_i]++;
			}
			inds[0]--;
		}

		if( nDims >= 2 ) {
			if( inds[1] > 0 ) {
				inds[1]--;
				domain = Grid_Project( grids[0][0], inds );
				if( Sync_TryGlobalToDomain( sync, domain, &domain ) ) {
					nbrs[v_i][nNbrs[v_i]] = domain;
					nNbrs[v_i]++;
				}
				inds[1]++;
			}

			if( inds[1] < grids[0][0]->sizes[1] - 1 ) {
				inds[1]++;
				domain = Grid_Project( grids[0][0], inds );
				if( Sync_TryGlobalToDomain( sync, domain, &domain ) ) {
					nbrs[v_i][nNbrs[v_i]] = domain;
					nNbrs[v_i]++;
				}
				inds[1]--;
			}

			if( nDims == 3 ) {
				if( inds[2] > 0 ) {
					inds[2]--;
					domain = Grid_Project( grids[0][0], inds );
					if( Sync_TryGlobalToDomain( sync, domain, &domain ) ) {
						nbrs[v_i][nNbrs[v_i]] = domain;
						nNbrs[v_i]++;
					}
					inds[2]++;
				}

				if( inds[2] < grids[0][0]->sizes[2] - 1 ) {
					inds[2]++;
					domain = Grid_Project( grids[0][0], inds );
					if( Sync_TryGlobalToDomain( sync, domain, &domain ) ) {
						nbrs[v_i][nNbrs[v_i]] = domain;
						nNbrs[v_i]++;
					}
					inds[2]--;
				}
			}
		}

		IGraph_SetIncidence( topo, MT_VERTEX, v_i, MT_VERTEX, nNbrs[v_i], nbrs[v_i] );
	}

	FreeArray( nNbrs );
	FreeArray( nbrs );
	FreeArray( inds );

	MPI_Barrier( self->mpiComm );
	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
}

void CartesianGenerator_MapToDomain( CartesianGenerator* self, Sync* sync, 
				     unsigned nIncEls, unsigned* incEls )
{
	unsigned	inc_i;

	assert( self );
	assert( sync );
	assert( nIncEls );
	assert( incEls );

	for( inc_i = 0; inc_i < nIncEls; inc_i++ )
		incEls[inc_i] = Sync_GlobalToDomain( sync, incEls[inc_i] );
}

#define MAX_LINE_LENGTH 1024
void CartesianGenerator_GenGeom( void* _self, void* _mesh, void* data ) {
	CartesianGenerator* self = (CartesianGenerator*)_self;
    Mesh*               mesh = (Mesh*)_mesh;
    Stream*			   stream = Journal_Register( Info_Type, (Name)self->type );
    Sync*			      sync;
    AbstractContext* 	context = (AbstractContext*)data;

    assert( self );
    assert( mesh  );

    Journal_Printf( stream, "Generating geometry...\n" );
    Stream_Indent( stream );

    /* Allocate for coordinates. */
    sync = (Sync*)IGraph_GetDomain( (IGraph*)mesh->topo, 0 );
    Mesh_GenerateVertices( mesh, Sync_GetNumDomains( sync ), mesh->topo->nDims );


/* Below is the algorithm to choose method to generate node geometry

    IF vtk node geometry file THEN
	READ IN VTK FORMAT
    ELSE IF loading from checkpoint THEN
	READ IN HDF5 FORMAT (or ascii - depends on build options )
    ELSE 
	USE THE OBJECTS FUNCTION calcGeomFunc

*/
    if( self->initVtkFile ) {

      CartesianGenerator_ReadFromVTK( self, mesh, self->initVtkFile );

    } else if( context && context->loadFromCheckPoint 
	    && context->timeStep == context->restartTimestep 
	    && self->readFromFile) {

	char*  meshReadFileName = strdup( self->initMeshFile );

	#ifdef READ_HDF5
        CartesianGenerator_ReadFromHDF5( self, mesh, meshReadFileName );
        Journal_Printf( stream, "... loading from file %s.\n", meshReadFileName );
	#else
	Journal_Printf( stream, "... loading from file %s.\n", meshReadFileName );
	CartesianGenerator_ReadFromASCII( self, mesh, meshReadFileName);      
	#endif
	Memory_Free( meshReadFileName );
	Mesh_Sync( mesh );

    } else { 
	Grid*			      grid;
	unsigned*		   inds;
	double*			   steps;
	unsigned          d_i;

	/* Build grid and space for indices. */
	grid = self->vertGrid;
	inds = Memory_Alloc_Array_Unnamed( unsigned, mesh->topo->nDims );

	/* Calculate steps. */
	steps = Memory_Alloc_Array_Unnamed( double, mesh->topo->nDims );
	for( d_i = 0; d_i < mesh->topo->nDims; d_i++ )
	 steps[d_i] = self->crdMax[d_i] - self->crdMin[d_i];

	self->calcGeomFunc( self, mesh, sync, grid, inds, steps );

	/* Free resources. */
	FreeArray( inds );
	FreeArray( steps );
    }


    MPI_Barrier( self->mpiComm );
    Journal_Printf( stream, "... done.\n" );
    Stream_UnIndent( stream );
}

void CartesianGenerator_CalcGeom( void* _self, Mesh* mesh, Sync* sync, Grid* grid, unsigned* inds, double* steps ) {	
	CartesianGenerator *self = (CartesianGenerator*)_self;
	unsigned		n_i, d_i;
	double*         	vert;
	unsigned        	gNode;

	/* Loop over domain nodes. */
	for( n_i = 0; n_i < Sync_GetNumDomains( sync ); n_i++ ) {
		gNode = Sync_DomainToGlobal( sync, n_i );
		Grid_Lift( grid, gNode, inds );
		vert = Mesh_GetVertex( mesh, n_i );

		/* Calculate coordinate. */
		for( d_i = 0; d_i < mesh->topo->nDims; d_i++ ) {
                   if( inds[d_i] <= self->contactDepth[d_i][0] ) {
                      Mesh_GetVertex( mesh, n_i )[d_i] = self->crdMin[d_i];
                      if( self->contactDepth[d_i][0] ) {
                         Mesh_GetVertex( mesh, n_i )[d_i] +=
                            ((double)inds[d_i] / (double)self->contactDepth[d_i][0]) *
                            self->contactGeom[d_i];
                      }
                   }
                   else if( inds[d_i] >= grid->sizes[d_i] - self->contactDepth[d_i][1] - 1 ) {
                      Mesh_GetVertex( mesh, n_i )[d_i] = self->crdMax[d_i];
                      if( self->contactDepth[d_i][1] ) {
                         Mesh_GetVertex( mesh, n_i )[d_i] -=
                            ((double)(grid->sizes[d_i] - 1 - inds[d_i]) /
                             (double)self->contactDepth[d_i][1]) *
                            self->contactGeom[d_i];
                      }
                   }
                   else {
                      vert[d_i] = self->crdMin[d_i] + (self->contactDepth[d_i][0] ? self->contactGeom[d_i] : 0.0) +
                         ((double)(inds[d_i] - self->contactDepth[d_i][0]) / 
                          (double)(grid->sizes[d_i] - (self->contactDepth[d_i][0] + self->contactDepth[d_i][1]) - 1)) *
                         (steps[d_i] - ((self->contactDepth[d_i][1] ? 1.0 : 0.0) + (self->contactDepth[d_i][0] ? 1.0 : 0.0)) * self->contactGeom[d_i]);
                   }
		}
	}
}

void CartesianGenerator_Destruct( CartesianGenerator* self ) {
	assert( self );

	CartesianGenerator_DestructTopology( self );
	CartesianGenerator_DestructGeometry( self );
	KillArray( self->minDecomp );
	KillArray( self->maxDecomp );
	Stg_Class_RemoveRef( self->comm );
}

void CartesianGenerator_DestructTopology( CartesianGenerator* self ) {
	assert( self );

	self->maxDecompDims = 0;
	memset( self->minDecomp, 0, self->nDims * sizeof(unsigned) );
	memset( self->maxDecomp, 0, self->nDims * sizeof(unsigned) );
	FreeObject( self->vertGrid );
	FreeObject( self->elGrid );
	FreeObject( self->procGrid );
	KillArray( self->origin );
	KillArray( self->range );
	KillArray( self->vertOrigin );
	KillArray( self->vertRange );
}

void CartesianGenerator_DestructGeometry( CartesianGenerator* self ) {
	assert( self );

	FreeArray( self->crdMin );
	FreeArray( self->crdMax );
}

void _quit_because_bad_vtkfile(FILE *inputFile, char *lineString, char *expected){
  printf("ERROR: Input vtk file has unexpected string\n");
  printf("\tUnderwrold expected a line starting with - \"%s\"\n", expected );
  printf("\tInput line is - \"%s\"\n", lineString );

  printf("Change input file to meet expected file:\n");
  printf("Underworld basic VTK reader assumes this block exists in the given VTK file:\n");
  printf("************************\n");  
  printf("DATASET STRUCTURED_GRID\n");
  printf("DIMENSIONS nx ny nz\n");
  printf("POINTS n dataType\n");	      
  printf("p0x p0y p0z\n");
  printf("p1x p1y p1z\n");
  printf("************************\n");

  printf("nx, ny, nz and read in as integer values\n");
  printf("p0x p0y p0z and read in as double values\n");
	  

  fclose( inputFile );
  exit(EXIT_FAILURE);
}

void CartesianGenerator_ReadFromVTK(  CartesianGenerator* self, Mesh* mesh, const char* filename ){
  /* Assume a LEGACY VTK FILE format to define the node points.
     For definition of LEGACY format see - http://www.vtk.org/VTK/img/file-formats.pdf

     Assumes only STRUCTURE_GRID is defined in VTK file.
  */

  Node_LocalIndex   lNode_I = 0;
  FILE*             inputFile;
  char              lineString[MAX_LINE_LENGTH];
  int               nLocalNodes, res[3];
  int		    myRank, nProcs;

	double crds[3];
	double *vert;
	unsigned inds[3], i,j,k, ni,nj,nk, gNode_I;

	long              offset_into_file = 0;
	MPI_Status        status;
	const int         FINISHED_WRITING_TAG = 100;
	int ierr;

  MPI_Comm_rank( self->mpiComm, &myRank );
  MPI_Comm_size( self->mpiComm, &nProcs );

  /* Here the file is read sequentially by each proc
	 * starting from 0,1....rank-1. An offset into the 
	 * file is maintained by each proc and passed to the
	 * next proc into the squence */

	/* wait for go-ahead from process ranked lower than me, to avoid competition writing to file */
	if ( myRank != 0  && offset_into_file == 0 ) {
		ierr=MPI_Recv( &offset_into_file, 1, MPI_LONG, myRank - 1, FINISHED_WRITING_TAG, self->mpiComm, &status );
	}

	// open the file
	inputFile = fopen( filename, "r" );
	assert(inputFile);

	// offset into the input file
	fseek( inputFile, offset_into_file, SEEK_SET );

	// let proc 0 check file's compatibility
	if( myRank == 0 ) {
		int good = 0;
		while( !feof(inputFile ) ) {
			// check DATASET, and leave file ptr at the line
			fgets( lineString, MAX_LINE_LENGTH, inputFile );
			if( 0 == strncmp(lineString, "DATASET STRUCTURED_GRID", 23 ) ){
				good++;
				break;
			}
		}

		if( good != 1 ) _quit_because_bad_vtkfile(inputFile, lineString, "DATASET STRUCTURED_GRID");

		// assume next line is dimensions - check they are compatible
		fgets( lineString, MAX_LINE_LENGTH, inputFile );
		if( 0 != strncmp(lineString, "DIMENSIONS ", 10 ) ){
			_quit_because_bad_vtkfile(inputFile, lineString, "DIMENSIONS ");
		}
		if( self->nDims == 2 ) {
			sscanf( lineString, "DIMENSIONS %d %d", &res[0], &res[1] );
			assert( self->vertGrid->sizes[0] == res[0] );
			assert( self->vertGrid->sizes[1] == res[1] );
		} else {
			sscanf( lineString, "DIMENSIONS  %d %d %d", res, res+1, res+2 );
			assert( self->vertGrid->sizes[0] == res[0] );
			assert( self->vertGrid->sizes[1] == res[1] );
			assert( self->vertGrid->sizes[2] == res[2] );
		}
		good++;

		// next line is "POINTS n <datatype>"
		fgets( lineString, MAX_LINE_LENGTH, inputFile );
		if( 0 != strncmp(lineString, "POINTS ", 6 ) ) _quit_because_bad_vtkfile(inputFile, lineString, "POINTS ");
	}

	// IMPORTANT to offset the file here
	offset_into_file = ftell( inputFile );

	nLocalNodes = Mesh_GetLocalSize( mesh, MT_VERTEX );
	lNode_I = 0;

	ni = self->vertGrid->sizes[0];
	nj = self->vertGrid->sizes[1];
	nk = self->vertGrid->sizes[2];

	// loops over the ijk mesh representation
	for( k=0; k<nk; k++ ) {
		for( j=0; j<nj; j++) {
			for( i=0; i<ni; i++) {
				// read in line
				fgets( lineString, MAX_LINE_LENGTH, inputFile );

				// ASSUME global ijk and calculate global node id
				inds[0] = i;
				inds[1] = j;
				inds[2] = k;
				gNode_I = Grid_Project( self->vertGrid, inds );
		
				// if local read in coords;
				if( Mesh_GlobalToDomain( mesh, MT_VERTEX, gNode_I, &lNode_I ) )	
				{
					sscanf( lineString, "%lg %lg %lg ", crds, crds + 1, crds + 2 );
					vert = Mesh_GetVertex( mesh, lNode_I );
					vert[0] = crds[0]; vert[1] = crds[1]; vert[2] = crds[2];
				}
			}
		}
	}
	
	// close this proc input file
	fclose( inputFile );

	/* send go-ahead from next proc, using offset_into_file as a message */
	if ( myRank != nProcs - 1 ) {
		MPI_Ssend( &offset_into_file, 1, MPI_LONG, myRank + 1, FINISHED_WRITING_TAG, self->mpiComm );
	}
}

#ifdef READ_HDF5
void CartesianGenerator_ReadFromHDF5(  CartesianGenerator* self, Mesh* mesh, const char* filename ){
	hid_t             file, fileSpace, fileData;
	hsize_t           start[2], count[2], size[2], maxSize[2];   
	hid_t             memSpace, error;
	double            buf[4];
   Node_LocalIndex   lNode_I = 0;
	Node_GlobalIndex  gNode_I = 0;
   int               totalVerts, ii;
   unsigned int		noffset;
   MeshCheckpointFileVersion ver;
   hid_t					attrib_id, group_id;
   herr_t				status;
   char*					verticeName;
	Stream*			   errorStream = Journal_Register( Error_Type, (Name)self->type  );   

   /* Open the file and data set. */
   file = H5Fopen( filename, H5F_ACC_RDONLY, H5P_DEFAULT );

   /** get the file attributes to sanity and version checks */
   #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Gopen_vers == 1
      group_id  = H5Gopen(file, "/");
      attrib_id = H5Aopen_name(group_id, "checkpoint file version");
   #else
      group_id  = H5Gopen(file, "/", H5P_DEFAULT);
      attrib_id = H5Aopen(group_id, "checkpoint file version", H5P_DEFAULT);
   #endif
   /** if this attribute does not exist (attrib_id < 0) then we assume MeshCHECKPOINT_V1 and continue without checking attributes */
   if(attrib_id < 0)
      ver = MeshCHECKPOINT_V1;
   else {
      int checkVer;
      int ndims;
      int res[self->nDims];
      unsigned*  sizes;

      /** check for known checkpointing version type */

      status = H5Aread(attrib_id, H5T_NATIVE_INT, &checkVer);
      H5Aclose(attrib_id);
      if(checkVer == 2)
         ver = MeshCHECKPOINT_V2;
      else
         Journal_Firewall( (0), errorStream,
            "\n\nError in %s for %s '%s'\n"
            "Unknown checkpoint version (%u) for checkpoint file (%s).\n", 
            __func__, self->type, self->name, (unsigned int) checkVer, filename);

      /** check for correct number of dimensions */

      #if H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8
         attrib_id = H5Aopen_name(group_id, "dimensions");
      #else
         attrib_id = H5Aopen(group_id, "dimensions", H5P_DEFAULT);
      #endif
      status = H5Aread(attrib_id, H5T_NATIVE_INT, &ndims);
      H5Aclose(attrib_id);      
      Journal_Firewall( (ndims == self->nDims), errorStream,
         "\n\nError in %s for %s '%s'\n"
         "Number of dimensions (%u) for checkpoint file (%s) does not correspond to simulation dimensions (%u).\n", 
         __func__, self->type, self->name, (unsigned int) ndims, filename,
         self->nDims);

      /** check for correct mesh size */
      
      #if H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8
         attrib_id = H5Aopen_name(group_id, "mesh resolution");
      #else
         attrib_id = H5Aopen(group_id, "mesh resolution", H5P_DEFAULT);
      #endif
      status = H5Aread(attrib_id, H5T_NATIVE_INT, &res);
      H5Aclose(attrib_id);

      sizes = Grid_GetSizes( self->elGrid ); /** global no. of elements in each dim */
      if(self->nDims == 2)
         Journal_Firewall( 
            ( (sizes[0] == res[0]) && (sizes[1] == res[1]) ), 
            errorStream,
            "\n\nError in %s for %s '%s'\n"
            "Size of mesh (%u,%u) for checkpoint file (%s) does not correspond to simulation mesh size (%u, %u).\n", 
            __func__, self->type, self->name, 
            (unsigned int) res[0], (unsigned int) res[1],
            filename,
            (unsigned int) sizes[0], (unsigned int) sizes[1]);
      else
         Journal_Firewall( 
            ( (sizes[0] == res[0]) && (sizes[1] == res[1]) && (sizes[2] == res[2]) ), 
            errorStream,
            "\n\nError in %s for %s '%s'\n"
            "Size of mesh (%u,%u,%u) for checkpoint file (%s) does not correspond to simulation mesh size (%u,%u,%u).\n", 
            __func__, self->type, self->name, 
            (unsigned int) res[0], (unsigned int) res[1], (unsigned int) res[2],
            filename,
            (unsigned int) sizes[0], (unsigned int) sizes[1], (unsigned int) sizes[2]);
   }
   H5Gclose(group_id);
   
   /** account for different versions of checkpointing */
   switch ( ver ) {
      case MeshCHECKPOINT_V1:
         noffset = 1;
         Stg_asprintf( &verticeName, "/data" );
         break;
      case MeshCHECKPOINT_V2:
         noffset = 0;
         Stg_asprintf( &verticeName, "/vertices" );
         break;
      default:
         Journal_Firewall( 0, errorStream,
            "Error: in %s: unknown checkpoint file version.\n",
            __func__ ); 
   }
   
   /* Prepare to read vertices from file */		
   #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
   fileData = H5Dopen( file, verticeName );
   #else
   fileData = H5Dopen( file, verticeName, H5P_DEFAULT );
   #endif
   fileSpace = H5Dget_space( fileData );
   Memory_Free( verticeName );
   start[1] = 0;
   count[0] = 1;
   count[1] = mesh->topo->nDims + noffset;
   memSpace = H5Screate_simple( 2, count, NULL );
   totalVerts = Mesh_GetGlobalSize( mesh, 0 );

   /* Get size of dataspace to check consistency */
   H5Sget_simple_extent_dims( fileSpace, size, maxSize ); 

   Journal_Firewall( 
      (maxSize[0] == totalVerts), 
      errorStream,
      "\n\nError in %s for %s '%s'\n"
      "Number of mesh vertices (%u) stored in %s does not correspond to total number of requested mesh vertices (%u).\n", 
      __func__, 
      self->type, 
      self->name, 
      (unsigned int)maxSize[0],
      filename,
      totalVerts);

   for( ii=0; ii<totalVerts; ii++ ) {   
      start[0] = ii;
            
      H5Sselect_hyperslab( fileSpace, H5S_SELECT_SET, start, NULL, count, NULL );
      H5Sselect_all( memSpace );
      
      error = H5Dread( fileData, H5T_NATIVE_DOUBLE, memSpace, fileSpace, H5P_DEFAULT, buf );
      gNode_I = ii;

      Journal_Firewall( 
         error >= 0, 
         errorStream,
         "\n\nError in %s for %s '%s' - Cannot read data in %s.\n", 
         __func__, 
         self->type, 
         self->name, 
         filename );
      
      if( Mesh_GlobalToDomain( mesh, MT_VERTEX, gNode_I, &lNode_I ) && 
          lNode_I < Mesh_GetLocalSize( mesh, MT_VERTEX ) )
      {
         double *vert;

         vert = Mesh_GetVertex( mesh, lNode_I );
         vert[0] = buf[0 + noffset];
         vert[1] = buf[1 + noffset];
         if( mesh->topo->nDims ==3 )
            vert[2] = buf[2 + noffset];
      }
   }
         
   /* Close handles */
   H5Sclose( memSpace );
   H5Sclose( fileSpace );
   H5Dclose( fileData );
   H5Fclose( file );
   
}
#endif

void CartesianGenerator_ReadFromASCII( CartesianGenerator* self, Mesh* mesh, const char* filename ){
   int               proc_I;
	Node_LocalIndex   lNode_I = 0;
	Node_GlobalIndex  gNode_I = 0;
	FILE*             inputFile;
	char              lineString[MAX_LINE_LENGTH];
	int			      rank, nRanks;

   MPI_Comm_rank( self->mpiComm, &rank );
   MPI_Comm_size( self->mpiComm, &nRanks );

   /* This loop used to stop 2 processors trying to open the file at the same time, which
   * seems to cause problems */
   for ( proc_I = 0; proc_I < nRanks; proc_I++ ) {
      MPI_Barrier( self->mpiComm );
      if ( proc_I == rank ) {
         /* Do the following since in parallel on some systems, the file
          * doesn't get re-opened at the start automatically. */
         inputFile = fopen( filename, "r" );

         rewind( inputFile );
      }
   }
   
   fgets( lineString, MAX_LINE_LENGTH, inputFile );
   fgets( lineString, MAX_LINE_LENGTH, inputFile );

   while ( !feof(inputFile) ) {
      fscanf( inputFile, "%u ", &gNode_I );
      if( Mesh_GlobalToDomain( mesh, MT_VERTEX, gNode_I, &lNode_I ) && 
          lNode_I < Mesh_GetLocalSize( mesh, MT_VERTEX ) )
      {
         double crds[3];
         double *vert;

         fscanf( inputFile, "%lg %lg %lg ", crds, crds + 1, crds + 2 );
         vert = Mesh_GetVertex( mesh, lNode_I );
         vert[0] = crds[0];
         if( self->nDims >= 2 )
            vert[1] = crds[1];
         if( self->nDims >=3 )
            vert[2] = crds[2];
      }
      else {
         fgets( lineString, MAX_LINE_LENGTH, inputFile );
      }
   }

}


