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
#include "ElementType.h"
#include "ElementType_Register.h"
#include "Element.h"
#include "FeMesh.h"
#include "FeEquationNumber.h"
#include "FeVariable.h"
#include "LinkedDofInfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if defined( READ_HDF5) || defined( WRITE_HDF5)
#include <hdf5.h>
#endif

const Type FeVariable_Type = "FeVariable";
const Name defaultFeVariableFeEquationNumberName = "defaultFeVariableFeEqName";

/* MPI Tags */
static const int DOF_VALUES_TAG = 10;

/* Global objects */
Stg_ObjectList* FeVariable_FileFormatImportExportList = NULL;

FeVariable* FeVariable_New_FromTemplate(
   Name                    name,
   DomainContext*          context,
   void*                   _templateFeVariable,
   DofLayout*              dofLayout, 
   void*                   ics,
   Bool                    isReferenceSolution,
   Bool                    loadReferenceEachTimestep,
   FieldVariable_Register* fV_Register )
{
   FeVariable* templateFeVariable = _templateFeVariable;
   FeVariable* newFeVariable = NULL;
   
   newFeVariable = FeVariable_New_Full(
      name, 
      context,
      templateFeVariable->feMesh,
      dofLayout,
      templateFeVariable->bcs,
      ics,
      templateFeVariable->linkedDofInfo,
      templateFeVariable,
      templateFeVariable->fieldComponentCount,
      templateFeVariable->dim,
      templateFeVariable->isCheckpointedAndReloaded,
      isReferenceSolution,
      loadReferenceEachTimestep,
      templateFeVariable->communicator,
      fV_Register );

   newFeVariable->templateFeVariable = templateFeVariable;

   return newFeVariable;
}

FeVariable* FeVariable_New(
   Name                    name,
   DomainContext*          context,
   void*                   feMesh,
   DofLayout*              dofLayout, 
   void*                   bcs,
   void*                   ics,
   void*                   linkedDofInfo,
   Dimension_Index         dim,
   Bool                    isCheckpointedAndReloaded,
   Bool                    isReferenceSolution,
   Bool                    loadReferenceEachTimestep,
   FieldVariable_Register* fV_Register )      
{
   return FeVariable_New_Full(
      name,
      context,
      feMesh,
      dofLayout,
      bcs,
      ics,
      linkedDofInfo,
      NULL,
      dofLayout->_totalVarCount,
      dim,
      isCheckpointedAndReloaded,
      isReferenceSolution,
      loadReferenceEachTimestep,
      ((IGraph*)((FeMesh*)feMesh)->topo)->remotes[MT_VERTEX]->comm->mpiComm, 
      fV_Register );
}

FeVariable* FeVariable_New_Full(
   Name                    name,
   DomainContext*          context,
   void*                   feMesh,
   DofLayout*              dofLayout, 
   void*                   bcs,
   void*                   ics,
   void*                   linkedDofInfo,
   void*                   templateFeVariable,
   Index                   fieldComponentCount,
   Dimension_Index         dim,
   Bool                    isCheckpointedAndReloaded,
   Bool                    isReferenceSolution,
   Bool                    loadReferenceEachTimestep,
   MPI_Comm                communicator,
   FieldVariable_Register* fieldVariable_Register )      
{
   FeVariable* self = _FeVariable_DefaultNew( name );

   self->isConstructed = True;

   _FieldVariable_Init( (FieldVariable*)self, context, fieldComponentCount, dim, isCheckpointedAndReloaded,
      NULL, communicator, fieldVariable_Register, False );

   _FeVariable_Init( self, feMesh, dofLayout, bcs, False, ics, linkedDofInfo, templateFeVariable,
      isReferenceSolution, loadReferenceEachTimestep );

   return self;
}

void* _FeVariable_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                                         _sizeOfSelf = sizeof( FeVariable );
   Type                                                                 type = FeVariable_Type;
   Stg_Class_DeleteFunction*                                         _delete = _FeVariable_Delete;
   Stg_Class_PrintFunction*                                           _print = _FeVariable_Print;
   Stg_Class_CopyFunction*                                             _copy = _FeVariable_Copy;
   Stg_Component_DefaultConstructorFunction*             _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_FeVariable_DefaultNew;
   Stg_Component_ConstructFunction*                               _construct = _FeVariable_AssignFromXML;
   Stg_Component_BuildFunction*                                       _build = _FeVariable_Build;
   Stg_Component_InitialiseFunction*                             _initialise = _FeVariable_Initialise;
   Stg_Component_ExecuteFunction*                                   _execute = _FeVariable_Execute;
   Stg_Component_DestroyFunction*                                   _destroy = _FeVariable_Destroy;
   AllocationType                                         nameAllocationType = NON_GLOBAL;
   FieldVariable_InterpolateValueAtFunction*             _interpolateValueAt = (FieldVariable_InterpolateValueAtFunction*)_FeVariable_InterpolateValueAt;
   FieldVariable_GetValueFunction*               _getMinGlobalFieldMagnitude = _FeVariable_GetMinGlobalFieldMagnitude;
   FieldVariable_GetValueFunction*               _getMaxGlobalFieldMagnitude = _FeVariable_GetMaxGlobalFieldMagnitude;
   FieldVariable_CacheValuesFunction*       _cacheMinMaxGlobalFieldMagnitude = _FeVariable_CacheMinMaxGlobalFieldMagnitude;
   FieldVariable_GetCoordFunction*                  _getMinAndMaxLocalCoords = _FeVariable_GetMinAndMaxLocalCoords;
   FieldVariable_GetCoordFunction*                 _getMinAndMaxGlobalCoords = _FeVariable_GetMinAndMaxGlobalCoords;
   FeVariable_InterpolateWithinElementFunction*    _interpolateWithinElement = _FeVariable_InterpolateNodeValuesToElLocalCoord;
   FeVariable_GetValueAtNodeFunction*                        _getValueAtNode = _FeVariable_GetValueAtNode;
   FeVariable_SyncShadowValuesFunc*                        _syncShadowValues = _FeVariable_SyncShadowValues;

   return _FeVariable_New( FEVARIABLE_PASSARGS ); /* feVariableList */
}

FeVariable* _FeVariable_New( FEVARIABLE_DEFARGS ) {
   FeVariable* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof( FeVariable) );
   
   self = (FeVariable*) _FieldVariable_New( FIELDVARIABLE_PASSARGS );
   
   /* General info */
   
   /* Virtual functions */
   self->_interpolateWithinElement = _interpolateWithinElement;
   self->_getValueAtNode = _getValueAtNode;
   self->_syncShadowValues = _syncShadowValues;
   
   /* FeVariable info */
   self->eqNum = NULL;
   self->tempData = NULL;
   return self;
}

void _FeVariable_Init( 
   FeVariable* self,
   void*       feMesh,
   DofLayout*  dofLayout, 
   void*       bcs,
   Bool        nonAABCs,
   void*       ics,
   void*       linkedDofInfo,
   void*       templateFeVariable,
   Bool        isReferenceSolution,
   Bool        loadReferenceEachTimestep )
{
   /* General and Virtual info should already be set */
   
   /* FeVariable info */
   self->debug = Stream_RegisterChild( StgFEM_Discretisation_Debug, self->type );
   self->feMesh = Stg_CheckType( feMesh, FeMesh );
   self->dofLayout = dofLayout;

	 self->nonAABCs = nonAABCs;

   if( bcs )
      self->bcs = Stg_CheckType( bcs, VariableCondition );
   if( linkedDofInfo )
      self->linkedDofInfo = Stg_CheckType( linkedDofInfo, LinkedDofInfo );
   self->shadowValuesSynchronised = False;

   if( templateFeVariable )
      self->templateFeVariable = Stg_CheckType( templateFeVariable, FeVariable );
   if( !isReferenceSolution ) {
      if( self->templateFeVariable ) {
         self->eqNum = self->templateFeVariable->eqNum;
      }
      else {
//         self->eqNum = FeEquationNumber_New(
//            defaultFeVariableFeEquationNumberName,
//            self->context,
//            self->feMesh,
//            self->dofLayout,
//            self->bcs,
//            linkedDofInfo );
//
//         self->eqNum->removeBCs = self->removeBCs;
//         memcpy( self->eqNum->periodic, self->periodic, 3*sizeof(Bool) );
      }
   }

   self->buildEqNums = True;

   self->inc = IArray_New();
   
   self->tempData = malloc(self->fieldComponentCount*sizeof(double));
}

void _FeVariable_CreateNewEqnNumber( void* variable ) {
    FeVariable* self = (FeVariable*)variable;
    Stg_Component_Destroy( self->eqNum, NULL, False );
    Stg_Class_Delete( self->eqNum );
    self->eqNum = FeEquationNumber_New(
        defaultFeVariableFeEquationNumberName,
        self->context,
        self->feMesh,
        self->dofLayout,
        self->bcs,
        NULL );

    self->eqNum->removeBCs = self->removeBCs;
    memcpy( self->eqNum->periodic, self->periodic, 3*sizeof(Bool) );
    Stg_Component_Build( self->eqNum, NULL, False );
    Stg_Component_Initialise( self->eqNum, NULL, False );
}

void _FeVariable_Delete( void* variable ) {
   FeVariable* self = (FeVariable*)variable;
   Journal_DPrintf( self->debug, "In %s- for \"%s\":\n", __func__, self->name );
   
   if ( self->tempData ) {
       free( self->tempData );
       self->tempData = NULL;
   }
   
   /* Stg_Class_Delete parent*/
   _Stg_Component_Delete( self );
}

/* --- Virtual Function Implementations --- */

void _FeVariable_Print( void* variable, Stream* stream ) {
   FeVariable* self = (FeVariable*)variable;
   
   /* General info */
   Journal_Printf( stream, "FeVariable(ptr): %p\n", self );
   
   /* Print parent */
   _Stg_Component_Print( self, stream );
   
   /* Virtual info */
   
   /* FeVariable info */
   Stg_Class_Print( self->feMesh, stream );
   if( self->dofLayout ) 
      Stg_Class_Print( self->dofLayout, stream );
   else 
      Journal_Printf( stream, "\tdofLayout: (null)... not provided(may be Operator type)\n" );

   if( self->bcs ) 
      Stg_Class_Print( self->bcs, stream );
   else 
      Journal_Printf( stream, "\tbcs: (null)... not provided(may be Operator type)\n" );

   if( self->linkedDofInfo ) 
      Stg_Class_Print( self->linkedDofInfo, stream );
   else 
      Journal_Printf( stream, "\tlinkedDofInfo: (null)... not provided\n" );
   
   if( self->eqNum ) 
      Stg_Class_Print( self->eqNum, stream );
   else 
      Journal_Printf( stream, "\teqNum: (null)... not built yet\n" );
}

void* _FeVariable_Copy( void* feVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
   FeVariable* self = (FeVariable*)feVariable;
   FeVariable* newFeVariable;
   PtrMap*     map = ptrMap;
   Bool        ownMap = False;
   
   if( !map ) {
      map = PtrMap_New( 10 );
      ownMap = True;
   }
   
   newFeVariable = _FieldVariable_Copy( self, dest, deep, nameExt, map );

   newFeVariable->templateFeVariable = self->templateFeVariable;
   
   if( deep ) {
      newFeVariable->debug = self->debug; 
      newFeVariable->feMesh = (FeMesh*)Stg_Class_Copy( self->feMesh, NULL, deep, nameExt, map );
      newFeVariable->dofLayout = (DofLayout*)Stg_Class_Copy( self->dofLayout, NULL, deep, nameExt, map );
      newFeVariable->bcs = (VariableCondition*)Stg_Class_Copy( self->bcs, NULL, deep, nameExt, map );

      if( self->linkedDofInfo == NULL ) 
         newFeVariable->linkedDofInfo = NULL;
      else 
         newFeVariable->linkedDofInfo = (LinkedDofInfo*)Stg_Class_Copy( self->linkedDofInfo, NULL,
            deep, nameExt, map );

      newFeVariable->eqNum = NULL;
   }
   else {
      newFeVariable->debug = self->debug;
      newFeVariable->feMesh = self->feMesh;
      newFeVariable->dofLayout = self->dofLayout;
      newFeVariable->bcs = self->bcs;
      newFeVariable->linkedDofInfo = self->linkedDofInfo;
      newFeVariable->eqNum = self->eqNum;
   }
   
   if( ownMap ) 
      Stg_Class_Delete( map );
   
   return(void*)newFeVariable;
}

void _FeVariable_Build( void* variable, void* data ) {
   FeVariable*    self = (FeVariable*)variable;
   unsigned       dim, numNodes;
   
      Journal_DPrintf( self->debug, "In %s- for %s:\n", __func__, self->name );
      Stream_IndentBranch( StgFEM_Debug );

      /* Build the BCs */
      Stg_Component_Build( self->feMesh, data, False );

      if( self->dofLayout )
         Stg_Component_Build( self->dofLayout, data, False );

      if( self->bcs )
         Stg_Component_Build( self->bcs, data, False );
    
      if( self->linkedDofInfo )
         Stg_Component_Build( self->linkedDofInfo, data, False );

      /* Extract component count. */
      if( self->dofLayout )
         self->fieldComponentCount = self->dofLayout->_totalVarCount;

      dim = Mesh_GetDimSize( self->feMesh );

      /* 
       * At least this will work for meshes with names other
       * than those listed above. I spent three hours finding
       * this out.
       */
      numNodes = FeMesh_GetElementNodeSize( self->feMesh, 0 );

      /* Allocate GNx here */
      self->GNx = Memory_Alloc_2DArray( double, dim, numNodes, (Name)"Global Shape Function Derivatives" );
      
      /* 
       * Don't build the equation numbers for fields that aren't being solved for 
       * (ie: error and reference fields).
       */

      if( self->buildEqNums )
         Stg_Component_Build( self->eqNum, data, False );

      Stream_UnIndentBranch( StgFEM_Debug );
}

void _FeVariable_AssignFromXML( void* variable, Stg_ComponentFactory* cf, void* data ) {
   FeVariable*        self = (FeVariable*)variable;
   FeMesh*            feMesh = NULL;
   DofLayout*         dofLayout = NULL;
   VariableCondition* bc = NULL;
   VariableCondition* ic = NULL;
   LinkedDofInfo*     linkedDofInfo = NULL;
   Bool               isReferenceSolution = False;
   Bool               loadReferenceEachTimestep = False;
   Bool               nonAABCs = False;

   _FieldVariable_AssignFromXML( self, cf, data );

   feMesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"FEMesh",
      FeMesh, True, data );

   dofLayout = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)DofLayout_Type,
      DofLayout, True, data );

   ic = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"IC",
      VariableCondition, False, data );

   bc = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"BC",
      VariableCondition, False, data );

   linkedDofInfo = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"LinkedDofInfo",
      LinkedDofInfo, False, data );

   isReferenceSolution = Stg_ComponentFactory_GetBool( cf, self->name,
      (Dictionary_Entry_Key)"isReferenceSolution", False );

   loadReferenceEachTimestep = Stg_ComponentFactory_GetBool( cf, self->name,
      (Dictionary_Entry_Key)"loadReferenceEachTimestep", False );

   nonAABCs = Stg_ComponentFactory_GetBool( cf, self->name,
      (Dictionary_Entry_Key)"nonAxisAlignedBCs", False );

   /* TODO: should really be a parameter */
   self->removeBCs = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"removeBCs", True );

   self->periodic[0] = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"PeriodicX", False );
   self->periodic[1] = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"PeriodicY", False );
   self->periodic[2] = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"PeriodicZ", False );

   _FeVariable_Init(
      self,
      feMesh,
      dofLayout,
      bc,
      nonAABCs,
      ic,
      linkedDofInfo,
      NULL,
      isReferenceSolution,
      loadReferenceEachTimestep );
}

void _FeVariable_Initialise( void* variable, void* data ) {
   FeVariable*             self = (FeVariable*)variable;
   
   Stream_IndentBranch( StgFEM_Debug );
   
   /* do basic mesh initialisation */
   Stg_Component_Initialise( self->feMesh, data, False );
   Stg_Component_Initialise( self->dofLayout, data, False );

   if( self->linkedDofInfo ) 
      Stg_Component_Initialise( self->linkedDofInfo, data, False );

   if( self->bcs ) {
      Stg_Component_Initialise( self->bcs, data, False );
      VariableCondition_Apply( self->bcs, data );
   }

   /** Sync the feVariable after all BC and IC have been loaded */
   _FeVariable_SyncShadowValues(self);
}

void FeVariable_ApplyBCs( void* variable, void* data ) {
   FeVariable* self = (FeVariable*)variable;

   if( self->bcs ) {
      Journal_DPrintf( self->debug, "In %s- for %s:\n", __func__, self->name );
      Journal_DPrintf( self->debug, "applying the B.C.s for this Variable.\n" ); 
      VariableCondition_Apply( self->bcs, data );
   }
}

Bool FeVariable_IsBC( void* variable, int node, int dof ) {
   FeVariable* self = (FeVariable*)variable;

   if( self->bcs && 
       VariableCondition_IsCondition( self->bcs, node, self->dofLayout->varIndices[node][dof] ) )
      return True;

   return False;
}

void _FeVariable_Execute( void* variable, void* data ) {
}

void _FeVariable_Destroy( void* variable, void* data ) {
   FeVariable* self = (FeVariable*)variable;

   Memory_Free( self->GNx );

   if( self->eqNum && ( NULL == self->templateFeVariable ) ) {
      Stg_Component_Destroy( self->eqNum, NULL, False );
      self->eqNum = NULL;
   }

   /* FeMesh bc and doflayout are purposely not deleted */
   if( self->inc != NULL ) {
      Stg_Class_Delete( self->inc );
      self->inc = NULL;
   }

   _FieldVariable_Destroy( self, data );
}

void FeVariable_PrintLocalDiscreteValues( void* variable, Stream* stream ) {
   FeVariable* self = (FeVariable*)variable;

   Journal_Printf( stream, "In %s: for FeVariable \"%s\":\n", __func__, self->name );

   _FeVariable_PrintLocalOrDomainValues(
      variable,
      FeMesh_GetNodeLocalSize( self->feMesh ),
      stream );
}

InterpolationResult _FeVariable_InterpolateValueAt( void* variable, const double* globalCoord, double* value ) {
   FeVariable*          self = (FeVariable*)variable;
   Element_DomainIndex  elementCoordIn = (unsigned)-1;
   Coord                elLocalCoord={0,0,0};
   InterpolationResult  retValue;


   retValue = FeVariable_GetElementLocalCoordAtGlobalCoord( self, (double*)globalCoord, elLocalCoord, &elementCoordIn );

   /** Now interpolate the value at that coordinate, using shape functions */
   if ( (retValue == LOCAL) || (retValue == SHADOW) )
      self->_interpolateWithinElement( self, elementCoordIn, elLocalCoord, value );

   return retValue;
}

double _FeVariable_GetMinGlobalFieldMagnitude( void* feVariable ) {
   return FieldVariable_GetMinGlobalFieldMagnitude( feVariable );
}

double _FeVariable_GetMaxGlobalFieldMagnitude( void* feVariable ) {
   return FieldVariable_GetMaxGlobalFieldMagnitude( feVariable );
}

void _FeVariable_CacheMinMaxGlobalFieldMagnitude( void* feVariable ) {
   FeVariable* self = (FeVariable*)feVariable;
   int      node_lI=0;
   int      nodeLocalCount = FeMesh_GetNodeLocalSize( self->feMesh );
   double   min = 0;
   double   max = 0;
   double   currValue;
   MPI_Comm comm = MPI_COMM_WORLD;

   min = FeVariable_GetScalarAtNode( self, 0 );
   max = FeVariable_GetScalarAtNode( self, 0 );

   /** Find upper and lower bounds on this processor */
   for ( node_lI = 1 ; node_lI < nodeLocalCount ; node_lI++ ) {
      currValue = FeVariable_GetScalarAtNode( self, node_lI );
      if( currValue < min )
         min = currValue;
      if( currValue > max )
         max = currValue;
   }

   /** Find upper and lower bounds on all processors */
   (void)MPI_Allreduce( &min, &self->magnitudeMin, 1, MPI_DOUBLE, MPI_MIN, comm);
   (void)MPI_Allreduce( &max, &self->magnitudeMax, 1, MPI_DOUBLE, MPI_MAX, comm);
}

void _FeVariable_GetMinAndMaxLocalCoords( void* feVariable, double* min, double* max ) {
   FeVariable* self = (FeVariable*)feVariable;

   assert( self && Stg_CheckType( self, FeVariable ) );
   Mesh_GetLocalCoordRange( self->feMesh, min, max );
}

void _FeVariable_GetMinAndMaxGlobalCoords( void* feVariable, double* min, double* max ) {
   FeVariable* self = (FeVariable*)feVariable;

   assert( self && Stg_CheckType( self, FeVariable ) );
   Mesh_GetGlobalCoordRange( self->feMesh, min, max );
}

double FeVariable_GetScalarAtNode( void* feVariable, Node_LocalIndex lNode_I ) {
   FeVariable* self = (FeVariable*)feVariable;
   Dof_Index   dofCountThisNode = 0;
   Dof_Index   nodeLocalDof_I = 0;
   
   dofCountThisNode = self->fieldComponentCount;
   FeVariable_GetValueAtNode( self, lNode_I, self->tempData );

   if( dofCountThisNode > 1) {
      double magnitude = 0;
      for( nodeLocalDof_I = 0; nodeLocalDof_I < dofCountThisNode; nodeLocalDof_I++ ) 
         magnitude += self->tempData[ nodeLocalDof_I ] * self->tempData[ nodeLocalDof_I ];
      return sqrt( magnitude );
   }
   else 
      return self->tempData[0];
}   

void _FeVariable_GetValueAtNode( void* feVariable, Node_DomainIndex dNode_I, double* value ) {
   FeVariable* self = (FeVariable*)feVariable;
   Variable*   currVariable = NULL;
   Dof_Index   dofCountThisNode = 0;
   Dof_Index   nodeLocalDof_I = 0;

   dofCountThisNode = self->dofLayout->dofCounts[dNode_I];
   
   for( nodeLocalDof_I=0; nodeLocalDof_I < dofCountThisNode; nodeLocalDof_I++ ) {
      currVariable = DofLayout_GetVariable( self->dofLayout, dNode_I, nodeLocalDof_I );
      value[ nodeLocalDof_I ] = Variable_GetValueDouble( currVariable, dNode_I );
   }
}

/* Finds the value of the field at the node and broadcasts it to the rest of the processors */
void FeVariable_GetValueAtNodeGlobal( void* feVariable, Node_GlobalIndex gNode_I, double* value ) {
   FeVariable*        self = (FeVariable*) feVariable;
   FeMesh*            mesh = self->feMesh;
   Element_LocalIndex lNode_I;
   int                rootRankL = 0;
   int                rootRankG = 0;
   MPI_Comm           comm = self->communicator;
   
   /* Find Local Index */
   if( Mesh_GlobalToDomain( mesh, MT_VERTEX, gNode_I, &lNode_I ) ) {
      /* If node is on local processor, then get value of field */
      FeVariable_GetValueAtNode( self, lNode_I, value );
      MPI_Comm_rank( comm, (int*)&rootRankL );
   }
   
   /* Send to other processors */
   (void)MPI_Allreduce( &rootRankL, &rootRankG, 1, MPI_INT, MPI_MAX, comm );
   MPI_Bcast( value, self->fieldComponentCount, MPI_DOUBLE, rootRankG, comm );
}

/* Finds the coordinate of the node and broadcasts it to the rest of the processors */
void FeVariable_GetCoordAtNodeGlobal( void* feVariable, Node_GlobalIndex gNode_I, double* coord ) {
   FeVariable*        self = (FeVariable*) feVariable;
   FeMesh*            mesh = self->feMesh;
   Element_LocalIndex lNode_I;
   int                rootRankL = 0;
   int                rootRankG = 0;
   MPI_Comm           comm = self->communicator;

   /* Find Local Index */
   if( Mesh_GlobalToDomain( mesh, MT_VERTEX, gNode_I, &lNode_I ) ) {
      /* If node is on local processor, then get value of field */
      memcpy( coord, Mesh_GetVertex( mesh, lNode_I ), self->dim * sizeof( double) );
      MPI_Comm_rank( comm, (int*)&rootRankL );
   }
   
   /* Send to other processors */
   (void)MPI_Allreduce( &rootRankL, &rootRankG, 1, MPI_INT, MPI_MAX, comm );
   MPI_Bcast( coord, self->dim, MPI_DOUBLE, rootRankG, comm );
}

void FeVariable_SetBC( void* feVariable, void* bc ) {
   FeVariable* self = (FeVariable*) feVariable;
   self->bcs = bc;
}


void FeVariable_ZeroField( void* feVariable ) {
   FeVariable* self = (FeVariable*) feVariable;
   double*     values =  Memory_Alloc_Array( double, self->fieldComponentCount, "tempValues" );
   Index       lNode_I, lNodeCount;

   lNodeCount = FeMesh_GetNodeLocalSize( self->feMesh );

   memset( values, 0, self->fieldComponentCount * sizeof(double) );

   for( lNode_I = 0 ; lNode_I < lNodeCount; lNode_I++ ) 
      FeVariable_SetValueAtNode( self, lNode_I, values );

   Memory_Free( values );
}

/* --- Public Functions --- */
InterpolationResult FeVariable_GetElementLocalCoordAtGlobalCoord(
   void*                feVariable,
   double*              globalCoord,
   double*              elLocalCoord,
   Element_DomainIndex* elementCoordInPtr )
{
   FeVariable*         self = (FeVariable*)feVariable;
   InterpolationResult retValue;
   unsigned            elInd;

   /* 
    * Locate which mesh element given coord is in : use inclusive upper boundaries to save
    * the need to use shadow space if possible.
    */
   if( !Mesh_SearchElements( self->feMesh, globalCoord, &elInd ) ) {
      Bool            outsideGlobal = False;
      double          min[3], max[3];
      Dimension_Index dim_I=0;

      FieldVariable_GetMinAndMaxGlobalCoords( self, min, max );

      for( dim_I = 0; dim_I < self->dim; dim_I++ ) {
         if( ( globalCoord[dim_I] < min[dim_I] ) || (globalCoord[dim_I] > max[dim_I] ) ) 
            outsideGlobal = True;
      }

      if( outsideGlobal == True ) 
         return OUTSIDE_GLOBAL;
      else 
         return OTHER_PROC;
   }
   else {
      /* We found the coord is within a local or shadow element */
      ElementType* elementType = NULL;
      *elementCoordInPtr = elInd;
      if( elInd < FeMesh_GetElementLocalSize( self->feMesh ) ) 
         retValue = LOCAL;
      else 
         retValue = SHADOW;

      /* Convert global coordinate to local co-ordinates of element the coord is in */
      elementType = FeMesh_GetElementType( self->feMesh, (*elementCoordInPtr) );

      ElementType_ConvertGlobalCoordToElLocal(
         elementType,
         self->feMesh,
         *elementCoordInPtr, 
         globalCoord,
         elLocalCoord );
   }
   return retValue;
}

void FeVariable_SetValueAtNode( void* feVariable, Node_DomainIndex dNode_I, double* componentValues ) {
   FeVariable* self = (FeVariable*)feVariable;
   Dof_Index   dofCountThisNode = 0;
   Dof_Index   nodeLocalDof_I = 0;

   dofCountThisNode = self->dofLayout->dofCounts[dNode_I];

   for( nodeLocalDof_I = 0; nodeLocalDof_I < dofCountThisNode; nodeLocalDof_I++ ) 
      DofLayout_SetValueDouble( (self)->dofLayout, dNode_I, nodeLocalDof_I, componentValues[nodeLocalDof_I] );
}

void FeVariable_PrintLocalDiscreteValues_2dBox( void* variable, Stream* stream ) {
   FeVariable*     self = (FeVariable*)variable;
   Node_LocalIndex node_lI=0;
   Index           x_I, y_I;
   Index           ii;
   Dof_Index       dof_I=0;
   Dof_Index       currNodeNumDofs=0;
   Index           nx = 0;
   Index           ny = 0;
   double          dx = 0;
   double          dy = 0;
   DofLayout*      dofLayout = self->dofLayout;
   Stream*         eStream = Journal_Register( Error_Type, (Name)self->type );
   Index           minLocalNodeX;
   Index           minLocalNodeY;
   Index           maxLocalNodeX;
   Index           maxLocalNodeY;
   Grid*           vertGrid;
   unsigned        inds[2];
   unsigned        vertInd;
   double*         verts[2];
   unsigned        *localOrigin, *localRange;
   double          min[2], max[2];

   if(self->feMesh->vertGridId == (unsigned)-1 || 
       Mesh_GetDimSize( self->feMesh ) != 2 ) {
      Journal_Printf( eStream,
         "Warning: %s called on variable \"%s\", but this isn't stored on a "
         "regular 2D mesh - so just returning.\n", __func__, self->name );
      return;
   }

   vertGrid = *(Grid**)ExtensionManager_Get( self->feMesh->info, self->feMesh, self->feMesh->vertGridId );

   localOrigin = (unsigned*)ExtensionManager_Get( self->feMesh->info, self->feMesh, self->feMesh->localOriginId );

   localRange = (unsigned*)ExtensionManager_Get( self->feMesh->info, self->feMesh, self->feMesh->localRangeId );

   memcpy( inds, localOrigin, Mesh_GetDimSize( self->feMesh ) * sizeof( unsigned) );

   insist( Mesh_GlobalToDomain( self->feMesh, MT_VERTEX, Grid_Project( vertGrid, inds ), &vertInd ), == True );

   verts[0] = Mesh_GetVertex( self->feMesh, vertInd );
   inds[0]++;
   inds[1]++;

   insist( Mesh_GlobalToDomain( self->feMesh, MT_VERTEX, Grid_Project( vertGrid, inds ), &vertInd ), == True );

   verts[1] = Mesh_GetVertex( self->feMesh, vertInd );
   
   nx = vertGrid->sizes[0];
   ny = vertGrid->sizes[1];
   dx = verts[1][0] - verts[0][0];
   dy = verts[1][1] - verts[0][1];

   minLocalNodeX = localOrigin[0];
   minLocalNodeY = localOrigin[1];
   maxLocalNodeX = minLocalNodeX + localRange[0] + 1;
   maxLocalNodeY = minLocalNodeY + localRange[1] + 1;

   Mesh_GetGlobalCoordRange( self->feMesh, min, max );

   Journal_Printf( stream, "display of Values in 2D box X:{%5.2f-%5.2f}, Y:{%5.2f-%5.2f}\n",
      min[I_AXIS], max[I_AXIS], min[J_AXIS], max[J_AXIS] );

   Journal_Printf( stream,
      "\twith %d elements in X(dx=%5.2f) and %d elements in Y(dy=%5.2f)\n\n",
      nx-1, dx, ny-1, dy );

   /*Header*/
   for( ii = 0; ii < 10; ii++ )
      Journal_Printf( stream, " " );

   for( x_I=0; x_I < nx; x_I++ ) 
      Journal_Printf( stream, "|  xNode=%3d   ", x_I );
   Journal_Printf( stream, "|\n", x_I );

   for( y_I = ny-1; y_I != (unsigned)-1; y_I-- ) {
      /*Blocks */
      for( ii = 0; ii < 10; ii++ )
         Journal_Printf( stream, " " );

      for( x_I = 0; x_I < nx; x_I++ ) {
         if(y_I == ny-1) 
            Journal_Printf( stream, "-" );
         else if(x_I == 0) 
            Journal_Printf( stream, "|" );
         else 
            Journal_Printf( stream, "*" );

         for(ii=0;ii<14;ii++)
            Journal_Printf( stream, "-" );
      }
      if(y_I == ny-1) 
         Journal_Printf( stream, "-\n" );
      else 
         Journal_Printf( stream, "|\n" );

      /* Now a row of y values */
      Journal_Printf( stream, "yNode=%3d |", y_I );
      for( x_I=0; x_I < nx; x_I++ ) {
         if( ( y_I >= minLocalNodeY ) && ( y_I < maxLocalNodeY )
            && ( x_I >= minLocalNodeX ) && ( x_I < maxLocalNodeX ) ) {

            inds[0] = x_I;
            inds[1] = y_I;
            node_lI = RegularMeshUtils_Node_3DTo1D( self->feMesh, inds );
            insist( Mesh_GlobalToDomain( self->feMesh, MT_VERTEX, node_lI, &node_lI ), == True );
            currNodeNumDofs = dofLayout->dofCounts[node_lI];

            if( currNodeNumDofs == 1 ) 
               Journal_Printf( stream, "   " );
            Journal_Printf( stream, "(" );
            for( dof_I=0; dof_I < currNodeNumDofs - 1 ; dof_I++ ) 
               Journal_Printf( stream, "%5.2f,", DofLayout_GetValueDouble( dofLayout, node_lI, dof_I ) );
            Journal_Printf( stream, "%5.2f )", DofLayout_GetValueDouble( dofLayout, node_lI, dof_I ) );
            
            if( currNodeNumDofs == 1 ) 
               Journal_Printf( stream, "   " );
            Journal_Printf( stream, "|" );
         }
         else {
            for( ii = 0; ii < 14; ii++)
               Journal_Printf( stream, "X" );
            Journal_Printf( stream, "|" );
         }
      }
      Journal_Printf( stream, "\n" );
   }
   
   /*Blocks */
   for(ii=0;ii<10;ii++)
      Journal_Printf( stream, " " );

   for( x_I=0; x_I < nx; x_I++ ) {
      Journal_Printf( stream, "-" );
      for(ii=0;ii<14;ii++) Journal_Printf( stream, "-" );
   }
   Journal_Printf( stream, "-\n", x_I );
}

InterpolationResult FeVariable_InterpolateDerivativesAt( void* variable, const double* globalCoord, double* value ) {
   FeVariable*         self = (FeVariable*)variable;
   Element_DomainIndex elementCoordIn = (unsigned)-1;
   Coord               elLocalCoord = {0,0,0};
   InterpolationResult retval = LOCAL;

   /* 
    * Need a special rule for points on this processor's boundary: instead of the normal
    * rule, "round" the point to lie inside the local space, rather than shadow.
    */
   
   /* 
    * Locate which mesh element given coord is in : use inclusive upper boundaries to save
    * the need to use shadow space if possible.
    */
   if( !Mesh_Algorithms_SearchElements( self->feMesh->algorithms, (double*)globalCoord, &elementCoordIn ) ) {
      /* If coord isn't inside domain elements list, bail out */
      return OUTSIDE_GLOBAL;
   }
   else /* We found the coord is within a local or shadow element */ {
      if( elementCoordIn >= FeMesh_GetElementLocalSize( self->feMesh ) )
        retval = SHADOW;  /* set to shadown in this case */
      /* convert global coordinate to local co-ordinates of element the coord is in */
      FeMesh_CoordGlobalToLocal( self->feMesh, elementCoordIn, globalCoord, elLocalCoord );

      /* Now interpolate the value at that coordinate, using shape functions */
      FeVariable_InterpolateDerivativesToElLocalCoord( self, elementCoordIn, elLocalCoord, value );
   }   
   return retval;
}

void FeVariable_InterpolateDerivativesToElLocalCoord(
   void*               _feVariable,
   Element_DomainIndex lElement_I,
   const Coord         elLocalCoord,
   double*             value )
{
   FeVariable*     self = (FeVariable*)_feVariable;
   ElementType*    elementType = FeMesh_GetElementType( self->feMesh, lElement_I );
   double**        GNx; 
   double          detJac;
   Dimension_Index dim = self->dim;

   GNx = self->GNx;

   /* Evaluate Global Shape Functions */
   ElementType_ShapeFunctionsGlobalDerivs( 
      elementType,
      self->feMesh,
      lElement_I,
      (double*)elLocalCoord,
      dim,
      &detJac,
      GNx );

   /* Do Interpolation */
   FeVariable_InterpolateDerivatives_WithGNx( self, lElement_I, GNx, value );
}

void FeVariable_InterpolateDerivatives_WithGNx(
   void*              _feVariable,
   Element_LocalIndex lElement_I,
   double**           GNx,
   double*            value )
{
   FeVariable*            self = (FeVariable*) _feVariable;
   Node_ElementLocalIndex elLocalNode_I;
   Node_LocalIndex        lNode_I;
   Dof_Index              dof_I;
   Dof_Index              dofCount;
   /* Variable*           dofVariable; */
   unsigned               nInc;
   int                    *inc;
   Dimension_Index        dim = self->dim;

   /* Gets number of degrees of freedom - assuming it is the same throughout the mesh */
   dofCount = self->dofLayout->dofCounts[0];

   /* Initialise */
   memset( value, 0, sizeof( double ) * dofCount * dim );

   FeMesh_GetElementNodes( self->feMesh, lElement_I, self->inc );
   nInc = IArray_GetSize( self->inc );
   inc = IArray_GetPtr( self->inc );

   /* get fevariable top data pointer */
   /* note that we now assume much simpler memory layouts */
   double* feData = Variable_GetPtrDouble( DofLayout_GetVariable( self->dofLayout, 0, 0 ), 0 );
   
   /* Interpolate derivative from nodes */
   for( elLocalNode_I = 0 ; elLocalNode_I < nInc ; elLocalNode_I++) {
      lNode_I = inc[ elLocalNode_I ];
    
      for( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
         double nodeValue = *(feData + lNode_I*dofCount + dof_I);
         value[dof_I*dim + 0] += GNx[0][elLocalNode_I] * nodeValue;
         value[dof_I*dim + 1] += GNx[1][elLocalNode_I] * nodeValue;

         if( dim == 3 ) 
            value[dof_I*dim + 2] += GNx[2][elLocalNode_I] * nodeValue;
      }
   }
}

void FeVariable_InterpolateValue_WithNi( void* _feVariable, Element_LocalIndex lElement_I, double* Ni, double* value ) {
   FeVariable*            self = (FeVariable*) _feVariable;
   Node_ElementLocalIndex elLocalNode_I;
   Node_LocalIndex        lNode_I;
   Dof_Index              dof_I;
   Dof_Index              dofCount;
   Variable*              dofVariable;
   double                 nodeValue;
   unsigned               nInc;
   int                    *inc;

   /* Gets number of degrees of freedom - assuming it is the same throughout the mesh */
   dofCount = self->dofLayout->dofCounts[0];

   /* Initialise */
   memset( value, 0, sizeof( double ) * dofCount );

   FeMesh_GetElementNodes( self->feMesh, lElement_I, self->inc );
   nInc = IArray_GetSize( self->inc );
   inc = IArray_GetPtr( self->inc );

   for( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
      /* Interpolate derivative from nodes */
      for( elLocalNode_I = 0 ; elLocalNode_I < nInc ; elLocalNode_I++) {
         lNode_I = inc[ elLocalNode_I ];
         dofVariable = DofLayout_GetVariable( self->dofLayout, lNode_I, dof_I );
         nodeValue = Variable_GetValueDouble( dofVariable, lNode_I );

         value[dof_I] += Ni[elLocalNode_I] * nodeValue;
      }
   }
}

void FeVariable_GetMinimumSeparation( void* feVariable, double* minSeparationPtr, double minSeparationEachDim[3] ) {
   FeVariable* self = (FeVariable*)feVariable;

   assert( self && Stg_CheckType( self, FeVariable ) );
   Mesh_GetMinimumSeparation( self->feMesh, minSeparationPtr, minSeparationEachDim );
}   

void _FeVariable_SyncShadowValues( void* feVariable ) {
   FeVariable* self = (FeVariable*)feVariable;
   DofLayout*  dofLayout;
   Sync*       vertSync;
   unsigned    var_i;

   assert( self );

   /* Shortcuts. */
   dofLayout = self->dofLayout;

   if( !dofLayout ) {
      self->shadowValuesSynchronised = True;
      return;
   }

   /* Create a distributed array based on the mesh's vertices. */
   vertSync = Mesh_GetSync( self->feMesh, MT_VERTEX );

   /*
    * For each variable in the dof layout, we need to create a distributed array and update
    * shadow values.
    */
   for( var_i = 0; var_i < dofLayout->_totalVarCount; var_i++ ) {
      unsigned  varInd;
      Variable* var;
      unsigned  field_i;

      /* Get the variable. */
      varInd = dofLayout->_varIndicesMapping[var_i];
      var = Variable_Register_GetByIndex( dofLayout->_variableRegister, varInd );

      /* Each field of the variable will need to be handled individually. */
      for( field_i = 0; field_i < var->offsetCount; field_i++ ) {
         unsigned offs, size;
         Stg_Byte *arrayStart, *arrayEnd;

         offs = var->offsets[field_i];
         size = var->dataSizes[field_i];

         arrayStart = (Stg_Byte*)var->arrayPtr + offs;
         arrayEnd = arrayStart + var->structSize * FeMesh_GetNodeLocalSize( self->feMesh );

         Sync_SyncArray(
            vertSync,
            arrayStart,
            var->structSize, 
            arrayEnd,
            var->structSize, 
            size );
      }
   }

   self->shadowValuesSynchronised = True;

}

void FeVariable_PrintDomainDiscreteValues( void* variable, Stream* stream ) {
   FeVariable* self = (FeVariable*)variable;

   Journal_Printf( stream, "In %s: for FeVariable \"%s\":\n", __func__, self->name );
   _FeVariable_PrintLocalOrDomainValues( variable, FeMesh_GetNodeDomainSize( self->feMesh ), stream );
}

void FeVariable_PrintCoordsAndValues( void* _feVariable, Stream* stream ) {
   FeVariable*     self = (FeVariable*) _feVariable;
   Node_LocalIndex node_I = 0;
   Node_LocalIndex nodeLocalCount = FeMesh_GetNodeLocalSize( self->feMesh );
   Dof_Index       currNodeNumDofs;
   Dof_Index       nodeLocalDof_I;
   Variable*       currVariable;
   double*         nodeCoord;
   
   /* Print Header of stream */
   Journal_Printf( stream, "# FeVariable - %s\n", self->name );
   Journal_Printf( stream, "#    x coord   |    y coord   |    z coord" );
   currNodeNumDofs = self->dofLayout->dofCounts[ 0 ];

   for( nodeLocalDof_I = 0; nodeLocalDof_I < currNodeNumDofs; nodeLocalDof_I++ ) {
      currVariable = DofLayout_GetVariable( self->dofLayout, node_I, nodeLocalDof_I );
      Journal_Printf( stream, "  |  %s", currVariable->name );
   }
   Journal_Printf( stream, "\n");
   
   /* Loop over local nodes */
   for( node_I=0; node_I < nodeLocalCount ; node_I++ ) {
      currNodeNumDofs = self->dofLayout->dofCounts[ node_I ];

      /* Get Coordinate of Node */
      nodeCoord = Mesh_GetVertex( self->feMesh, node_I );

      Journal_Printf(
         stream,
         "%12.6g   %12.6g   %12.6g   ", 
         nodeCoord[ I_AXIS ],
         nodeCoord[ J_AXIS ],
         nodeCoord[ K_AXIS ] );
      
      /* Print each dof */
      for( nodeLocalDof_I = 0; nodeLocalDof_I < currNodeNumDofs; nodeLocalDof_I++ ) {
         currVariable = DofLayout_GetVariable( self->dofLayout, node_I, nodeLocalDof_I );
         Journal_Printf( stream, "%12.6g   ", Variable_GetValueDouble( currVariable, node_I ) );
      }
      Journal_Printf( stream, "\n" );
   }   
}

#define MAX_ELEMENT_NODES 27
/* --- Private Functions --- */
void _FeVariable_InterpolateNodeValuesToElLocalCoord(
   void*               feVariable,
   Element_DomainIndex element_lI,
   const Coord       elLocalCoord,
   double*             value )
{
   FeVariable*            self = (FeVariable*) feVariable;
   ElementType*           elementType=NULL;
   Dof_Index              nodeLocalDof_I=0;
   Dof_Index              dofCountThisNode=0;
   Node_ElementLocalIndex elLocalNode_I=0;
   Node_LocalIndex        lNode_I=0;
   Variable*              currVariable=NULL;
   double                 dofValueAtCurrNode=0;
   unsigned               nInc;
   int                    *inc;
   double                 shapeFuncsEvaluated[MAX_ELEMENT_NODES];

   FeMesh_GetElementNodes( self->feMesh, element_lI, self->inc );
   nInc = IArray_GetSize( self->inc );
   inc = IArray_GetPtr( self->inc );

   /* Gets number of degrees of freedom - assuming it is the same throughout the mesh */
   dofCountThisNode = self->dofLayout->dofCounts[lNode_I];

   /* Evaluate shape function values of current element at elLocalCoords */
   elementType = FeMesh_GetElementType( self->feMesh, element_lI );
   ElementType_EvaluateShapeFunctionsAt( elementType, elLocalCoord, (double*)&shapeFuncsEvaluated );

   memset( value, 0, dofCountThisNode * sizeof(double) );

   /* Now for each node, add that node's contribution at point */
   for( elLocalNode_I=0; elLocalNode_I < nInc; elLocalNode_I++ ) {
      lNode_I = inc[elLocalNode_I];
      
      for( nodeLocalDof_I=0; nodeLocalDof_I < dofCountThisNode; nodeLocalDof_I++ ) {
         currVariable = DofLayout_GetVariable( self->dofLayout, lNode_I, nodeLocalDof_I );
         dofValueAtCurrNode = Variable_GetValueDouble( currVariable, lNode_I );
         value[nodeLocalDof_I] += dofValueAtCurrNode * shapeFuncsEvaluated[elLocalNode_I];
      }   
   }
}

void _FeVariable_PrintLocalOrDomainValues( void* variable, Index localOrDomainCount, Stream* stream ) {
   FeVariable*        self = (FeVariable*)variable;
   Node_Index         node_I = 0;
   Node_GlobalIndex   gNode_I = 0;
   Dof_Index          currNodeNumDofs;
   Dof_Index          nodeLocalDof_I;
   Dof_EquationNumber currEqNum;
   Variable*          currVariable;
   
   for( node_I=0; node_I < localOrDomainCount; node_I++ ) {
      gNode_I = FeMesh_NodeDomainToGlobal( self->feMesh, node_I );
      Journal_Printf( stream, "node %d(global index %d):\n", node_I, gNode_I );
      
      currNodeNumDofs = self->fieldComponentCount;
      
      /* Print each dof */
      for( nodeLocalDof_I = 0; nodeLocalDof_I < currNodeNumDofs; nodeLocalDof_I++ ) {
         currVariable = DofLayout_GetVariable( self->dofLayout, node_I, nodeLocalDof_I );

         Journal_Printf(
            stream,
            "\tdof %d \"%s\": %6g - ",
            nodeLocalDof_I,
            currVariable->name,
            Variable_GetValueDouble( currVariable, node_I ) );

         currEqNum = self->eqNum->destinationArray[node_I][nodeLocalDof_I];

         if( currEqNum == -1 ) 
            Journal_Printf( stream, "(from BC)", currEqNum );
         else 
            Journal_Printf( stream, "(eq num %d)", currEqNum );
         Journal_Printf( stream, "\n", currEqNum );
      }
   }
}

InterpolationResult FeVariable_InterpolateFromMeshLocalCoord(
   void*               feVariable,
   FeMesh*             mesh,
   Element_DomainIndex dElement_I,
   double*             localCoord,
   double*             value )
{
   FeVariable* self = (FeVariable*)feVariable;

   if( mesh == self->feMesh ) {
      /* 
       * If the meshes are identical - 
       * then we can just interpolate within the elements because the elements are the same.
       */
      FeVariable_InterpolateWithinElement( self, dElement_I, localCoord, value );
      return LOCAL;
   }
   else {
      Coord globalCoord;

      /* 
       * If the meshes are different - 
       * then we must find the global coordinates and interpolate to that.
       */
      FeMesh_CoordLocalToGlobal( mesh, dElement_I, localCoord, globalCoord );
      return FieldVariable_InterpolateValueAt( feVariable, globalCoord, value );
   }
}

/* 
 * TODO: can't assume all swarms have particles of type integrationPoint anymore.
 * should check that the given swarm does have I.P for the rest of these functions.
 */
double FeVariable_IntegrateElement_AxisIndependent( 
   void*               feVariable,
   void*               _swarm, 
   Element_DomainIndex dElement_I,
   Dimension_Index     dim, 
   Axis                axis0,
   Axis                axis1, 
   Axis                axis2 ) 
{
   FeVariable*          self = (FeVariable*)feVariable;
   Swarm*               swarm = (Swarm*)_swarm;
   FeMesh*              feMesh = self->feMesh;
   FeMesh*              mesh;
   ElementType*         elementType;
   Cell_LocalIndex      cell_I;
   Particle_InCellIndex cParticle_I;
   Particle_InCellIndex cellParticleCount;
   IntegrationPoint*    particle;
   double               detJac;
   double               integral;
   double               value;
   
   /* Initialise Summation of Integral */
   integral = 0.0;

   Journal_Firewall( self->fieldComponentCount == 1,
         NULL,
         "Error in func %s:\nThis function can only handle integrating scalar fields. But field '%s' has '%d' components, can you reduce the component count by using an OperatorFeVariable, eg, where Operator is 'TakeFirstComponent'\n\n", __func__, self->fieldComponentCount ); 

   /* Use feVariable's mesh as geometry mesh if one isn't passed in */
   if( Stg_Class_IsInstance( feMesh->algorithms, Mesh_CentroidAlgorithms_Type ) )
      mesh = (FeMesh*)((Mesh_CentroidAlgorithms*)feMesh->algorithms)->elMesh;
   else
      mesh = feMesh;

   elementType = FeMesh_GetElementType( mesh, dElement_I );

   /* Determine number of particles in element */
   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, dElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   /* Loop over all particles in element */
   for( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      /* Get Pointer to particle */
      particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );

      /* Interpolate Value of Field at Particle */
      FeVariable_InterpolateWithinElement( feVariable, dElement_I, particle->xi, &value );

      Journal_DPrintfL(
         self->debug,
         3,
         "%s: Integrating element %d - particle %d - Value = %g\n",
         self->name,
         dElement_I,
         cParticle_I,
         value );

      /* Calculate Determinant of Jacobian */
      detJac = ElementType_JacobianDeterminant_AxisIndependent( 
         elementType,
         mesh,
         dElement_I,
         particle->xi,
         dim,
         axis0,
         axis1,
         axis2 );

      /* Sum Integral */
      integral += detJac * particle->weight * value;
   }
   
   return integral;
}

double FeVariable_Integrate( void* feVariable, void* _swarm ) {
   FeVariable*        self = (FeVariable*)feVariable;
   Swarm*             swarm = (Swarm*)_swarm;
   FeMesh*            feMesh = self->feMesh;
   Element_LocalIndex lElement_I;
   Element_LocalIndex elementLocalCount = FeMesh_GetElementLocalSize( feMesh );
   double             integral, integralGlobal;
   
   /* Initialise Summation of Integral */
   integral = 0.0;

   /* Loop over all local elements */
   for( lElement_I = 0 ; lElement_I < elementLocalCount ; lElement_I++ ) {
      integral += FeVariable_IntegrateElement( self, swarm, lElement_I );

      Journal_DPrintfL(
         self->debug,
         3,
         "%s: Integrating element %d - Accumulated Integral = %g\n",
         self->name,
         lElement_I,
         integral );
   }
      
   /* Gather and sum integrals from other processors */
   (void)MPI_Allreduce( &integral, &integralGlobal, 1, MPI_DOUBLE, MPI_SUM, self->communicator );

   return integralGlobal;
}

double FeVariable_AverageTopLayer( void* feVariable, void* swarm, Axis layerAxis ) {
   FeVariable* self = (FeVariable*)feVariable;
   Grid*       elGrid;

   elGrid = *(Grid**)ExtensionManager_Get(
      self->feMesh->info,
      self->feMesh, 
      self->feMesh->elGridId);

   return FeVariable_AverageLayer( self, swarm, layerAxis, elGrid->sizes[1] - 1 );
}

double FeVariable_AverageBottomLayer( void* feVariable, void* swarm, Axis layerAxis ) {
   FeVariable* self = (FeVariable*)feVariable;

   return FeVariable_AverageLayer( self, swarm, layerAxis, 0 );
}

double FeVariable_AverageLayer( void* feVariable, void* swarm, Axis layerAxis, Index layerIndex ) {
   FeVariable*     self = (FeVariable*)feVariable;
   Axis            aAxis = ( layerAxis == I_AXIS ? J_AXIS : I_AXIS );
   Axis            bAxis = ( layerAxis == K_AXIS ? J_AXIS : K_AXIS );
   Dimension_Index dim = self->dim;
   double          integral;
   double          layerThickness = 0.0;
   double          sendThickness;
   Grid*           vertGrid;
   unsigned*       inds;
   double          heights[2];
   unsigned        localInd[2], globalInd[2];
   double          *min, *max;
   unsigned        d_i;
   
   integral = FeVariable_IntegrateLayer( self, swarm, layerAxis, layerIndex );

   /* Calculate layer thickness.  This assumes the mesh is regular. */
   vertGrid = *(Grid**)ExtensionManager_Get(
      self->feMesh->info,
      self->feMesh, 
self->feMesh->vertGridId );

   inds = Memory_Alloc_Array_Unnamed( unsigned, Mesh_GetDimSize( self->feMesh ) );

   for( d_i = 0; d_i < Mesh_GetDimSize( self->feMesh ); d_i++ ) {
      if( d_i != layerAxis )
         inds[d_i] = 0;
      else
         inds[d_i] = layerIndex;
   }

   globalInd[0] = Grid_Project( vertGrid, inds );
   inds[layerAxis]++;
   globalInd[1] = Grid_Project( vertGrid, inds );

   if( Mesh_GlobalToDomain( self->feMesh, MT_VERTEX, globalInd[0], &localInd[0] ) && 
       Mesh_GlobalToDomain( self->feMesh, MT_VERTEX, globalInd[1], &localInd[1] ) ) {
      heights[0] = Mesh_GetVertex( self->feMesh, localInd[0] )[layerAxis];
      heights[1] = Mesh_GetVertex( self->feMesh, localInd[1] )[layerAxis];
      sendThickness = heights[1] - heights[0];
   }
   else 
      sendThickness = 0.0;

   (void)MPI_Allreduce( &sendThickness, &layerThickness, 1, MPI_DOUBLE, MPI_MAX, self->communicator );
   FreeArray( inds );

   min = Memory_Alloc_Array_Unnamed( double, Mesh_GetDimSize( self->feMesh ) );
   max = Memory_Alloc_Array_Unnamed( double, Mesh_GetDimSize( self->feMesh ) );
   Mesh_GetGlobalCoordRange( self->feMesh, min, max );
   integral /= layerThickness * (max[aAxis] - min[aAxis]);

   if( dim == 3 )
      integral /= max[ bAxis ] - min[ bAxis ];
      
   FreeArray( min );
   FreeArray( max );

   return integral;
}

double FeVariable_IntegrateLayer_AxisIndependent( 
   void*           feVariable,
   void*           _swarm,
   Axis            layerAxis,
   Index           layerIndex,
   Dimension_Index dim, 
   Axis            axis0,
   Axis            axis1,
   Axis            axis2 ) 
{ 
   FeVariable*         self = (FeVariable*)feVariable;
   Swarm*              swarm = (Swarm*)_swarm;
   Element_LocalIndex  lElement_I;
   Element_GlobalIndex gElement_I;
   IJK                 elementIJK;
   double              elementIntegral;
   double              integral;
   double              integralGlobal;

   Journal_DPrintf( self->debug, "In %s() for FeVariable \"%s\":\n", __func__, self->name );

   /* Initialise Sumation of Integral */
   integral = 0.0;

   Stream_Indent( self->debug );

   for( gElement_I = 0 ; gElement_I < FeMesh_GetElementGlobalSize( self->feMesh ); gElement_I++ ) {
      RegularMeshUtils_Element_1DTo3D( self->feMesh, gElement_I, elementIJK );

      /* Check if element is in layer plane */
      if( elementIJK[ layerAxis ] != layerIndex )
         continue;

      /* Check if element is local */
      if( !FeMesh_ElementGlobalToDomain( self->feMesh, gElement_I, &lElement_I ) || 
          lElement_I >= FeMesh_GetElementLocalSize( self->feMesh ) )
         continue;

      elementIntegral = FeVariable_IntegrateElement_AxisIndependent(
         self,
         swarm,
         lElement_I,
         dim,
         axis0,
         axis1,
         axis2 );

      Journal_DPrintfL( self->debug, 2, "Integral of element %d was %f\n", lElement_I, elementIntegral );
      integral += elementIntegral;
   }
   Stream_UnIndent( self->debug );

   /* Gather and sum integrals from other processors */
   (void)MPI_Allreduce( &integral, &integralGlobal, 1, MPI_DOUBLE, MPI_SUM, self->communicator );

   Journal_DPrintf(
      self->debug,
      "Calculated global integral of layer %d in Axis %d was %f\n",
      layerIndex,
      layerAxis,
      integralGlobal );

   return integralGlobal;
}


double FeVariable_AveragePlane( void* feVariable, Axis planeAxis, double planeHeight ) {
   FeVariable*     self = (FeVariable*)feVariable;
   double          integral;
   Axis            aAxis = ( planeAxis == I_AXIS ? J_AXIS : I_AXIS );
   Axis            bAxis = ( planeAxis == K_AXIS ? J_AXIS : K_AXIS );
   Dimension_Index dim = self->dim;
   double          min[3], max[3];
   
   integral = FeVariable_IntegratePlane( self, planeAxis, planeHeight );

   Mesh_GetGlobalCoordRange( self->feMesh, min, max );

   integral /= max[ aAxis ] - min[ aAxis ];

   if( dim == 3 )
      integral /= max[ bAxis ] - min[ bAxis ];

   return integral;
}

double FeVariable_IntegratePlane( void* feVariable, Axis planeAxis, double planeHeight ) {
   FeVariable*                self = (FeVariable*)feVariable;
   IJK                        planeIJK;
   Element_LocalIndex         lElement_I;
   Element_GlobalIndex        gElement_I;
   Element_LocalIndex         elementLocalCount = FeMesh_GetElementLocalSize( self->feMesh );
   Axis                       aAxis = ( planeAxis == I_AXIS ? J_AXIS : I_AXIS );
   Axis                       bAxis = ( planeAxis == K_AXIS ? J_AXIS : K_AXIS );
   double                     integral;
   /* Swarm Stuff */
   Swarm*                     tmpSwarm;
   Bool                       dimExists[] = { False, False, False };
   ExtensionManager_Register* extensionMgr_Register;
   SingleCellLayout*          singleCellLayout;
   GaussParticleLayout*       gaussParticleLayout;
   Particle_Index             lParticle_I;
   IntegrationPoint*          particle;
   /* Plane location stuff */
   double                     storedXi_J_AXIS;
   Coord                      planeCoord;
   double                     planeXi = -1;
   double                     planeXiGlobal;
   Index                      planeLayer = 0;
   Index                      planeLayerGlobal;
   Particle_InCellIndex       particlesPerDim[] = {2,2,2};

   /* Find Elements which plane cuts through */
   memcpy( planeCoord, Mesh_GetVertex( self->feMesh, 0 ), sizeof( Coord ) );
   planeCoord[ planeAxis ] = planeHeight;

   if( Mesh_Algorithms_SearchElements( self->feMesh->algorithms, planeCoord, &lElement_I ) && 
       lElement_I < elementLocalCount )
   {
      Coord planeXiCoord;

      gElement_I = FeMesh_ElementDomainToGlobal( self->feMesh, lElement_I );
      RegularMeshUtils_Element_1DTo3D( self->feMesh, gElement_I, planeIJK );
      planeLayer = planeIJK[ planeAxis ];
      
      /* Find Local Coordinate of plane */
      FeMesh_CoordGlobalToLocal( self->feMesh, lElement_I, planeCoord, planeXiCoord );
      planeXi = planeXiCoord[ planeAxis ];
   }
   
   /* Should be broadcast */
   (void)MPI_Allreduce( &planeXi, &planeXiGlobal, 1, MPI_DOUBLE, MPI_MAX, self->communicator );
   (void)MPI_Allreduce( &planeLayer, &planeLayerGlobal, 1, MPI_UNSIGNED, MPI_MAX, self->communicator );

   /* Create Swarm in plane */
   extensionMgr_Register = ExtensionManager_Register_New();
   dimExists[ aAxis ] = True;

   if(self->dim == 3)
      dimExists[ bAxis ] = True;
   
   singleCellLayout = SingleCellLayout_New(
      "cellLayout",
      (AbstractContext*)self->context,
      dimExists,
      NULL,
      NULL );

   particlesPerDim[ planeAxis ] = 1;

   gaussParticleLayout = GaussParticleLayout_New(
      "particleLayout",
      NULL,
      LocalCoordSystem,
      True,
      self->dim - 1,
      particlesPerDim );

   tmpSwarm = Swarm_New( 
      "tmpgaussSwarm", NULL,
      singleCellLayout, 
      gaussParticleLayout,
      self->dim,
      sizeof( IntegrationPoint), 
      extensionMgr_Register, 
      NULL,
      self->communicator,
      NULL );

   Stg_Component_Build( tmpSwarm, NULL, False );

   /* Change Positions of the particles */
   Stg_Component_Initialise( tmpSwarm, NULL, False );

   for( lParticle_I = 0; lParticle_I < tmpSwarm->particleLocalCount; lParticle_I++ ) {
      particle = (IntegrationPoint*) Swarm_ParticleAt( tmpSwarm, lParticle_I );
      storedXi_J_AXIS = particle->xi[ J_AXIS ];
      particle->xi[ aAxis ] = particle->xi[ I_AXIS ];
      particle->xi[ bAxis ] = storedXi_J_AXIS;
      particle->xi[ planeAxis ] = planeXiGlobal;
   }
   
   integral = FeVariable_IntegrateLayer_AxisIndependent(
      self,
      tmpSwarm,
      planeAxis,
      planeLayerGlobal, 
      self->dim - 1,
      aAxis,
      bAxis,
      planeAxis );

   /* Delete */
   Stg_Class_Delete( tmpSwarm );
   Stg_Class_Delete( gaussParticleLayout );
   Stg_Class_Delete( singleCellLayout );
   Stg_Class_Delete( extensionMgr_Register );
   
   return integral;
}


void FeVariable_ImportExportInfo_Delete( void* ptr ) {
   /* Nothing to do - the ObjectAdaptor will take care of deleting the actual struct itself */
}

void FeVariable_SaveToFile( void* feVariable, const char* filename ) {
#ifdef WRITE_HDF5
   FeVariable*       self = (FeVariable*)feVariable;
   Node_LocalIndex   lNode_I;
   Dof_Index         dofAtEachNodeCount;
   MPI_Comm          comm = Comm_GetMPIComm( Mesh_GetCommTopology( self->feMesh, MT_VERTEX ) );
   int               myRank;
   int               nProcs;
   MPI_Status        status;
   const int         FINISHED_WRITING_TAG = 100;
   int               confirmation = 0;
   hid_t             file, fileSpace, fileData;
   hid_t             memSpace;
   hsize_t           start[2], count[2], size[2];
   
   lNode_I = 0;

   MPI_Comm_size( comm, (int*)&nProcs );
   MPI_Comm_rank( comm, (int*)&myRank );
   
   /* Note: assumes same number of dofs at each node */
   dofAtEachNodeCount = self->fieldComponentCount;
   /** wait for go-ahead from process ranked lower than me, to avoid competition writing to file.  add barrier for good measure */
   MPI_Barrier( comm );
   if( myRank != 0 ) {
      (void)MPI_Recv( &confirmation, 1, MPI_INT, myRank - 1, FINISHED_WRITING_TAG, comm, &status );
   }   

   /* Open the file */
   if( myRank == 0 ) {
      /* Open the HDF5 output file. */
      file = H5Fcreate( filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );

      Journal_Firewall(
         file >= 0,
         NULL,
         "Error in %s for %s '%s' - Cannot create file %s.\n", 
         __func__,
         self->type,
         self->name,
         filename );

      size[0] = Mesh_GetGlobalSize( self->feMesh, 0 );;
      size[1] = dofAtEachNodeCount;

      /* Create filespace */
      fileSpace = H5Screate_simple( 2, size, NULL );         
      #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
      fileData = H5Dcreate( file, "/data", H5T_NATIVE_DOUBLE, fileSpace, H5P_DEFAULT );
      #else
      fileData = H5Dcreate(
         file,
         "/data",
         H5T_NATIVE_DOUBLE,
         fileSpace,
         H5P_DEFAULT,
         H5P_DEFAULT,
         H5P_DEFAULT );
      #endif
   }
   else {
      /* Open the HDF5 output file. */
      file = H5Fopen( filename, H5F_ACC_RDWR, H5P_DEFAULT );

      Journal_Firewall(
         file >= 0, NULL,
         "Error in %s for %s '%s' - Cannot open file %s.\n", 
         __func__,
         self->type,
         self->name,
         filename );

      /* Get the filespace */   
      #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
      fileData = H5Dopen( file, "/data" );
      #else
      fileData = H5Dopen( file, "/data", H5P_DEFAULT );
      #endif
      /* Get the filespace handle */
      fileSpace = H5Dget_space( fileData );
   }   
   
   /* 
    * Get the section of fileSpace to write to... set start point to be the
    * global index of first local node.
    */
   count[0] = 1;
   count[1] = dofAtEachNodeCount;

   /* Create memSpace */
   memSpace = H5Screate_simple( 2, count, NULL );
   H5Sselect_all( memSpace );
   
   for( lNode_I = 0; lNode_I < FeMesh_GetNodeLocalSize( self->feMesh ); lNode_I++ ) {
      
      /* Add field value at current node to array */
      FeVariable_GetValueAtNode( self, lNode_I, self->tempData );

      /* select the region of dataspace to write to  */
      start[0] = FeMesh_NodeDomainToGlobal( self->feMesh, lNode_I );
      start[1] = 0;
      H5Sselect_hyperslab( fileSpace, H5S_SELECT_SET, start, NULL, count, NULL );
      /* Write the array to file */
      H5Dwrite( fileData, H5T_NATIVE_DOUBLE, memSpace, fileSpace, H5P_DEFAULT, self->tempData );
   }   
   
   /* Close all handles */
   H5Dclose( fileData );
   H5Sclose( memSpace );
   H5Sclose( fileSpace );
   H5Fclose( file );
   
   /* Send go-ahead to process ranked above me, to avoid competition writing to file */
   if( myRank != nProcs - 1 ) {
      MPI_Ssend( &confirmation, 1, MPI_INT, myRank + 1, FINISHED_WRITING_TAG, comm );
   }   

#endif
}

void FeVariable_ReadFromFile( void* feVariable, const char* filename ) {
#ifdef READ_HDF5
   FeVariable*             self = (FeVariable*)feVariable;
   Node_LocalIndex         lNode_I = 0;
   Node_GlobalIndex        gNode_I = 0;
   Dof_Index               dof_I;
   double                  variableVal;
   hid_t                   file, fileSpace, fileData, error;
   unsigned                totalNodes, ii;
   hid_t                   memSpace;
   hsize_t                 start[2], count[2], size[2], maxSize[2];
   double*                 buf; 
   hid_t                   group_id;
    
   /* Open the file and data set. */
   file = H5Fopen( filename, H5F_ACC_RDONLY, H5P_DEFAULT );
   
   Journal_Firewall(
      file >= 0,
      NULL,
      "Error in %s for %s '%s' - Cannot open file %s.\n", 
      __func__,
      self->type,
      self->name,
      filename );

   #if H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8 || H5Gopen_vers == 1
      group_id  = H5Gopen( file, "/");
   #else
      group_id  = H5Gopen( file, "/", H5P_DEFAULT );
   #endif

   /* Prepare to read vertices from file */
   #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
   fileData = H5Dopen( file, "/data" );
   #else
   fileData = H5Dopen( file, "/data", H5P_DEFAULT );
   #endif
   fileSpace = H5Dget_space( fileData );
   
   H5Sget_simple_extent_dims( fileSpace, size, maxSize );

   if( self->fieldComponentCount != maxSize[1] ){
       /* Close all handles */
       H5Dclose( fileData );
       H5Sclose( fileSpace );
       H5Gclose( group_id );
       H5Fclose( file );

       Journal_Firewall(
          (self->fieldComponentCount == maxSize[1]),
          NULL,
          "\n\nError in %s for %s '%s'\n"
          "Data size (%u) within file (%s) does not correspond to fevariable size (%u).\n",
          __func__,
          self->type,
          self->name,
          (unsigned int) maxSize[1],
          filename,
          self->fieldComponentCount );
      
   }
   

   totalNodes = Mesh_GetGlobalSize( self->feMesh, 0 );
   
   if ( maxSize[0] != totalNodes ) {
       /* Close all handles */
       H5Dclose( fileData );
       H5Sclose( fileSpace );
       H5Gclose( group_id );
       H5Fclose( file );

       Journal_Firewall(
          (maxSize[0] == totalNodes),
          NULL,
          "\n\nError in %s for %s '%s'\n"
          "Number of nodes (%u) stored in file (%s) does not correspond to fevariable node count (%u).\n",
          __func__,
          self->type,
          self->name,
          (unsigned int)maxSize[0],
          filename,
          totalNodes );
   }

   start[1] = 0;
   count[0] = 1;
   count[1] = maxSize[1];
    
   memSpace = H5Screate_simple( 2, count, NULL );
   buf = Memory_Alloc_Array( double, count[1], "fileBuffer" );

   /* Read from HDF5 checkpint file */
   for( ii=0; ii<totalNodes; ii++ ) {   
      start[0] = ii;
               
      H5Sselect_hyperslab( fileSpace, H5S_SELECT_SET, start, NULL, count, NULL );
      H5Sselect_all( memSpace );
         
      error = H5Dread( fileData, H5T_NATIVE_DOUBLE, memSpace, fileSpace, H5P_DEFAULT, buf );
      gNode_I = ii;

      if (error < 0) {
          /* Close all handles */
          H5Dclose( fileData );
          H5Sclose( fileSpace );
          H5Gclose( group_id );
          H5Fclose( file );
          Journal_Firewall(
             error >= 0,
             NULL,
             "\n\nError in %s for %s '%s' - Cannot read data in %s.\n", 
             __func__,
             self->type,
             self->name,
             filename );
      }
      
      if( Mesh_GlobalToDomain( self->feMesh, MT_VERTEX, gNode_I, &lNode_I ) && 
         lNode_I < Mesh_GetLocalSize( self->feMesh, MT_VERTEX ) )
      {
        for( dof_I = 0; dof_I < self->fieldComponentCount; dof_I++ ) {
           variableVal = buf[dof_I];
           DofLayout_SetValueDouble( self->dofLayout, lNode_I, dof_I, variableVal );
        }  
      }
   }   
   Memory_Free( buf );

   /* Close all handles */
   H5Dclose( fileData );
   H5Sclose( memSpace );
   H5Sclose( fileSpace );
   H5Gclose( group_id );
   H5Fclose( file );
   

   Mesh_DeformationUpdate( self->feMesh );

   /* Sync shadow values now, as all procs should have required input */
   FeVariable_SyncShadowValues( self );
#endif
}

FeVariable* FeVariable_ReadFromFileWithMesh( void* templateFeVariable, const char* feVarFilename, const char* meshFilename, Bool partitioned ) {
   FeVariable*         templateSelf = (FeVariable*)templateFeVariable;
   Stream*             errorStr = Journal_Register( Error_Type, (Name)templateSelf->type );   
#ifdef READ_HDF5
   CartesianGenerator* gen;
   C0Generator*        C0gen;
   FeMesh              *feMesh, *C0feMesh, *elementMesh;
   DofLayout*          dofs;
   Variable_Register*  varReg;
   Variable*           var;
   FeVariable*         feVar;
   hid_t               file, fileData;
   hid_t               attrib_id, group_id;
   herr_t              status;
   int                 res[3];
   int                 checkVer;
   int                 ndims;
   double              crdMin[3], crdMax[3];
   unsigned            nDomainVerts; 
   double*             arrayPtr;
   Name                varName[9];
   char                *newName=NULL;
      
   /* Open the file and data set. */
   file = H5Fopen( meshFilename, H5F_ACC_RDONLY, H5P_DEFAULT );
   
   Journal_Firewall(
      file >= 0,
      errorStr,
      "Error in %s for %s '%s' - Cannot open file %s.\n",
      __func__,
      templateSelf->type,
      templateSelf->name, meshFilename );

   /* get the file attributes to sanity and version checks */
   #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Gopen_vers == 1
      group_id  = H5Gopen( file, "/");
      attrib_id = H5Aopen_name( group_id, "checkpoint file version");
   #else
      group_id  = H5Gopen( file, "/", H5P_DEFAULT );
      attrib_id = H5Aopen( group_id, "checkpoint file version", H5P_DEFAULT );
   #endif
   /* if this attribute does not exist(attrib_id < 0) then we assume MeshCHECKPOINT_V1 which is not supported  */
   if( attrib_id < 0) {
      Journal_Firewall(
         0,
         errorStr,
         "\nError in %s for %s '%s' \n\n Interpolation restart not supported for Version 1 Checkpoint files \n\n",
         __func__,
         templateSelf->type,
         templateSelf->name );
   }

   /* check for known checkpointing version type */

   status = H5Aread( attrib_id, H5T_NATIVE_INT, &checkVer );
   H5Aclose( attrib_id );

   if( checkVer != 2) {
      Journal_Firewall(
         (0),
         errorStr,
         "\n\nError in %s for %s '%s'\n" "Unknown checkpoint version(%u) for checkpoint file(%s).\n",
         __func__,
         templateSelf->type, templateSelf->name, (unsigned int) checkVer, meshFilename );
   }

   /* check for correct number of dimensions */

   #if H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8
      attrib_id = H5Aopen_name( group_id, "dimensions");
   #else
      attrib_id = H5Aopen( group_id, "dimensions", H5P_DEFAULT );
   #endif
   status = H5Aread( attrib_id, H5T_NATIVE_INT, &ndims );
   H5Aclose( attrib_id );

   Journal_Firewall(
      (ndims == templateSelf->dim),
      errorStr,
      "\n\nError in %s for %s '%s'\n"
      "Number of dimensions(%u) for checkpoint file(%s) does not correspond to simulation dimensions(%u).\n", 
      __func__,
      templateSelf->type, templateSelf->name, (unsigned int) ndims, meshFilename,
      templateSelf->dim );

   /* check for correct mesh size if expecting a cartesian mesh*/
   
   #if H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8
      attrib_id = H5Aopen_name( group_id, "mesh resolution");
   #else
      attrib_id = H5Aopen( group_id, "mesh resolution", H5P_DEFAULT );
   #endif
   status = H5Aread( attrib_id, H5T_NATIVE_INT, &res );
   H5Aclose( attrib_id );

   /* Read in minimum coord. */
   #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
      fileData = H5Dopen( file, "/min" );
   #else
      fileData = H5Dopen( file, "/min", H5P_DEFAULT );
   #endif
   H5Dread( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &crdMin );
   H5Dclose( fileData );
      
   /* Read in maximum coord. */
   #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
      fileData = H5Dopen( file, "/max" );
   #else
      fileData = H5Dopen( file, "/max", H5P_DEFAULT );
   #endif
   H5Dread( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &crdMax );
   H5Dclose( fileData );

   /* Close all handles */
   H5Gclose( group_id );
   H5Fclose( file );

   /* Create a mesh generator always Cartesian even for a C0 mesh. 
      *****************************************
      This algorithm needs refactoring BIG time
      *****************************************
    */
   Stg_asprintf( &newName, "new-%s",  templateSelf->feMesh->generator->type );
   if (!Stg_Class_IsInstance(templateSelf->feMesh->generator, CartesianGenerator_Type) ) {
      gen=CartesianGenerator_New( StG_Strdup(newName), (AbstractContext*)templateSelf->context );
      if( partitioned == False ) MeshGenerator_SetMPIComm( gen, MPI_COMM_SELF ); // Jules hack 
   }
   else { 
      gen = templateSelf->feMesh->generator->_defaultConstructor( StG_Strdup(newName) );
      // initialise gen
      _MeshGenerator_Init( (MeshGenerator*)gen, (AbstractContext*)templateSelf->context );
      _CartesianGenerator_Init( gen );
      if( partitioned == False ) MeshGenerator_SetMPIComm( gen, MPI_COMM_SELF ); // Jules hack 
      free( newName );
   }

   /* Use the feVariable dimension size for mesh generator */
   gen->setDimSizeFunc( gen, templateSelf->dim );

   /* Use the element size read in from the checkpoint file for the new mesh generator */
   gen->setTopologyParamsFunc( gen, (unsigned*)res, 0, NULL, NULL );

   /* 
    * Use the feVariable's mesh's generator's crdMin and crdMax 
    * (which have been previously read in from checkpointed mesh file.
    */
   CartesianGenerator_SetGeometryParams( gen, (double*)&crdMin, (double*)&crdMax );

   /* 
    * Set it so that the generator does not read in the mesh from a file - we will 
    * explicitly do this after we build the feMesh using the provided mesh checkpoint file.
    */
   gen->readFromFile = False;
   /* create a feMesh */
   Stg_asprintf( &newName, "new-%s",  templateSelf->feMesh->type );
   feMesh = FeMesh_New( StG_Strdup(newName) );
   free( newName );
   /* set feMesh to use generator we just created  */
   Mesh_SetGenerator( feMesh, gen );

   /* 
    * Set the feMesh family to be the same as that of the feVariable we are initialising/interpolating ie constant, linear, etc 
    * unless we are initialising a constant mesh, in which case we set the element family to linear, and the mesh will be used 
    * as the elementMesh for the C0 generator.
    */
   if(Stg_Class_IsInstance(templateSelf->feMesh->generator, CartesianGenerator_Type)) {
      FeMesh_SetElementFamily( feMesh, templateSelf->feMesh->feElFamily );
      /* set periodicity according to current simulation */
      gen->periodic[0] = ((CartesianGenerator*)templateSelf->feMesh->generator)->periodic[0];
      gen->periodic[1] = ((CartesianGenerator*)templateSelf->feMesh->generator)->periodic[1];
      gen->periodic[2] = ((CartesianGenerator*)templateSelf->feMesh->generator)->periodic[2];
   }
   else if(Stg_Class_IsInstance(templateSelf->feMesh->generator, C0Generator_Type)) {
      FeMesh_SetElementFamily( feMesh, "linear" );
      /* set periodicity according to current simulation */
      gen->periodic[0] = ((CartesianGenerator*)((C0Generator*)templateSelf->feMesh->generator)->elMesh)->periodic[0];
      gen->periodic[1] = ((CartesianGenerator*)((C0Generator*)templateSelf->feMesh->generator)->elMesh)->periodic[1];
      gen->periodic[2] = ((CartesianGenerator*)((C0Generator*)templateSelf->feMesh->generator)->elMesh)->periodic[2];
   }
   else {
      FeMesh_SetElementFamily( feMesh, templateSelf->feMesh->feElFamily );
      gen->periodic[0] = False; 
      gen->periodic[1] = False;
      gen->periodic[2] = False;

      Journal_Printf(
         errorStr,
         "\n\n Warning - Mesh generator not recognised.. assuming no periodicity \n\n" );
   }

   /* Now build the mesh, then read in the required coordinates from the given file */
   Stg_Component_Build( feMesh, NULL, False );
   CartesianGenerator_ReadFromHDF5( gen, (Mesh*) feMesh, meshFilename );

   /* Where we are dealing with a constant mesh feVariable, we have to build a new C0 mesh */ 
   if(Stg_Class_IsInstance(templateSelf->feMesh->generator, C0Generator_Type)) {
      elementMesh = feMesh;
      /* create the C0 generator */
      C0gen = C0Generator_New( "", (AbstractContext*)templateSelf->context );
      /* set it's element mesh to the feMesh create just above */
      C0Generator_SetElementMesh( C0gen, (void*) elementMesh );
      /* create a new feMesh */
      C0feMesh = FeMesh_New( "" );
      /* set feMesh to use generator we just created, and set its type to constant mesh  */
      Mesh_SetGenerator( C0feMesh, C0gen );
      FeMesh_SetElementFamily( C0feMesh, templateSelf->feMesh->feElFamily );
      /* now build the mesh, which will generate the mesh using the provided generator */
      Stg_Component_Build( C0feMesh, NULL, False );
      /* reset the feMesh pointer to point to this C0 mesh */
      feMesh = C0feMesh;
   }
   Stg_Component_Initialise( feMesh, NULL, False );
   /* 
    * Set the mesh algorithms to be the same as those used for template feVariable 
    * do not delete old algorithms lest we delete the feMesh etc... need ref counters to get around this problem.
    */
   if(templateSelf->feMesh->algorithms->type != feMesh->algorithms->type){
      feMesh->algorithms = templateSelf->feMesh->algorithms;
      Mesh_Algorithms_SetMesh( feMesh->algorithms, feMesh );
   }

   /* get the number of mesh vertices stored locally */   
   nDomainVerts = Mesh_GetDomainSize( feMesh, MT_VERTEX );   

   varReg = Variable_Register_New();

   if(templateSelf->fieldComponentCount == 1) {
      var = Variable_NewScalar(
         "interpolation_temp_scalar",
         (AbstractContext*)templateSelf->context,
         Variable_DataType_Double,
         (Index*)&nDomainVerts,
         NULL,
         (void**)&arrayPtr,
         varReg );
   }
   else {
      unsigned var_I;

      for( var_I = 0; var_I < templateSelf->fieldComponentCount; var_I++ )
         Stg_asprintf( &varName[var_I], "%s-loaded-Component-%d", templateSelf->name, var_I );

      var = Variable_NewVector(
         "interpolation_temp_vector", 
         (AbstractContext*)templateSelf->context,
         Variable_DataType_Double,
         templateSelf->fieldComponentCount,
         &nDomainVerts,
         NULL,
         (void**)&arrayPtr,
         varReg, 
         varName[0], varName[1], varName[2],
         varName[3], varName[4], varName[5],
         varName[6], varName[7], varName[8] );
   }
   Variable_Register_Add( varReg, var );
   var->allocateSelf = True;   
   Stg_Component_Build( var, NULL, False );

   dofs = DofLayout_New( "interpolation_temp_dof", templateSelf->context, varReg, nDomainVerts, feMesh );

   if( templateSelf->fieldComponentCount == 1 )
      DofLayout_AddAllFromVariableArray( dofs, 1, &var );
   else {
      unsigned  var_I, node_I;
      Variable* variable;

      for( var_I = 0; var_I < templateSelf->fieldComponentCount; var_I++ ) {
         variable = Variable_Register_GetByName( varReg, varName[var_I] );
         variable->arrayPtrPtr = &var->arrayPtr;

         for( node_I = 0; node_I < nDomainVerts; node_I++ )
            DofLayout_AddDof_ByVarName( dofs, varName[var_I], node_I );

         Memory_Free( varName[var_I] );
      }
   }
   Stg_Component_Build( dofs, NULL, False );
   Stg_Component_Initialise( dofs, NULL, False );

   feVar = FeVariable_New(
      "interpolation_temp_fevar", 
      templateSelf->context,
      feMesh, 
      dofs, 
      NULL, 
      NULL, 
      NULL, 
      templateSelf->fieldComponentCount, 
      False, 
      True, 
      False, 
      NULL );

   Stg_Component_Build( feVar, templateSelf->context, False );

   /* not sure why these aren't being set correctly, so a little(tri/ha)ckery */
   feVar->fieldComponentCount = templateSelf->fieldComponentCount;
   feVar->dim = templateSelf->dim;
   FeVariable_ReadFromFile( feVar, feVarFilename );
   FeVariable_SyncShadowValues( feVar );

#if 0 
   /* clean up things that shouldn't be required any longer */
   Stg_Class_Delete( varReg );
 
   if(!strcmp( templateSelf->feMesh->generator->type, C0Generator_Type)) {
      Stg_Component_Destroy( C0gen , NULL, False );
      Stg_Component_Destroy( elementMesh, NULL, False );
   }
   Stg_Component_Destroy( gen, NULL, False );
#endif

   return feVar;
#else
   Journal_Firewall(
      !templateSelf->context->interpolateRestart, 
      errorStr,
      "\n\n FeVariable_ReadFromFileWithMesh function not supported for ASCII checkpoint files \n\n" );
#endif   
}

void FeVariable_InterpolateFromFile( void* feVariable, const char* feVarFilename, const char* meshFilename ) {
   FeVariable* self = (FeVariable*)feVariable;
   FeVariable* feVar;   
#ifdef READ_HDF5
   unsigned    totalNodes, ii;
   double*     value;

   /* load old resolution fevariable, final arg = False sets field to be non partioned (i.e, fully stored on each proc) */
   feVar = FeVariable_ReadFromFileWithMesh( self, feVarFilename, meshFilename, False );
   
   totalNodes = Mesh_GetLocalSize( self->feMesh, MT_VERTEX );
   value      = Memory_Alloc_Array( double, self->fieldComponentCount, "interValue" );
         
   /* step through nodes, interpolating the required values from our newly created feVariable */
   for( ii=0; ii<totalNodes; ii++ ) {
      _FeVariable_InterpolateValueAt( feVar, Mesh_GetVertex( self->feMesh, ii ), value );
      FeVariable_SetValueAtNode( self, ii, value );
   }
   Memory_Free( value );

   /* our work is done, so we clean up after ourselves */
   Stg_Component_Destroy( feVar->feMesh, NULL, False );
   Stg_Component_Destroy( feVar->dofLayout, NULL, False );
   Stg_Component_Destroy( feVar , NULL, False );

#else
   Stream* errorStr = Journal_Register( Error_Type, (Name)self->type );
   Journal_Firewall(
      !self->context->interpolateRestart, 
      errorStr,
      "\n\n Interpolation restart not supported for ASCII checkpoint files \n\n" );
#endif   
}
