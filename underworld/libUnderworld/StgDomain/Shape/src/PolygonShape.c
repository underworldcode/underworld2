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
#include "PolygonShape.h"

#include <assert.h>
#include <string.h>
#include <math.h>

/* Textual name of this class */
const Type PolygonShape_Type = "PolygonShape";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

PolygonShape* PolygonShape_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount,
		XYZ                                   start,
		XYZ                                   end )
{ 
	PolygonShape* self = (PolygonShape*)_PolygonShape_DefaultNew( name );

   _Stg_Shape_Init( self, dim, centre, False, alpha, beta, gamma);
   _PolygonShape_Init( self, vertexList, vertexCount, start, end );
	return self;
}

PolygonShape* _PolygonShape_New(  POLYGONSHAPE_DEFARGS  )
{
	PolygonShape* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PolygonShape) );
	self = (PolygonShape*)_Stg_Shape_New(  STG_SHAPE_PASSARGS  );
	
	/* General info */
	return self;
}

void _PolygonShape_Init( void* polygon, Coord_List vertexList, Index vertexCount, XYZ start, XYZ end ) {
	PolygonShape* self = (PolygonShape*)polygon;
   /* original coordinate storage */
   XYZ           normal, centroid;
   double alpha, beta, gamma;
   double plane_constant;
   int v_i;

   self->vertexList = Memory_Alloc_Array( Coord, vertexCount, "vertexList" );
	memcpy( self->vertexList , vertexList, sizeof(Coord) * vertexCount );
	self->vertexCount = vertexCount;
	memcpy( self->start , start, sizeof(XYZ) );
	memcpy( self->end , end, sizeof(XYZ) );

   centroid[0] = centroid[1] = centroid[2] = 0.0;
   normal[0] = normal[1] = normal[2] = 0.0;
   normal[2] = 1.0;
   if( self->dim == 3 ) {
      /* using any point, find plane eqn */
      plane_constant = normal[0]*vertexList[0][0] + normal[1]*vertexList[0][1] + normal[2]*vertexList[0][2];
   }
   /* find centroid and check each vertex is satisfies this plane equation */
   for( v_i = 0 ; v_i < vertexCount ; v_i++ ) {
      if( self->dim == 3 ) {
         double tmp = normal[0]*vertexList[v_i][0] + normal[1]*vertexList[v_i][1] + normal[2]*vertexList[v_i][2];
         if( fabs(tmp - plane_constant ) > 1e-5 ) {
            printf("Error in plane check, vertex %d, with coord (%g, %g, %g) has a plane constant of %g, where as other have plane constant of %g\n\n", v_i, vertexList[v_i][0], vertexList[v_i][1], vertexList[v_i][2], tmp, plane_constant );
         }
      }
      /* do centroid calculation */
      centroid[0] += vertexList[v_i][0];
      centroid[1] += vertexList[v_i][1];
      centroid[2] += vertexList[v_i][2];
   }

   self->centroid[0] = centroid[0] / vertexCount;
   self->centroid[1] = centroid[1] / vertexCount;
   if( self->dim == 3 ) self->centroid[2] = 0.5 * (self->start[2]+self->end[2]);
   else self->centroid[2] = 0;

   /* setup other rotations */
   alpha = self->rotations[0];
   beta = self->rotations[1];
   gamma = self->rotations[2];
}
	
/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PolygonShape_Delete( void* polygon ) {
	PolygonShape*       self = (PolygonShape*)polygon;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _PolygonShape_Print( void* polygon, Stream* stream ) {
	PolygonShape* self = (PolygonShape*)polygon;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _PolygonShape_Copy( void* polygon, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	PolygonShape*	self = (PolygonShape*)polygon;
	PolygonShape*	newPolygonShape;
	
	newPolygonShape = (PolygonShape*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );

	newPolygonShape->vertexList = Memory_Alloc_Array( Coord, self->vertexCount, "vertexList" );
	memcpy( newPolygonShape->vertexList , self->vertexList, sizeof(Coord) * self->vertexCount );

	newPolygonShape->vertexList  = self->vertexList;
	newPolygonShape->vertexCount = self->vertexCount;
	memcpy( newPolygonShape->start, self->start, sizeof(XYZ) );
	memcpy( newPolygonShape->end, self->end, sizeof(XYZ) );
	
	return (void*)newPolygonShape;
}

void* _PolygonShape_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                  _sizeOfSelf = sizeof(PolygonShape);
	Type                                                          type = PolygonShape_Type;
	Stg_Class_DeleteFunction*                                  _delete = _PolygonShape_Delete;
	Stg_Class_PrintFunction*                                    _print = _PolygonShape_Print;
	Stg_Class_CopyFunction*                                      _copy = _PolygonShape_Copy;
	Stg_Component_DefaultConstructorFunction*      _defaultConstructor = _PolygonShape_DefaultNew;
	Stg_Component_ConstructFunction*                        _construct = _PolygonShape_AssignFromXML;
	Stg_Component_BuildFunction*                                _build = _PolygonShape_Build;
	Stg_Component_InitialiseFunction*                      _initialise = _PolygonShape_Initialise;
	Stg_Component_ExecuteFunction*                            _execute = _PolygonShape_Execute;
	Stg_Component_DestroyFunction*                            _destroy = _PolygonShape_Destroy;
	Stg_Shape_IsCoordInsideFunction*                    _isCoordInside = _PolygonShape_IsCoordInside;
	Stg_Shape_CalculateVolumeFunction*                _calculateVolume = _PolygonShape_CalculateVolume;
	Stg_Shape_DistanceFromCenterAxisFunction*  _distanceFromCenterAxis = _PolygonShape_DistanceFromCenterAxis;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _PolygonShape_New(  POLYGONSHAPE_PASSARGS  );
}


void _PolygonShape_AssignFromXML( void* polygon, Stg_ComponentFactory* cf, void* data ) {
	PolygonShape*           self       = (PolygonShape*)polygon;
	Index                   vertexCount;
	Index                   vertex_I;
	Coord_List              vertexList;
	XYZ                     start;
	XYZ                     end;
	double*                 coord;
	Dictionary_Entry_Value* optionSet;
	Dictionary_Entry_Value* optionsList;
	Dictionary*             dictionary  = Dictionary_GetDictionary( cf->componentDict, self->name );
	Stream*                 stream      = cf->infoStream;
	Stream*                 errorStream = Journal_Register( Error_Type, (Name)self->type  );
	
	_Stg_Shape_AssignFromXML( self, cf, data );

	start[K_AXIS] = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"startZ", 0.0  );
	end[K_AXIS]   = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"endZ", 0.0  );

   /* check if depth parameters are ok */
   if( ( start[2] > end[2] ) && self->dim == 3 ) {
      /* swap start and end, because algorithm assumes start is less than end */
      double tmp;
      tmp = start[2]; 
      start[2] = end[2];
      end[2] = tmp;
   }

	optionsList = Dictionary_Get( dictionary, (Dictionary_Entry_Key)"vertices" );
	
	Journal_Firewall( optionsList != NULL , errorStream, 
			"\n\nUnable to find vertices list required to build shape '%s' named '%s'.\n"
			"Please check that you have specified a vertices list, and that the list is named correctly.\n\n\n",
			self->type, self->name );

	vertexCount = Dictionary_Entry_Value_GetCount(optionsList );
	Journal_Firewall( vertexCount >= 3, errorStream, 
			"Too few vertices given in trying to build shape '%s' named '%s'.\n"
			"A polygon needs at least three vertices.\n",
			self->type, self->name );

	/* Allocate space */
	vertexList = Memory_Alloc_Array( Coord , vertexCount, "Vertex Array" );
	memset( vertexList, 0, vertexCount * sizeof(Coord) );

	Stream_Indent( stream );
	for ( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++) { 
		optionSet = Dictionary_Entry_Value_GetElement(optionsList, vertex_I );
		coord = vertexList[vertex_I];
		/* Read Vertex */
			coord[ I_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, (Dictionary_Entry_Key)"x"));
			coord[ J_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, (Dictionary_Entry_Key)"y"));
			coord[ K_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, (Dictionary_Entry_Key)"z") );

		/* Print Position */
		Journal_PrintfL( stream, 2, "(%0.3g, %0.3g, %0.3g)\n", coord[I_AXIS], coord[J_AXIS], coord[K_AXIS] );
	}
	Stream_UnIndent( stream );

	_PolygonShape_Init( self, vertexList, vertexCount, start, end );

	Memory_Free( vertexList );
}

void _PolygonShape_Build( void* polygon, void* data ) {
	PolygonShape*	self = (PolygonShape*)polygon;

	_Stg_Shape_Build( self, data );
}
void _PolygonShape_Initialise( void* polygon, void* data ) {
	PolygonShape*	self = (PolygonShape*)polygon;
	
	_Stg_Shape_Initialise( self, data );
}
void _PolygonShape_Execute( void* polygon, void* data ) {
	PolygonShape*	self = (PolygonShape*)polygon;
	
	_Stg_Shape_Execute( self, data );
}
void _PolygonShape_Destroy( void* polygon, void* data ) {
	PolygonShape*	self = (PolygonShape*)polygon;

	Coord_List     vertexList = self->vertexList;
	Memory_Free( vertexList );

	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/
/* Algorithm describe Paul Bourke's page http://astronomy.swin.edu.au/~pbourke/geometry/insidepoly/ (see solution 2)
 *
 * Algorithm works by summing the angles between the test coordinate and each pair of vertices that make up an edge 
 * in the polygon. An inside point will give an angle of 2pi and and outside point will give an angle of 0 */

Bool _PolygonShape_IsCoordInside( void* polygon, const Coord coord ) {
	PolygonShape*        self                = (PolygonShape*) polygon;
	Index           vertexCount         = self->vertexCount;
   /* original coordinate storage */
	Coord_List      o_vertexList  = self->vertexList;
   /* transformed coordinate storage */
	XYZ             vectorToStartVertex = { 0.0, 0.0, 0.0 };
	XYZ             vectorToEndVertex   = { 0.0, 0.0, 0.0 };
	XYZ             crossproduct        = { 0.0, 0.0, 0.0 };
	double          currAngle;
	double          totalAngle          = 0.0;
	Index           vertex_I;
	double*         startVertex;
	double*         endVertex;

	/* Check to make sure that the coordinate is within startZ and endZ in 3D */
	if ( self->dim == 3 && ( coord[ 2 ] < self->start[2] || coord[ 2 ] > self->end[2] ))
		return False;	

	for ( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++ ) {
		/* Get vertices of current edge */
		startVertex = o_vertexList[ vertex_I ];
		endVertex   = o_vertexList[ (vertex_I + 1) % vertexCount ];

		/* Work out vectors */
		StGermain_VectorSubtraction( vectorToStartVertex, coord, startVertex, 3 );
		StGermain_VectorSubtraction( vectorToEndVertex,   coord, endVertex,  3 );

		vectorToStartVertex[ 2 ] = 0;
		vectorToEndVertex[ 2 ] = 0;

		/* Work out angle - just by doing dot product - will always be positive */
		currAngle = StGermain_AngleBetweenVectors( vectorToStartVertex, vectorToEndVertex, 3 );

		/* Work out 'sign' of angle but working out cross product */
		StGermain_VectorCrossProduct( crossproduct, vectorToEndVertex, vectorToStartVertex );

		if ( crossproduct[ 2 ] > 0.0 )
			totalAngle += currAngle;
		else
			totalAngle -= currAngle;
	}

	/* work out whether the coord is within the polygon */
	if ( fabs( totalAngle ) < M_PI )
		return False;
	else  
		return True;
}


double _PolygonShape_CalculateVolume( void* polygon ) {
	assert( 0 );
	return 0.0;
}

void _PolygonShape_DistanceFromCenterAxis( void* shape, Coord coord, double* disVec ){
	Stg_Shape* self = (Stg_Shape*)shape;
	Journal_Firewall( False, Journal_Register( Error_Type, (Name)self->type  ),
	"Error in function %s: This functions hasn't been implemented.", __func__ );
}


