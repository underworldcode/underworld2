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
#include <StgFEM/StgFEM.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char* argv[] ) {
	MPI_Comm		CommWorld;
	int			rank;
	int			numProcessors;
	int			procToWatch;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	/* Read input */

	if( !StGermain_Init( &argc, &argv ) || !StgDomain_Init( &argc, &argv ) ) {
		fprintf( stderr, "Error initialising StGermain, exiting.\n" );
		exit( EXIT_FAILURE );
	}
	StgFEM_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Stg_Object* testDirectory;
		printf( "Watching rank: %i\n", rank );
		/* Testing entries in xmlDictionary */
		testDirectory = Stg_ObjectList_Get( xmlSearchPaths, (Name)"StGermain" );
		if (testDirectory != NULL) {
			printf("StGermain XML library Path found.\n");
		}
		else {
			printf("StGermain XML library Path not found.\n"); 
		}
		/* For build in the same build directory */
		if (strcmp((char* )LIB_DIR, (char*)testDirectory)) {
			printf("StgFEM XML library Path found.\n");
		}
		/* For build in separate directories */
		else{
			testDirectory = Stg_ObjectList_Get( xmlSearchPaths, (Name)"StgFEM" );
			if (testDirectory != NULL) {
				printf("StgFEM XML library Path found.\n");
			}
			else {
				printf("StgFEM XML library Path not found.\n"); 
			}
		}
	
	}
	 
	StgFEM_Finalise();
	StGermain_Finalise();
	
	/* Close off MPI */
	MPI_Finalize( );

	return 0; /* success */
}


