/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <gLucifer/Base/Base.h>

#include "DrawingObjects.h"

/* Alias placeholders */
const Type lucSwarmSquares_Type = "lucSwarmSquares";
Dictionary* lucSwarmSquares_MetaAsDictionary() {return Dictionary_New();}
Dictionary* lucSwarmSquares_Type_MetaAsDictionary() {return lucSwarmSquares_MetaAsDictionary();}

Bool lucDrawingObjects_Init()
{
   Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

   Journal_Printf( Journal_Register( Debug_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

   Stg_ComponentRegister_Add( componentRegister, lucColourBar_Type, (Name)"0", _lucColourBar_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucFieldSampler_Type, (Name)"0", _lucFieldSampler_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucMeshSampler_Type, (Name)"0", _lucMeshSampler_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucIsosurface_Type, (Name)"0", _lucIsosurface_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucIsosurfaceCrossSection_Type, (Name)"0", _lucIsosurfaceCrossSection_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucCrossSection_Type, (Name)"0", _lucCrossSection_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, lucMeshCrossSection_Type, (Name)"0", _lucMeshCrossSection_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, lucScalarFieldCrossSection_Type, (Name)"0", _lucScalarFieldCrossSection_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, lucScalarField_Type, (Name)"0", _lucScalarField_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucVectorArrowCrossSection_Type, (Name)"0", _lucVectorArrowCrossSection_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, lucVectorArrows_Type, (Name)"0", _lucVectorArrows_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucTextureMap_Type, (Name)"0", _lucTextureMap_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucContour_Type, (Name)"0", _lucContour_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucContourCrossSection_Type, (Name)"0", _lucContourCrossSection_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, lucFeVariableSurface_Type, (Name)"0", _lucFeVariableSurface_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucSwarmViewer_Type, (Name)"0", _lucSwarmViewer_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucSwarmVectors_Type, (Name)"0", _lucSwarmVectors_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucSwarmShapes_Type, (Name)"0", _lucSwarmShapes_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucSwarmSquares_Type, (Name)"0", _lucSwarmShapes_DefaultNew  );  /* Alias for back compat */
   Stg_ComponentRegister_Add( componentRegister, lucHistoricalSwarmTrajectory_Type, (Name)"0", _lucHistoricalSwarmTrajectory_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucEigenvectorsCrossSection_Type, (Name)"0", _lucEigenvectorsCrossSection_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucEigenvectors_Type, (Name)"0", _lucEigenvectors_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucSwarmRGBColourViewer_Type, (Name)"0", _lucSwarmRGBColourViewer_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucMeshViewer_Type, (Name)"0", _lucMeshViewer_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucScalarFieldOnMeshCrossSection_Type, (Name)"0", _lucScalarFieldOnMeshCrossSection_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, lucScalarFieldOnMesh_Type, (Name)"0", _lucScalarFieldOnMesh_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, lucVectorArrowMeshCrossSection_Type, (Name)"0", _lucVectorArrowMeshCrossSection_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, lucVectorArrowsOnMesh_Type, (Name)"0", _lucVectorArrowsOnMesh_DefaultNew  );


   /* Register Parents for type checking */
   RegisterParent( lucCrossSection_Type,                    lucDrawingObject_Type );
   RegisterParent( lucContourCrossSection_Type,             lucCrossSection_Type );
   RegisterParent( lucIsosurfaceCrossSection_Type,          lucCrossSection_Type );
   RegisterParent( lucScalarFieldCrossSection_Type,         lucCrossSection_Type );
   RegisterParent( lucMeshCrossSection_Type,                lucCrossSection_Type );
   RegisterParent( lucVectorArrowCrossSection_Type,         lucCrossSection_Type );
   RegisterParent( lucEigenvectorsCrossSection_Type,        lucCrossSection_Type );
   RegisterParent( lucVectorArrowMeshCrossSection_Type,     lucMeshCrossSection_Type );
   RegisterParent( lucScalarFieldOnMeshCrossSection_Type,   lucMeshCrossSection_Type );
   RegisterParent( lucContour_Type,                         lucContourCrossSection_Type );
   RegisterParent( lucScalarField_Type,                     lucScalarFieldCrossSection_Type );
   RegisterParent( lucScalarFieldOnMesh_Type,               lucScalarFieldOnMeshCrossSection_Type );
   RegisterParent( lucVectorArrows_Type,                    lucVectorArrowCrossSection_Type );
   RegisterParent( lucVectorArrowsOnMesh_Type,              lucVectorArrowMeshCrossSection_Type );
   RegisterParent( lucEigenvectors_Type,                    lucEigenvectorsCrossSection_Type );
   RegisterParent( lucColourBar_Type,                       lucDrawingObject_Type );
   RegisterParent( lucFieldSampler_Type,                    lucCrossSection_Type );
   RegisterParent( lucMeshSampler_Type,                     lucMeshCrossSection_Type );
   RegisterParent( lucIsosurface_Type,                      lucDrawingObject_Type );
   RegisterParent( lucTextureMap_Type,                      lucDrawingObject_Type );
   RegisterParent( lucFeVariableSurface_Type,               lucMeshCrossSection_Type );
   RegisterParent( lucSwarmViewer_Type,               lucDrawingObject_Type );
   RegisterParent( lucSwarmVectors_Type,              lucSwarmViewer_Type );
   RegisterParent( lucSwarmShapes_Type,               lucSwarmViewer_Type );
   RegisterParent( lucHistoricalSwarmTrajectory_Type, lucDrawingObject_Type );
   RegisterParent( lucSwarmRGBColourViewer_Type,      lucSwarmViewer_Type );
   RegisterParent( lucMeshViewer_Type,                lucDrawingObject_Type );




   return True;
}


