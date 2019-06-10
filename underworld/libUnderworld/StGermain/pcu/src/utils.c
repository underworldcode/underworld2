/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <string.h>
#include "utils.h"

void* pcu_memdup( const void* buf, int len ) {
   void* tmp;

   tmp = malloc( len );
   memcpy( tmp, buf, len );
   return tmp;
}

char* pcu_strdup( const char* str ) {
   return str ? (char*)pcu_memdup( str, strlen( str ) + 1 ) : NULL;
}


