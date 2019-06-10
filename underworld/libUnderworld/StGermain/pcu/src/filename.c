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
#include <string.h>
#include <assert.h>
#include <mpi.h>
#include "types.h"


unsigned _pcu_filename_expectedLen( const char* expectedFileName, const char* moduleDir ) {
   const char*          fileType = "expected";

   assert( expectedFileName );

   return 1+1+strlen(moduleDir)+1+strlen(fileType)+1+strlen(expectedFileName) + 1;
}


/* Callers of this function should already have allocated the fullPathFileName buffer to the correct size using
 * pcu_filename_expectedLen */
void _pcu_filename_expected( const char* const expectedFileName, char* const fullPathFileName,
      const char* moduleDir )
{
   const char*          fileType = "expected";

   assert( expectedFileName );
   assert( fullPathFileName );
   sprintf( fullPathFileName, "./%s/%s/%s", moduleDir, fileType, expectedFileName );
}


unsigned _pcu_filename_inputLen( const char* inputFileName, const char* moduleDir ) {
   const char*          fileType = "input";

   assert( inputFileName );

   return 1+1+strlen(moduleDir)+1+strlen(fileType)+1+strlen(inputFileName) + 1;
}


void _pcu_filename_input( const char* const inputFileName, char* const fullPathFileName,
      const char* moduleDir )
{
   const char*          fileType = "input";

   sprintf( fullPathFileName, "./%s/%s/%s", moduleDir, fileType, inputFileName );
}


