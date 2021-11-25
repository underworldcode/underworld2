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
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <assert.h>
#include "VelicIC.h"

const Type Underworld_VelicIC_Type = "Underworld_VelicIC";

#define SMALL 1.0e-5

/* Works with SolA */
void Underworld_VelicIC_Sinusoidal( Node_LocalIndex node_lI, StgVariable_Index var_I, void* _context, void* _data, void* _result ) {
   DomainContext* context = (DomainContext*)_context;
   FeVariable*    temperatureField = (FeVariable*) FieldVariable_Register_GetByName( context->fieldVariable_Register, "TemperatureField" );
   FeMesh*        feMesh = temperatureField->feMesh;
   Dictionary*    dictionary = context->dictionary;
   double*        result = (double*) _result;
   double*        coord;
   double         x; 
   double         y;
   double         kx;
   double         ky;
   int            wavenumberX;
   double         wavenumberY;
   double         sigma;
   double         Lx;
   double         min[3], max[3];
   
   /* Find coordinate of node */
   coord = Mesh_GetVertex( feMesh, node_lI );
   Mesh_GetGlobalCoordRange( feMesh, min, max );

   /* Make sure that the box has right dimensions */
   assert( ( max[ J_AXIS ] - min[ J_AXIS ] - 1.0 ) < SMALL );
   Lx = max[ I_AXIS ] - min[ I_AXIS ];

   x = coord[ I_AXIS ] - min[ I_AXIS ];
   y = coord[ J_AXIS ] - min[ J_AXIS ];

   wavenumberX = Dictionary_GetInt_WithDefault( dictionary, (Dictionary_Entry_Key)"wavenumberX", 1 );
   wavenumberY = Dictionary_GetDouble_WithDefault( dictionary, (Dictionary_Entry_Key)"wavenumberY", 1.0 );
   sigma = Dictionary_GetDouble_WithDefault( dictionary, (Dictionary_Entry_Key)"sigma", 1.0 );
   
   assert( sigma > 0.0 );
   assert( wavenumberY > 0.0 );
   assert( wavenumberX > 0.0 );
   
   kx = (double)wavenumberX * M_PI / Lx;
   ky = (double)wavenumberY * M_PI;

   *result = sigma * sin( ky * y ) * cos( kx * x  );
}

/* Works with SolB */
void Underworld_VelicIC_Hyperbolic( Node_LocalIndex node_lI, StgVariable_Index var_I, void* _context, void* _data, void* _result ) {
   DomainContext* context = (DomainContext*)_context;
   FeVariable*    temperatureField = (FeVariable*)FieldVariable_Register_GetByName( context->fieldVariable_Register, "TemperatureField" );
   FeMesh*        feMesh = temperatureField->feMesh;
   Dictionary*    dictionary = context->dictionary;
   double*        result = (double*) _result;
   double*        coord;
   double         x; 
   double         y;
   double         km; /*  for y-direction */
   double         kn; /*  for x-direction */
   double         wavenumberX;
   double         wavenumberY;
   double         L;
   double         sigma;
   double         min[3], max[3];
   
   /* Find coordinate of node */
   coord = Mesh_GetVertex( feMesh, node_lI );
   Mesh_GetGlobalCoordRange( feMesh, min, max );

   /* Make sure that the box has right dimensions */
   assert( (max[ J_AXIS ] - min[ J_AXIS ] - 1.0 ) < SMALL );
   L = max[ I_AXIS ] - min[ I_AXIS ];

   x = coord[ I_AXIS ] - min[ I_AXIS ];
   y = coord[ J_AXIS ] - min[ J_AXIS ];

   wavenumberX = Dictionary_GetInt_WithDefault( dictionary, (Dictionary_Entry_Key)"wavenumberX", 1 );
   wavenumberY = Dictionary_GetDouble_WithDefault( dictionary, (Dictionary_Entry_Key)"wavenumberY", 2.0 );
   assert( wavenumberX != wavenumberY  );
   sigma = Dictionary_GetDouble_WithDefault( dictionary, (Dictionary_Entry_Key)"sigma", 1.0 );

   kn = wavenumberX * M_PI / L;
   /* TODO: Re-write Mirko's code and/or Documentation so the input parameters for these ICs are less confusing */
   km = wavenumberY / L;

   *result = sigma * sinh( km * y ) * cos( kn * x  );
}

void _Underworld_VelicIC_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
   AbstractContext*   context;
   ConditionFunction* condFunc;

   context = (AbstractContext*)Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, True, data ); 
   
   condFunc = ConditionFunction_New( Underworld_VelicIC_Sinusoidal, (Name)"VelicIC_Sinusoidal", NULL );
   ConditionFunction_Register_Add( condFunc_Register, condFunc );
   condFunc = ConditionFunction_New( Underworld_VelicIC_Hyperbolic, (Name)"VelicIC_Hyperbolic", NULL );
   ConditionFunction_Register_Add( condFunc_Register, condFunc );
}   

void* _Underworld_VelicIC_DefaultNew( Name name ) {
   return Codelet_New(
      Underworld_VelicIC_Type,
      _Underworld_VelicIC_DefaultNew,
      _Underworld_VelicIC_AssignFromXML,
      _Codelet_Build,
      _Codelet_Initialise,
      _Codelet_Execute,
      _Codelet_Destroy,
      name );
}

Index Underworld_VelicIC_Register( PluginsManager* pluginsManager ) {
   Journal_DPrintf( StgFEM_Debug, "In: %s( void* )\n", __func__ );

   return PluginsManager_Submit( pluginsManager, Underworld_VelicIC_Type, (Name)"0", _Underworld_VelicIC_DefaultNew );
}


