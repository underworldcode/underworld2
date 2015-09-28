/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <string.h>
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "Base.h"

#include <gLucifer/DrawingObjects/DrawingObjects.h>

#include "Database.h"
#include "Viewport.h"

#ifndef MASTER
#define MASTER 0
#endif

const Type lucWindow_Type = "lucWindow";

lucWindow* _lucWindow_New(  LUCWINDOW_DEFARGS  )
{
   lucWindow*    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucWindow) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (lucWindow*) _Stg_Component_New(  STG_COMPONENT_PASSARGS  );

   /* Virtual functions */
   self->_displayWindow = _displayWindow;

   return self;
}

void _lucWindow_Init(
   lucWindow*                                         self,
   lucViewport**                                      viewportList,
   Viewport_Index                                     viewportCount,
   AbstractContext*                                   context,
   Pixel_Index                                        width,
   Pixel_Index                                        height,
   Name                                               backgroundColourName,
   Bool                                               antialias,
   lucDatabase*                                       database,
   Bool                                               useModelBounds,
   Bool                                               disabled)
{
   self->context = (DomainContext*)context;
   self->width = width;
   self->height = height;
   self->antialias = antialias;
   self->useModelBounds = useModelBounds;
   self->disabled = disabled;
   if(self->context)
      self->disabled = !self->context->vis;
   self->database = database;

   if (!self->database)
   {
      /* Use a default database if necessary, stored in a global so other windows will share it */
      if (!defaultDatabase)
         defaultDatabase = lucDatabase_New(context, 0, True, False, False, True, True, "gLuciferDefault", NULL);
      self->database = defaultDatabase;
   }

   self->viewportList = viewportList; 
   self->viewportCount = viewportCount;

   lucColour_FromString( &self->backgroundColour, backgroundColourName );

   if (!disabled && context)
   {
      /* Get all windows to update first. */
      //EP_PrependClassHook( Context_GetEntryPoint( context, AbstractContext_EP_DumpClass ), lucWindow_Update, self );
      /* Get window to 'execute' at each 'Dump' entry point. */
      EP_AppendClassHook(  Context_GetEntryPoint( context, AbstractContext_EP_DumpClass ), self->_execute, self );
   }

   self->title = Memory_Alloc_Array( char, 500, "title string" );
   strcpy(self->title, " gLucifer output: ");
   strcat(self->title, self->name);
   
   int rank;
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   if (rank == 0 )
      self->isMaster = True;
   else
      self->isMaster = False;

}

void _lucWindow_Delete( void* window )
{
   lucWindow*     self      = (lucWindow*)window;

   if (self->database == defaultDatabase) _lucDatabase_Delete(self->database);
   defaultDatabase = NULL;

   _Stg_Component_Delete( window );
}

void _lucWindow_Print( void* window, Stream* stream )
{
   lucWindow*          self        = window;

   Journal_Printf( stream, "lucWindow: %s\n", self->name );

   Stream_Indent( stream );

   /* Print Parent */
   _Stg_Component_Print( self, stream );


   Journal_PrintValue( stream, self->width );
   Journal_PrintValue( stream, self->height );

   Stream_UnIndent( stream );
}

void* _lucWindow_Copy( void* window, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap )
{
   lucWindow* self        = window;
   lucWindow* newWindow;

   newWindow = _Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );

   /* TODO */
   abort();

   return (void*) newWindow;
}

void* _lucWindow_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof( lucWindow );
   Type                                                      type = lucWindow_Type;
   Stg_Class_DeleteFunction*                              _delete = _lucWindow_Delete;
   Stg_Class_PrintFunction*                                _print = _lucWindow_Print;
   Stg_Class_CopyFunction*                                  _copy = _lucWindow_Copy;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _lucWindow_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _lucWindow_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _lucWindow_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _lucWindow_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _lucWindow_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _lucWindow_Destroy;
   lucWindow_DisplayFunction*                      _displayWindow = lucWindow_Display;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return _lucWindow_New(  LUCWINDOW_PASSARGS  );
}

void _lucWindow_AssignFromXML( void* window, Stg_ComponentFactory* cf, void* data )
{
   lucWindow*               self = window;
   lucViewport**            viewportList;
   Viewport_Index           viewportCount;
   Pixel_Index              width;
   Pixel_Index              height;
   AbstractContext*         context;

   /* Grab information about what viewports are going to be plotted in this window */
   viewportList = lucWindow_ConstructViewportList( self, cf, &viewportCount, data );
   width = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"width", self->width );
   height = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"height", self->height  );

   /* The window needs information about the context so that it can attach itself
    * onto the AbstractContext_EP_DumpClass entry point. */
   context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
   if (!context)
      context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, False, data  );

   _lucWindow_Init(
      self,
      viewportList,
      viewportCount,
      context,
      width,
      height,
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"backgroundColour", "white"  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"antialias", True ),
      Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Database", lucDatabase, False, data ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"useModelBounds", True ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"disable", False  )
   );
}

void _lucWindow_Build( void* window, void* data )
{
   lucWindow* self = (lucWindow*)window;
   Stg_Component_Build(self->database, data, False);
}

void _lucWindow_Initialise( void* window, void* data )
{
   lucWindow* self = (lucWindow*)window;
   /* Create viewer, setup window, viewports, drawing objects, colourmaps */
   if (!self->disabled && self->isMaster)
      lucDatabase_OutputWindow(self->database, self);
}

void _lucWindow_Execute( void* window, void* data )
{
   lucWindow* self = (lucWindow*) window ;
   Viewport_Index          viewport_I;

   /* Call setup on viewports */
   for ( viewport_I = 0 ; viewport_I < self->viewportCount ; viewport_I++ )
   {
      lucViewport_Setup( self->viewportList[ viewport_I ], self->database);
      /* Flag viewports need to re-render new information */
      self->viewportList[ viewport_I ]->needsToDraw = True;
   }

   /* Draw Window, initial output */
   lucWindow_Display(window);

   /* Clean up drawing objects */
   lucWindow_CleanUp( window );
}

void _lucWindow_Destroy( void* window, void* data )
{
   lucWindow* self = (lucWindow*)window;

   Memory_Free(self->title);
}

void lucWindow_Display( void* window )
{
   /* Display function */
   lucWindow* self = (lucWindow*)window;
   lucViewport*          viewport;
   Viewport_Index        viewport_I;
   Viewport_Index        viewportCount     = self->viewportCount;

   /* Process each viewport in list for the active window */
   for ( viewport_I = 0 ; viewport_I < viewportCount ; viewport_I++ )
   {
      viewport = self->viewportList[ viewport_I ];

      Journal_DPrintfL( lucDebug, 2, "In loop for viewport '%s'.\n", viewport->name );
      Stream_Indent( lucDebug );

      /* Set viewport */
      Journal_DPrintfL( lucDebug, 2, "Rendering viewport: (%s)\n", viewport->name);

      if ( ! viewport->needsToDraw )
      {
         Journal_DPrintfL( lucDebug, 2, "Viewport '%s' doesn't need to be redrawn.\n", viewport->name );
         Stream_UnIndent( lucDebug );
         continue;
      }

      lucViewport_Draw(viewport, self->database);
   }
}

void lucWindow_CleanUp( void* window )
{
   lucWindow*     self      = (lucWindow*) window;
   Viewport_Index viewport_I;
   Viewport_Index viewportCount = self->viewportCount;
   lucViewport*   viewport;

   for ( viewport_I = 0 ; viewport_I < viewportCount ; viewport_I++ )
   {
      viewport = self->viewportList[ viewport_I ];

      lucViewport_CleanUp( viewport );
   }
}

lucViewport** lucWindow_ConstructViewportList(
   lucWindow* self,
   Stg_ComponentFactory* cf,
   Viewport_Index* viewportCount,
   void* data )
{
   Viewport_Index          viewport_I, vertical_I, horizontal_I;
   Viewport_Index          verticalCount, horizontalCount, maxHorizontalCount = 0;
   Viewport_Index          total_I                 = 0;
   Dictionary_Entry_Value* list;
   char*                   charPtr;
   char*                   horizontalVP_String;
   const char*             breakChars              = "\n\t ,;";
   lucViewport**           viewportList            = NULL;
   Dictionary*             dictionary              = Dictionary_GetDictionary( cf->componentDict, self->name );
   Name                    viewportName[16];
   //char*                   savePtr;

   *viewportCount = 0;
   list = Dictionary_Get( dictionary, (Dictionary_Entry_Key)"Viewport"  );
   Journal_Firewall( list != NULL, lucError, "Cannot Find 'Viewport' for %s '%s'.\n", self->type, self->name );

   /* Layout saved as number of rows then number of columns in each row */ 
   verticalCount = Dictionary_Entry_Value_GetCount( list );
   self->viewportLayout = Memory_Alloc_Array(int, verticalCount+1, "viewportLayout");
   self->viewportLayout[0] = verticalCount;

   for ( vertical_I = 0 ; vertical_I < verticalCount ; vertical_I++ )
   {
      horizontalVP_String = StG_Strdup( Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( list, vertical_I ) ) );

      /* Find number of horizontal layers */
      horizontalCount = 0;
      char* horizVPstr_ptr = horizontalVP_String;
      //charPtr = strtok( horizVPstr_ptr , breakChars, &savePtr);
      charPtr = strtok(horizVPstr_ptr , breakChars);
      while ( charPtr != NULL )
      {
         /* Check for duplicates in list, not allowed to re-use same viewport more than once in a window */
         for (viewport_I = 0; viewport_I < total_I; viewport_I++)
            Journal_Firewall( strcmp(charPtr, viewportList[viewport_I]->name) != 0, lucError,
                     "%s Error loading viewport '%s' in window '%s' - Duplicate viewport in window.\n",
                     __func__, charPtr, self->name );

         viewportName[horizontalCount] = StG_Strdup( charPtr );
         horizontalCount++;
         //charPtr = strtok_r( NULL , breakChars, &savePtr);
         charPtr = strtok(NULL , breakChars);
         /* Maximum 16 viewports across in single window */
         if (charPtr != NULL && horizontalCount == 16)
         {
            Journal_Printf( lucError, "Error in func '%s' - too many viewports in window %s, maximum = 16 wide.\n", __func__, self->name);
            break;
         }
      }

      /* Sum up total number of viewports and save number of columns in layout */
      *viewportCount += horizontalCount;
      self->viewportLayout[vertical_I+1] = horizontalCount;

      /* Reallocate */
      viewportList = Memory_Realloc_Array( viewportList, lucViewport*, *viewportCount );

      /* Read String to get colour map */
      for ( horizontal_I = 0 ; horizontal_I < horizontalCount ; horizontal_I++ )
      {
         /* Find viewport */
         viewportList[total_I] = Stg_ComponentFactory_ConstructByName( cf, viewportName[horizontal_I], lucViewport, True, data ) ;
         Memory_Free( viewportName[horizontal_I] );
         total_I++;
      }

      Memory_Free( horizontalVP_String );
      if (horizontalCount > maxHorizontalCount) maxHorizontalCount = horizontalCount;
   }

   /* Set defaults for window height and width */
   self->height = verticalCount * 400;
   self->width = maxHorizontalCount * 400;

   Journal_Firewall( total_I == *viewportCount, lucError,
                     "Something went wrong in %s for %s '%s' - Incorrectly counted number of viewports.\n",
                     __func__, self->type, self->name );

   return viewportList;
}

