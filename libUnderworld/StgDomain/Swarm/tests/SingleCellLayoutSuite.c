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
#include "StgDomain/Geometry/Geometry.h"
#include "StgDomain/Shape/Shape.h"
#include "StgDomain/Mesh/Mesh.h"
#include "StgDomain/Utils/Utils.h"
#include "StgDomain/Swarm/Swarm.h"

#include "SingleCellLayoutSuite.h"

typedef struct {
	SingleCellLayout*	singleCellLayout;
	unsigned int		dimExists[3];
	Cell_Points			cellPoints;
	MPI_Comm				comm;
	int					rank;
	int					nProcs;
} SingleCellLayoutSuiteData;


void SingleCellLayoutSuite_Setup( SingleCellLayoutSuiteData* data ) {
	/* MPI Initializations */
	data->comm = MPI_COMM_WORLD;  
	MPI_Comm_rank( data->comm, &data->rank );
	MPI_Comm_size( data->comm, &data->nProcs );

	data->dimExists[0] = True; data->dimExists[1] = True; data->dimExists[2] = True;
	/* Configure the single-cell-layout */
	data->singleCellLayout = SingleCellLayout_New( "singleCellLayout", NULL, data->dimExists, NULL, NULL );
}

void SingleCellLayoutSuite_Teardown( SingleCellLayoutSuiteData* data ) {
}

void SingleCellLayoutSuite_Driver( SingleCellLayoutSuiteData* data ) {
	Cell_Index  cell;

	for( cell = 0; cell < data->singleCellLayout->_cellLocalCount( data->singleCellLayout ); cell++ ) {
		Cell_PointIndex count;

		count = data->singleCellLayout->_pointCount( data->singleCellLayout, cell );
		data->cellPoints = Memory_Alloc_Array( Cell_Point, count, "cellsPoints" );
		data->singleCellLayout->_initialisePoints( data->singleCellLayout, cell, count, data->cellPoints );
	}
}

void SingleCellLayoutSuite_TestMapElement( SingleCellLayoutSuiteData* data ) {
	int procToWatch;

	procToWatch = data->nProcs >=2 ? 1 : 0;

	if( data->rank == procToWatch ) {
		SingleCellLayoutSuite_Driver( data );
		
		pcu_check_true( CellLayout_MapElementIdToCellId( data->singleCellLayout, 0 ) == 0 );
      pcu_check_true( CellLayout_MapElementIdToCellId( data->singleCellLayout, 5 ) == 0 );
      pcu_check_true( CellLayout_MapElementIdToCellId( data->singleCellLayout, 100 ) == 0 );
      pcu_check_true( CellLayout_CellOf( data->singleCellLayout, data->cellPoints[0] ) == 0 );

		Memory_Free( data->cellPoints );
		Stg_Class_Delete( data->singleCellLayout );	
	}
}

void SingleCellLayoutSuite_TestIsInCell( SingleCellLayoutSuiteData* data ) {
	int		procToWatch;
	double*	testCoord;		

	procToWatch = data->nProcs >=2 ? 1 : 0;

	if( data->rank == procToWatch ) {
		SingleCellLayoutSuite_Driver( data );
		testCoord = Memory_Alloc_Array_Unnamed( double, 3 );
		
		testCoord[0] = testCoord[1] = testCoord[2] = 0;
		pcu_check_true( CellLayout_IsInCell( data->singleCellLayout, 0, &testCoord ) );
		testCoord[0] = testCoord[1] = testCoord[2] = 1;
		pcu_check_true( CellLayout_IsInCell( data->singleCellLayout, 0, &testCoord ) );
		testCoord[0] = testCoord[1] = testCoord[2] = 2;
		pcu_check_true( !CellLayout_IsInCell( data->singleCellLayout, 0, &testCoord ) );
	
		Memory_Free( testCoord );
		Memory_Free( data->cellPoints );
		Stg_Class_Delete( data->singleCellLayout );	
	}
}

void SingleCellLayoutSuite( pcu_suite_t* suite ) {
	pcu_suite_setData( suite, SingleCellLayoutSuiteData );
	pcu_suite_setFixtures( suite, SingleCellLayoutSuite_Setup, SingleCellLayoutSuite_Teardown );
	pcu_suite_addTest( suite, SingleCellLayoutSuite_TestMapElement );
	pcu_suite_addTest( suite, SingleCellLayoutSuite_TestIsInCell );
}


