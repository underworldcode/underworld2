/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <stdio.h>

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include "Discretisation.h"


Stream* StgFEM_Debug = NULL;
Stream* StgFEM_Warning = NULL;
Stream* StgFEM_Discretisation_Debug = NULL;

/* 
 * Initialises the Linear Algebra package, then any init for this package
 * such as streams etc.
 */
Bool StgFEM_Discretisation_Init( int* argc, char** argv[] ) {
   Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();
   int                    tmp;

      
   Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context" ), "In: %s\n", __func__ );
   tmp = Stream_GetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" ) );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" ), 0 );

   Stream_Flush( Journal_Register( InfoStream_Type, (Name)"Context" ) );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" ), tmp );
   
   /* initialise this level's streams */
   StgFEM_Debug = Journal_Register( DebugStream_Type, (Name)"StgFEM" );
   StgFEM_Discretisation_Debug = Stream_RegisterChild( StgFEM_Debug, "Discretisation" );
   StgFEM_Warning = Journal_Register( ErrorStream_Type, (Name)"StgFEM" );

   elementType_Register = ElementType_Register_New( "elementType_Register"  );
   ElementType_Register_Add( elementType_Register, (ElementType*)ConstantElementType_New( "constantElementType" ) );
   ElementType_Register_Add( elementType_Register, (ElementType*)LinearElementType_New( "linearElementType" ) );
   ElementType_Register_Add( elementType_Register, (ElementType*)BilinearElementType_New( "bilinearElementType" ) );
   ElementType_Register_Add( elementType_Register, (ElementType*)TrilinearElementType_New( "trilinearElementType" ) );
   ElementType_Register_Add( elementType_Register, (ElementType*)LinearTriangleElementType_New( "linearElementType" ) );
   ElementType_Register_Add( elementType_Register, (ElementType*)Biquadratic_New( "biquadraticElementType" ) );
   ElementType_Register_Add( elementType_Register, (ElementType*)Triquadratic_New( "triquadraticElementType" ) );
   //ElementType_Register_Add( elementType_Register, (ElementType*)dQ12DElType_New( "dQ12DElType" ) );
   //ElementType_Register_Add( elementType_Register, (ElementType*)dQ13DElType_New( "dQ13DElType" ) );
   
   Stg_ComponentRegister_Add( componentRegister, FeVariable_Type, (Name)"0", _FeVariable_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, LinkedDofInfo_Type, (Name)"0", _LinkedDofInfo_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, FeMesh_Type, (Name)"0", _FeMesh_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, C0Generator_Type, (Name)"0", C0Generator_New );
   Stg_ComponentRegister_Add( componentRegister, C2Generator_Type, (Name)"0", C2Generator_New );
   Stg_ComponentRegister_Add( componentRegister, Inner2DGenerator_Type, (Name)"0", Inner2DGenerator_New );
   Stg_ComponentRegister_Add( componentRegister, dQ1Generator_Type, (Name)"0", dQ1Generator_New );
   
   /* Register Parents for type checking */
   RegisterParent( FeMesh_Algorithms_Type, Mesh_Algorithms_Type );
   RegisterParent( FeMesh_ElementType_Type, Mesh_HexType_Type );
   RegisterParent( ElementType_Type, Stg_Component_Type );
   RegisterParent( LinearElementType_Type, ElementType_Type );
   RegisterParent( BilinearElementType_Type, ElementType_Type );
   RegisterParent( TrilinearElementType_Type, ElementType_Type );
   RegisterParent( Biquadratic_Type, ElementType_Type );
   RegisterParent( Triquadratic_Type, ElementType_Type );
   RegisterParent( ConstantElementType_Type, ElementType_Type );
   RegisterParent( LinearTriangleElementType_Type, ElementType_Type );

   RegisterParent( dQ12DElType_Type, ElementType_Type );
   RegisterParent( dQ13DElType_Type, ElementType_Type );

   RegisterParent( ElementType_Register_Type, Stg_Component_Type );
   RegisterParent( FeEquationNumber_Type, Stg_Component_Type );
   RegisterParent( LinkedDofInfo_Type, Stg_Component_Type );
   RegisterParent( FeMesh_Type, Mesh_Type );
   RegisterParent( C0Generator_Type, MeshGenerator_Type );
   RegisterParent( C2Generator_Type, CartesianGenerator_Type );
   RegisterParent( Inner2DGenerator_Type, MeshGenerator_Type );
   RegisterParent( dQ1Generator_Type, MeshGenerator_Type );

   RegisterParent( FeVariable_Type, FieldVariable_Type );
   {
      PetscErrorCode ec;
      ec = PetscInitialize( argc, argv, (char*)0, NULL );
      CheckPETScError( ec );
   }

   Stg_ComponentRegister_Add( componentRegister, IrregularMeshGaussLayout_Type, (Name)"0", _IrregularMeshGaussLayout_DefaultNew );
   RegisterParent( IrregularMeshGaussLayout_Type, GaussParticleLayout_Type );

   return True;
}

