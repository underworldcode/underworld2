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

#include <gLucifer/Base/Base.h>


#ifdef HAVE_GL2PS
#include <gl2ps.h>
#endif


#include "types.h"
#include <gLucifer/Base/DrawingObject.h>
#include "Title.h"

const Type lucTitle_Type = "lucTitle";

lucTitle* _lucTitle_New(  LUCTITLE_DEFARGS  )
{
   lucTitle*    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucTitle) );
   self = (lucTitle*)  _lucDrawingObject_New(  LUCDRAWINGOBJECT_PASSARGS  );


   return self;
}

void lucTitle_Init(
   lucTitle*                                          self,
   char*                                              titleString,
   int                                                yPos )
{
   self->titleString = Memory_Alloc_Array( char, (strlen(titleString) + 1), "self->titleString" );
   strcpy( self->titleString, titleString );
   self->yPos = yPos;
}

void _lucTitle_Delete( void* drawingObject )
{
   lucTitle*  self = (lucTitle*)drawingObject;

   Memory_Free( self->titleString );
   _lucDrawingObject_Delete( self );
}

void _lucTitle_Print( void* drawingObject, Stream* stream )
{
   lucTitle*  self = (lucTitle*)drawingObject;

   _lucDrawingObject_Print( self, stream );
}

void* _lucTitle_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof( lucTitle );
   Type                                                             type = lucTitle_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucTitle_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucTitle_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucTitle_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucTitle_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucTitle_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucTitle_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucTitle_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucTitle_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = lucDrawingObject_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucTitle_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return _lucTitle_New(  LUCTITLE_PASSARGS  );
}

void _lucTitle_AssignFromXML( void* title, Stg_ComponentFactory* cf, void* data )
{
   lucTitle*             self               = (lucTitle*) title;

   /* Construct Parent */
   _lucDrawingObject_AssignFromXML( self, cf, data );

   lucTitle_Init( self,
                  Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"string", ""  ),
                  Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"yPos", 10 )  );
}

void _lucTitle_Build( void* Title, void* data ) { }
void _lucTitle_Initialise( void* Title, void* data ) { }
void _lucTitle_Execute( void* Title, void* data ) { }
void _lucTitle_Destroy( void* Title, void* data ) { }

void _lucTitle_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   /* Deprecated: Title now set on viewport */
}


