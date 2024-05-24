/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/Foundation/forwardDecl.h" /* For Journal stuff */
#include "TimeMonitorSuite.h"

typedef struct {
} TimeMonitorSuiteData;

void TimeMonitorSuite_Setup( TimeMonitorSuiteData* data ) {
}

void TimeMonitorSuite_Teardown( TimeMonitorSuiteData* data ) {
}

void TimeMonitorSuite_TestTimingPeriod( TimeMonitorSuiteData* data ) {
   #define           MAXLINE 1000
   TimeMonitorData   tmData;
   Stg_TimeMonitor*  tm=NULL;
   double            percentOfTotalCalc;

   Stg_TimeMonitor_SetTimerWatchCriteria( 0.5 );

   tm = Stg_TimeMonitor_New( "test", True, False /*Don't print*/, MPI_COMM_WORLD );
   sleep( 1 );
   Stg_TimeMonitor_Begin( tm );
   sleep( 2 );
   Stg_TimeMonitor_End( tm, &tmData );

	printf( "Total Since Init: %f\n", tmData.totalSinceInit );
   pcu_check_true( ( (double)2.95 < tmData.totalSinceInit ) && ( tmData.totalSinceInit < (double)5.05 ) );
   pcu_check_true( ( 1.95 < tmData.dt ) && ( tmData.dt < 2.05 ) );
   pcu_check_true( ( 1.95 < tmData.aveProcDt ) && ( tmData.aveProcDt < 2.05 ) );
   percentOfTotalCalc = tmData.aveProcDt / tmData.totalSinceInit * 100;
   pcu_check_true( fabs( percentOfTotalCalc - tmData.percentTM_ofTotal ) < 0.01 );
   pcu_check_true( tmData.criterionPassed == True );

   Stg_TimeMonitor_Delete( tm );
}

void TimeMonitorSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, TimeMonitorSuiteData );
   pcu_suite_setFixtures( suite, TimeMonitorSuite_Setup, TimeMonitorSuite_Teardown );
   pcu_suite_addTest( suite, TimeMonitorSuite_TestTimingPeriod );
}


