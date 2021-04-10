/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef StGermain_JournalSuite_h
#define StGermain_JournalSuite_h

void JournalSuite( pcu_suite_t* suite );

typedef struct {
   Journal*       savedJournal;
   /* For the sake of testing, we want special "stderr" and "stdout" streams for the journal, which just save the output
    * of the last command in a buffer */  
   char*          testStdOutFilename;
   char*          testStdErrFilename;
   FILE*          testStdOutFile;
   FILE*          testStdErrFile;
   MPI_Comm       comm;
   int				rank;
   int 				nProcs;
} JournalSuiteData;

#endif
