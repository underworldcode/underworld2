/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE needed for asprintf */
#include <stdio.h>
#include <stdlib.h>
#include "common-driver-utils.h"

int BSSCR_create_execute_script( void )
{
       int argc;
        char **args;
        int i;
        PetscViewer viewer, viewer_dated;
        time_t      currTime;
        struct tm*  timeInfo;
        int         adjustedYear;
        int         adjustedMonth;
        char        *filename;


        /* get file name with date */
        currTime = time( NULL );
        timeInfo = localtime( &currTime );
        /* See man localtime() for why to adjust these */
        adjustedYear = 1900 + timeInfo->tm_year;
        adjustedMonth = 1 + timeInfo->tm_mon;
        /* Format; name-YYYY.MM.DD-HH.MM.SS.sh */
        asprintf( &filename, "%s-%.4d.%.2d.%.2d-%.2d.%.2d.%.2d.%s",
                        "execute-job",
                        adjustedYear, adjustedMonth, timeInfo->tm_mday,
                        timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec ,
                        "sh" );

        PetscViewerASCIIOpen( PETSC_COMM_SELF, filename, &viewer_dated );


        /* get file name without */
        PetscViewerASCIIOpen( PETSC_COMM_SELF, "execute-job.sh", &viewer );


        /* ouput what the command line arguments were */
        PetscGetArgs( &argc, &args );

        for( i=0; i<argc; i++ ) {
                PetscViewerASCIIPrintf( viewer, "%s ", args[i] );
                PetscViewerASCIIPrintf( viewer_dated, "%s ", args[i] );
        }


        Stg_PetscViewerDestroy(&viewer);
        Stg_PetscViewerDestroy(&viewer_dated);
        free( filename );


        return 0;

}



