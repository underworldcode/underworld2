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
#include <assert.h>
#include <string.h>
#include <mpi.h>

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "StgFEM/Discretisation/Discretisation.h"


#include "types.h"

#include "FiniteElementContext.h"
#include "StiffnessMatrix.h"
#include "StiffnessMatrixTerm.h"
#include "SystemLinearEquations.h"
#include "EntryPoint.h"
#include "SolutionVector.h"
#include "ForceVector.h"
#include "Assembler.h"

void __StiffnessMatrix_NewAssemble( void* stiffnessMatrix, Bool removeBCs, void* _sle, void* _context );
void StiffnessMatrix_NewAssemble( void* stiffnessMatrix, Bool removeBCs, void* _sle, void* _context );
Bool StiffnessMatrix_ZeroBCsAsm_RowR( void* stiffMat, Assembler* assm );
Bool StiffnessMatrix_ZeroBCsAsm_ColR( void* stiffMat, Assembler* assm );
Bool StiffnessMatrix_BCAsm_ColR( void* stiffMat, Assembler* assm );
Bool StiffnessMatrix_TransBCAsm_ColR( void* stiffMat, Assembler* assm );
Bool StiffnessMatrix_DiagBCsAsm_RowR( void* stiffMat, Assembler* assm );


/* Textual name of this class */
const Type StiffnessMatrix_Type = "StiffnessMatrix";

/** First part of name for build entry point */
static const char	StiffnessMatrix_assembleStiffnessMatrixStr[] = "assembleStiffnessMatrix";


void* StiffnessMatrix_DefaultNew( Name name )
{
    /* Variables set in this function */
    SizeT                                                               _sizeOfSelf = sizeof(StiffnessMatrix);
    Type                                                                       type = StiffnessMatrix_Type;
    Stg_Class_DeleteFunction*                                               _delete = _StiffnessMatrix_Delete;
    Stg_Class_PrintFunction*                                                 _print = _StiffnessMatrix_Print;
    Stg_Class_CopyFunction*                                                   _copy = _StiffnessMatrix_Copy;
    Stg_Component_DefaultConstructorFunction*                   _defaultConstructor = StiffnessMatrix_DefaultNew;
    Stg_Component_ConstructFunction*                                     _construct = _StiffnessMatrix_AssignFromXML;
    Stg_Component_BuildFunction*                                             _build = _StiffnessMatrix_Build;
    Stg_Component_InitialiseFunction*                                   _initialise = _StiffnessMatrix_Initialise;
    Stg_Component_ExecuteFunction*                                         _execute = _StiffnessMatrix_Execute;
    Stg_Component_DestroyFunction*                                         _destroy = _StiffnessMatrix_Destroy;
    Bool                                                                   initFlag = False;
    StiffnessMatrix_CalculateNonZeroEntriesFunction*       _calculateNonZeroEntries = StiffnessMatrix_CalcNonZeros;
		/* why are so many data structures being passed into here??? */
    void*                                                               rowVariable = NULL;
    void*                                                            columnVariable = NULL;
    void*                                                                       rhs = NULL;
    Stg_Component*                                               applicationDepInfo = NULL;
    Dimension_Index                                                             dim = 0;
    Bool                                                                isNonLinear = False;
    Bool                                              allowZeroElementContributions = False;
    void*                                                       entryPoint_Register = NULL;
    MPI_Comm                                                                   comm = 0;

    /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return _StiffnessMatrix_New(  STIFFNESSMATRIX_PASSARGS  );
}


StiffnessMatrix* StiffnessMatrix_New(
    Name                                             name,
    void*                                            rowVariable,
    void*                                            columnVariable,
    void*                                            rhs,
    Stg_Component*                                   applicationDepInfo,
    Dimension_Index                                  dim,
    Bool                                             isNonLinear,
    Bool                                             allowZeroElementContributions,
    void*                                            entryPoint_Register,
    MPI_Comm                                         comm )
{
    /* Variables set in this function */
    SizeT                                                          _sizeOfSelf = sizeof(StiffnessMatrix);
    Type                                                                  type = StiffnessMatrix_Type;
    Stg_Class_DeleteFunction*                                          _delete = _StiffnessMatrix_Delete;
    Stg_Class_PrintFunction*                                            _print = _StiffnessMatrix_Print;
    Stg_Class_CopyFunction*                                              _copy = _StiffnessMatrix_Copy;
    Stg_Component_DefaultConstructorFunction*              _defaultConstructor = StiffnessMatrix_DefaultNew;
    Stg_Component_ConstructFunction*                                _construct = _StiffnessMatrix_AssignFromXML;
    Stg_Component_BuildFunction*                                        _build = _StiffnessMatrix_Build;
    Stg_Component_InitialiseFunction*                              _initialise = _StiffnessMatrix_Initialise;
    Stg_Component_ExecuteFunction*                                    _execute = _StiffnessMatrix_Execute;
    Stg_Component_DestroyFunction*                                    _destroy = _StiffnessMatrix_Destroy;
    Bool                                                              initFlag = True;
    StiffnessMatrix_CalculateNonZeroEntriesFunction*  _calculateNonZeroEntries = StiffnessMatrix_CalcNonZeros;

    /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return _StiffnessMatrix_New(  STIFFNESSMATRIX_PASSARGS  );
}


StiffnessMatrix* _StiffnessMatrix_New(  STIFFNESSMATRIX_DEFARGS  )
{
    StiffnessMatrix*	self;
	
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(StiffnessMatrix) );
    /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
    /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
       and so should be set to ZERO in any children of this class. */
    nameAllocationType = NON_GLOBAL;

    self = (StiffnessMatrix*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
    /* General info */
	
    /* Virtual functions */
    self->_calculateNonZeroEntries = _calculateNonZeroEntries;
	
    if( initFlag ){
        _StiffnessMatrix_Init( self, rowVariable, columnVariable, rhs, applicationDepInfo, dim,
                               isNonLinear, allowZeroElementContributions, entryPoint_Register, comm );
    }
	
    return self;
}

void _StiffnessMatrix_Init(
    StiffnessMatrix*                                 self,
    void*                                            rowVariable,
    void*                                            columnVariable,
    void*                                            rhs,
    Stg_Component*                                   applicationDepInfo,
    Dimension_Index                                  dim,
    Bool                                             isNonLinear,
    Bool                                             allowZeroElementContributions,
    void*                                            entryPoint_Register,
    MPI_Comm                                         comm )
{
    Stream*		error = Journal_Register( ErrorStream_Type, (Name)self->type  );
    Stream*		stream;

    /* General and Virtual info should already be set */
    stream = Journal_Register( Info_Type, (Name)self->type  );
    Stream_SetPrintingRank( stream, 0 );
	
    /* StiffnessMatrix info */
    self->isConstructed = True;
    self->debug = Stream_RegisterChild( StgFEM_SLE_SystemSetup_Debug, self->type );
    Journal_Firewall( (rowVariable != NULL), error, "Error: NULL row FeVariable provided to \"%s\" %s.\n", self->name, self->type );

    self->rowVariable = (FeVariable*)rowVariable;
    Journal_Firewall( (columnVariable != NULL), error, "Error: NULL column FeVariable provided to \"%s\" %s.\n", self->name, self->type );

    self->columnVariable = (FeVariable*)columnVariable;
    Journal_Firewall( (rhs != NULL), error, "Error: NULL rhs ForceVector provided to \"%s\" %s.\n", self->name, self->type );

    self->rhs = (ForceVector*)rhs;
    self->applicationDepInfo = applicationDepInfo;
    self->comm = comm;
    self->dim = dim;
    self->isNonLinear = isNonLinear;
    self->allowZeroElementContributions = allowZeroElementContributions;
	
    self->rowLocalSize = 0;
    self->colLocalSize = 0;
    self->nonZeroCount = 0;
    self->diagonalNonZeroCount = 0;
    self->offDiagonalNonZeroCount = 0;
    self->diagonalNonZeroIndices = NULL;
    self->offDiagonalNonZeroIndices = NULL;
	
    self->entryPoint_Register = (EntryPoint_Register*)entryPoint_Register;

    Stg_asprintf( &self->_assembleStiffnessMatrixEPName, "%s-%s", self->name, StiffnessMatrix_assembleStiffnessMatrixStr );
    self->assembleStiffnessMatrix = FeEntryPoint_New( self->_assembleStiffnessMatrixEPName, FeEntryPoint_AssembleStiffnessMatrix_CastType );

    if (self->entryPoint_Register)
        EntryPoint_Register_Add( self->entryPoint_Register, self->assembleStiffnessMatrix );

    self->stiffnessMatrixTermList = Stg_ObjectList_New();

    /* Set default function for Global Stiffness Matrix Assembly */
    EP_ReplaceAll( self->assembleStiffnessMatrix, __StiffnessMatrix_NewAssemble );

    /* We need some assembler contexts. */
    self->zeroBCsAsm = Assembler_New();
    self->bcAsm = Assembler_New();
    self->transBCAsm = Assembler_New();

    if( rowVariable == columnVariable )
        self->diagBCsAsm = Assembler_New();

    self->elStiffMat = NULL;
    self->bcVals = NULL;
    self->nRowDofs = 0;
    self->nColDofs = 0;
    self->transRHS = NULL;

    self->rowInc = IArray_New();
    self->colInc = IArray_New();

    self->nModifyCBs = 0;
    self->modifyCBs = NULL;

    self->matrix = PETSC_NULL;
}

void _StiffnessMatrix_Delete( void* stiffnessMatrix ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;
    /* Stg_Class_Delete parent*/
    _Stg_Component_Delete( self );
	
}

void _StiffnessMatrix_Print( void* stiffnessMatrix, Stream* stream ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;
	
    /* Set the Journal for printing informations */
    Stream* stiffnessMatrixStream = stream;
	
    /* General info */
    Journal_Printf( stiffnessMatrixStream, "StiffnessMatrix (ptr): %p\n", self );
	
    /* Print parent */
    _Stg_Component_Print( self, stiffnessMatrixStream );
	
    /* Virtual info */
    Journal_Printf( stiffnessMatrixStream, "\t_build (func ptr): %p\n", self->_build );
	
    Journal_Printf( stiffnessMatrixStream, "\tassembleStiffnessMatrix e.p. (ptr): %p\n", self->assembleStiffnessMatrix );
    EntryPoint_PrintConcise( self->assembleStiffnessMatrix, stream );
	
    /* StiffnessMatrix info */
    Journal_Printf( stiffnessMatrixStream, "\trowVariable (ptr): %p\n", self->rowVariable );
    Journal_Printf( stiffnessMatrixStream, "\t\tvariable name: %s\n", self->rowVariable->name );
    Journal_Printf( stiffnessMatrixStream, "\tcolumnVariable (ptr): %p\n", self->columnVariable );
    Journal_Printf( stiffnessMatrixStream, "\t\tvariable name: %s\n", self->columnVariable->name );
    Journal_Printf( stiffnessMatrixStream, "\tMatrix (ptr): %p\n", self->matrix );
    Journal_Printf( stiffnessMatrixStream, "\tComm: %u\n", self->comm );
    Journal_Printf( stiffnessMatrixStream, "\trowLocalSize: %u\n", self->rowLocalSize );
    Journal_Printf( stiffnessMatrixStream, "\tcolLocalSize: %u\n", self->colLocalSize );
    Journal_Printf( stiffnessMatrixStream, "\tnonZeroCount: %u\n", self->nonZeroCount );
    Journal_Printf( stiffnessMatrixStream, "\tisNonLinear: %s\n", StG_BoolToStringMap[self->isNonLinear] );
    Journal_Printf( stiffnessMatrixStream, "\tallowZeroElementContributions: %s\n", StG_BoolToStringMap[self->allowZeroElementContributions] );
}

void* _StiffnessMatrix_Copy( void* stiffnessMatrix, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
    StiffnessMatrix*	self = (StiffnessMatrix*)stiffnessMatrix;
    StiffnessMatrix*	newStiffnessMatrix;
    PtrMap*			map = ptrMap;
    Bool			ownMap = False;
	
    if( !map ) {
        map = PtrMap_New( 10 );
        ownMap = True;
    }
	
    newStiffnessMatrix = _Stg_Component_Copy( self, dest, deep, nameExt, map );
	
    /* Virtual functions */
    newStiffnessMatrix->_calculateNonZeroEntries = self->_calculateNonZeroEntries;
	
    /* TODO: copy matrix */
    newStiffnessMatrix->matrix = self->matrix;
/* 	newStiffnessMatrix->shellMatrix = self->shellMatrix; */
    newStiffnessMatrix->entryPoint_Register = self->entryPoint_Register;
    newStiffnessMatrix->comm = self->comm;
    newStiffnessMatrix->rowLocalSize = self->rowLocalSize;
    newStiffnessMatrix->colLocalSize = self->colLocalSize;
    newStiffnessMatrix->dim = self->dim;
    newStiffnessMatrix->nonZeroCount = self->nonZeroCount;
    newStiffnessMatrix->diagonalNonZeroCount = self->diagonalNonZeroCount;
    newStiffnessMatrix->offDiagonalNonZeroCount = self->offDiagonalNonZeroCount;
	
    if( deep ) {
        newStiffnessMatrix->debug = (Stream*)Stg_Class_Copy( self->debug, NULL, deep, nameExt, map );
        newStiffnessMatrix->rowVariable = (FeVariable*)Stg_Class_Copy( self->rowVariable, NULL, deep, nameExt, map );
        newStiffnessMatrix->columnVariable = (FeVariable*)Stg_Class_Copy( self->columnVariable, NULL, deep, nameExt, map );
        newStiffnessMatrix->rhs =(ForceVector*)Stg_Class_Copy( self->rhs, NULL, deep, nameExt, map );
        newStiffnessMatrix->assembleStiffnessMatrix = (FeEntryPoint*)Stg_Class_Copy( self->assembleStiffnessMatrix, NULL, deep, nameExt, map );
		
        if( self->_assembleStiffnessMatrixEPName ) {
            if( nameExt ) {
                Stg_asprintf( &newStiffnessMatrix->_assembleStiffnessMatrixEPName, "%s%s", 
                              self->_assembleStiffnessMatrixEPName, nameExt );
            }
            else {
                newStiffnessMatrix->_assembleStiffnessMatrixEPName = StG_Strdup( self->_assembleStiffnessMatrixEPName );
            }
        }
        else {
            newStiffnessMatrix->_assembleStiffnessMatrixEPName = NULL;
        }
		
        /* Arrays */
        if( (newStiffnessMatrix->diagonalNonZeroIndices = PtrMap_Find( map, self->diagonalNonZeroIndices )) == NULL ) {
            if( self->diagonalNonZeroIndices ) {
                newStiffnessMatrix->diagonalNonZeroIndices = Memory_Alloc_Array( int, 
                                                                                 newStiffnessMatrix->rowLocalSize, "diagonalNonZeroIndices" );
                memcpy( newStiffnessMatrix->diagonalNonZeroIndices, self->diagonalNonZeroIndices, 
                        newStiffnessMatrix->rowLocalSize * sizeof( int ) );
                PtrMap_Append( map, self->diagonalNonZeroIndices, newStiffnessMatrix->diagonalNonZeroIndices );
            }
            else {
                newStiffnessMatrix->diagonalNonZeroIndices = NULL;
            }
        }
		
        if( (newStiffnessMatrix->offDiagonalNonZeroIndices = PtrMap_Find( map, self->offDiagonalNonZeroIndices )) == NULL ) {
            if( self->offDiagonalNonZeroIndices ) {
                newStiffnessMatrix->offDiagonalNonZeroIndices = Memory_Alloc_Array( int, 
                                                                                    newStiffnessMatrix->rowLocalSize, "diagonalNonZeroIndices" );
                memcpy( newStiffnessMatrix->offDiagonalNonZeroIndices, self->offDiagonalNonZeroIndices, 
                        newStiffnessMatrix->rowLocalSize * sizeof( int ) );
                PtrMap_Append( map, self->offDiagonalNonZeroIndices, newStiffnessMatrix->offDiagonalNonZeroIndices );
            }
            else {
                newStiffnessMatrix->offDiagonalNonZeroIndices = NULL;
            }
        }
    }
    else {
        newStiffnessMatrix->debug = self->debug;
        newStiffnessMatrix->rowVariable = self->rowVariable;
        newStiffnessMatrix->columnVariable = self->columnVariable;
        newStiffnessMatrix->rhs = self->rhs;
        newStiffnessMatrix->diagonalNonZeroIndices = self->diagonalNonZeroIndices;
        newStiffnessMatrix->offDiagonalNonZeroIndices = self->offDiagonalNonZeroIndices;
    }
	
    if( ownMap ) {
        Stg_Class_Delete( map );
    }
	
    return (void*)newStiffnessMatrix;
}

void _StiffnessMatrix_AssignFromXML( void* stiffnessMatrix, Stg_ComponentFactory* cf, void* data ) {
    StiffnessMatrix* self               = (StiffnessMatrix*)stiffnessMatrix;
    Stream*		 stream;
    FeVariable*      rowVar             = NULL;
    FeVariable*      colVar             = NULL;
    ForceVector*     fVector            = NULL;
    Stg_Component*   applicationDepInfo = NULL;
    void*            entryPointRegister = NULL;
    Dimension_Index  dim                = 0;
    Bool             isNonLinear;
    Bool             allowZeroElementContributions;
	
    self->context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", FiniteElementContext, False, data );
    if( !self->context  )
        self->context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", FiniteElementContext, False, data  );

    rowVar             = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"RowVariable", FeVariable, True, data  );
    colVar             = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ColumnVariable", FeVariable, True, data  );
    fVector            = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"RHS", ForceVector, False, data  );
    applicationDepInfo = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"ApplicationDepInfo", Stg_Component, False, data);
	
//    entryPointRegister = self->context->entryPoint_Register; 
//    assert( entryPointRegister  );
	
    dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0 );
    dim = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dim", dim  );
    assert( dim  );

    isNonLinear = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"isNonLinear", False  );

    /* Default is to allow zero element contributions - to allow backward compatibility */
    allowZeroElementContributions = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"allowZeroElementContributions", True  );

    _StiffnessMatrix_Init( 
        self, 
        rowVar, 
        colVar, 
        fVector, 
        applicationDepInfo, 
        dim,
        isNonLinear,
        allowZeroElementContributions,
        entryPointRegister, 
        0 );

    /* Do we need to use the transpose? */
    self->transRHS = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"transposeRHS", ForceVector, False, data  );

    /* Setup the stream. */
    stream = Journal_Register( Info_Type, (Name)self->type  );
    if( Dictionary_GetBool_WithDefault( cf->rootDict, (Dictionary_Entry_Key)"watchAll", False ) == True  )
        Stream_SetPrintingRank( stream, STREAM_ALL_RANKS );
    else {
        unsigned	rankToWatch;

        rankToWatch = Dictionary_GetUnsignedInt_WithDefault( cf->rootDict, "rankToWatch", 0 );
        Stream_SetPrintingRank( stream, rankToWatch );
    }
}

void _StiffnessMatrix_Build( void* stiffnessMatrix, void* data ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;

    Journal_DPrintf( self->debug, "In %s - for matrix %s\n", __func__, self->name );

    /* ensure variables are built */
    if( self->rowVariable )
        Stg_Component_Build( self->rowVariable, data, False );

    /* If we don't have a communicator, grab one off the mesh. */
    if( !self->comm ) {
        self->comm = Mesh_GetCommTopology( self->rowVariable->feMesh, MT_VERTEX )->mpiComm;
        Journal_Firewall( (self->comm != 0), self->debug, "Error: NULL Comm provided to \"%s\" %s.\n",
                          self->name, self->type );
    }
	
    if( self->columnVariable )
        Stg_Component_Build( self->columnVariable, data, False );

    /* ensure the rhs vector is built */
    Stg_Component_Build( self->rhs, data, False );

	
#if DEBUG
    if ( Stream_IsPrintableLevel( self->debug, 3 ) ) {
        Journal_DPrintf( self->debug, "Row variable(%s) I.D. array calc. as:\n", self->rowVariable->name );
        FeEquationNumber_PrintDestinationArray( self->rowVariable->eqNum, self->debug );
        Journal_DPrintf( self->debug, "Column variable(%s) I.D. array calc. as:\n", self->columnVariable->name );
        FeEquationNumber_PrintDestinationArray( self->columnVariable->eqNum, self->debug );
    }
#endif
	
    /* update the row and column sizes for the variables */	
    self->rowLocalSize = self->rowVariable->eqNum->localEqNumsOwnedCount;
    self->colLocalSize = self->columnVariable->eqNum->localEqNumsOwnedCount;
	
    if( (self->rowLocalSize == 0) || (self->colLocalSize == 0) ){
       Stream_SetPrintingRank( self->debug, STREAM_ALL_RANKS );
       Journal_Firewall( 0, self->debug,
                         "Error - in %s(): local matrix row or column count is zero.\n"
                         "This might be occur in the follow situations:\n"
			 "   * your problem is possibly over decomposed \n"
                         "   * and/or your simulation resolution is insufficient\n"
			 "   * and/or your boundary condition configuration results in zero local unknowns.",
                         __func__ );
    }
    MPI_Barrier(self->comm);
    /* update the number of non zero entries from the finite element variables */
    StiffnessMatrix_CalcNonZeros( self );
	
    Journal_DPrintf( self->debug, "row(%s) localSize = %d : col(%s) localSize = %d \n", self->rowVariable->name,
                     self->rowLocalSize, self->columnVariable->name, self->colLocalSize );
	
    /* assert( self->nonZeroCount ); */

    StiffnessMatrix_RefreshMatrix( self );

    Journal_DPrintf( self->debug, "Matrix allocated.\n" );

    Assembler_SetVariables( self->zeroBCsAsm, self->rowVariable, self->columnVariable );
    Assembler_SetCallbacks( self->zeroBCsAsm, NULL, StiffnessMatrix_ZeroBCsAsm_RowR, NULL, 
                            StiffnessMatrix_ZeroBCsAsm_ColR, NULL, 
                            self );
    Assembler_SetVariables( self->bcAsm, self->rowVariable, self->columnVariable );
    Assembler_SetCallbacks( self->bcAsm, 
                            NULL, 
                            NULL, NULL, 
                            StiffnessMatrix_BCAsm_ColR, NULL, 
                            self );
    Assembler_SetVariables( self->transBCAsm, self->columnVariable, self->rowVariable );
    Assembler_SetCallbacks( self->transBCAsm, 
                            NULL, 
                            NULL, NULL, 
                            StiffnessMatrix_TransBCAsm_ColR, NULL,
                            self );
    if( self->rowVariable == self->columnVariable ) {
        Assembler_SetVariables( self->diagBCsAsm, self->rowVariable, self->columnVariable );
        Assembler_SetCallbacks( self->diagBCsAsm, 
                                NULL, 
                                StiffnessMatrix_DiagBCsAsm_RowR, NULL, 
                                NULL, NULL, 
                                self );
    }

}


void _StiffnessMatrix_Initialise( void* stiffnessMatrix, void* data ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;
	
    Journal_DPrintf( self->debug, "In %s - for matrix %s\n", __func__, self->name );
    /* ensure variables are initialised */
    if( self->rowVariable )
        Stg_Component_Initialise( self->rowVariable, data, False );
	
    if( self->columnVariable )
        Stg_Component_Initialise( self->columnVariable, data, False );

    /* ensure the rhs vector is built */
    Stg_Component_Initialise( self->rhs, data, False );
}


void _StiffnessMatrix_Execute( void* stiffnessMatrix, void* data ) {
}

void _StiffnessMatrix_Destroy( void* stiffnessMatrix, void* data ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;
	
    Journal_DPrintf( self->debug, "In %s - for matrix %s\n", __func__, self->name );

    Stg_MatDestroy(&self->matrix );
    FreeObject( self->stiffnessMatrixTermList );
    FreeArray( self->_assembleStiffnessMatrixEPName );
    FreeArray( self->diagonalNonZeroIndices );
    FreeArray( self->offDiagonalNonZeroIndices );
    FreeObject( self->zeroBCsAsm );
    FreeObject( self->bcAsm );
    FreeObject( self->transBCAsm );
    FreeObject( self->diagBCsAsm );
    /* Don't delete entry points: E.P. register will delete them automatically */
    Stg_Class_Delete( self->rowInc );
    Stg_Class_Delete( self->colInc );


}

void StiffnessMatrix_CalculateNonZeroEntries( void* stiffnessMatrix ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;
	
    self->_calculateNonZeroEntries( self );
}

void _StiffnessMatrix_CalculateNonZeroEntries( void* stiffnessMatrix ) {
    StiffnessMatrix*		self = (StiffnessMatrix*)stiffnessMatrix;
    FeMesh*					rFeMesh = self->rowVariable->feMesh;
    FeMesh*					cFeMesh = self->columnVariable->feMesh;
    Dof_EquationNumber	currMatrixRow = 0;
    Node_LocalIndex		rowNode_lI = 0;
    Index						activeEqsAtCurrRowNodeCount = 0;
#ifdef DEBUG
    unsigned int			totalNonZeroEntries = 0;
#endif

    Journal_DPrintf( self->debug, "In %s - for matrix %s\n", __func__, self->name );
    Stream_IndentBranch( StgFEM_Debug );

    assert ( self->rowVariable );
    assert ( self->columnVariable );

    Journal_DPrintfL( self->debug, 1, "row nodeLocalCount %d\n", FeMesh_GetNodeLocalSize( rFeMesh ) );
    Journal_DPrintfL( self->debug, 1, "column nodeLocalCount %d\n", FeMesh_GetNodeLocalSize( cFeMesh ) );
	
    self->diagonalNonZeroIndices = Memory_Alloc_Array  ( int, self->rowLocalSize, "diagonalNonZeroIndices" );
    self->offDiagonalNonZeroIndices = Memory_Alloc_Array  ( int, self->rowLocalSize, "offDiagonalNonZeroIndices" );
    for ( rowNode_lI = 0; rowNode_lI < self->rowLocalSize; rowNode_lI++ ) { 
        self->diagonalNonZeroIndices[rowNode_lI] = 0;
        self->offDiagonalNonZeroIndices[rowNode_lI] = 0;
    }

    for( rowNode_lI = 0; rowNode_lI < FeMesh_GetNodeLocalSize( rFeMesh ); rowNode_lI++ ) { 
        _StiffnessMatrix_CalcAndUpdateNonZeroEntriesAtRowNode( self, rowNode_lI, currMatrixRow, activeEqsAtCurrRowNodeCount );
    }

#ifdef DEBUG
    for ( rowNode_lI = 0; rowNode_lI < self->rowLocalSize; rowNode_lI++ ) { 
        totalNonZeroEntries += self->diagonalNonZeroIndices[rowNode_lI];
        totalNonZeroEntries += self->offDiagonalNonZeroIndices[rowNode_lI];
    }	

    Journal_DPrintfL( self->debug, 1, "Calculated %d non-zero entries in Matrix (results in %d bytes storage)\n",
                      totalNonZeroEntries, totalNonZeroEntries * sizeof(double) );
#endif	

    Stream_UnIndentBranch( StgFEM_Debug );
}


void _StiffnessMatrix_CalcAndUpdateNonZeroEntriesAtRowNode(
    StiffnessMatrix*	self,
    Node_LocalIndex		rowNode_lI,
    Dof_EquationNumber	currMatrixRow,
    Index			activeEqsAtCurrRowNode )
{
    FeMesh*			rFeMesh = self->rowVariable->feMesh;
    FeMesh*			cFeMesh = self->columnVariable->feMesh;
    DofLayout*		colDofLayout = self->columnVariable->dofLayout;
    FeEquationNumber*	rowEqNum = self->rowVariable->eqNum;
    FeEquationNumber*	colEqNum = self->columnVariable->eqNum;
    Element_Index		rowNodeElement_I = 0;
    Element_DomainIndex	element_dI = 0;
    Node_DomainIndex	colNode_dI = 0;
    Dof_Index		colNodeDof_I = 0;
    int*  countTableToAdjust = 0;
    Dof_EquationNumber	currColEqNum = 0;
    Node_DomainIndex*	uniqueRelatedColNodes = NULL;
    Node_Index		uniqueRelatedColNodesCount = 0;
    Node_Index		uniqueRelatedColNodes_AllocCount = 0;
    Node_Index		uniqueRelatedColNode_I = 0;
    Dof_Index		currNodeDof_I = 0;
    Dof_EquationNumber	currDofMatrixRow = 0;
    int		nNodeInc, *nodeInc;

    Journal_DPrintfL( self->debug, 3, "In %s - for row local node %d\n", __func__, rowNode_lI );
    Stream_Indent( self->debug );

    FeMesh_GetNodeElements( rFeMesh, rowNode_lI, self->rowInc );
    nNodeInc = IArray_GetSize( self->rowInc );
    nodeInc = IArray_GetPtr( self->rowInc );
    for ( rowNodeElement_I = 0; rowNodeElement_I < nNodeInc; rowNodeElement_I++ ) {
        unsigned	nElInc;

        /* note the dI (domain index) - some of these elements may be in shadow space */
        element_dI = nodeInc[rowNodeElement_I];

        nElInc = FeMesh_GetElementNodeSize( cFeMesh, element_dI );
        uniqueRelatedColNodes_AllocCount += nElInc;
    }
	
    Journal_DPrintfL( self->debug, 3, "Calculated the max possible number of unique related nodes as %d\n",
                      uniqueRelatedColNodes_AllocCount );
    uniqueRelatedColNodes = Memory_Alloc_Array( Node_DomainIndex, uniqueRelatedColNodes_AllocCount, "uniqueRelatedColNodes" );

    _StiffnessMatrix_CalculatedListOfUniqueRelatedColNodes( self, rowNode_lI, uniqueRelatedColNodes, &uniqueRelatedColNodesCount);
	
    Journal_DPrintfL( self->debug, 3, "Searching the %d unique related col nodes for active dofs\n",
                      uniqueRelatedColNodesCount );
    Stream_Indent( self->debug );
    for ( uniqueRelatedColNode_I = 0; uniqueRelatedColNode_I < uniqueRelatedColNodesCount; uniqueRelatedColNode_I++ ) {
        colNode_dI = uniqueRelatedColNodes[uniqueRelatedColNode_I];

        Journal_DPrintfL( self->debug, 3, "col node_dI %d: has %d dofs\n", colNode_dI, colDofLayout->dofCounts[colNode_dI] );
        Stream_Indent( self->debug );
        for ( colNodeDof_I = 0; colNodeDof_I < colDofLayout->dofCounts[colNode_dI]; colNodeDof_I++ ) {

            currColEqNum = colEqNum->destinationArray[colNode_dI][colNodeDof_I];
            Journal_DPrintfL( self->debug, 3, "dof %d: ", colNodeDof_I );
            if ( currColEqNum == -1 ) {
                Journal_DPrintfL( self->debug, 3, "is a BC.\n" );
            }
            else {
                if( STreeMap_HasKey( colEqNum->ownedMap, &currColEqNum ) ) {
                    Journal_DPrintfL( self->debug, 3, "is diagonal (eq %d)\n", currColEqNum );
                    countTableToAdjust = self->diagonalNonZeroIndices;
                }
                else {
                    Journal_DPrintfL( self->debug, 3, "is off-diagonal (eq %d)\n", currColEqNum );
                    countTableToAdjust = self->offDiagonalNonZeroIndices;
                }

                for ( currNodeDof_I = 0; currNodeDof_I < self->rowVariable->dofLayout->dofCounts[rowNode_lI]; currNodeDof_I++) {
                    if ( -1 != rowEqNum->destinationArray[rowNode_lI][currNodeDof_I] ) {
                        currDofMatrixRow = *(int*)STreeMap_Map(
                            rowEqNum->ownedMap,
                            rowEqNum->destinationArray[rowNode_lI] + currNodeDof_I );

                        /* Because of periodic BCs, the eq num may be lower than the normal
                         * lowest held on this processor, so we need to check this */
                        if ( currDofMatrixRow >= self->rowLocalSize ) {	
                            Journal_DPrintfL( self->debug, 3, "Found currDofMatRow(=%d) >= self->rowLocalSize(=%d) : for "
                                              "rowNode_lI=%d, currMatRow=%d, colNode_dI=%d, colNodeDof_I = %d, "
                                              "currNodeDof_I = %d\n", currDofMatrixRow,
                                              self->rowLocalSize, rowNode_lI, currMatrixRow,
                                              colNode_dI, colNodeDof_I, currNodeDof_I ); 
                        }		
                        else if ( currDofMatrixRow < 0 ) {	
                            Journal_DPrintfL( self->debug, 3, "Found currDofMatRow(=%d) < 0 : for "
                                              "rowNode_lI=%d, currMatRow=%d, colNode_dI=%d, colNodeDof_I = %d, "
                                              "currNodeDof_I = %d\n", currDofMatrixRow,
                                              rowNode_lI, currMatrixRow,
                                              colNode_dI, colNodeDof_I, currNodeDof_I ); 
                        }		
                        else {	
                            Journal_DPrintfL( self->debug, 3, "(incrementing app. count at row %d)\n",
                                              currDofMatrixRow );

                            countTableToAdjust[ currDofMatrixRow ] += 1;
                        }
                    }
                }
            }	
        }	
        Stream_UnIndent( self->debug );
    }		
    Stream_UnIndent( self->debug );

    Journal_DPrintfL( self->debug, 3, "diagonal count\t%d off diagonal count\t%d\n", 
                      self->diagonalNonZeroIndices[currMatrixRow], self->offDiagonalNonZeroIndices[currMatrixRow]);

    Memory_Free( uniqueRelatedColNodes );

/* 	 TODO: do we need to check that diag is set to at least 1, as the PETSc webpage on MatCreate_MPIAIJ suggests? */

    Stream_UnIndent( self->debug );
}			


void _StiffnessMatrix_CalculatedListOfUniqueRelatedColNodes(
    StiffnessMatrix*	self,
    Node_LocalIndex		rowNode_lI,
    Node_DomainIndex*	uniqueRelatedColNodes,
    Node_Index*		uniqueRelatedColNodesCountPtr )
{
    FeMesh*			rFeMesh = self->rowVariable->feMesh;
    FeMesh*			cFeMesh = self->columnVariable->feMesh;
    Element_Index		rowNodeElement_I = 0;
    Element_DomainIndex	element_dI = 0;
    Node_Index		colElLocalNode_I = 0;
    Node_DomainIndex	colNode_dI = 0;
    Node_Index		uniqueRelatedColNode_I = 0;
    int		nNodeInc, *nodeInc;

    FeMesh_GetNodeElements( rFeMesh, rowNode_lI, self->rowInc );
    nNodeInc = IArray_GetSize( self->rowInc );
    nodeInc = IArray_GetPtr( self->rowInc );
    Journal_DPrintfL( self->debug, 3, "Searching the %d elements this node belongs to for unique related col nodes:\n",
                      nNodeInc );
	
    Stream_Indent( self->debug );
    for ( rowNodeElement_I = 0; rowNodeElement_I < nNodeInc; rowNodeElement_I++ ) {
        int   nElInc, *elInc;

        /* note the dI (domain index) - some of these elements may be in shadow space */
        element_dI = nodeInc[rowNodeElement_I];

        Journal_DPrintfL( self->debug, 3, "rowNodeElement_I: ", rowNodeElement_I );
        Journal_DPrintfL( self->debug, 3, "domain element %d\n", element_dI );
		
        Stream_Indent( self->debug );
        FeMesh_GetElementNodes( cFeMesh, element_dI, self->colInc );
        nElInc = IArray_GetSize( self->colInc );
        elInc = IArray_GetPtr( self->colInc );
        Journal_DPrintfL( self->debug, 3, "Searching the %d column var nodes in this el:\n", nElInc );
        for ( colElLocalNode_I =0; colElLocalNode_I < nElInc; colElLocalNode_I++ ) {
            colNode_dI = elInc[colElLocalNode_I];
			
            Journal_DPrintfL( self->debug, 3, "Col domain node %d: ", colNode_dI );
            for ( uniqueRelatedColNode_I = 0; uniqueRelatedColNode_I < (*uniqueRelatedColNodesCountPtr); uniqueRelatedColNode_I++ )
            {
                if ( colNode_dI == uniqueRelatedColNodes[uniqueRelatedColNode_I] ) {
                    Journal_DPrintfL( self->debug, 3, "already in list -> skip to next.\n" );
                    break;
                }
            }
            if ( uniqueRelatedColNode_I == (*uniqueRelatedColNodesCountPtr) ) {
                Journal_DPrintfL( self->debug, 3, "is unique so far -> add to list.\n" );
                uniqueRelatedColNodes[uniqueRelatedColNode_I] = colNode_dI;
                (*uniqueRelatedColNodesCountPtr)++;
            }
        }
        Stream_UnIndent( self->debug );
    }
    Stream_UnIndent( self->debug );
}


void StiffnessMatrix_Assemble( void* stiffnessMatrix, Bool bcRemoveQuery, void* _sle, void* _context ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;
    int ii;

    StiffnessMatrix_RefreshMatrix( self );

    Journal_DPrintf( self->debug, "In %s - for matrix \"%s\" - callings the \"%s\" E.P.\n", __func__, self->name,
                     self->assembleStiffnessMatrix->name );
    /* Call the Entry point directly from the base class */
    /* Note that it may be empty: this is deliberate. */
    ((FeEntryPoint_AssembleStiffnessMatrix_CallFunction*)EntryPoint_GetRun( self->assembleStiffnessMatrix ))(
        self->assembleStiffnessMatrix,
        self,
        bcRemoveQuery,
        _sle,
        _context );

    /* Run all the modify callbacks. */
    for( ii = 0; ii < self->nModifyCBs; ii++ ) {
        void* callback = self->modifyCBs[ii].callback;
        void* object = self->modifyCBs[ii].object;
        ((void(*)(void*))callback)( object );
    }
}

/* Callback version */
void __StiffnessMatrix_NewAssemble( void* stiffnessMatrix, Bool removeBCs, void* _sle, void* _context ) {
    const double one = 1.0;
    StiffnessMatrix*		self = (StiffnessMatrix*)stiffnessMatrix;
    SystemLinearEquations*		sle = (SystemLinearEquations*)_sle;
    FeVariable			*rowVar, *colVar;
    FeMesh				*rowMesh, *colMesh;
    FeEquationNumber		*rowEqNum, *colEqNum;
    DofLayout			*rowDofs, *colDofs;
    unsigned			nRowEls;
    int			nRowNodes, *rowNodes;
    int			nColNodes, *colNodes;
    unsigned			maxDofs, maxRCDofs, nDofs, nRowDofs, nColDofs;
    double**			elStiffMat;
    double*				bcVals;
    Mat                             matrix = self->matrix;
    Vec				vector, transVector;
    int nRowNodeDofs, nColNodeDofs;
    int rowInd, colInd;
    double bc;
    unsigned			e_i, n_i, dof_i, n_j, dof_j;

    assert( self && Stg_CheckType( self, StiffnessMatrix ) );

    rowVar = self->rowVariable;
    colVar = self->columnVariable ? self->columnVariable : rowVar;
    rowEqNum = rowVar->eqNum;
    colEqNum = colVar->eqNum;
    rowMesh = rowVar->feMesh;
    colMesh = colVar->feMesh;
    rowDofs = rowVar->dofLayout;
    colDofs = colVar->dofLayout;
    nRowEls = FeMesh_GetElementLocalSize( rowMesh );
    assert( (rowVar == colVar) ? !self->transRHS : 1 );

    //matrix = self->matrix;
    vector = self->rhs ? self->rhs->vector : NULL;
    transVector = self->transRHS ? self->transRHS->vector : NULL;
    elStiffMat = NULL;
    bcVals = NULL;
    maxDofs = 0;

    /* Begin assembling each element. */
    for( e_i = 0; e_i < nRowEls; e_i++ ) {
        FeMesh_GetElementNodes( rowMesh, e_i, self->rowInc );
        nRowNodes = IArray_GetSize( self->rowInc );
        rowNodes = IArray_GetPtr( self->rowInc );
        FeMesh_GetElementNodes( colMesh, e_i, self->colInc );
        nColNodes = IArray_GetSize( self->colInc );
        colNodes = IArray_GetPtr( self->colInc );

        /* Do we need more space to assemble this element? */
        nRowDofs = 0;
        for( n_i = 0; n_i < nRowNodes; n_i++ )
            nRowDofs += rowDofs->dofCounts[rowNodes[n_i]];
        nColDofs = 0;
        for( n_i = 0; n_i < nColNodes; n_i++ )
            nColDofs += colDofs->dofCounts[colNodes[n_i]];
        nDofs = nRowDofs * nColDofs;
        self->nRowDofs = nRowDofs;

        self->nColDofs = nColDofs;
        if( nDofs > maxDofs ) {
            maxRCDofs = (nRowDofs > nColDofs) ? nRowDofs : nColDofs;
            elStiffMat = ReallocArray2D( elStiffMat, double, nRowDofs, nColDofs );
            bcVals = ReallocArray( bcVals, double, maxRCDofs );
            maxDofs = nDofs;
            self->elStiffMat = elStiffMat;
            self->bcVals = bcVals;
        }

        /* Assemble the element. */
        memset( elStiffMat[0], 0, nDofs * sizeof(double) );
        StiffnessMatrix_AssembleElement( self, e_i, sle, _context, elStiffMat );

        /* Correct for BCs providing I'm not keeping them in. */
        if( vector ) {
            memset( bcVals, 0, nRowDofs * sizeof(double) );

            rowInd = 0;
            for( n_i = 0; n_i < nRowNodes; n_i++ ) {
                nRowNodeDofs = rowDofs->dofCounts[rowNodes[n_i]];
                for( dof_i = 0; dof_i < nRowNodeDofs; dof_i++ ) {
                    if( !FeVariable_IsBC( rowVar, rowNodes[n_i], dof_i ) ) {
                        colInd = 0;
                        for( n_j = 0; n_j < nColNodes; n_j++ ) {
                            nColNodeDofs = colDofs->dofCounts[colNodes[n_j]];
                            for( dof_j = 0; dof_j < nColNodeDofs; dof_j++ ) {
                                if( FeVariable_IsBC( colVar, colNodes[n_j], dof_j ) ) {
                                    bc = DofLayout_GetValueDouble( colDofs, colNodes[n_j], dof_j );
                                    bcVals[rowInd] -= bc * elStiffMat[rowInd][colInd];
                                }
                                colInd++;
                            }
                        }
                    }
                    rowInd++;
                }
            }

            VecSetValues( vector, nRowDofs, (int*)rowEqNum->locationMatrix[e_i][0], bcVals, ADD_VALUES );
        }
        if( transVector ) {
            memset( bcVals, 0, nColDofs * sizeof(double) );

            colInd = 0;
            for( n_i = 0; n_i < nColNodes; n_i++ ) {
                nColNodeDofs = colDofs->dofCounts[colNodes[n_i]];
                for( dof_i = 0; dof_i < nColNodeDofs; dof_i++ ) {
                    if( !FeVariable_IsBC( colVar, colNodes[n_i], dof_i ) ) {
                        rowInd = 0;
                        for( n_j = 0; n_j < nRowNodes; n_j++ ) {
                            nRowNodeDofs = rowDofs->dofCounts[rowNodes[n_j]];
                            for( dof_j = 0; dof_j < nRowNodeDofs; dof_j++ ) {
                                if( FeVariable_IsBC( rowVar, rowNodes[n_j], dof_j ) ) {
                                    bc = DofLayout_GetValueDouble( rowDofs, rowNodes[n_j], dof_j );
                                    bcVals[colInd] -= bc * elStiffMat[rowInd][colInd];
                                }
                                rowInd++;
                            }
                        }
                    }
                    colInd++;
                }
            }

            VecSetValues( transVector, nColDofs, (int*)colEqNum->locationMatrix[e_i][0], bcVals, ADD_VALUES );
        }

        /* If keeping BCs in, zero corresponding entries in the element stiffness matrix. */
        if( !rowEqNum->removeBCs || !colEqNum->removeBCs ) {
            rowInd = 0;
            for( n_i = 0; n_i < nRowNodes; n_i++ ) {
                nRowNodeDofs = rowDofs->dofCounts[rowNodes[n_i]];
                for( dof_i = 0; dof_i < nRowNodeDofs; dof_i++ ) {
                    if( FeVariable_IsBC( rowVar, rowNodes[n_i], dof_i ) ) {
                        memset( elStiffMat[rowInd], 0, nColDofs * sizeof(double) );
                    }
                    else {
                        colInd = 0;
                        for( n_j = 0; n_j < nColNodes; n_j++ ) {
                            nColNodeDofs = colDofs->dofCounts[colNodes[n_j]];
                            for( dof_j = 0; dof_j < nColNodeDofs; dof_j++ ) {
                                if( FeVariable_IsBC( colVar, colNodes[n_j], dof_j ) )
                                    elStiffMat[rowInd][colInd] = 0.0;
                                colInd++;
                            }
                        }
                    }
                    rowInd++;
                }
            }
        }

        /* Add to stiffness matrix. */
        MatSetValues( matrix, 
                      nRowDofs, (int*)rowEqNum->locationMatrix[e_i][0], 
                      nColDofs, (int*)colEqNum->locationMatrix[e_i][0], 
                      elStiffMat[0], ADD_VALUES );
    }

    FreeArray( elStiffMat );
    FreeArray( bcVals );

    /* If keeping BCs in and rows and columnns use the same variable, put ones in all BC'd diagonals. */
    if( !colEqNum->removeBCs && rowVar == colVar ) {
        for( n_i = 0; n_i < FeMesh_GetNodeLocalSize( colMesh ); n_i++ ) {
            nColNodeDofs = colDofs->dofCounts[n_i];
            for( dof_i = 0; dof_i < nColNodeDofs; dof_i++ ) {
                if( FeVariable_IsBC( colVar, n_i, dof_i ) ) {
                    MatSetValues( self->matrix,
                                  1, colEqNum->destinationArray[n_i] + dof_i,
                                  1, colEqNum->destinationArray[n_i] + dof_i,
                                  (double*)&one, ADD_VALUES );
                }
            }
        }
    }

    /* Reassemble the matrix and vectors. */
    MatAssemblyBegin( matrix, MAT_FINAL_ASSEMBLY );
    MatAssemblyEnd( matrix, MAT_FINAL_ASSEMBLY );
    if( vector ) {
        VecAssemblyBegin( vector );
        VecAssemblyEnd( vector );
    }
    if( transVector) {
        VecAssemblyBegin( transVector );
        VecAssemblyEnd( transVector );
    }

    MatAssemblyBegin( matrix, MAT_FINAL_ASSEMBLY );
    MatAssemblyEnd( matrix, MAT_FINAL_ASSEMBLY );
}

/* +++ PRIVATE FUNCTIONS +++ */

void _StiffnessMatrix_PrintElementStiffnessMatrix(
    StiffnessMatrix* self,
    Element_LocalIndex element_lI,
    Dof_EquationNumber** rowElementLM,
    Dof_EquationNumber** colElementLM,
    double** elStiffMatToAdd )
{
    FeMesh*			rFeMesh = self->rowVariable->feMesh;
    FeMesh*			cFeMesh = self->columnVariable->feMesh;
    Dof_Index		rowDofsPerNode;
    Dof_Index		colDofsPerNode;
    Node_LocalIndex		rowNodesThisEl;
    Node_LocalIndex		colNodesThisEl;
    Node_LocalIndex		rowNode_I, colNode_I;
    Dof_Index		rowDof_I, colDof_I;
    Index			rowIndex, colIndex;
    int   nRowElInc, *rowElInc;
    int   nColElInc, *colElInc;

    FeMesh_GetElementNodes( rFeMesh, element_lI, self->rowInc );
    nRowElInc = IArray_GetSize( self->rowInc );
    rowElInc = IArray_GetPtr( self->rowInc );
    FeMesh_GetElementNodes( cFeMesh, element_lI, self->colInc );
    nColElInc = IArray_GetSize( self->colInc );
    colElInc = IArray_GetPtr( self->colInc );

    rowDofsPerNode = self->rowVariable->dofLayout->dofCounts[rowElInc[0]];
    colDofsPerNode = self->columnVariable->dofLayout->dofCounts[colElInc[0]];
    rowNodesThisEl = nRowElInc;
    colNodesThisEl = nColElInc;

    for ( rowNode_I=0; rowNode_I < rowNodesThisEl; rowNode_I++ ) {
        for ( rowDof_I = 0; rowDof_I < rowDofsPerNode; rowDof_I++ ) {
            for ( colNode_I=0; colNode_I < colNodesThisEl; colNode_I++ ) {
                for ( colDof_I = 0; colDof_I < colDofsPerNode; colDof_I++ ) {
                    rowIndex = rowNode_I*rowDofsPerNode + rowDof_I;
                    colIndex = colNode_I*colDofsPerNode + colDof_I;

                    Journal_DPrintf( self->debug, "Row [%d][%d], Col [%d][%d] (LM (%4d,%4d)) = %.3f\n",
                                     rowNode_I, rowDof_I,
                                     colNode_I, colDof_I,
                                     rowElementLM[rowNode_I][rowDof_I],
                                     colElementLM[colNode_I][colDof_I],
                                     elStiffMatToAdd[rowIndex][colIndex] ); 
                }			
            }
        }
    }
}

void StiffnessMatrix_AssembleElement(
    void* stiffnessMatrix,
    Element_LocalIndex element_lI,
    SystemLinearEquations* sle,
    FiniteElementContext* context,
    double** elStiffMatToAdd )
{
    StiffnessMatrix*        self                      = (StiffnessMatrix*) stiffnessMatrix;
    Index                   stiffnessMatrixTermCount  = Stg_ObjectList_Count( self->stiffnessMatrixTermList );
    Index                   stiffnessMatrixTerm_I;
    StiffnessMatrixTerm*    stiffnessMatrixTerm;

    for ( stiffnessMatrixTerm_I = 0 ; stiffnessMatrixTerm_I < stiffnessMatrixTermCount ; stiffnessMatrixTerm_I++ ) {
        stiffnessMatrixTerm = (StiffnessMatrixTerm*) Stg_ObjectList_At( self->stiffnessMatrixTermList, stiffnessMatrixTerm_I );
        StiffnessMatrixTerm_AssembleElement( stiffnessMatrixTerm, self, element_lI, sle, context, elStiffMatToAdd );
    }
}

void StiffnessMatrix_AddStiffnessMatrixTerm( void* stiffnessMatrix, void* stiffnessMatrixTerm ) {
    StiffnessMatrix* self = (StiffnessMatrix*) stiffnessMatrix;

    stiffnessMatrixTerm = Stg_CheckType( stiffnessMatrixTerm, StiffnessMatrixTerm );
    Stg_ObjectList_Append( self->stiffnessMatrixTermList, stiffnessMatrixTerm );
}

void StiffnessMatrix_RefreshMatrix( StiffnessMatrix* self ) {
	/*@
		StiffnessMatrix_RefreshMatrix - creates, or recreates if preexisting, the PETSC AIJ matrix for the StiffnessMatrix.

		The appropriate size and non-zero structure for the matrix is taken from the precalculated StiffnessMatrix data structure
	@*/	
    int nProcs;
	
    assert( self && Stg_CheckType( self, StiffnessMatrix ) );

    if( self->matrix != PETSC_NULL )
        Stg_MatDestroy(&self->matrix );

    MatCreate( self->comm, &self->matrix );
    MatSetSizes( self->matrix, self->rowLocalSize, self->colLocalSize, PETSC_DETERMINE, PETSC_DETERMINE );
    MatSetFromOptions( self->matrix );
    MPI_Comm_size( self->comm, &nProcs );

    if( self->diagonalNonZeroIndices || self->offDiagonalNonZeroIndices ) {
        if( nProcs > 1 )
            MatMPIAIJSetPreallocation( self->matrix, PETSC_NULL, self->diagonalNonZeroIndices, PETSC_NULL, self->offDiagonalNonZeroIndices );
        else
            MatSeqAIJSetPreallocation( self->matrix, PETSC_NULL, self->diagonalNonZeroIndices );
    }
    else {
        if( nProcs > 1 )
            MatMPIAIJSetPreallocation( self->matrix, self->nonZeroCount, PETSC_NULL, self->nonZeroCount, PETSC_NULL );
        else
            MatSeqAIJSetPreallocation( self->matrix, self->nonZeroCount, PETSC_NULL );
    }

#if ( (PETSC_VERSION_MAJOR>=3) && (PETSC_VERSION_MINOR>=3) )
    // required as of petsc-3.3 and above - JG 15-Nov-2012
    MatSetOption(self->matrix,MAT_NEW_NONZERO_ALLOCATION_ERR,PETSC_FALSE);
#endif
}

void StiffnessMatrix_CalcNonZeros( void* stiffnessMatrix ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;
    Stream *stream;
    FeVariable *rowVar, *colVar;
    FeMesh *rowMesh, *colMesh;
    FeEquationNumber *rowEqNum, *colEqNum;
    DofLayout *rowDofs, *colDofs;
    int nRowEqs;
    int nColNodes, *colNodes;
    int nNodeEls, *nodeEls;
    int *nDiagNonZeros, *nOffDiagNonZeros;
    int rowEq, colEq, localRowEq;
    int netNonZeros;
    STree *candColEqs;
    int e_i;
    int n_i, dof_i;
    int n_j, dof_j;

    assert( self && Stg_CheckType( self, StiffnessMatrix ) );
    assert( self->rowVariable );

    stream = Journal_Register( Info_Type, (Name)self->type  );
    Journal_Printf( stream, "Stiffness matrix: '%s'\n", self->name );
    Stream_Indent( stream );
    Journal_Printf( stream, "Calculating number of nonzero entries...\n" );
    Stream_Indent( stream );

    rowVar = self->rowVariable;
    colVar = self->columnVariable ? self->columnVariable : rowVar;
    rowMesh = rowVar->feMesh;
    colMesh = colVar->feMesh;
    rowEqNum = rowVar->eqNum;
    colEqNum = colVar->eqNum;
    nRowEqs = rowEqNum->localEqNumsOwnedCount;
    rowDofs = rowVar->dofLayout;
    colDofs = colVar->dofLayout;

    candColEqs = STree_New();
    STree_SetIntCallbacks( candColEqs );
    STree_SetItemSize( candColEqs, sizeof(int) );
    nDiagNonZeros = AllocArray( int, nRowEqs );
    nOffDiagNonZeros = AllocArray( int, nRowEqs );
    memset( nDiagNonZeros, 0, nRowEqs * sizeof(int) );
    memset( nOffDiagNonZeros, 0, nRowEqs * sizeof(int) );
    netNonZeros = 0;

    for( n_i = 0; n_i < FeMesh_GetNodeLocalSize( rowMesh ); n_i++ ) {
        for( dof_i = 0; dof_i < rowDofs->dofCounts[n_i]; dof_i++ ) {
            rowEq = rowEqNum->destinationArray[n_i][dof_i];

            if( rowEq == -1 ) continue;
            if( !STreeMap_HasKey( rowEqNum->ownedMap, &rowEq ) ) continue;

            localRowEq = *(int*)STreeMap_Map( rowEqNum->ownedMap, &rowEq );
            FeMesh_GetNodeElements( rowMesh, n_i, self->rowInc );
            nNodeEls = IArray_GetSize( self->rowInc );
            nodeEls = IArray_GetPtr( self->rowInc );
            STree_Clear( candColEqs );

            for( e_i = 0; e_i < nNodeEls; e_i++ ) {
                /* ASSUME: Row and column meshes have one-to-one element overlap. */
                FeMesh_GetElementNodes( colMesh, nodeEls[e_i], self->colInc );
                nColNodes = IArray_GetSize( self->colInc );
                colNodes = IArray_GetPtr( self->colInc );

                for( n_j = 0; n_j < nColNodes; n_j++ ) {
                    for( dof_j = 0; dof_j < colDofs->dofCounts[colNodes[n_j]]; dof_j++ ) {
                        colEq = colEqNum->destinationArray[colNodes[n_j]][dof_j];

                        if( colEq == -1 ) continue;
                        if( !STree_Has( candColEqs, &colEq  ) ) {
                            STree_Insert( candColEqs, &colEq );
                            if( STreeMap_HasKey( colEqNum->ownedMap, &colEq ) )
                                nDiagNonZeros[localRowEq]++;
                            else
                                nOffDiagNonZeros[localRowEq]++;
                            netNonZeros++;
                        }
                    }
                }
            }
        }
    }
    self->diagonalNonZeroIndices = nDiagNonZeros;
    self->offDiagonalNonZeroIndices = nOffDiagNonZeros;

    {
        int tmp, ierr;
        ierr = MPI_Allreduce( &netNonZeros, &tmp, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
        netNonZeros = tmp;
    }
    Journal_Printf( stream, "Found %d nonzero entries.\n", netNonZeros );
    Journal_Printf( stream, "Done.\n" );
    Stream_UnIndent( stream );
    Stream_UnIndent( stream );
}


Bool StiffnessMatrix_ZeroBCsAsm_RowR( void* stiffMat, Assembler* assm ) {
    memset( ((StiffnessMatrix*)stiffMat)->elStiffMat[assm->rowInd], 0, ((StiffnessMatrix*)stiffMat)->nColDofs * sizeof(double) );

    return False;
}

Bool StiffnessMatrix_ZeroBCsAsm_ColR( void* stiffMat, Assembler* assm ) {
    ((StiffnessMatrix*)stiffMat)->elStiffMat[assm->rowInd][assm->colInd] = 0.0;
    return True;
}

Bool StiffnessMatrix_BCAsm_ColR( void* stiffMat, Assembler* assm ) {
    double	bc;

    bc = DofLayout_GetValueDouble( assm->colVar->dofLayout, assm->colNodeInd, assm->colDofInd );
    ((StiffnessMatrix*)stiffMat)->bcVals[assm->rowInd] -= bc * ((StiffnessMatrix*)stiffMat)->elStiffMat[assm->rowInd][assm->colInd];

    return True;
}

Bool StiffnessMatrix_TransBCAsm_ColR( void* stiffMat, Assembler* assm ) {
    double	bc;

    bc = DofLayout_GetValueDouble( assm->colVar->dofLayout, assm->colNodeInd, assm->colDofInd );
    ((StiffnessMatrix*)stiffMat)->bcVals[assm->rowInd] -= bc * ((StiffnessMatrix*)stiffMat)->elStiffMat[assm->colInd][assm->rowInd];

    return True;
}

Bool StiffnessMatrix_DiagBCsAsm_RowR( void* stiffMat, Assembler* assm ) {
    const double	one = 1.0;

    MatSetValues( ((StiffnessMatrix*)stiffMat)->matrix, 1, (int*)&assm->rowEq, 1, (int*)&assm->rowEq, (double*)&one, ADD_VALUES );

    return True;
}

void StiffnessMatrix_AddModifyCallback( StiffnessMatrix* self, void* callback, void* object ) {
    self->nModifyCBs++;
    self->modifyCBs = ReallocArray( self->modifyCBs, Callback, self->nModifyCBs );
    self->modifyCBs[self->nModifyCBs - 1].callback = callback;
    self->modifyCBs[self->nModifyCBs - 1].object = object;
}




