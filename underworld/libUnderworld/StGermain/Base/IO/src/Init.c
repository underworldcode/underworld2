/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdarg.h>
#include <mpi.h>
#include <libxml/xmlerror.h>

#include <StGermain/Base/Foundation/src/Foundation.h>

#include "types.h"
#include "shortcuts.h"
#include "Dictionary.h"
#include "Dictionary_Entry.h"
#include "Dictionary_Entry_Value.h"
#include "IO_Handler.h"
#include "XML_IO_Handler.h"
#include "Journal.h"
#include "File.h"
#include "CFile.h"
#include "MPIFile.h"
#include "Stream.h"
#include "CStream.h"
#include "MPIStream.h"
#include "BinaryStream.h"
#include "StreamFormatter.h"
#include "LineFormatter.h"
#include "IndentFormatter.h"
#include "RankFormatter.h"
#include "PathUtils.h"
#include "CmdLineArgs.h"
#include "Init.h"

const Name LiveDebugName = "LiveDebug";
Stream*    LiveDebug = NULL;

Stream* stgErrorStream;

Bool BaseIO_Init( int* argc, char** argv[] ) {
   Stream* general;
   
   /* Set up a useful map in the XML_IO_Handler */
   XML_IO_Handler_MergeTypeMap[Dictionary_MergeType_Append] = APPEND_TAG;
   XML_IO_Handler_MergeTypeMap[Dictionary_MergeType_Merge] = MERGE_TAG;
   XML_IO_Handler_MergeTypeMap[Dictionary_MergeType_Replace] = REPLACE_TAG;

   stgStreamFormatter_Buffer = StreamFormatter_Buffer_New(); 

	stJournal = Journal_New();
   stJournal->firewallProtected = False;
	/* Create default Typed Streams. */
	Journal_SetupDefaultTypedStreams();

   Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); 
   
   /* more inits from the Foundation level */
   Stream_Enable( Journal_Register( Info_Type, Stg_TimeMonitor_InfoStreamName ), False );

   /* The LiveDebug stream */
   LiveDebug = Journal_Register( Info_Type, LiveDebugName );
   Stream_Enable( LiveDebug, True );
   Stream_SetLevel( LiveDebug, 1 );

   /* General streams. */
   general = Journal_Register( Info_Type, "general" );
   Stream_SetPrintingRank( general, 0 );
   stgErrorStream = Journal_Register( Error_Type, "stgErrorStream" );

   /* Handle the output of libXML properly, by redirecting to the XML_IO_Handler error stream */
   xmlSetGenericErrorFunc( NULL, XML_IO_Handler_LibXMLErrorHandler );
   
   _stgFilePaths = Stg_ObjectList_New();

   return True;
}


