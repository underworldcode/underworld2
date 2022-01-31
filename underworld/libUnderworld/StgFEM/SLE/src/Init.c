/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include "StgFEM/Discretisation/src/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/src/SystemSetup.h"
#include "StgFEM/SLE/ProvidedSystems/src/ProvidedSystems.h"

#include "types.h"
#include "Init.h"

#include <stdio.h>

/* 
 * Initialises the Linear Algebra package, then any init for this package
 * such as streams etc.
 */
Bool StgFEM_SLE_Init( int* argc, char** argv[] ) {
   int tmp;
   
    
   Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ );
   tmp = Stream_GetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" )  );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context"  ), 0 );
   Stream_Flush( Journal_Register( InfoStream_Type, (Name)"Context" )  );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context"  ), tmp );
   
   //StgFEM_SLE_LinearAlgebra_Init( argc, argv );
   StgFEM_SLE_SystemSetup_Init( argc, argv );
   StgFEM_SLE_ProvidedSystems_Init( argc, argv );

   return True;
}


