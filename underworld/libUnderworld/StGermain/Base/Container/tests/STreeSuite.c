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
#include <string.h>

#include "pcu/pcu.h"
#include "StGermain/Base/Foundation/Foundation.h"
#include "StGermain/Base/IO/IO.h"
#include "StGermain/Base/Container/Container.h"
#include "STreeSuite.h"

typedef struct {
   STree*    sTree;
} STreeSuiteData;


void calcDepth( const STreeNode* node, int* curDepth, int* depth ) {
   if( !node )
      return;
   if( ++(*curDepth) > *depth )
      *depth = *curDepth;
   if( node->left )
      calcDepth( node->left, curDepth, depth );
   if( node->right )
      calcDepth( node->right, curDepth, depth );
   (*curDepth)--;
}


void STreeSuite_Setup( STreeSuiteData* data ) {
   data->sTree = STree_New();
   STree_SetItemSize( data->sTree, sizeof(int) );
   STree_SetIntCallbacks( data->sTree );
   STree_SetAlpha( data->sTree, 0.5 );
}


void STreeSuite_Teardown( STreeSuiteData* data ) {
   Stg_Class_Delete( data->sTree );
}


void STreeSuite_TestConstruct( STreeSuiteData* data ) {
  pcu_check_true( STree_GetSize( data->sTree ) == 0 );
}


void STreeSuite_TestInsert( STreeSuiteData* data ) {
   int depth = 0, curDepth = 0;
   int i_i;

   for( i_i = 0; i_i < 15; i_i++ )
      STree_Insert( data->sTree, &i_i );
   pcu_check_true( STree_GetSize( data->sTree ) == 15 );
   pcu_check_true( (calcDepth( STree_GetRoot( data->sTree ), &curDepth, &depth), depth) == 4 );
}


void STreeSuite_TestRemove( STreeSuiteData* data ) {
   int depth = 0, curDepth = 0;
   int i_i;

   for( i_i = 0; i_i < 30; i_i++ )
      STree_Insert( data->sTree, &i_i );
   for( i_i = 0; i_i < 30; i_i += 2 )
      STree_Remove( data->sTree, &i_i );
   pcu_check_true( STree_GetSize( data->sTree ) == 15 );
   pcu_check_true( (calcDepth( STree_GetRoot( data->sTree ), &curDepth, &depth), depth) == 4 );
}


void STreeSuite_TestHas( STreeSuiteData* data ) {
   int i_i;

   for( i_i = 0; i_i < 15; i_i++ )
      STree_Insert( data->sTree, &i_i );
   for( i_i = 0; i_i < 15; i_i++ ) {
      pcu_check_true( STree_Has( data->sTree, &i_i ) );
   }
   for( i_i = 15; i_i < 30; i_i++ ) {
      pcu_check_true( !STree_Has( data->sTree, &i_i ) );
   }
}


void STreeSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, STreeSuiteData );
   pcu_suite_setFixtures( suite, STreeSuite_Setup, STreeSuite_Teardown );
   pcu_suite_addTest( suite, STreeSuite_TestConstruct );
   pcu_suite_addTest( suite, STreeSuite_TestInsert );
   pcu_suite_addTest( suite, STreeSuite_TestRemove );
   pcu_suite_addTest( suite, STreeSuite_TestHas );
}


