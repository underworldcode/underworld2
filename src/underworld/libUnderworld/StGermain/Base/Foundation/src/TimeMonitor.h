/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StGermain_Base_Foundation_TimeMonitor_h__
#define __StGermain_Base_Foundation_TimeMonitor_h__

#include <mpi.h>

   extern double Stg_TimeMonitor_t0;
   extern double Stg_TimerWatchCriteria;

   extern const Type Stg_TimeMonitor_Type;
   extern const Type Stg_TimeMonitor_InfoStreamName;
   extern const Type Stg_TimeMonitor_TagType;

   typedef struct {
      double totalSinceInit;
      double dt;
      double aveProcDt;	
      double minProcDt;
      double maxProcDt;
      double percentTM_ofTotal;
      Bool   criterionPassed;
   } TimeMonitorData;

   typedef struct {
      double   t1;
      double   t2;
      char*    tag;
      Bool     criteria;
      Bool     print;
      MPI_Comm communicator;
   } Stg_TimeMonitor;

   void Stg_TimeMonitor_Initialise();
   void Stg_TimeMonitor_Finalise();
   void Stg_TimeMonitor_SetTimerWatchCriteria( double ratioOfTotalTime );

   Stg_TimeMonitor* Stg_TimeMonitor_New( char* tag, Bool criteria, Bool print, MPI_Comm communicator );

   void Stg_TimeMonitor_Delete( Stg_TimeMonitor* tm );

   void Stg_TimeMonitor_Begin( Stg_TimeMonitor* tm );
   /* As well as printing, will also store all relevant info in the passed-in tmData reference */
   double Stg_TimeMonitor_End( Stg_TimeMonitor* tm, TimeMonitorData* tmData );

#endif
