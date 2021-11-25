/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <StGermain/Base/Foundation/src/Foundation.h>

#include "types.h"
#include "shortcuts.h"
#include "Journal.h"
#include "StreamFormatter.h"
#include "Finalise.h"
#include "IO.h"


#include <stdio.h>

Bool BaseIO_Finalise( void )
{
	Stream* stream;
	if ( stJournal->flushCount > 0 ) {
		stream = Journal_Register( Debug_Type, (char*)__func__ );
		Journal_Printf( stream, "StGermain IO Report - File Flush called %d times.\n", stJournal->flushCount );
	}
	Journal_Delete();
	stJournal = NULL;

	StreamFormatter_Buffer_Delete( stgStreamFormatter_Buffer );
	stgStreamFormatter_Buffer = NULL;
	Stg_Class_Delete( _stgFilePaths );
	_stgFilePaths = NULL;
	Stg_Class_Delete( Project_XMLSearchPaths );
	Project_XMLSearchPaths = NULL;
	
	return True;
}


