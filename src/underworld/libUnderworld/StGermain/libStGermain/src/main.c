/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include "StGermain/Base/src/Base.h"
#include "StGermain/Utils/src/Utils.h"

#include "main.h"

#include <stdio.h>
#include <time.h>

void stgGenerateFlattenedXML( Dictionary* dictionary, Dictionary* sources, char* timeStamp ) {
   XML_IO_Handler* ioHandler;
   char*           outputPath;
   char*           flatFilename;
   char*           flatFilenameStamped;
   char*           slimFilename;
   Stream*         s;
   Bool            isEnabled;
   Bool            ret;
   Bool            outputSlim;

   s = Journal_Register( Info_Type, (Name)XML_IO_Handler_Type );

   /* Avoid confusing messages from XML_IO_Handler. Turn it off temporarily. */
   isEnabled = Stream_IsEnable( s );
   Stream_EnableSelfOnly( s, False );

   ioHandler = XML_IO_Handler_New();
   if( sources == NULL )
      ioHandler->writeSources = False;
   outputPath = StG_Strdup( Dictionary_Entry_Value_AsString( Dictionary_GetDefault( dictionary,
      (Dictionary_Entry_Key)"outputPath", Dictionary_Entry_Value_FromString( "./" ) ) ) );
   outputSlim = Dictionary_Entry_Value_AsBool( Dictionary_GetDefault( dictionary,
      (Dictionary_Entry_Key)"outputSlimmedXML", Dictionary_Entry_Value_FromBool( True ) ) );

   if( ! Stg_DirectoryExists( outputPath ) ) {
      if( Stg_FileExists( outputPath ) )
         Journal_Firewall( 0, s, "outputPath '%s' is a file an not a directory! Exiting...\n", outputPath );

      Journal_Printf( s, "outputPath '%s' does not exist, attempting to create...\n", outputPath );
      ret = Stg_CreateDirectory( outputPath );
      Journal_Firewall( ret, s, "Unable to create non-existing outputPath to '%s'\n", outputPath );
      Journal_Printf( s, "outputPath '%s' successfully created!\n", outputPath );
   }

   /* Set file names. */
   Stg_asprintf( &flatFilename, "%s/%s", outputPath, "input.xml" );

   IO_Handler_WriteAllToFile( ioHandler, flatFilename, dictionary, sources );

   /* Format; path/input-YYYY.MM.DD-HH.MM.SS.xml. */
   if (timeStamp) {
      Stg_asprintf( &flatFilenameStamped, "%s/%s-%s.%s",
                   outputPath, "input", timeStamp, "xml" );
      IO_Handler_WriteAllToFile( ioHandler, flatFilenameStamped, dictionary, sources );
      Memory_Free( flatFilenameStamped );
   }

   
   if( outputSlim && timeStamp ) {
      ioHandler->writeSources = False;
      Stg_asprintf( &slimFilename, "%s/%s-%s.%s",
         outputPath, "input-basic", timeStamp, "xml" );
      IO_Handler_WriteAllToFile( ioHandler, slimFilename, dictionary, NULL);
   }

   Stream_EnableSelfOnly( s, isEnabled );
   Stg_Class_Delete( ioHandler );

   Memory_Free( flatFilename );
}

/* TODO: Need to find a way to add different communicators for different contexts. */
Stg_ComponentFactory* stgMainConstruct( Dictionary* dictionary, Dictionary* sources, MPI_Comm communicator, void* _context ) {
   Stg_ComponentFactory* cf;
   Dictionary*           componentDict;
   Stg_Component*        component;
   AbstractContext*      context=NULL;
   unsigned              component_I;
   char*                 timeStamp;
   time_t                currTime;
   struct tm*            timeInfo;
   int                   adjustedYear;
   int                   adjustedMonth;
   unsigned              rank;

   MPI_Comm_rank( communicator, &rank );

   currTime = time( NULL );
   timeInfo = localtime( &currTime );

   /* See man localtime() for why to adjust these. */
   adjustedYear = 1900 + timeInfo->tm_year;
   adjustedMonth = 1 + timeInfo->tm_mon;

   Stg_asprintf( &timeStamp, "%.4d.%.2d.%.2d-%.2d.%.2d.%.2d",
      adjustedYear, adjustedMonth, timeInfo->tm_mday,
      timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec );

   if( ( componentDict = Dictionary_Entry_Value_AsDictionary(
      Dictionary_Get( dictionary, (Dictionary_Entry_Key)"components" ) ) ) == NULL )
      componentDict = Dictionary_New();
   
   CheckDictionaryKeys( componentDict, "Component dictionary must have unique names\n" );
   /* lets go right ahead and delete the component register. */
   /* this is mainly required for the pcu tests which pass through here a number of times
      without calling StGermain_Finalise */
   LiveComponentRegister_Delete();
   cf = Stg_ComponentFactory_New( dictionary, componentDict );

   if( _context ) {
      context = (AbstractContext*)_context;
      context->CF = cf;
      context->dictionary = dictionary;
      context->communicator = communicator;
      context->timeStamp = timeStamp;
      LiveComponentRegister_Add( cf->LCRegister, (Stg_Component*)context );
   }

   /* Instantion phase. */
   Stg_ComponentFactory_CreateComponents( cf );

   /* 
    * Assign the dictionary, componentFactory & the communicator for the contexts.
    * TODO: if different contexts require different communicators, 
    * then StG. components will be required for these, and they should be passed in from the XML 
    * Also, this is a little hacky, as nothing is known about the other 
    * layers of StG or their associated contexts here.
    */
   for( component_I = 0; component_I < LiveComponentRegister_GetCount( cf->LCRegister ); component_I++ ) {
      component = LiveComponentRegister_At( cf->LCRegister, component_I );

      if( Stg_CompareType( component, AbstractContext ) ) { 
         Journal_Firewall(
            dictionary->count,
            Journal_Register( Error_Type, "Error Stream" ), 
            "Error in %s: The dictionary is empty, "
            "meaning no input parameters have been feed into your program. "
            "Perhaps you've forgot to pass any input files ( or command-line arguments ) in.\n",
            __func__ );

         context = (AbstractContext*)component;
         context->dictionary = dictionary;
         context->CF = cf;
         context->timeStamp = timeStamp;
         //context->communicator = communicator;
      }
   }

   /* generate the Flattened xml file last once Scaling has occured */
   if( rank==0 ) stgGenerateFlattenedXML( dictionary, sources, timeStamp );
   
   /* Construction phase. */
   Stg_ComponentFactory_ConstructComponents( cf, NULL );
   
   return cf;
}

void stgMainBuildAndInitialise( Stg_ComponentFactory* cf ) {
   /* Building phase. */
   Stg_ComponentFactory_BuildComponents( cf, NULL );
   
   /* Initialisaton phase. */
   Stg_ComponentFactory_InitialiseComponents( cf, NULL );
}

Stg_ComponentFactory* stgMainInitFromXML( char* xmlInputFilename, MPI_Comm communicator, void* _context ) {
   Dictionary*           dictionary = NULL;
   Dictionary*           sources = NULL;
   Bool                  result;
   XML_IO_Handler*       ioHandler;
   Stg_ComponentFactory* cf;

   dictionary = Dictionary_New();
   sources = Dictionary_New();
   ioHandler = XML_IO_Handler_New();
   result = IO_Handler_ReadAllFromFile( ioHandler, xmlInputFilename, dictionary, sources );

   /* In case the user has put any journal configuration in the XML, read here. */
   Journal_ReadFromDictionary( dictionary );
   cf = stgMainConstruct( dictionary, sources, communicator, _context );

   /* now dereference aliases */
   DictionaryUtils_AliasDereferenceDictionary( dictionary );
   /* 
    * We don't need the XML IO handler again (however don't delete the dictionary as it's 
    * 'owned' by the context from hereon.
    */
   Stg_Class_Delete( ioHandler );
   Stg_Class_Delete( sources );

   return cf;
}

void stgMainLoop( Stg_ComponentFactory* cf ) {
   /* Run (Solve) phase. */

   /* 
    * Do this by running the contexts, 
    * which manage the entry points which call the _Execute() funcs for the other components.
    */
   unsigned component_i;
   Stg_Component* component;
   AbstractContext* context;
   
   for( component_i = 0; component_i < LiveComponentRegister_GetCount( cf->LCRegister ); component_i++ ) {
      component = LiveComponentRegister_At( cf->LCRegister, component_i );
      if( Stg_CompareType( component, AbstractContext ) ) { 
         context = (AbstractContext*)component;
         Stg_Component_Execute( context, 0, True );
      }
   }
}

void stgMainDestroy( Stg_ComponentFactory* cf ) {
   /* Destruct phase. */

   LiveComponentRegister_DestroyAll( LiveComponentRegister_GetLiveComponentRegister() );
   LiveComponentRegister_DeleteAll( LiveComponentRegister_GetLiveComponentRegister() );
   Stg_Class_Delete( cf );
   LiveComponentRegister_Delete();
}

void stgImportToolbox( Dictionary* dictionary, char* toolboxName ) {
   Dictionary_Entry_Value* dev = Dictionary_Get( dictionary, (Dictionary_Entry_Key)"import" );
   
   if( !dev ) {
      dev = Dictionary_Entry_Value_NewList();
      Dictionary_Add( dictionary, (Dictionary_Entry_Key)"import", dev );
   }
   
   Dictionary_Entry_Value_AddElement( dev, Dictionary_Entry_Value_FromString( toolboxName ) );
}

