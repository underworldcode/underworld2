/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __lucCamera_h__
#define __lucCamera_h__

extern const Type lucCamera_Type;

#define __lucCamera \
		__Stg_Component \
		AbstractContext*	context; \
		FieldVariable*		centreFieldVariable; \
      lucCoordinateSystem  coordSystem;   \
		Coord					focalPoint; \
		Coord					translate; \
		Coord					rotate; \
		Coord					rotationCentre; \
		XYZ					upDirection; \
		double				focalLength; \
		double				aperture; \
        int               autoZoomTimestep; \
        int               useBoundingBox; \
		double            modelSize; \
		Bool					autoFitMode; \

struct lucCamera
{
   __lucCamera
};

/** Constructors */
lucCamera* lucCamera_New(
   Name					name,
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
   int                  useBoundingBox);

#ifndef ZERO
#define ZERO 0
#endif

#define LUCCAMERA_DEFARGS \
                STG_COMPONENT_DEFARGS

#define LUCCAMERA_PASSARGS \
                STG_COMPONENT_PASSARGS

lucCamera* _lucCamera_New(  LUCCAMERA_DEFARGS  );

/** Virtual Functions */
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
   int                  useBoundingBox);

void _lucCamera_Delete( void* camera );
void _lucCamera_Print( void* camera, Stream* stream );
void _lucCamera_Copy( void* camera, void* dest );
void* _lucCamera_DefaultNew( Name name );
void _lucCamera_AssignFromXML( void* camera, Stg_ComponentFactory* cf, void* data );
void _lucCamera_Build( void* camera, void* data );
void _lucCamera_Initialise( void* camera, void* data );
void _lucCamera_Execute( void* camera, void* data );
void _lucCamera_Destroy( void* camera, void* data );

/** Public Functions */
double lucCamera_GetFocalLength( void* camera );
void lucCamera_CentreFromFieldVariable( void* camera ) ;

#endif

