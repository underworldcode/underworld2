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

#include "types.h"
#include "ShapeClass.h"
#include "ConvexHull.h"

#include <assert.h>
#include <string.h>
#include <math.h>

/* Textual name of this class */
const Type ConvexHull_Type = "ConvexHull";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

ConvexHull* ConvexHull_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount
		)
{
	ConvexHull* self = (ConvexHull*)_ConvexHull_DefaultNew( name );

   _Stg_Shape_Init( self, dim, centre, False, alpha, beta, gamma);
   _ConvexHull_Init( self, vertexList, vertexCount);

	return self;
}

ConvexHull* _ConvexHull_New(  CONVEXHULL_DEFARGS  )
{
	ConvexHull* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ConvexHull) );
	self = (ConvexHull*)_Stg_Shape_New(  STG_SHAPE_PASSARGS  );
	
	/* General info */

	return self;
}

void _ConvexHull_Init( void* convexHull, Coord_List vertexList, Index vertexCount) {
	ConvexHull* self = (ConvexHull*)convexHull;
	Index       numberOfFaces;
	Index       dimensions; /* this probably doesn't need to be here */
	Index       vertex_I;
	
	double        tmpVector1[3];
	double        tmpVector2[3];
	double        tmpVector3[3];
	
	dimensions = 3;
	/* Copy vertexCount */
	self->vertexCount = vertexCount;
	/* Copy vertexList */
	self->vertexList = Memory_Alloc_Array( Coord, vertexCount, "vertexList" );
	memcpy( self->vertexList , vertexList, sizeof(Coord) * vertexCount );
	
	/* Now Construct normal and store them in self->facesList */
	
	/* 1st allocate memory	 */
	if(vertexCount == 3) numberOfFaces = 3; 
	else                 numberOfFaces = 4; 
	
	self->facesList = Memory_Alloc_Array( XYZ, numberOfFaces, "facesList" );

	/* 2nd Calculate Normals on faces */
	/* in 2-D */
	if( numberOfFaces == 3 ) {
		for( vertex_I = 0; vertex_I < vertexCount ; vertex_I++ ) {
			
			StGermain_VectorSubtraction(tmpVector1, vertexList[ vertex_I % 3 ], vertexList[ (vertex_I+1) % 3 ], dimensions);
			StGermain_VectorSubtraction(tmpVector2, vertexList[ vertex_I % 3 ], vertexList[ (vertex_I+2) % 3 ], dimensions);
				
			tmpVector3[0] = tmpVector1[1];
			tmpVector3[1] = -1 * tmpVector1[0];
			tmpVector3[2] = 0;
                        /*			Now Vec3 is possibly normal vector */
				if( StGermain_VectorDotProduct(tmpVector3, tmpVector2, dimensions) < 0 )
				{ tmpVector3[0] = -1*tmpVector3[0]; tmpVector3[1] = -1 * tmpVector3[1]; }
			
			memcpy( self->facesList[ vertex_I ], tmpVector3, sizeof(XYZ) );
		}
	}
	else {
		for( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++ ) {
			StGermain_NormalToPlane( tmpVector3, vertexList[ vertex_I % 4], vertexList[ (vertex_I+1) % 4], vertexList[ (vertex_I+2) % 4]);
			StGermain_VectorSubtraction( tmpVector2, vertexList[ vertex_I % 4 ], vertexList[ (vertex_I+3) % 4 ], dimensions);

			if( StGermain_VectorDotProduct(tmpVector3, tmpVector2, dimensions) < 0 ) 
				{ Vec_Div3D( tmpVector3, tmpVector3, -1.0 ); }
			memcpy( self->facesList[ vertex_I ], tmpVector3, sizeof(XYZ) );

		}
	}
}
	
/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _ConvexHull_Delete( void* convexHull ) {
	ConvexHull*       self       = (ConvexHull*)convexHull;

	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _ConvexHull_Print( void* convexHull, Stream* stream ) {
	ConvexHull* self = (ConvexHull*)convexHull;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}

void* _ConvexHull_Copy( void* convexHull, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ConvexHull*	self = (ConvexHull*)convexHull;
	ConvexHull*	newConvexHull;
	
	newConvexHull = (ConvexHull*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );

	newConvexHull->vertexList = Memory_Alloc_Array( Coord, self->vertexCount, "vertexList" );
	memcpy( newConvexHull->vertexList , self->vertexList, sizeof(Coord) * self->vertexCount );

	newConvexHull->facesList = Memory_Alloc_Array( XYZ, self->vertexCount, "facesList" );
	memcpy( newConvexHull->facesList, self->facesList, sizeof(XYZ) * self->vertexCount );

	newConvexHull->vertexList  = self->vertexList;
	newConvexHull->vertexCount = self->vertexCount;
	newConvexHull->facesList   = self->facesList;
	
	return (void*)newConvexHull;
}

void* _ConvexHull_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                  _sizeOfSelf = sizeof(ConvexHull);
	Type                                                          type = ConvexHull_Type;
	Stg_Class_DeleteFunction*                                  _delete = _ConvexHull_Delete;
	Stg_Class_PrintFunction*                                    _print = _ConvexHull_Print;
	Stg_Class_CopyFunction*                                      _copy = _ConvexHull_Copy;
	Stg_Component_DefaultConstructorFunction*      _defaultConstructor = _ConvexHull_DefaultNew;
	Stg_Component_ConstructFunction*                        _construct = _ConvexHull_AssignFromXML;
	Stg_Component_BuildFunction*                                _build = _ConvexHull_Build;
	Stg_Component_InitialiseFunction*                      _initialise = _ConvexHull_Initialise;
	Stg_Component_ExecuteFunction*                            _execute = _ConvexHull_Execute;
	Stg_Component_DestroyFunction*                            _destroy = _ConvexHull_Destroy;
	Stg_Shape_IsCoordInsideFunction*                    _isCoordInside = _ConvexHull_IsCoordInside;
	Stg_Shape_CalculateVolumeFunction*                _calculateVolume = _ConvexHull_CalculateVolume;
	Stg_Shape_DistanceFromCenterAxisFunction*  _distanceFromCenterAxis = _ConvecHull_DistanceFromCenterAxis;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _ConvexHull_New(  CONVEXHULL_PASSARGS  );
}


void _ConvexHull_AssignFromXML( void* convexHull, Stg_ComponentFactory* cf, void* data ) {
	ConvexHull*             self       = (ConvexHull*)convexHull;
	Index                   vertexCount;
	Index                   vertex_I;
	Coord_List              vertexList;
	double*                 coord;
	Dictionary_Entry_Value* optionSet;
	Dictionary_Entry_Value* optionsList;
	Dictionary*             dictionary = Dictionary_GetDictionary( cf->componentDict, self->name );
	Stream*                 stream     = cf->infoStream;

	
	_Stg_Shape_AssignFromXML( self, cf, data );

	optionsList = Dictionary_Get( dictionary, (Dictionary_Entry_Key)"vertices"  );
	Journal_Firewall( optionsList != NULL, 
		Journal_Register( Error_Type, (Name)self->type  ),
		"In func %s: The list 'vertices' specifying the convexHull is NULL.\n", __func__);

	vertexCount = Dictionary_Entry_Value_GetCount(optionsList);
	Journal_Firewall( ( self->dim == 2 && vertexCount < 4 ) || ( self->dim == 3 && vertexCount < 5 ),
		Journal_Register( Error_Type, (Name)self->type  ),
		"In func %s: Sorry, but we got lazy, you can only specify 3 (2D) or 4 (3D) points. " 
		"Please feel free to hassle developers for this feature.\n", __func__);

	/* Allocate space */
	vertexList = Memory_Alloc_Array( Coord , vertexCount, "Vertex Array" );
	memset( vertexList, 0, vertexCount * sizeof(Coord) );
	
	Stream_Indent( stream );
	for ( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++) { 
		optionSet = Dictionary_Entry_Value_GetElement(optionsList, vertex_I );
		coord = vertexList[vertex_I];

		/* Read Vertex */
		coord[ I_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, (Dictionary_Entry_Key)"x") );
		coord[ J_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, (Dictionary_Entry_Key)"y") );
		
		coord[ K_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, (Dictionary_Entry_Key)"z"));
		optionSet = optionSet->next;
	}
	Stream_UnIndent( stream  );


	_ConvexHull_Init( self, vertexList, vertexCount);
}

void _ConvexHull_Build( void* convexHull, void* data ) {
	ConvexHull*	self = (ConvexHull*)convexHull;

	_Stg_Shape_Build( self, data );
}
void _ConvexHull_Initialise( void* convexHull, void* data ) {
	ConvexHull*	self = (ConvexHull*)convexHull;
	
	_Stg_Shape_Initialise( self, data );
}
void _ConvexHull_Execute( void* convexHull, void* data ) {
	ConvexHull*	self = (ConvexHull*)convexHull;
	
	_Stg_Shape_Execute( self, data );
}
void _ConvexHull_Destroy( void* convexHull, void* data ) {
	ConvexHull*	self = (ConvexHull*)convexHull;
   Coord_List        vertexList = self->vertexList;
	XYZ*              facesList  = self->facesList;

	Memory_Free( vertexList );
	Memory_Free( facesList );
	 
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/
Bool _ConvexHull_IsCoordInside( void* convexHull, Coord point ) {
	ConvexHull*     self                 = (ConvexHull*)convexHull;
	Index           vertex_I;
	XYZ             tmpVector;
	/*Stream*                 stream     = cf->infoStream; */


	/* for the particle to be inside the shape, the dot product of its position vector with the shape face normal's must be non-positive */
	for( vertex_I = 0 ; vertex_I < self->vertexCount ; vertex_I++ ) {
		StGermain_VectorSubtraction(tmpVector, point, self->vertexList[ vertex_I ], self->dim );
		if( StGermain_VectorDotProduct(self->facesList[ vertex_I ], tmpVector, self->dim ) > 0 ) {
			return False;
		}
	}
	return True;
}

double _ConvexHull_CalculateVolume( void* convexHull ) {
	assert( 0 );
	return 0.0;
}
void _ConvecHull_DistanceFromCenterAxis( void* shape, Coord coord, double* disVec ) {
	Stg_Shape* self = (Stg_Shape*)shape;
	Journal_Firewall( False, Journal_Register( Error_Type, (Name)self->type  ),
	"Error in function %s: This functions hasn't been implemented.",  __func__ );
}




