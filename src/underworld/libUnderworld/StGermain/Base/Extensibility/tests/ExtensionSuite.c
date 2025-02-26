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

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "StGermain/Base/Extensibility/src/Extensibility.h"
#include "ExtensionSuite.h"

typedef struct {
} ExtensionSuiteData;

typedef struct {
   double x;
   double y;
   double z;
   char dim;
} BaseClass;

typedef struct {
   char type;
} ExtensionStruct0;
const Type Type0 = "Type0";
const Type Type1 = "Type1";

typedef struct {
   double temp;
} ExtensionStruct1;
const Type Temp0 = "Temp0";
const Type Temp1 = "Temp1";

typedef struct {
   double pres;
   char flag;
} ExtensionStruct2;
const Type Pres0 = "Pres0";
const Type Pres1 = "Pres1";

typedef struct {
   char dd;
   char cc;
   int bc;
} ExtensionStruct3;
const Type BC_Set0 = "BC_Set0";
const Type BC_Set1 = "BC_Set1";

typedef struct {
   float wf;
} ExtensionStruct4;
const Type Weight0 = "Weight0";
const Type Weight1 = "Weight1";

void ExtensionSuite_Setup( ExtensionSuiteData* data ) {
}

void ExtensionSuite_Teardown( ExtensionSuiteData* data ) {
}

void ExtensionSuite_TestOfStruct( ExtensionSuiteData* data ) {
   ExtensionManager*    extensionMgr;
   BaseClass*           nArray;
   BaseClass*           n;
   ExtensionStruct0*    nType0;
   ExtensionStruct1*    nTemp0;
   ExtensionStruct2*    nPres0;
   ExtensionStruct3*    nBC_Set0;
   ExtensionStruct4*    nWeight0;
   ExtensionStruct0*    nType1;
   ExtensionStruct1*    nTemp1;
   ExtensionStruct2*    nPres1;
   ExtensionStruct3*    nBC_Set1;
   ExtensionStruct4*    nWeight1;
   Index                ii;
   const Index          ArraySize = 100;
   Index                numExtensions = 0;      /* variable to check number of extensions */
   SizeT                size0;
   SizeT                size1;
   SizeT                size2;
   SizeT                size3;
   SizeT                size4;

   size0 = ExtensionManager_Align( sizeof(ExtensionStruct0) );
   size1 = ExtensionManager_Align( sizeof(ExtensionStruct1) );
   size2 = ExtensionManager_Align( sizeof(ExtensionStruct2) );
   size3 = ExtensionManager_Align( sizeof(ExtensionStruct3) );
   size4 = ExtensionManager_Align( sizeof(ExtensionStruct4) );

   /* Build the extensionMgr */
   extensionMgr = ExtensionManager_New_OfStruct( "Node", sizeof(BaseClass) );
   ExtensionManager_Add( extensionMgr, (Name)Type0, sizeof(ExtensionStruct0)  );     numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Temp0, sizeof(ExtensionStruct1)  );     numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Pres0, sizeof(ExtensionStruct2)  );     numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)BC_Set0, sizeof(ExtensionStruct3)  );   numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Weight0, sizeof(ExtensionStruct4)  );   numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Type1, sizeof(ExtensionStruct0)  );     numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Temp1, sizeof(ExtensionStruct1)  );     numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Pres1, sizeof(ExtensionStruct2)  );     numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)BC_Set1, sizeof(ExtensionStruct3)  );   numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Weight1, sizeof(ExtensionStruct4) );   numExtensions++;

   /* "Is the initial size correct" */
   pcu_check_true( extensionMgr->initialSize == sizeof(BaseClass)  );

   /* Is the first offset at the correct position */
   pcu_check_true( ExtensionInfo_At( extensionMgr->extInfos, 0 )->offset == sizeof(BaseClass) );

   /* Is the final size correct */
   pcu_check_true( extensionMgr->finalSize == 
         sizeof(BaseClass) + 
         size0 + size1 + size2 + size3 + size4 +
         size0 + size1 + size2 + size3 + size4 );

   /* Is the number of extensions correct */
   pcu_check_true( extensionMgr->extInfos->count == numExtensions );

   /* Alloc, initialise and view*/
   nArray = (BaseClass*)ExtensionManager_Malloc( extensionMgr, ArraySize );

   /* Can we malloc the extension */
   pcu_check_true( nArray != NULL );

   n = (BaseClass*)ExtensionManager_At( extensionMgr, nArray, 0 );

   nType0 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, 0 );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nType0->type - (ArithPointer)n) == sizeof(BaseClass) );

   nTemp0 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, 1 );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nTemp0->temp - (ArithPointer)n) == sizeof(BaseClass) + size0 );
   
   nPres0 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, 2 );      
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nPres0->pres - (ArithPointer)n) == sizeof(BaseClass) + size0 + size1 );

   nBC_Set0 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, 3 );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nBC_Set0->dd - (ArithPointer)n) == sizeof(BaseClass) + size0 + size1 + size2 );

   nWeight0 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n,
      ExtensionManager_GetHandle( extensionMgr, (Name)"Weight0" ));
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nWeight0->wf - (ArithPointer)n) == 
         sizeof(BaseClass) + size0 + size1 + size2 + size3 );

   nType1 = (ExtensionStruct0* )ExtensionManager_Get( extensionMgr, n,
      ExtensionManager_GetHandle( extensionMgr, (Name)"Type1" ) );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nType1->type - (ArithPointer)n) == 
         sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 );

   nTemp1 = (ExtensionStruct1* )ExtensionManager_Get( extensionMgr, n,
      ExtensionManager_GetHandle( extensionMgr, (Name)"Temp1" ) );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nTemp1->temp - (ArithPointer)n) == 
         sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 + size0 );

   nPres1 = (ExtensionStruct2* )ExtensionManager_Get( extensionMgr, n,
      ExtensionManager_GetHandle( extensionMgr, (Name)"Pres1" ) );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nPres1->pres - (ArithPointer)n) == 
         sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 + size0 + size1 );

   nBC_Set1 = (ExtensionStruct3* )ExtensionManager_Get( extensionMgr, n,
      ExtensionManager_GetHandle( extensionMgr, (Name)"BC_Set1" ) );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nBC_Set1->dd - (ArithPointer)n) == 
         sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 + size0 + size1 + size2 );

   nWeight1 = (ExtensionStruct4* )ExtensionManager_Get( extensionMgr, n,
      ExtensionManager_GetHandle( extensionMgr, (Name)"Weight1" ) );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nWeight1->wf - (ArithPointer)n) == 
         sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 + size0 + size1 + size2 + size3 );

   for( ii = 0; ii < ArraySize; ii++ ) {
      n = (BaseClass* )ExtensionManager_At( extensionMgr, nArray, ii );
      
      n->x = 1.0f;
      n->y = 2.0f;
      n->z = 3.0f;
      n->dim = 4;
      
      nType0 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, 0 );
      nTemp0 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, 1 );
      nPres0 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, 2 );
      nBC_Set0 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, 3 );
      nWeight0 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Weight0" ) );
      nType1 = (ExtensionStruct0* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Type1" ) );
      nTemp1 = (ExtensionStruct1* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Temp1" ) );
      nPres1 = (ExtensionStruct2* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Pres1" ) );
      nBC_Set1 = (ExtensionStruct3* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"BC_Set1" ) );
      nWeight1 = (ExtensionStruct4* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Weight1" ) );

      nType0->type = 5;
      nTemp0->temp = 6.0f;
      nPres0->pres = 7.0f;
      nPres0->flag = 8;
      nBC_Set0->dd = 9;
      nBC_Set0->cc = 10;
      nBC_Set0->bc = 11;
      nWeight0->wf = 12.0f;
      nType1->type = 13;
      nTemp1->temp = 14.0f;
      nPres1->pres = 15.0f;
      nPres1->flag = 16;
      nBC_Set1->dd = 17;
      nBC_Set1->cc = 18;
      nBC_Set1->bc = 19;
      nWeight1->wf = 20.0f;
   }

   for( ii = 0; ii < ArraySize; ii++ ) {
      n = (BaseClass* )ExtensionManager_At( extensionMgr, nArray, ii );

      nType0 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, 0 );
      nTemp0 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, 1 );
      nPres0 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, 2 );
      nBC_Set0 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, 3 );
      nWeight0 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Weight0" ) );
      nType1 = (ExtensionStruct0* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Type1" ) );
      nTemp1 = (ExtensionStruct1* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Temp1" ) );
      nPres1 = (ExtensionStruct2* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Pres1" ) );
      nBC_Set1 = (ExtensionStruct3* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"BC_Set1" ) );
      nWeight1 = (ExtensionStruct4* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Weight1" ) );

      pcu_check_true( n->x == 1.0f );
      pcu_check_true( n->y == 2.0f );
      pcu_check_true( n->z == 3.0f );
      pcu_check_true( n->dim == 4 );

      pcu_check_true( nType0->type == 5 );
      pcu_check_true( nTemp0->temp == 6.0f );
      pcu_check_true( nPres0->pres == 7.0f );
      pcu_check_true( nPres0->flag == 8 );
      pcu_check_true( nBC_Set0->dd == 9 );
      pcu_check_true( nBC_Set0->cc == 10 );
      pcu_check_true( nBC_Set0->bc == 11 );
      pcu_check_true( nWeight0->wf == 12.0f );
      pcu_check_true( nType1->type == 13 );
      pcu_check_true( nTemp1->temp == 14.0f );
      pcu_check_true( nPres1->pres == 15.0f );
      pcu_check_true( nPres1->flag == 16 );
      pcu_check_true( nBC_Set1->dd == 17 );
      pcu_check_true( nBC_Set1->cc == 18 );
      pcu_check_true( nBC_Set1->bc == 19 );
      pcu_check_true( nWeight1->wf == 20.0f  );
   }
   
   /* Stg_Class_Delete stuff */
   ExtensionManager_Free( extensionMgr, nArray );
   Stg_Class_Delete( extensionMgr );
}


void ExtensionSuite_TestOfExistingObject( ExtensionSuiteData* data ) {
   ExtensionManager*    extensionMgr;
   BaseClass*           n;
   ExtensionStruct0*    nType0;
   ExtensionStruct1*    nTemp0;
   ExtensionStruct2*    nPres0;
   ExtensionStruct3*    nBC_Set0;
   ExtensionStruct4*    nWeight0;
   ExtensionStruct0*    nType1;
   ExtensionStruct1*    nTemp1;
   ExtensionStruct2*    nPres1;
   ExtensionStruct3*    nBC_Set1;
   ExtensionStruct4*    nWeight1;
   Index                numExtensions = 0;  /* variable to check number of extensions */
   SizeT                size0;
   SizeT                size1;
   SizeT                size2;
   SizeT                size3;
   SizeT                size4;

   size0 = ExtensionManager_Align( sizeof(ExtensionStruct0) );
   size1 = ExtensionManager_Align( sizeof(ExtensionStruct1) );
   size2 = ExtensionManager_Align( sizeof(ExtensionStruct2) );
   size3 = ExtensionManager_Align( sizeof(ExtensionStruct3) );
   size4 = ExtensionManager_Align( sizeof(ExtensionStruct4) );

   /* Build the extensionMgr */
   n = Memory_Alloc_Unnamed( BaseClass );
   extensionMgr = ExtensionManager_New_OfExistingObject( "Node", n );
   ExtensionManager_Add( extensionMgr, (Name)Type0, sizeof(ExtensionStruct0)  );      numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Temp0, sizeof(ExtensionStruct1)  );      numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Pres0, sizeof(ExtensionStruct2)  );      numExtensions++; 
   ExtensionManager_Add( extensionMgr, (Name)BC_Set0, sizeof(ExtensionStruct3)  );   numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Weight0, sizeof(ExtensionStruct4)  );   numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Type1, sizeof(ExtensionStruct0)  );      numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Temp1, sizeof(ExtensionStruct1)  );      numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Pres1, sizeof(ExtensionStruct2)  );      numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)BC_Set1, sizeof(ExtensionStruct3)  );   numExtensions++;
   ExtensionManager_Add( extensionMgr, (Name)Weight1, sizeof(ExtensionStruct4) );   numExtensions++;

   /* Is the initial size correct */
   pcu_check_true( extensionMgr->initialSize == 0  );

   /* Is the first offset at the correct position */
   pcu_check_true( ExtensionInfo_At( extensionMgr->extInfos, 0 )->offset == 0 );

   /* Is the final size correct */
   pcu_check_true( extensionMgr->finalSize ==
         size0 + size1 + size2 + size3 + size4 +
         size0 + size1 + size2 + size3 + size4 );

   /* Is the number of extensions correct */
   pcu_check_true( extensionMgr->extInfos->count == numExtensions );

   n->x = 1.0f;
   n->y = 2.0f;
   n->z = 3.0f;
   n->dim = 4;
   
   nType0 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, 0 );
   /* Is offset correct */
   pcu_check_true( ((ArithPointer)&nType0->type - (ArithPointer)extensionMgr->_extensionsToExisting) ==  0 );
   nType0->type = 5;
   
   nTemp0 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, 1 );
   /* Is offset correct */
   pcu_check_true(
      ((ArithPointer)&nTemp0->temp - (ArithPointer)extensionMgr->_extensionsToExisting) ==
         size0 );
   nTemp0->temp = 6.0f;
   
   nPres0 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, 2 );
   /* Is offset correct */
   pcu_check_true(
      ((ArithPointer)&nPres0->pres - (ArithPointer)extensionMgr->_extensionsToExisting) ==
              size0 + size1 );
   nPres0->pres = 7.0f;
   nPres0->flag = 8;
   
   nBC_Set0 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, 3 );
   /* Is offset correct */
   pcu_check_true(
      ((ArithPointer)&nBC_Set0->dd - (ArithPointer)extensionMgr->_extensionsToExisting) ==
         size0 + size1 + size2 );
   nBC_Set0->dd = 9;
   nBC_Set0->cc = 10;
   nBC_Set0->bc = 11;
   
   nWeight0 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Weight0" ) );
   /* Is offset correct */
   pcu_check_true(
      ((ArithPointer)&nWeight0->wf - (ArithPointer)extensionMgr->_extensionsToExisting) ==
         size0 + size1 + size2 + size3 );
   nWeight0->wf = 12.0f;
   
   nType1 = (ExtensionStruct0* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Type1" ) );
   /* Is offset correct */
   pcu_check_true(
      ((ArithPointer)&nType1->type - (ArithPointer)extensionMgr->_extensionsToExisting) ==
         size0 + size1 + size2 + size3 + size4 );
   nType1->type = 13;
   
   nTemp1 = (ExtensionStruct1* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Temp1" ) );
   /* Is offset correct */
   pcu_check_true(
      ((ArithPointer)&nTemp1->temp - (ArithPointer)extensionMgr->_extensionsToExisting) ==
         size0 + size1 + size2 + size3 + size4 + size0 );
   nTemp1->temp = 14.0f;
   
   nPres1 = (ExtensionStruct2* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Pres1" ) );
   /* Is offset correct */
   pcu_check_true(
      ((ArithPointer)&nPres1->pres - (ArithPointer)extensionMgr->_extensionsToExisting) ==
         size0 + size1 + size2 + size3 + size4 + size0 + size1 );
   nPres1->pres = 15.0f;
   nPres1->flag = 16;
   
   nBC_Set1 = (ExtensionStruct3* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"BC_Set1" ) );
   /* Is offset correct */
   pcu_check_true(
      ((ArithPointer)&nBC_Set1->dd - (ArithPointer)extensionMgr->_extensionsToExisting) ==
         size0 + size1 + size2 + size3 + size4 + size0 + size1 + size2 );
   nBC_Set1->dd = 17;
   nBC_Set1->cc = 18;
   nBC_Set1->bc = 19;
   
   /* Is offset correct */
   nWeight1 = (ExtensionStruct4* )ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, (Name)"Weight1" ) );
   pcu_check_true(
      ((ArithPointer)&nWeight1->wf - (ArithPointer)extensionMgr->_extensionsToExisting) ==
         size0 + size1 + size2 + size3 + size4 + size0 + size1 + size2 + size3  );

   nWeight1->wf = 20.0f;
   
   /* Now check we can get them all over again, and read the values correctly */
   nType0 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, 0 );
   nTemp0 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, 1 );
   nPres0 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, 2 );
   nBC_Set0 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, 3 );
   nWeight0 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, 4 );
   nType1 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, 5 );
   nTemp1 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, 6 );
   nPres1 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, 7 );
   nBC_Set1 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, 8 );
   nWeight1 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, 9 );
   pcu_check_true( n->x == 1.0f );
   pcu_check_true( n->y == 2.0f );
   pcu_check_true( n->z == 3.0f );
   pcu_check_true( n->dim == 4 );
   pcu_check_true( nType0->type == 5 );
   pcu_check_true( nTemp0->temp == 6.0f );
   pcu_check_true( nPres0->pres == 7.0f );
   pcu_check_true( nPres0->flag == 8 );
   pcu_check_true( nBC_Set0->dd == 9 );
   pcu_check_true( nBC_Set0->cc == 10 );
   pcu_check_true( nBC_Set0->bc == 11 );
   pcu_check_true( nWeight0->wf == 12.0f );
   pcu_check_true( nType1->type == 13 );
   pcu_check_true( nTemp1->temp == 14.0f );
   pcu_check_true( nPres1->pres == 15.0f );
   pcu_check_true( nPres1->flag == 16 );
   pcu_check_true( nBC_Set1->dd == 17 );
   pcu_check_true( nBC_Set1->cc == 18 );
   pcu_check_true( nBC_Set1->bc == 19 );
   pcu_check_true( nWeight1->wf == 20.0f );

   /* Stg_Class_Delete stuff */
   Stg_Class_Delete( extensionMgr );
   Memory_Free( n );
}


void ExtensionSuite_TestAddArrayToExistingObject( ExtensionSuiteData* data ) {
   ExtensionManager*    objExtension=NULL;
   BaseClass*           baseObject=NULL;
   Index                ii=0;
   ExtensionStruct0*    objType0=NULL;
   ExtensionStruct1*    objTemp0=NULL;

   baseObject = Memory_Alloc( BaseClass, "BaseObject" );
   objExtension = ExtensionManager_New_OfExistingObject( "obj", baseObject );

   ExtensionManager_Add( objExtension, (Name)Type0, sizeof(ExtensionStruct0)  );
   ExtensionManager_AddArray( objExtension, Temp0, sizeof(ExtensionStruct1), 10 );
   
   {
      objType0 = (ExtensionStruct0*)ExtensionManager_Get(
         objExtension, 
         baseObject, 
         ExtensionManager_GetHandle( objExtension, (Name)"Type0" ) );
      
      objTemp0 = (ExtensionStruct1* )ExtensionManager_Get( 
         objExtension, 
         baseObject, 
         ExtensionManager_GetHandle( objExtension, (Name)"Temp0" ) );

      objType0->type = 'a';
      for ( ii = 0; ii < 10; ++ii ) {
         objTemp0[ii].temp = (double)ii;
      }
   }

   objTemp0 = (ExtensionStruct1* )ExtensionManager_Get( 
      objExtension, 
      baseObject, 
      ExtensionManager_GetHandle( objExtension, (Name)"Temp0" ) );

   for ( ii = 0; ii < 10; ++ii ) {
      pcu_check_true( objTemp0[ii].temp == ii );
   }

   Memory_Free( baseObject );
   Stg_Class_Delete( objExtension );
}


void ExtensionSuite_TestExtendOfExtendedArray( ExtensionSuiteData* data  ) {
   ExtensionManager*       structExtension=NULL;
   ExtensionManager*       arrayExtension=NULL;
   BaseClass*              nArray=NULL;
   BaseClass*              current=NULL;
   ExtensionStruct0*       nType0=NULL;
   ExtensionStruct1*       nTemp0=NULL;
   ExtensionStruct2*       nPres0=NULL;
   Index                   array_I=0;
   const Index             ArraySize = 8;
   Index                   ii=0;

   structExtension = ExtensionManager_New_OfStruct( "Node", sizeof(BaseClass) );

   ExtensionManager_Add( structExtension, (Name)Type0, sizeof(ExtensionStruct0)  );
   ExtensionManager_AddArray( structExtension, Temp0, sizeof(ExtensionStruct1), 10 );
   
   nArray = (BaseClass*)ExtensionManager_Malloc( structExtension, ArraySize );

   /* Testing "Simple" extension on allocated extended struct */
   arrayExtension = ExtensionManager_New_OfExtendedArray( "Ext", nArray, structExtension, ArraySize );

   /* Since the nArray is already allocated, this function should realloc it to put the extended array
    * at the end */
   ExtensionManager_Add( arrayExtension, (Name)Pres0, sizeof(ExtensionStruct2) );

   for ( array_I = 0; array_I < ArraySize; ++array_I  ) {
      current = (BaseClass*)ExtensionManager_At( structExtension, nArray, array_I );

      nType0 = (ExtensionStruct0*)ExtensionManager_Get(
         structExtension,
         current,
         ExtensionManager_GetHandle( structExtension, (Name)"Type0" ) );
      nTemp0 = (ExtensionStruct1* )ExtensionManager_Get(
         structExtension,
         current,
         ExtensionManager_GetHandle( structExtension, (Name)"Temp0" ) );

      nType0->type = 'a';
      for ( ii = 0; ii < 10; ++ii ) {
         nTemp0[ii].temp = (double)((array_I * 10) + ii);
      }
      
      nPres0 = (ExtensionStruct2* )ExtensionManager_Get(
         arrayExtension,
         ExtensionManager_At( arrayExtension, nArray, array_I ),
         ExtensionManager_GetHandle( arrayExtension, (Name)"Pres0" ) );
      
      nPres0->pres = (double)array_I;
      nPres0->flag = 'b';
   }
   
   for ( array_I = 0; array_I < ArraySize; ++array_I  ) {
      current = (BaseClass*)ExtensionManager_At( structExtension, nArray, array_I );
      nType0 = (ExtensionStruct0*)ExtensionManager_Get(
         structExtension,
         current,
         ExtensionManager_GetHandle( structExtension, (Name)"Type0" ) );
      nTemp0 = (ExtensionStruct1* )ExtensionManager_Get(
         structExtension,
         current,
         ExtensionManager_GetHandle( structExtension, (Name)"Temp0" ) );

      pcu_check_true( nType0->type == 'a' );
      for ( ii = 0; ii < 10; ++ii ) {
         pcu_check_true( nTemp0[ii].temp == (double)((array_I * 10) + ii) );
      }

      nPres0 = (ExtensionStruct2* )ExtensionManager_Get(
         arrayExtension,
         ExtensionManager_At( arrayExtension, nArray, array_I ),
         ExtensionManager_GetHandle( arrayExtension, (Name)"Pres0" ) );
   
      pcu_check_true( nPres0->pres == (double)array_I );
      pcu_check_true( nPres0->flag == 'b'  );
   }

   ExtensionManager_Free( structExtension, nArray );

   Stg_Class_Delete( structExtension );
   Stg_Class_Delete( arrayExtension );
}


void ExtensionSuite_TestCopyExtendedArray( ExtensionSuiteData* data ) {
   ExtensionManager*       structExtension=NULL;
   ExtensionManager*       arrayExtension=NULL;
   BaseClass*              nArray=NULL;
   BaseClass*              current=NULL;
   ExtensionStruct0*       nType0=NULL;
   ExtensionStruct1*       nTemp0=NULL;
   ExtensionStruct2*       nPres0=NULL;
   Index                   array_I=0;
   const Index             ArraySize = 8;
   Index                   ii=0;
   ExtensionManager*       structExtensionCopy=NULL;
   ExtensionManager*       arrayExtensionCopy=NULL;
   BaseClass*              nArrayCopy=NULL;
   PtrMap*                 copyMap=NULL;

   structExtension = ExtensionManager_New_OfStruct( "Node", sizeof(BaseClass) );

   ExtensionManager_Add( structExtension, (Name)Type0, sizeof(ExtensionStruct0)  );
   ExtensionManager_AddArray( structExtension, Temp0, sizeof(ExtensionStruct1), 10 );
   
   nArray = (BaseClass*)ExtensionManager_Malloc( structExtension, ArraySize );

   /* Testing "Simple" extension on allocated extended struct */
   arrayExtension = ExtensionManager_New_OfExtendedArray( "Ext", nArray, structExtension, ArraySize );

   /* Since the nArray is already allocated, this function should realloc it to put the extended array
    * at the end */
   ExtensionManager_Add( arrayExtension, (Name)Pres0, sizeof(ExtensionStruct2) );

   for ( array_I = 0; array_I < ArraySize; ++array_I  ) {
      current = (BaseClass*)ExtensionManager_At( structExtension, nArray, array_I );

      nType0 = (ExtensionStruct0*)ExtensionManager_Get(
         structExtension,
         current,
         ExtensionManager_GetHandle( structExtension, (Name)"Type0" ) );
      nTemp0 = (ExtensionStruct1* )ExtensionManager_Get(
         structExtension,
         current,
         ExtensionManager_GetHandle( structExtension, (Name)"Temp0" ) );

      nType0->type = 'a';
      for ( ii = 0; ii < 10; ++ii ) {
         nTemp0[ii].temp = (double)((array_I * 10) + ii);
      }
      
      nPres0 = (ExtensionStruct2* )ExtensionManager_Get(
         arrayExtension,
         ExtensionManager_At( arrayExtension, nArray, array_I ),
         ExtensionManager_GetHandle( arrayExtension, (Name)"Pres0" ) );
      
      nPres0->pres = (double)array_I;
      nPres0->flag = 'b';
   }

   /* Copy time! */
   
   copyMap = PtrMap_New( 1  );
   arrayExtensionCopy = (ExtensionManager*)Stg_Class_Copy( arrayExtension, NULL, True, "_dup", copyMap );
   structExtensionCopy = (ExtensionManager*)Stg_Class_Copy( structExtension, NULL, True, "_dup", copyMap );
   nArrayCopy = (BaseClass*)PtrMap_Find( copyMap, arrayExtension->_array );

   pcu_check_true( nArrayCopy != NULL && nArrayCopy == arrayExtensionCopy->_array );

   for ( array_I = 0; array_I < ArraySize; ++array_I ) {
      current = (BaseClass*)ExtensionManager_At( arrayExtensionCopy, nArrayCopy, array_I );

      nType0 = (ExtensionStruct0*)ExtensionManager_Get(
         arrayExtensionCopy,
         current,
         ExtensionManager_GetHandle( structExtension, (Name)"Type0" ) );
      nTemp0 = (ExtensionStruct1* )ExtensionManager_Get(
         arrayExtensionCopy,
         current,
         ExtensionManager_GetHandle( structExtension, (Name)"Temp0" ) );
   
      pcu_check_true( nType0->type == 'a' );
      for ( ii = 0; ii < 10; ++ii ) {
         pcu_check_true( nTemp0[ii].temp == (double)((array_I * 10) + ii) );
      }

      nPres0 = (ExtensionStruct2* )ExtensionManager_Get(
         arrayExtension,
         ExtensionManager_At( arrayExtension, nArray, array_I ),
         ExtensionManager_GetHandle( arrayExtension, (Name)"Pres0" ) );
   
      pcu_check_true( nPres0->pres == (double)array_I );
      pcu_check_true( nPres0->flag == 'b'  );
   }

   ExtensionManager_Free( structExtension, nArray );
   Stg_Class_Delete( structExtension );
   Stg_Class_Delete( arrayExtension );

   ExtensionManager_Free( structExtensionCopy, nArrayCopy );
   Stg_Class_Delete( structExtensionCopy );
   Stg_Class_Delete( arrayExtensionCopy );
   Stg_Class_Delete( copyMap );
}


void ExtensionSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ExtensionSuiteData );
   pcu_suite_setFixtures( suite, ExtensionSuite_Setup, ExtensionSuite_Teardown );
   pcu_suite_addTest( suite, ExtensionSuite_TestOfStruct );
   pcu_suite_addTest( suite, ExtensionSuite_TestOfExistingObject );
   pcu_suite_addTest( suite, ExtensionSuite_TestAddArrayToExistingObject );
   pcu_suite_addTest( suite, ExtensionSuite_TestExtendOfExtendedArray );
   pcu_suite_addTest( suite, ExtensionSuite_TestCopyExtendedArray );
}


