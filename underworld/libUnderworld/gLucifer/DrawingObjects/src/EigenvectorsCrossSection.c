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


#include "types.h"
#include "CrossSection.h"
#include "EigenvectorsCrossSection.h"

/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
const Type lucEigenvectorsCrossSection_Type = "lucEigenvectorsCrossSection";

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */
lucEigenvectorsCrossSection* _lucEigenvectorsCrossSection_New(  LUCEIGENVECTORSCROSSSECTION_DEFARGS  )
{
   lucEigenvectorsCrossSection*               self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucEigenvectorsCrossSection) );
   self = (lucEigenvectorsCrossSection*) _lucCrossSection_New(  LUCCROSSSECTION_PASSARGS  );

   return self;
}

void _lucEigenvectorsCrossSection_Init(
   lucEigenvectorsCrossSection*  self,
   Dimension_Index               dim,
   Name                          leastColourName,
   Name                          middleColourName,
   Name                          greatestColourName,
   double                        arrowHeadSize,
   double                        lengthScale,
   int                           glyphs,
   Bool                          useEigenValue,
   Bool                          plotEigenVector,
   Bool                          plotEigenValue,
   Name                          leastColourForNegativeName,
   Name                          middleColourForNegativeName,
   Name                          greatestColourForNegativeName)
{
   /*if ( dim == 2 )
   {
      lucColour_FromString( &self->colours[0], leastColourName );
      lucColour_FromString( &self->colours[1], greatestColourName );
      lucColour_FromString( &self->colourForNegative[0], leastColourForNegativeName );
      lucColour_FromString( &self->colourForNegative[1], greatestColourForNegativeName );
   }
   else
   {
      lucColour_FromString( &self->colours[0], leastColourName );
      lucColour_FromString( &self->colours[1], middleColourName );
      lucColour_FromString( &self->colours[2], greatestColourName );

      lucColour_FromString( &self->colourForNegative[0], leastColourForNegativeName );
      lucColour_FromString( &self->colourForNegative[1], middleColourForNegativeName );
      lucColour_FromString( &self->colourForNegative[2], greatestColourForNegativeName );
   }*/

   self->arrowHead = arrowHeadSize;
   self->scaling = lengthScale;
   self->glyphs = glyphs;

   self->useEigenValue = useEigenValue;
   self->plotEigenVector = plotEigenVector;
   self->plotEigenValue = plotEigenValue;

   /* Get the sampler to move away from boundaries */
   self->offsetEdges = True;

   /* Append to property string */
   //OK: DEPRECATED, SET VIA PYTHON
   //lucDrawingObject_AppendProps(self, "glyphs=%d\nscaling=%g\narrowhead=%g\n", glyphs, lengthScale, arrowHeadSize); 
}

void _lucEigenvectorsCrossSection_Delete( void* drawingObject )
{
   lucEigenvectorsCrossSection*  self = (lucEigenvectorsCrossSection*)drawingObject;

   _lucCrossSection_Delete( self );
}

void _lucEigenvectorsCrossSection_Print( void* drawingObject, Stream* stream )
{
   lucEigenvectorsCrossSection*  self = (lucEigenvectorsCrossSection*)drawingObject;

   _lucCrossSection_Print( self, stream );
}

void* _lucEigenvectorsCrossSection_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                                     _sizeOfSelf = sizeof(lucEigenvectorsCrossSection);
   Type                                                             type = lucEigenvectorsCrossSection_Type;
   Stg_Class_DeleteFunction*                                     _delete = _lucEigenvectorsCrossSection_Delete;
   Stg_Class_PrintFunction*                                       _print = _lucEigenvectorsCrossSection_Print;
   Stg_Class_CopyFunction*                                         _copy = NULL;
   Stg_Component_DefaultConstructorFunction*         _defaultConstructor = _lucEigenvectorsCrossSection_DefaultNew;
   Stg_Component_ConstructFunction*                           _construct = _lucEigenvectorsCrossSection_AssignFromXML;
   Stg_Component_BuildFunction*                                   _build = _lucEigenvectorsCrossSection_Build;
   Stg_Component_InitialiseFunction*                         _initialise = _lucEigenvectorsCrossSection_Initialise;
   Stg_Component_ExecuteFunction*                               _execute = _lucEigenvectorsCrossSection_Execute;
   Stg_Component_DestroyFunction*                               _destroy = _lucEigenvectorsCrossSection_Destroy;
   lucDrawingObject_SetupFunction*                                _setup = _lucCrossSection_Setup;
   lucDrawingObject_DrawFunction*                                  _draw = _lucEigenvectorsCrossSection_Draw;
   lucDrawingObject_CleanUpFunction*                            _cleanUp = lucDrawingObject_CleanUp;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*) _lucEigenvectorsCrossSection_New(  LUCEIGENVECTORSCROSSSECTION_PASSARGS  );
}

void _lucEigenvectorsCrossSection_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data )
{
   lucEigenvectorsCrossSection* self = (lucEigenvectorsCrossSection*)drawingObject;

   /* Construct Parent */
   self->defaultResolution = 8;  /* Default sampling res */
   _lucCrossSection_AssignFromXML( self, cf, data );
   self->gatherData = False;     /* Drawn in parallel */

   _lucEigenvectorsCrossSection_Init(
      self,
      Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 2  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"leastColour", "black"  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"middleColour", "black"  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"greatestColour", "black"  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"arrowHeadSize", 0.3  ),
      Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"lengthScale", 1.0  ),
      Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"glyphs", 3),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"useEigenValue", True  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"plotEigenVector", True  ),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"plotEigenValue", False  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"leastColourForNegative", "black"  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"middleColourForNegative", "black"  ),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"greatestColourForNegative", "black" ) );
}

void _lucEigenvectorsCrossSection_Build( void* drawingObject, void* data )
{
   /* Build field variable in parent */
   _lucCrossSection_Build(drawingObject, data);
}

void _lucEigenvectorsCrossSection_Initialise( void* drawingObject, void* data ) {}
void _lucEigenvectorsCrossSection_Execute( void* drawingObject, void* data ) {}
void _lucEigenvectorsCrossSection_Destroy( void* drawingObject, void* data ) {}

void _lucEigenvectorsCrossSection_Draw( void* drawingObject, lucDatabase* database, void* _context )
{
   lucEigenvectorsCrossSection*       self            = (lucEigenvectorsCrossSection*)drawingObject;
   _lucEigenvectorsCrossSection_DrawCrossSection( self, database);
}

void _lucEigenvectorsCrossSection_DrawCrossSection( void* drawingObject, lucDatabase* database)
{
   lucEigenvectorsCrossSection*  self           = (lucEigenvectorsCrossSection*)drawingObject;
   Dimension_Index dim = self->dim;
   SymmetricTensor   tensor;
   Eigenvector       eigenvectorList[3];
   Dimension_Index   dim_I;
   Index          aIndex, bIndex;

   /* Sample the 2d cross-section */
   lucCrossSection_SampleField(self, False);

   for ( aIndex = 0 ; aIndex < self->resolutionA ; aIndex++ )
   {
      for ( bIndex = 0 ; bIndex < self->resolutionB ; bIndex++ )
      {
         if (self->values[aIndex][bIndex][0] != HUGE_VAL)
         {
            /* Get tensor data & position */
            int t;
            for (t=0; t<self->fieldComponentCount; t++)
               tensor[t] = self->values[aIndex][bIndex][t];

            SymmetricTensor_CalcAllEigenvectors( tensor, dim, eigenvectorList );

            float pos[3] = {self->vertices[aIndex][bIndex][0], self->vertices[aIndex][bIndex][1], self->vertices[aIndex][bIndex][2]};

            if (self->plotEigenVector)
            {
               for ( dim_I = 0 ; dim_I < dim ; dim_I++ )
               {
                  float vec[3] = {eigenvectorList[ dim_I ].vector[0], eigenvectorList[ dim_I ].vector[1], eigenvectorList[ dim_I ].vector[2]};

                  lucDatabase_AddVertices(database, 1, lucVectorType, pos);
                  lucDatabase_AddRGBA(database, lucVectorType, self->opacity, &self->colours[ dim_I ]);
                  if (self->useEigenValue)
                  {
                     vec[0] *= eigenvectorList[ dim_I ].eigenvalue;
                     vec[1] *= eigenvectorList[ dim_I ].eigenvalue;
                     if (dim > 2)
                        vec[2] *= eigenvectorList[ dim_I ].eigenvalue;
                     else
                        vec[2] = 0;
                  }
                  lucDatabase_AddVectors(database, 1, lucVectorType, 0, 1, vec);

                  //Apply a tiny shift so all 3 vectors can be seen even if exactly same position/magnitude
                  pos[0] += 0.000001;
                  pos[1] += 0.000001;
                  pos[2] += 0.000001;
               }
            }
            if (self->plotEigenValue)
            {
               float pointSize = 0;

               for ( dim_I = 0 ; dim_I < dim ; dim_I++ )
               {
                  lucDatabase_AddVertices(database, 1, lucShapeType, pos);
                  /* The EigenValue can be negative.... Got to attribute a potential */
                  /* colour for negative values, one for each dim as well */
                  if ( eigenvectorList[ dim_I ].eigenvalue >= 0)
                  {
                     pointSize = eigenvectorList[ dim_I ].eigenvalue;
                     lucDatabase_AddRGBA(database, lucShapeType, self->opacity, &self->colours[ dim_I ]);
                  }
                  else
                  {
                     pointSize = -eigenvectorList[ dim_I ].eigenvalue;
                     lucDatabase_AddRGBA(database, lucShapeType, self->opacity, &self->colourForNegative[ dim_I ]);
                  }
                  lucDatabase_AddValues(database, 1, lucShapeType, lucXWidthData, NULL, &pointSize);
               }
            }
         }
      }
   }

   lucCrossSection_FreeSampleData(self);
}


