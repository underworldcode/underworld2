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
#include <StgDomain/StgDomain.h>
#include "types.h"
#include "Camera.h"
#include "Init.h"
#include <gLucifer/Base/Base.h>
#include <gLucifer/Base/Viewport.h>

#include <string.h>
#include <assert.h>

const Type lucCamera_Type = "lucCamera";

lucCamera* lucCamera_New(
   Name                    name,
   lucCoordinateSystem     coordSystem,
   Coord                   focalPoint,
   Coord                   translate,
   Coord                   rotate,
   Coord                   rotationCentre,
   XYZ                     upDirection,
   double                  focalLength,
   double                  aperture,
   FieldVariable*          centreFieldVariable,
   int                     autoZoomTimestep,
   int                     useBoundingBox)
{
   lucCamera* self = (lucCamera*) _lucCamera_DefaultNew( name );

   self->isConstructed = True;
   _lucCamera_Init( self, coordSystem, focalPoint, translate, rotate, rotationCentre, upDirection, focalLength, aperture, centreFieldVariable, autoZoomTimestep, useBoundingBox );

   return self;
}

void* _lucCamera_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof( lucCamera );
   Type                                                      type = lucCamera_Type;
   Stg_Class_DeleteFunction*                              _delete = _lucCamera_Delete;
   Stg_Class_PrintFunction*                                _print = _lucCamera_Print;
   Stg_Class_CopyFunction*                                  _copy = _Stg_Class_Copy;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _lucCamera_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _lucCamera_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _lucCamera_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _lucCamera_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _lucCamera_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _lucCamera_Destroy;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return _lucCamera_New(  LUCCAMERA_PASSARGS  );
}

lucCamera* _lucCamera_New(  LUCCAMERA_DEFARGS  )
{
   lucCamera* self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucCamera) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (lucCamera*) _Stg_Component_New(  STG_COMPONENT_PASSARGS  );

   return self;
}

void _lucCamera_Init(
   void*                camera,
   lucCoordinateSystem  coordSystem,
   Coord                focalPoint,
   Coord                translate,
   Coord                rotate,
   Coord                rotationCentre,
   XYZ                  upDirection,
   double               focalLength,
   double               aperture,
   FieldVariable*       centreFieldVariable,
   int                  autoZoomTimestep,
   int                  useBoundingBox )
{
   lucCamera* self = camera;

   memcpy( self->focalPoint, focalPoint, sizeof(Coord) );
   memcpy( self->translate, translate, sizeof(Coord) );
   memcpy( self->rotate, rotate, sizeof(Coord) );
   memcpy( self->rotationCentre, rotationCentre, sizeof(Coord) );
   memcpy( self->upDirection, upDirection, sizeof(XYZ) );

   self->coordSystem           = coordSystem;
   self->focalLength           = focalLength;
   self->aperture              = aperture;
   self->centreFieldVariable   = centreFieldVariable;
   self->autoZoomTimestep      = autoZoomTimestep;
   self->useBoundingBox        = useBoundingBox;

   /* Default model size */
   self->modelSize = 1.0;

   self->autoFitMode = False;
}

void _lucCamera_Delete( void* camera )
{
   lucCamera* self = camera;

   _Stg_Component_Delete( self );
}

void _lucCamera_Print( void* camera, Stream* stream )
{
   lucCamera* self        = camera;

   Journal_Printf( stream, "lucCamera: %s\n", self->name );

   Stream_Indent( stream );

   /* Print Parent */
   _Stg_Component_Print( self, stream );

   Journal_Printf( stream, "self->coordSystem = ");
   switch ( self->coordSystem )
   {
   case lucLeftHanded:
      Journal_Printf( stream, "lucLeftHanded\n" ); break;
   case lucRightHanded:
      Journal_Printf( stream, "lucRightHanded\n" ); break;
   }

   /* Flip z before output if using left-handed coords, then reverse */
   self->focalPoint[K_AXIS] *= (double)self->coordSystem;
   self->rotationCentre[K_AXIS] *= (double)self->coordSystem;
   self->upDirection[K_AXIS] *= (double)self->coordSystem;
   Journal_PrintArray( stream, self->focalPoint, 3 );
   Journal_PrintArray( stream, self->translate, 3 );
   Journal_PrintArray( stream, self->rotate, 3 );
   Journal_PrintArray( stream, self->rotationCentre, 3 );
   Journal_PrintArray( stream, self->upDirection, 3 );
   self->focalPoint[K_AXIS] *= (double)self->coordSystem;
   self->rotationCentre[K_AXIS] *= (double)self->coordSystem;
   self->upDirection[K_AXIS] *= (double)self->coordSystem;

   Journal_PrintValue( stream, self->aperture );
   Journal_PrintValue( stream, self->autoZoomTimestep );

   Stream_UnIndent( stream );
}

void _lucCamera_Copy( void* camera, void* dest )
{
   lucCamera* self        = camera;
   lucCamera* newCamera   = dest;

   memcpy( newCamera->focalPoint,     self->focalPoint,     sizeof(Coord) );
   memcpy( newCamera->translate,      self->translate,      sizeof(Coord) );
   memcpy( newCamera->rotate,         self->rotate,         sizeof(Coord) );
   memcpy( newCamera->rotationCentre, self->rotationCentre, sizeof(Coord) );
   memcpy( newCamera->upDirection,    self->upDirection,    sizeof(XYZ  ) );

   newCamera->coordSystem   = self->coordSystem;
   newCamera->focalLength   = self->focalLength;
   newCamera->aperture      = self->aperture;
   newCamera->autoZoomTimestep = self->autoZoomTimestep;
}

void _lucCamera_AssignFromXML( void* camera, Stg_ComponentFactory* cf, void* data )
{
   lucCamera*              self               = (lucCamera*) camera;
   Coord                   focalPoint;
   Coord                   translate;
   Coord                   rotate;
   Coord                   rotationCentre;
   XYZ                     upDirection;
   FieldVariable*          centreFieldVariable;
   double                  focalLength;
   double                  aperture;
   Name                    coordSystemName;
   lucCoordinateSystem     coordSystem;
   double                  coordZ;
   int                     autoZoomTimestep;
   int                     useBoundingBox;
 
   /* Get Coordinate System */
   /* - left-handed has positive z-axis pointing into screen */
   /* - defaults to OpenGL default right-handed, negative z-axis points into screen */
   coordSystemName = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"CoordinateSystem", "RightHanded" );
   if ( strcasecmp( coordSystemName, "LeftHanded" ) == 0 ) 
      coordSystem = lucLeftHanded;
   else 
      coordSystem = lucRightHanded;

   focalPoint[I_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"focalPointX", 0.0);
   focalPoint[J_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"focalPointY", 0.0);
   focalPoint[K_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"focalPointZ", 0.0);

   /* Old camera used hard coded coord position, retrieve Z f available and use as default Z translation */
   coordZ  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"coordZ", 0.0);
   translate[I_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"translateX", 0.0);
   translate[J_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"translateY", 0.0);
   translate[K_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"translateZ", -coordZ * (double)coordSystem);

   rotate[I_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"rotateX", 0.0);
   rotate[J_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"rotateY", 0.0);
   rotate[K_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"rotateZ", 0.0);

   rotationCentre[I_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"rotationCentreX", focalPoint[I_AXIS]);
   rotationCentre[J_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"rotationCentreY", focalPoint[J_AXIS]);
   rotationCentre[K_AXIS]  = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"rotationCentreZ", focalPoint[K_AXIS]);

   upDirection[I_AXIS] = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"upDirectionX", 0.0);
   upDirection[J_AXIS] = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"upDirectionY", 1.0);
   upDirection[K_AXIS] = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"upDirectionZ", 0.0);

   /* Adjust for coord system in Z-Axis as these coords are used before applying scaling for left-handed coords */
   upDirection[K_AXIS] *= (double)coordSystem;
   rotationCentre[K_AXIS] *= (double)coordSystem;
   focalPoint[K_AXIS] *= (double)coordSystem;

   /* These parameters alow overriding some view properties, but usually the calculated defaults are the best settings */
   focalLength = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"focalLength", 0);
   aperture = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"aperture", 45.0  );
   autoZoomTimestep = Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"autoZoomTimestep", 0 );
   useBoundingBox = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"useBoundingBox", False );

   centreFieldVariable = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"CentreFieldVariable", FieldVariable, False, data );

   _lucCamera_Init( self, coordSystem, focalPoint, translate, rotate, rotationCentre, upDirection, focalLength, aperture, centreFieldVariable, autoZoomTimestep, useBoundingBox);
}

void _lucCamera_Build( void* camera, void* data )
{
   lucCamera*     self = (lucCamera*) camera;
   FieldVariable* fieldVariable = self->centreFieldVariable;

   if ( fieldVariable )
   {
      Stg_Component_Build( fieldVariable, data, False );
   }
}
void _lucCamera_Initialise( void* camera, void* data )
{
   lucCamera*     self = (lucCamera*) camera;
   FieldVariable* fieldVariable = self->centreFieldVariable;

   if ( fieldVariable )
   {
      Stg_Component_Initialise( fieldVariable, data, False );
      lucCamera_CentreFromFieldVariable( self );
   }
}

void _lucCamera_Execute( void* camera, void* data ) { }

void _lucCamera_Destroy( void* camera, void* data )
{
}

double lucCamera_GetFocalLength( void* camera )
{
   lucCamera* self = camera;
   double     focalLength;

   /* Default focal length is distance between viewpoint and focal point */
   focalLength = self->focalLength;
   if (!focalLength)
      focalLength = StGermain_VectorMagnitude(self->translate, 3);

   return focalLength;
}

void lucCamera_CentreFromFieldVariable( void* camera )
{
   lucCamera*     self = (lucCamera*) camera;
   Coord          min, max, dims;
   Coord          oldFocalPoint;
   FieldVariable* fieldVariable = self->centreFieldVariable;

   if ( !fieldVariable ) return;

   /* Ensure z coord zero or will be unitialised in 2d */
   min[K_AXIS] = max[K_AXIS] = dims[K_AXIS] = 0.0;
   FieldVariable_GetMinAndMaxGlobalCoords( fieldVariable, min, max );

   /* Calculate dimensions */
   dims[I_AXIS] = (max[I_AXIS] - min[I_AXIS]);
   dims[J_AXIS] = (max[J_AXIS] - min[J_AXIS]);
   if (fieldVariable->dim > 2) dims[K_AXIS] = (max[K_AXIS] - min[K_AXIS]);

   /* Set up focal point */
   memcpy( oldFocalPoint, self->focalPoint, sizeof(Coord) );
   self->focalPoint[I_AXIS] = min[I_AXIS] + 0.5 * dims[I_AXIS];
   self->focalPoint[J_AXIS] = min[J_AXIS] + 0.5 * dims[J_AXIS];
   self->focalPoint[K_AXIS] = (min[K_AXIS] + 0.5 * dims[K_AXIS]) * (double)self->coordSystem;

   /* Save the largest dimension of the model */
   self->modelSize = dims[I_AXIS];
   if (dims[J_AXIS] > self->modelSize)
      self->modelSize = dims[J_AXIS];
   if (fieldVariable->dim > 2 && dims[K_AXIS] > self->modelSize)
      self->modelSize = dims[K_AXIS];

   /* Set a default viewpoint translation: 
    * move view back based on largest dimension of the model */
   if (!self->translate[K_AXIS])
   {
      if (fieldVariable->dim > 2)
         self->translate[K_AXIS] = -self->modelSize * (60.0 / self->aperture + 0.55);
      else
         self->translate[K_AXIS] = -self->modelSize * (60.0 / self->aperture + 0.1);
   }

   /*fprintf(stderr, "coordSys %s -- min %f,%f,%f max %f,%f,%f model Size %f focal point %f,%f,%f\n", 
      (self->coordSystem > 0 ? "RH":"LH" ), min[0], min[1], min[2], max[0], max[1], max[2], self->modelSize, self->focalPoint[0], self->focalPoint[1], self->focalPoint[2]);*/

   /* Copy focal point to the rotation centre */
   memcpy( self->rotationCentre, self->focalPoint, sizeof( Coord ) );
}


