/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <mpi.h>
#include "types.h"
#include "test.h"
#include "checks.h"

void _pcu_check_fileEq( const char* const fileName1, const char* const fileName2,
      const char* const fName1Expr, const char* const fName2Expr, pcu_suite_t* pcu_cursuite,
      const char* sourceFile, const unsigned int sourceLine )
{
   const int      MAXLINE = 10000;
   FILE*          testFile1=NULL;
   FILE*          testFile2=NULL;
   char           file1Line[MAXLINE];
   char           file2Line[MAXLINE];
   char           file2LineCopy[MAXLINE];
   int				rank;
   char*          ret1;
   char*          ret2;
   unsigned int   ii=0;
   char           exprString[5000];
   char           msgString[5000];
   unsigned int   file2LineLen = 0;

   MPI_Comm_rank( MPI_COMM_WORLD, &rank );

   if ( rank != 0 ) return;   /* Only do file I/O with proc 0: otherwise errors can result */

   sprintf( exprString, "%s equal file to %s", fName1Expr, fName2Expr );

   // TODO: we need to pass in the actual string expression somehow. Otherwise can't recover it.
   _pcu_check_eval2( fileName1 != NULL, exprString, "First filename passed was a NULL pointer",
      "equal files-preCheck", sourceFile, sourceLine );
   _pcu_check_eval2( fileName2 != NULL, exprString, "Second filename passed was a NULL pointer",
      "equal files-preCheck", sourceFile, sourceLine );

   if ( !fileName1 || !fileName2 ) return;

   _pcu_check_eval2( testFile1 = fopen(fileName1, "r"), exprString, "Unable to open first filename",
      "equal files-preCheck", sourceFile, sourceLine );
   _pcu_check_eval2( testFile2 = fopen(fileName2, "r"), exprString, "Unable to open second filename",
      "equal files-preCheck", sourceFile, sourceLine );

   if ( !testFile1 || !testFile2 ) return;

   for ( ii=1; 1 ; ii++ ) {
      ret1 = fgets( file1Line, MAXLINE, testFile1 );
      ret2 = fgets( file2Line, MAXLINE, testFile2 );
      if ( !ret1 && ret2 ) {
         sprintf( msgString, "Different file lengths: file %s (%s) has %u lines, but file %s (%s) has >=%u lines.", fName1Expr, fileName1, ii-1,
            fName2Expr, fileName2, ii );
         _pcu_check_eval2( 0, exprString, msgString, "equal files", sourceFile, sourceLine );
         break;
      }
      else if ( ret1 && !ret2 ) {
         sprintf( msgString, "Different file lengths: file %s (%s) has >=%u lines, but file %s (%s) has %u lines.", fName1Expr, fileName1, ii,
            fName2Expr, fileName2, ii-1 );
         _pcu_check_eval2( 0, exprString, msgString, "equal files", sourceFile, sourceLine );
         break;
      }
      else if ( !ret1 && !ret2 ) {
         /* End of both files reached simultaneously: good. */
         break;
      }
      else {
         /* Get rid of 2nd newline that will be unnecessary in printed msg */
         strcpy( file2LineCopy, file2Line );
         file2LineLen = strlen( file2Line );
         if ( file2LineCopy[file2LineLen-1] == '\n' ) {
            file2LineCopy[file2LineLen-1] = '\0';
         }
         sprintf( msgString, "Mismatched line:\nLine %u in %s (%s):\n%sLine %u in %s (%s):\n%s", ii, fName1Expr, fileName1, file1Line,
            ii, fName2Expr, fileName2, file2LineCopy );
         _pcu_check_eval2( 0 == strcmp( file1Line, file2Line), exprString, msgString, "equal files", sourceFile, sourceLine );
      }
   }
   fclose(testFile1);
   fclose(testFile2);
}


