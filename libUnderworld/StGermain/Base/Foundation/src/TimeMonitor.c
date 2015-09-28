/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include <mpi.h>

#include "types.h"
#include "shortcuts.h"
#include "forwardDecl.h"
#include "Memory.h"
#include "TimeMonitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define Stg_TimeMonitor_t0_ZERO_CONDITION (-1.0)

double Stg_TimeMonitor_t0 = Stg_TimeMonitor_t0_ZERO_CONDITION;
double Stg_TimerWatchCriteria;

const Type Stg_TimeMonitor_Type = "Stg_TimeMonitor";
const Type Stg_TimeMonitor_InfoStreamName = "TimeMonitor";
const Type Stg_TimeMonitor_TagType = "Stg_TimeMonitor_Tag";

void Stg_TimeMonitor_Initialise() {
	Stg_TimeMonitor_t0 = MPI_Wtime();
	Stg_TimerWatchCriteria = 0.2;
}

void Stg_TimeMonitor_Finalise() {

}

void Stg_TimeMonitor_SetTimerWatchCriteria( double ratioOfTotalTime ) {
	Stg_TimerWatchCriteria = ratioOfTotalTime;
}

Stg_TimeMonitor* Stg_TimeMonitor_New( char* tag, Bool criteria, Bool print, MPI_Comm communicator ) {
	Stg_TimeMonitor* tm;
	
	tm = Memory_Alloc_Unnamed( Stg_TimeMonitor );
	tm->tag = (char*)Memory_Alloc_Bytes_Unnamed( strlen( tag ) + 1, Stg_TimeMonitor_TagType );
	strcpy( tm->tag, tag );
	tm->criteria = criteria;
	tm->print = print;
	tm->communicator = communicator;
	
	return tm;
}

void Stg_TimeMonitor_Delete( Stg_TimeMonitor* tm ) {
	if( tm->tag ) {
		Memory_Free( tm->tag );
	}
	
	Memory_Free( tm );
}

void Stg_TimeMonitor_Begin( Stg_TimeMonitor* tm ) {
	Journal_Firewall(
		Stg_TimeMonitor_t0 > Stg_TimeMonitor_t0_ZERO_CONDITION,
		Journal_Register( Error_Type, Stg_TimeMonitor_Type ),
		"Time monitor used but not initialised!\n" );
	
	tm->t1 = MPI_Wtime();
	tm->t2 = tm->t1;	
}

double Stg_TimeMonitor_End( Stg_TimeMonitor* tm, TimeMonitorData* tmData ) {
	int rank;
	int nProc;

	tm->t2 = MPI_Wtime();

	tmData->dt = tm->t2 - tm->t1;
	tmData->totalSinceInit = tm->t2 - Stg_TimeMonitor_t0;

	/* Note: maybe Stg_Components should store rank and comm??? how do the find their comm? */
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &nProc );

	if ( nProc == 1 ) {
		tmData->maxProcDt = tmData->dt;
		tmData->minProcDt = tmData->dt;
		tmData->aveProcDt = tmData->dt;
	}
	else {
		double sumdt;

		/*
		MPI_Reduce( &tmData->dt, &tmData->maxProcDt, 1, MPI_DOUBLE, MPI_MAX, 0, tm->communicator );
		MPI_Reduce( &tmData->dt, &tmData->minProcDt, 1, MPI_DOUBLE, MPI_MIN, 0, tm->communicator );
		MPI_Allreduce( &tmData->dt, &sumdt, 1, MPI_DOUBLE, MPI_SUM, tm->communicator );
		avedt = sumdt / size;
		*/
		/* Note: Above is commented out because cannot use MPI_Reduce functions unless we are sure
		 * that ALL procs will call end(). This is currently not the case with Stg_Component_Initialise()
		 * phase as some procs will have more/less variables to call Initialise() on via Variable_Condition
		 * due to decomposition and Wall boundary conditions */
		sumdt = tmData->dt * nProc;
		tmData->maxProcDt = tmData->dt;
		tmData->minProcDt = tmData->dt;
		tmData->aveProcDt = (double)tmData->dt;
	}

	tmData->percentTM_ofTotal = tmData->aveProcDt / tmData->totalSinceInit * 100;
	tmData->criterionPassed = False;

	if ( tm->criteria ) {
		tmData->criterionPassed = (Bool) (tm->t2 > (Stg_TimerWatchCriteria * tmData->totalSinceInit));
	}

	if( tm->print && (rank == 0) && ((tm->criteria == False) || tmData->criterionPassed )) {
		if ( nProc == 1 ) {
			Journal_Printf( 
				Journal_Register( Info_Type, Stg_TimeMonitor_InfoStreamName ),
				"\t%s(%s):  curr sim time=%.2g (secs), dt= %.2gs (%.2g%% of total)\n", 
				Stg_TimeMonitor_InfoStreamName,
				tm->tag,
				tmData->totalSinceInit,
				tmData->aveProcDt,
				tmData->percentTM_ofTotal );
		}
		else {
			Journal_Printf( 
				Journal_Register( Info_Type, Stg_TimeMonitor_InfoStreamName ),
				"\t%s(%s):  curr sim time=%.2g (secs), ave dt/proc= %.2gs (%.2g%% of total)\n"
				"\t\t(individual proc time spent min/max = %.2g/%.2gs)\n", 
				Stg_TimeMonitor_InfoStreamName,
				tm->tag,
				tmData->totalSinceInit,
				tmData->aveProcDt,
				tmData->percentTM_ofTotal,
				tmData->maxProcDt,
				tmData->minProcDt);
		}
	}
	
	return tmData->aveProcDt;
}
