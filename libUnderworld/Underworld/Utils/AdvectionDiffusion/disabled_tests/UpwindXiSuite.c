/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>

#include "pcu/pcu.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "StgFEM/Discretisation/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/SystemSetup.h"
#include "StgFEM/SLE/ProvidedSystems/AdvectionDiffusion/AdvectionDiffusion.h"
#include "UpwindXiSuite.h"

typedef struct {
} UpwindXiSuiteData;


void UpwindXiSuite_Setup( UpwindXiSuiteData* data ) {
	Journal_Enable_AllTypedStream( False );
}

void UpwindXiSuite_Teardown( UpwindXiSuiteData* data ) {
}


void UpwindXiSuite_Test( UpwindXiSuiteData* data ) {
	Stream* dataStream;
	double  minPercletNumber = -15.0;
	double  maxPercletNumber = 15.0;
	double  dPerceltNumber = 0.5;
	double  perceltNumber;
	char	expectedFile[PCU_PATH_MAX];
	int	rank;
	char	outputFilename[100];

	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	sprintf( outputFilename, "output_%2d.dat", rank );

	Journal_Enable_TypedStream( InfoStream_Type, True );
	dataStream = Journal_Register( Info_Type, (Name)"DataStream"  );
	Stream_RedirectFile( dataStream, outputFilename );
		
	Journal_Printf( dataStream, "# File to compare code with Brooks, Hughes 1982 - Fig 3.3\n");
	Journal_Printf( dataStream, "# Integration rule for the optimal upwind scheme, doubly asymptotic approximation and critical approximation.\n");
	Journal_Printf( dataStream, "# Plot using line:\n");
	Journal_Printf( dataStream, "# plot \"%s\" using 1:2 title \"Exact\" with line, ", "output.dat" );
	Journal_Printf( dataStream, "\"%s\" using 1:3 title \"DoublyAsymptoticAssumption\" with line, ", "output.dat" );
	Journal_Printf( dataStream, "\"%s\" using 1:4 title \"CriticalAssumption\" with line\n", "output.dat" );

	Journal_Printf( dataStream, "# Perclet Number \t Exact \t DoublyAsymptoticAssumption \t CriticalAssumption\n" );
	for ( perceltNumber = minPercletNumber ; perceltNumber < maxPercletNumber ; perceltNumber += dPerceltNumber )
		Journal_Printf( dataStream, "%0.3g \t\t %0.3g \t\t %0.3g \t\t %0.3g\n", 
				perceltNumber, 
				AdvDiffResidualForceTerm_UpwindXiExact( NULL, perceltNumber),
				AdvDiffResidualForceTerm_UpwindXiDoublyAsymptoticAssumption( NULL, perceltNumber),
				AdvDiffResidualForceTerm_UpwindXiCriticalAssumption( NULL, perceltNumber) );

	pcu_filename_expected( "UpwindXi.expected", expectedFile );
	pcu_check_fileEq( outputFilename, expectedFile );
	remove( outputFilename );
	/*Journal_Purge( );*/
}

void UpwindXiSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, UpwindXiSuiteData );
   pcu_suite_setFixtures( suite, UpwindXiSuite_Setup, UpwindXiSuite_Teardown );
   pcu_suite_addTest( suite, UpwindXiSuite_Test );
}



