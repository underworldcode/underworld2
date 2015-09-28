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
#include <StgFEM/StgFEM.h>

#include "types.h"
#include "SemiLagrangianIntegrator.h"

#include <assert.h>

/** Textual name of this class */
const Type SemiLagrangianIntegrator_Type = "SemiLagrangianIntegrator";

SemiLagrangianIntegrator* _SemiLagrangianIntegrator_New(  SEMILAGRANGIANINTEGRATOR_DEFARGS  )
{
   SemiLagrangianIntegrator*		self;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(SemiLagrangianIntegrator) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (SemiLagrangianIntegrator*) _Stg_Component_New(  STG_COMPONENT_PASSARGS  );

   /* General info */
   self->variableList = Stg_ObjectList_New();
   self->varStarList  = Stg_ObjectList_New();
   self->varOldList   = Stg_ObjectList_New();

   return self;
}

void* _SemiLagrangianIntegrator_Copy( void* slIntegrator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
   SemiLagrangianIntegrator*		self = (SemiLagrangianIntegrator*)slIntegrator;
   SemiLagrangianIntegrator*		newSemiLagrangianIntegrator;
   PtrMap*			map = ptrMap;
   Bool			ownMap = False;

   if( !map ) {
      map = PtrMap_New( 10 );
      ownMap = True;
   }

   newSemiLagrangianIntegrator = _Stg_Component_Copy( self, dest, deep, nameExt, map );

   if( deep ) {
      if( (newSemiLagrangianIntegrator->velocityField = PtrMap_Find( map, self->velocityField )) == NULL ) {
         newSemiLagrangianIntegrator->velocityField = Stg_Class_Copy( self->velocityField, NULL, deep, nameExt, map );
         PtrMap_Append( map, self->velocityField, newSemiLagrangianIntegrator->velocityField );
      }
   }
   else {
      newSemiLagrangianIntegrator->velocityField = Stg_Class_Copy( self->velocityField, NULL, deep, nameExt, map );
   }

   if( ownMap ) {
      Stg_Class_Delete( map );
   }

   return (void*)newSemiLagrangianIntegrator;
}


void _SemiLagrangianIntegrator_Delete( void* slIntegrator ) {
   SemiLagrangianIntegrator*		self = (SemiLagrangianIntegrator*)slIntegrator;
   Stg_Class_Delete( self->variableList );
   Stg_Class_Delete( self->varStarList );
   Stg_Class_Delete( self->varOldList );
}

void _SemiLagrangianIntegrator_Print( void* slIntegrator, Stream* stream ) {
   SemiLagrangianIntegrator*		self = (SemiLagrangianIntegrator*)slIntegrator;

   _Stg_Component_Print( self, stream );

   Journal_PrintPointer( stream, self->velocityField );
}

void* _SemiLagrangianIntegrator_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof(SemiLagrangianIntegrator);
   Type                                                      type = SemiLagrangianIntegrator_Type;
   Stg_Class_DeleteFunction*                              _delete = _SemiLagrangianIntegrator_Delete;
   Stg_Class_PrintFunction*                                _print = _SemiLagrangianIntegrator_Print;
   Stg_Class_CopyFunction*                                  _copy = _SemiLagrangianIntegrator_Copy;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _SemiLagrangianIntegrator_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _SemiLagrangianIntegrator_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _SemiLagrangianIntegrator_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _SemiLagrangianIntegrator_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _SemiLagrangianIntegrator_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _SemiLagrangianIntegrator_Destroy;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*)_SemiLagrangianIntegrator_New(  SEMILAGRANGIANINTEGRATOR_PASSARGS  );
}

void _SemiLagrangianIntegrator_AssignFromXML( void* slIntegrator, Stg_ComponentFactory* cf, void* data ) {
   SemiLagrangianIntegrator*	self 		= (SemiLagrangianIntegrator*)slIntegrator;
   Dictionary*			dict;
   Dictionary_Entry_Value*		dev;
   unsigned			field_i;
   Name				fieldName;
   FeVariable*	   feVariable;

   Stg_Component_AssignFromXML( self, cf, data, False );

   self->context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", FiniteElementContext, False, data );
   if( !self->context  )
      self->context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", FiniteElementContext, True, data  );

   self->velocityField = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"VelocityField", FeVariable, False, NULL  );
   self->advectedField = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"AdvectedField", FeVariable, False, NULL  );

   dict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( cf->componentDict, (Dictionary_Entry_Key)self->name )  );
   dev  = Dictionary_Get( dict, (Dictionary_Entry_Key)"fields" );
   for( field_i = 0; field_i < Dictionary_Entry_Value_GetCount( dev ); field_i += 3  ) {
      fieldName = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( dev, field_i ) );
      feVariable = Stg_ComponentFactory_ConstructByName( cf, (Name)fieldName, FeVariable, True, data  );
      Stg_ObjectList_Append( self->variableList, feVariable );

      /* the corresponding _* field */
      fieldName = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( dev, field_i + 1 ) );
      feVariable = Stg_ComponentFactory_ConstructByName( cf, (Name)fieldName, FeVariable, True, data  );
      Stg_ObjectList_Append( self->varStarList, feVariable );

      /* the corresponding old field */
      fieldName = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( dev, field_i + 2 ) );
      feVariable = Stg_ComponentFactory_ConstructByName( cf, (Name)fieldName, FeVariable, True, data  );
      Stg_ObjectList_Append( self->varOldList, feVariable );
   }

   self->sle = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"SLE", Energy_SLE, False, NULL  );

   /* for problems with temporally evolving velocity */
   self->prevVelField = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"PreviousTimeStepVelocityField", FeVariable, False, data );
   if( self->prevVelField  ) {
      EP_AppendClassHook( Context_GetEntryPoint( self->context, AbstractContext_EP_UpdateClass ), SemiLagrangianIntegrator_UpdatePreviousVelocityField, self );
      EP_InsertClassHookAfter( Context_GetEntryPoint( self->context, AbstractContext_EP_UpdateClass ), "SemiLagrangianIntegrator_UpdatePreviousVelocityField", SemiLagrangianIntegrator_InitSolve, self );
   } else {
      EP_AppendClassHook( Context_GetEntryPoint( self->context, AbstractContext_EP_UpdateClass ), SemiLagrangianIntegrator_InitSolve, self );
   }

   if( self->sle ) {
      /** also set sle to run where required */
      EP_InsertClassHookAfter( Context_GetEntryPoint( self->context, AbstractContext_EP_UpdateClass ), "SemiLagrangianIntegrator_InitSolve", SystemLinearEquations_GetRunEPFunction(), self->sle );
      /** remember to disable the standard run at execute */
      SystemLinearEquations_SetRunDuringExecutePhase( self->sle, False);
   }

   self->isConstructed = True;
}

void _SemiLagrangianIntegrator_Build( void* slIntegrator, void* data ) {
   SemiLagrangianIntegrator*	self 		= (SemiLagrangianIntegrator*)slIntegrator;
   FeVariable*			feVariable;
   FeVariable*			feVarOld;
   FeVariable*			feVarStar;
   unsigned			   field_i;

   if(self->velocityField) Stg_Component_Build(self->velocityField, data, False);
   if(self->advectedField) Stg_Component_Build(self->advectedField, data, False);
   if(self->prevVelField)  Stg_Component_Build(self->prevVelField , data, False);

   for( field_i = 0; field_i < self->variableList->count; field_i++ ) {
      feVariable = (FeVariable*) self->variableList->data[field_i];
      feVarOld   = (FeVariable*) self->varOldList->data[field_i];
      feVarStar  = (FeVariable*) self->varStarList->data[field_i];
      if(feVariable) Stg_Component_Build(feVariable, data, False);
      if(feVarOld  ) Stg_Component_Build(feVarOld  , data, False);
      if(feVarStar ) Stg_Component_Build(feVarStar , data, False);
   }
}

void _SemiLagrangianIntegrator_Initialise( void* slIntegrator, void* data ) {
   SemiLagrangianIntegrator*	self 		= (SemiLagrangianIntegrator*)slIntegrator;
   FeVariable*			feVariable;
   FeVariable*			feVarOld;
   FeVariable*			feVarStar;
   unsigned			   lMeshSize;
   unsigned			   field_i, node_i;
   double				phi[3];

   if(self->velocityField) Stg_Component_Initialise(self->velocityField, data, False);
   if(self->advectedField) Stg_Component_Initialise(self->advectedField, data, False);
   if(self->prevVelField) {
      Stg_Component_Initialise(self->prevVelField , data, False);
      SemiLagrangianIntegrator_UpdatePreviousVelocityField( slIntegrator, NULL );
   }

   for( field_i = 0; field_i < self->variableList->count; field_i++ ) {
      feVariable = (FeVariable*) self->variableList->data[field_i];
      feVarOld   = (FeVariable*) self->varOldList->data[field_i];
      feVarStar  = (FeVariable*) self->varStarList->data[field_i];
      if(feVariable) Stg_Component_Initialise(feVariable, data, False);
      if(feVarOld  ) Stg_Component_Initialise(feVarOld  , data, False);
      if(feVarStar ) Stg_Component_Initialise(feVarStar , data, False);
   }
}

void _SemiLagrangianIntegrator_Execute( void* slIntegrator, void* data ) {
}

void _SemiLagrangianIntegrator_Destroy( void* slIntegrator, void* data ) {
   SemiLagrangianIntegrator*	self 	= (SemiLagrangianIntegrator*)slIntegrator;
   FeVariable*			feVariable;
   FeVariable*			feVarOld;
   FeVariable*			feVarStar;
   unsigned			   field_i;

   if(self->velocityField) Stg_Component_Destroy(self->velocityField, data, False);
   if(self->advectedField) Stg_Component_Destroy(self->advectedField, data, False);
   if(self->prevVelField)  Stg_Component_Destroy(self->prevVelField , data, False);

   for( field_i = 0; field_i < self->variableList->count; field_i++ ) {
      feVariable = (FeVariable*) self->variableList->data[field_i];
      feVarOld   = (FeVariable*) self->varOldList->data[field_i];
      feVarStar  = (FeVariable*) self->varStarList->data[field_i];
      if(feVariable) Stg_Component_Destroy(feVariable, data, False);
      if(feVarOld  ) Stg_Component_Destroy(feVarOld  , data, False);
      if(feVarStar ) Stg_Component_Destroy(feVarStar , data, False);
   }
}

void SemiLagrangianIntegrator_InitSolve( void* _self, void* _context ) {
   SemiLagrangianIntegrator*	self			= (SemiLagrangianIntegrator*) _self;
   unsigned			   field_i, node_i;
   FeVariable*			feVariable;
   FeVariable*			feVarOld;
   FeVariable*			feVarStar;
   double				dt			= self->context->dt;
   double            phi[3];
   unsigned			   lMeshSize;
   FeMesh*				mesh;

   for( field_i = 0; field_i < self->variableList->count; field_i++ ) {
      feVariable = (FeVariable*) self->variableList->data[field_i];
      feVarStar  = (FeVariable*) self->varStarList->data[field_i];
      feVarOld   = (FeVariable*) self->varOldList->data[field_i];

      /* we're assuming that the solution vector has already been updated onto the FeVariable (in the SLE class) */
      mesh = feVariable->feMesh;
      lMeshSize = Mesh_GetLocalSize( mesh, MT_VERTEX );
      for( node_i = 0; node_i < lMeshSize; node_i++ ) {
         FeVariable_GetValueAtNode( feVariable, node_i, phi );
         FeVariable_SetValueAtNode( feVarOld, node_i, phi );
      }
      FeVariable_SyncShadowValues( feVarOld );

      /* generate the _* field */
      SemiLagrangianIntegrator_Solve( self, feVarOld, feVarStar );
   }
}

#define INV6 0.166666666667

void IntegrateRungeKutta( FeVariable* velocityField, double dt, double* origin, double* position ) {
   unsigned		ndims		     = Mesh_GetDimSize( velocityField->feMesh );
   unsigned		dim_i;
   double			min[3], max[3];
   double			k[4][3];
   double			coordPrime[3];
   unsigned*		periodic	     = ((CartesianGenerator*)velocityField->feMesh->generator)->periodic;

   Mesh_GetGlobalCoordRange( velocityField->feMesh, min, max );

   FieldVariable_InterpolateValueAt( velocityField, origin, k[0] );
   for( dim_i = 0; dim_i < ndims; dim_i++ ) {
      coordPrime[dim_i] = origin[dim_i] - 0.5 * dt * k[0][dim_i];
      PeriodicUpdate( coordPrime, min, max, dim_i, periodic[dim_i] );
   }
   FieldVariable_InterpolateValueAt( velocityField, coordPrime, k[1] );

   for( dim_i = 0; dim_i < ndims; dim_i++ ) {
      coordPrime[dim_i] = origin[dim_i] - 0.5 * dt * k[1][dim_i];
      PeriodicUpdate( coordPrime, min, max, dim_i, periodic[dim_i] );
   }
   FieldVariable_InterpolateValueAt( velocityField, coordPrime, k[2] );

   for( dim_i = 0; dim_i < ndims; dim_i++ ) {
      coordPrime[dim_i] = origin[dim_i] - dt * k[2][dim_i];
      PeriodicUpdate( coordPrime, min, max, dim_i, periodic[dim_i] );
   }
   FieldVariable_InterpolateValueAt( velocityField, coordPrime, k[3] );

   for( dim_i = 0; dim_i < ndims; dim_i++ ) {
      position[dim_i] = origin[dim_i] -
         INV6 * dt * ( k[0][dim_i] + 2.0 * k[1][dim_i] + 2.0 * k[2][dim_i] + k[3][dim_i] );
      PeriodicUpdate( position, min, max, dim_i, periodic[dim_i] );
   }
}

/* 2nd order acurate runge kutta algorithm for interpolating backwards in time through a temporally evolving velocity field
   Durran, D. "Numerical Methods for Wave Equations in Geophysical Fluid Dynamics" (1999), pages 310-313

   u(t^(n+0.5)) = 1.5u(t^n) - 0.5u(t^(n-1))
   x_* = x^(n+1) - 0.5*dt*u(x^(n+1),t^n)
   x_j^n = x^(n+1) - dt*u(x_*,t^(n+0.5))

   Force term:
      F = 0.5*[3*S(x_j^n,t^n) - S(x_j^(n-1),t^(n-1))]
 */
void IntegrateRungeKutta_VariableVelocity( FeVariable* currVelField, FeVariable* interVelField, double dt, double* origin, double* position ) {
   unsigned		nDims		     = Mesh_GetDimSize( currVelField->feMesh );
   unsigned		dim_i;
   double			min[3], max[3];
   double			midPoint[3];
   double			velCurr[3], velInter[3];
   CartesianGenerator*	gen		     = (CartesianGenerator*)currVelField->feMesh->generator;

   Mesh_GetGlobalCoordRange( currVelField->feMesh, min, max );

   FieldVariable_InterpolateValueAt( currVelField, origin, velCurr );
   for( dim_i = 0; dim_i < nDims; dim_i++ ) {
      midPoint[dim_i] = origin[dim_i] - 0.5 * dt * velCurr[dim_i];
      PeriodicUpdate( midPoint, min, max, dim_i, gen->periodic[dim_i] );
   }

   FieldVariable_InterpolateValueAt( interVelField, midPoint, velInter );

   /* 2nd order approximation of velocity at time current + dt/2 */
   for( dim_i = 0; dim_i < nDims; dim_i++ ) {
      position[dim_i] = origin[dim_i] - dt * velInter[dim_i];
      PeriodicUpdate( position, min, max, dim_i, gen->periodic[dim_i] );
   }
}

/* for case of temporally evolving velocity field, when we need to integrate backwards in time
 * throught this to find our take off point */
void SemiLagrangianIntegrator_UpdatePreviousVelocityField( void* _self, void* _context ) {
   SemiLagrangianIntegrator*	self		= (SemiLagrangianIntegrator*) _self;
   FeVariable*			currVelField	= self->velocityField;
   FeVariable*			prevVelField	= self->prevVelField;
   unsigned			node_i;
   double				vel[3];

   for( node_i = 0; node_i < Mesh_GetLocalSize( currVelField->feMesh, MT_VERTEX ); node_i++ ) {
      FeVariable_GetValueAtNode( currVelField, node_i, vel );
      FeVariable_SetValueAtNode( prevVelField, node_i, vel );
   }

   FeVariable_SyncShadowValues( currVelField );
   FeVariable_SyncShadowValues( prevVelField );
}

/* cubic Lagrangian interpoation in 1-D */
void InterpLagrange( double x, double* coords, double** values, unsigned numdofs, double* result ) {
   unsigned	node_i, dof_i;
   unsigned	otherIndices[3];
   unsigned	otherIndexCount, otherIndex_i;
   double		factor;

   for( dof_i = 0; dof_i < numdofs; dof_i++ )
      result[dof_i] = 0.0;

   for( node_i = 0; node_i < 4; node_i++ ) {
      otherIndexCount = 0;
      for( otherIndex_i = 0; otherIndex_i < 4; otherIndex_i++ )
         if( otherIndex_i != node_i )
            otherIndices[otherIndexCount++] = otherIndex_i;

      factor = 1.0;
      for( otherIndex_i = 0; otherIndex_i < 3; otherIndex_i++ )
         factor *= ( x - coords[otherIndices[otherIndex_i]] ) / ( coords[node_i] - coords[otherIndices[otherIndex_i]] );

      for( dof_i = 0; dof_i < numdofs; dof_i++ ) {
         result[dof_i] += ( values[node_i][dof_i] * factor );
      }
   }
}

Bool PeriodicUpdate( double* pos, double* min, double* max, unsigned dim, Bool isPeriodic ) {
   if( pos[dim] < min[dim] ) {
      pos[dim] = (isPeriodic) ? max[dim] - min[dim] + pos[dim] : min[dim];
      return True;
        }
        if( pos[dim] > max[dim] ) {
         pos[dim] = (isPeriodic) ? min[dim] - max[dim] + pos[dim] : max[dim];
      return True;
        }

   return False;
}

void BicubicInterpolator( FeVariable* feVariable, double* position, double* delta, unsigned* nNodes, double* result ) {
   FeMesh*		feMesh			= feVariable->feMesh;
   Index		elementIndex;
   unsigned	nInc, *inc;
   int		x_0, y_0, z_0;
   int		x_i, y_i, z_i;
   double		localMin[3], localMax[3];
   Index		gNode_I, lNode_I;
   double		px[4], py[4], pz[4];
   unsigned	nDims			= Mesh_GetDimSize( feMesh );
   unsigned	nodeIndex[4][4];
   unsigned	node_I3D[4][4][4];
   unsigned	numdofs			= feVariable->dofLayout->dofCounts[0];
   double**	ptsX			= Memory_Alloc_2DArray_Unnamed( double, 4, 3 );
   double**	ptsY			= Memory_Alloc_2DArray_Unnamed( double, 4, 3 );
   double**	ptsZ			= Memory_Alloc_2DArray_Unnamed( double, 4, 3 );

   Mesh_SearchElements( feMesh, position, &elementIndex );
   FeMesh_GetElementNodes( feMesh, elementIndex, feVariable->inc );
   nInc = IArray_GetSize( feVariable->inc );
   inc = IArray_GetPtr( feVariable->inc );

   Mesh_GetLocalCoordRange( feMesh, localMin, localMax );
   gNode_I = Mesh_DomainToGlobal( feMesh, MT_VERTEX, inc[0] );

   x_0 = (int) gNode_I % nNodes[0];
   y_0 = ( (int) gNode_I / nNodes[0] ) % nNodes[1];
   if( nDims == 3 )
      z_0 = (int) gNode_I / ( nNodes[0] * nNodes[1] );

   /* get the number of nodes across and up that the point lies... */
   if( nInc % 3 == 0 ) { /* quadratic mesh */
      /* bottom left corner of stencil is closer to LHS of element */
      if( position[0] <= Mesh_GetVertex( feMesh, inc[1] )[0] ) x_0--;
      /* bottom left corner of stencil is closer to bottom of element */
      if( position[1] <= Mesh_GetVertex( feMesh, inc[3] )[1] ) y_0--;

      /* LHS node is global domain boundary */
      if( position[0] <= localMin[0] + delta[0] ) x_0++;
      /* RHS node is global domain boundary */
      else if( position[0] >= localMax[0] - delta[0] ) x_0--;

      /* top node is global domain boundary */
      if( position[1] <= localMin[1] + delta[1] ) y_0++;
      /* bottom node is global domain boundary */
      else if( position[1] >= localMax[1] - delta[1] ) y_0--;

      if( nDims == 3 ) {
         if( position[2] <= Mesh_GetVertex( feMesh, inc[9] )[2] ) z_0--;

         if( position[2] <= localMin[2] + delta[2] ) z_0++;
         else if( position[2] >= localMax[2] - delta[2] ) z_0--;
      }
   }
   else if ( nInc % 2 == 0 ) { /* linear mesh */
      if( position[0] > localMin[0] + delta[0] ) x_0--;
      if( position[0] >= localMax[0] - delta[0] ) x_0--;

      if( position[1] > localMin[1] + delta[1] ) y_0--;
      if( position[1] >= localMax[1] - delta[1] ) y_0--;

      if( nDims == 3 ) {
         if( position[2] > localMin[2] + delta[2] ) z_0--;
         if( position[2] >= localMax[2] - delta[2] ) z_0--;
      }
   }
   else abort();

   /* interpolate using Lagrange's formula */
   if( nDims == 2 ) {
      for( y_i = 0; y_i < 4; y_i++ )
         for( x_i = 0; x_i < 4; x_i++ ) {
            gNode_I = x_0 + x_i + ( y_0 + y_i ) * nNodes[0];
            if( !Mesh_GlobalToDomain( feMesh, MT_VERTEX, gNode_I, &lNode_I ) ) abort();
            else
               nodeIndex[x_i][y_i] = lNode_I;
         }
   }
   else {
      for( z_i = 0; z_i < 4; z_i++ )
         for( y_i = 0; y_i < 4; y_i++ )
            for( x_i = 0; x_i < 4; x_i++ ) {
               gNode_I = x_0 + x_i + ( y_0 + y_i ) * nNodes[0] + ( z_0 + z_i ) * nNodes[0] * nNodes[1];
               if( !Mesh_GlobalToDomain( feMesh, MT_VERTEX, gNode_I, &lNode_I ) ) abort();
               else
                  node_I3D[x_i][y_i][z_i] = lNode_I;
            }
   }

   if( nDims == 3 ) {
      for( x_i = 0; x_i < 4; x_i++ )
         px[x_i] = Mesh_GetVertex( feMesh, node_I3D[x_i][0][0] )[0];
      for( y_i = 0; y_i < 4; y_i++ )
         py[y_i] = Mesh_GetVertex( feMesh, node_I3D[0][y_i][0] )[1];
      for( z_i = 0; z_i < 4; z_i++ )
         pz[z_i] = Mesh_GetVertex( feMesh, node_I3D[0][0][z_i] )[2];

      for( z_i = 0; z_i < 4; z_i++ ) {
         for( y_i = 0; y_i < 4; y_i++ ) {
            for( x_i = 0; x_i < 4; x_i++ )
               FeVariable_GetValueAtNode( feVariable, node_I3D[x_i][y_i][z_i], ptsX[x_i] );

            InterpLagrange( position[0], px, ptsX, numdofs, ptsY[y_i] );
         }

         InterpLagrange( position[1], py, ptsY, numdofs, ptsZ[z_i] );
      }

      InterpLagrange( position[2], pz, ptsZ, numdofs, result );
   }
   else {
      for( x_i = 0; x_i < 4; x_i++ )
         px[x_i] = Mesh_GetVertex( feMesh, nodeIndex[x_i][0] )[0];
      for( y_i = 0; y_i < 4; y_i++ )
         py[y_i] = Mesh_GetVertex( feMesh, nodeIndex[0][y_i] )[1];

      for( y_i = 0; y_i < 4; y_i++ ) {
         for( x_i = 0; x_i < 4; x_i++ )
            FeVariable_GetValueAtNode( feVariable, nodeIndex[x_i][y_i], ptsX[x_i] );

         InterpLagrange( position[0], px, ptsX, numdofs, ptsY[y_i] );
      }

      InterpLagrange( position[1], py, ptsY, numdofs, result );
   }

   Memory_Free( ptsX );
   Memory_Free( ptsY );
   Memory_Free( ptsZ );
}

void SemiLagrangianIntegrator_Solve( void* slIntegrator, FeVariable* variableField, FeVariable* varStarField ) {
   SemiLagrangianIntegrator*	self 		     = (SemiLagrangianIntegrator*)slIntegrator;
   FiniteElementContext*		context		     = self->context;
   unsigned			node_I;
   FeMesh*				feMesh		     = variableField->feMesh;
   unsigned			meshSize	     = Mesh_GetLocalSize( feMesh, MT_VERTEX );
   FeVariable*			velocityField	     = self->velocityField;
   double				dt		     = AbstractContext_Dt( context );
   unsigned			dim_I;
   unsigned			nDims		     = Mesh_GetDimSize( feMesh );
   double				position[3];
   double				var[3];
   Grid**				grid		     = (Grid**) Mesh_GetExtension( feMesh, Grid*,  feMesh->elGridId );
   unsigned*			sizes		     = Grid_GetSizes( *grid );
   unsigned			nNodes[3];
   double				delta[3];
   unsigned			nInc;
   unsigned*			inc;

   FeMesh_GetElementNodes( variableField->feMesh, 0, variableField->inc );
   nInc = IArray_GetSize( variableField->inc );
   inc = IArray_GetPtr( variableField->inc );

   delta[0] = Mesh_GetVertex( feMesh, inc[1] )[0] - Mesh_GetVertex( feMesh, inc[0] )[0];
   if( nInc % 3 == 0 ) /* quadratic elements */ {
      delta[1] = Mesh_GetVertex( feMesh, inc[3] )[1] - Mesh_GetVertex( feMesh, inc[0] )[1];
      if( nDims == 3 )
         delta[2] = Mesh_GetVertex( feMesh, inc[9] )[2] - Mesh_GetVertex( feMesh, inc[0] )[2];
      for( dim_I = 0; dim_I < nDims; dim_I++ )
         nNodes[dim_I] = 2 * sizes[dim_I] + 1;
   }
   else {
      delta[1] = Mesh_GetVertex( feMesh, inc[2] )[1] - Mesh_GetVertex( feMesh, inc[0] )[1];
      if( nDims == 3 )
         delta[2] = Mesh_GetVertex( feMesh, inc[4] )[2] - Mesh_GetVertex( feMesh, inc[0] )[2];

      for( dim_I = 0; dim_I < nDims; dim_I++ )
         nNodes[dim_I] = sizes[dim_I] + 1;
   }

   FeVariable_SyncShadowValues( velocityField );
   FeVariable_SyncShadowValues( variableField );

   /* assume that the variable mesh is the same as the velocity mesh */
   for( node_I = 0; node_I < meshSize; node_I++ ) {
      IntegrateRungeKutta( velocityField, dt, Mesh_GetVertex( feMesh, node_I ), position );

      BicubicInterpolator( variableField, position, delta, nNodes, var );

      FeVariable_SetValueAtNode( varStarField, node_I, var );
   }
   FeVariable_SyncShadowValues( varStarField );
}



