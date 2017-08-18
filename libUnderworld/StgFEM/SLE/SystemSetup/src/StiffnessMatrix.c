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


#include <petscblaslapack.h>


void __StiffnessMatrix_NewAssemble( void* stiffnessMatrix,void* _sle, void* _context );

/* Textual name of this class */
const Type StiffnessMatrix_Type = "StiffnessMatrix";

void* StiffnessMatrix_DefaultNew( Name name )
{
    /* Variables set in this function */
    SizeT                                                               _sizeOfSelf = sizeof(StiffnessMatrix);
    Type                                                                       type = StiffnessMatrix_Type;
    Stg_Class_DeleteFunction*                                               _delete = _StiffnessMatrix_Delete;
    Stg_Class_PrintFunction*                                                 _print = _StiffnessMatrix_Print;
    Stg_Class_CopyFunction*                                                   _copy = NULL;
    Stg_Component_DefaultConstructorFunction*                   _defaultConstructor = StiffnessMatrix_DefaultNew;
    Stg_Component_ConstructFunction*                                     _construct = _StiffnessMatrix_AssignFromXML;
    Stg_Component_BuildFunction*                                             _build = _StiffnessMatrix_Build;
    Stg_Component_InitialiseFunction*                                   _initialise = _StiffnessMatrix_Initialise;
    Stg_Component_ExecuteFunction*                                         _execute = _StiffnessMatrix_Execute;
    Stg_Component_DestroyFunction*                                         _destroy = _StiffnessMatrix_Destroy;
    Bool                                                                   initFlag = False;
    StiffnessMatrix_CalculateNonZeroEntriesFunction*       _calculateNonZeroEntries = StiffnessMatrix_CalcNonZeros;
    StiffnessMatrix_AssembleFunc*                                 _assemblyFunction = NULL;
		/* why are so many data structures being passed into here??? */
    void*                                                               rowVariable = NULL;
    void*                                                            columnVariable = NULL;
    void*                                                                       rhs = NULL;
    Stg_Component*                                               applicationDepInfo = NULL;
    Dimension_Index                                                             dim = 0;
    Bool                                                                isNonLinear = False;
    Bool                                                            assembleOnNodes = False;
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
    Bool                                             assembleOnNodes,
    void*                                            entryPoint_Register,
    MPI_Comm                                         comm )
{
    /* Variables set in this function */
    SizeT                                                          _sizeOfSelf = sizeof(StiffnessMatrix);
    Type                                                                  type = StiffnessMatrix_Type;
    Stg_Class_DeleteFunction*                                          _delete = _StiffnessMatrix_Delete;
    Stg_Class_PrintFunction*                                            _print = _StiffnessMatrix_Print;
    Stg_Class_CopyFunction*                                              _copy = NULL;
    Stg_Component_DefaultConstructorFunction*              _defaultConstructor = StiffnessMatrix_DefaultNew;
    Stg_Component_ConstructFunction*                                _construct = _StiffnessMatrix_AssignFromXML;
    Stg_Component_BuildFunction*                                        _build = _StiffnessMatrix_Build;
    Stg_Component_InitialiseFunction*                              _initialise = _StiffnessMatrix_Initialise;
    Stg_Component_ExecuteFunction*                                    _execute = _StiffnessMatrix_Execute;
    Stg_Component_DestroyFunction*                                    _destroy = _StiffnessMatrix_Destroy;
    Bool                                                              initFlag = True;
    StiffnessMatrix_CalculateNonZeroEntriesFunction*  _calculateNonZeroEntries = StiffnessMatrix_CalcNonZeros;
    StiffnessMatrix_AssembleFunc*                            _assemblyFunction = NULL;

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
                               isNonLinear, assembleOnNodes, entryPoint_Register, comm );
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
    Bool                                             assembleOnNodes,
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

    self->rhs = (ForceVector*)rhs;
    self->applicationDepInfo = applicationDepInfo;
    self->comm = comm;
    self->dim = dim;
    self->isNonLinear = isNonLinear;
    self->assembleOnNodes = assembleOnNodes;

    self->rowLocalSize = 0;
    self->colLocalSize = 0;
    self->nonZeroCount = 0;
    self->diagonalNonZeroCount = 0;
    self->offDiagonalNonZeroCount = 0;
    self->diagonalNonZeroIndices = NULL;
    self->offDiagonalNonZeroIndices = NULL;

    self->entryPoint_Register = (EntryPoint_Register*)entryPoint_Register;

    self->stiffnessMatrixTermList = Stg_ObjectList_New();

    /* Set default function for Global Stiffness Matrix Assembly */
    self->_assemblyFunction = __StiffnessMatrix_NewAssemble;

    self->elStiffMat = NULL;
    self->bcVals = NULL;
    self->nRowDofs = 0;
    self->nColDofs = 0;
    self->transRHS = NULL;

    self->rowInc = IArray_New();
    self->colInc = IArray_New();

    self->rotMat = NULL;
    self->tmpMat = NULL;

    self->rotMatTerm = NULL;

    self->matrix = PETSC_NULL;
}

void _StiffnessMatrix_Delete( void* stiffnessMatrix ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;
    /* Stg_Class_Delete parent*/
    _Stg_Component_Delete( self );

}

void _StiffnessMatrix_Print( void* stiffnessMatrix, Stream* stream ) {}

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
    Bool             assembleOnNodes;

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
    assembleOnNodes = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"assembleOnNodes", False  );

    _StiffnessMatrix_Init(
        self,
        rowVar,
        colVar,
        fVector,
        applicationDepInfo,
        dim,
        isNonLinear,
        assembleOnNodes,
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

    Journal_Firewall( (self->rowEqNum!=NULL), NULL, "SitffnessMatrix could not be built, it doesn't have an row equation number object.\n"
                                                    "Python should pass this to C\nPlease contact developers." );
    Journal_Firewall( (self->colEqNum!=NULL), NULL, "SitffnessMatrix could not be built, it doesn't have an row equation number object.\n"
                                                    "Python should pass this to C\nPlease contact developers." );

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
    if( self->rhs )
        Stg_Component_Build( self->rhs, data, False );


#if DEBUG
    if ( Stream_IsPrintableLevel( self->debug, 3 ) ) {
        Journal_DPrintf( self->debug, "Row variable(%s) I.D. array calc. as:\n", self->rowVariable->name );
        FeEquationNumber_PrintmapNodeDof2Eq( self->rowEqNum, self->debug );
        Journal_DPrintf( self->debug, "Column variable(%s) I.D. array calc. as:\n", self->columnVariable->name );
        FeEquationNumber_PrintmapNodeDof2Eq( self->colEqNum, self->debug );
    }
#endif

    /* update the row and column sizes for the variables */
    self->rowLocalSize = self->rowEqNum->localEqNumsOwnedCount;
    self->colLocalSize = self->colEqNum->localEqNumsOwnedCount;

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

    // alloc some memory to these guys
   self->rotMat = ReallocArray( self->rotMat, double, (24*24) );
   self->tmpMat = ReallocArray( self->tmpMat, double, (24*24) );

}


void _StiffnessMatrix_Initialise( void* stiffnessMatrix, void* data ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;

    /* ensure variables are initialised */
    if( self->rowVariable )
        Stg_Component_Initialise( self->rowVariable, data, False );

    if( self->columnVariable )
        Stg_Component_Initialise( self->columnVariable, data, False );

    /* ensure the rhs vector is built */
    if( self->rhs )
        Stg_Component_Initialise( self->rhs, data, False );
}


void _StiffnessMatrix_Execute( void* stiffnessMatrix, void* data ) {
}

void _StiffnessMatrix_Destroy( void* stiffnessMatrix, void* data ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;

    Journal_DPrintf( self->debug, "In %s - for matrix %s\n", __func__, self->name );

    Stg_MatDestroy(&self->matrix );
    FreeObject( self->stiffnessMatrixTermList );
    FreeArray( self->diagonalNonZeroIndices );
    FreeArray( self->offDiagonalNonZeroIndices );

    FreeArray( self->rotMat );
    FreeArray( self->tmpMat );

    /* Don't delete entry points: E.P. register will delete them automatically */
    Stg_Class_Delete( self->rowInc );
    Stg_Class_Delete( self->colInc );


}

void StiffnessMatrix_Assemble( void* stiffnessMatrix, void* _sle, void* _context ) {
    StiffnessMatrix* self = (StiffnessMatrix*)stiffnessMatrix;

    StiffnessMatrix_RefreshMatrix( self );

    self->_assemblyFunction( self, _sle, _context );

}

void StiffnessMatrix_SetRotationTerm( void* mat, void* rotTerm ) {
    StiffnessMatrix* self = (StiffnessMatrix*)mat;
    // set the ptr manually
    self->rotMatTerm = (StiffnessMatrixTerm*)rotTerm;
}

/* Callback version */
void __StiffnessMatrix_NewAssemble( void* stiffnessMatrix, void* _sle, void* _context ) {
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
    rowEqNum = self->rowEqNum;
    colEqNum = self->colEqNum;
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
        if( self->assembleOnNodes ) {
          MatSetValues( matrix,
                        nRowDofs, (int*)rowEqNum->locationMatrix[e_i][0],
                        nColDofs, (int*)colEqNum->locationMatrix[e_i][0],
                        elStiffMat[0], INSERT_VALUES );
        } else {
          MatSetValues( matrix,
                        nRowDofs, (int*)rowEqNum->locationMatrix[e_i][0],
                        nColDofs, (int*)colEqNum->locationMatrix[e_i][0],
                        elStiffMat[0], ADD_VALUES );

        }
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
                                  1, colEqNum->mapNodeDof2Eq[n_i] + dof_i,
                                  1, colEqNum->mapNodeDof2Eq[n_i] + dof_i,
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

void blasMatrixMult( double A[], double B[], int rowA, int colB, int colA, double *C )
{
   /*@
   	Performs [C] = [A]*[B],

   	where [A], [B], [C] are single arrays, with rowMajor ordering, so they can be vectors.
        [C] must be pre allocated
   @*/
   char n='N';
   PetscScalar zero=0.0;
   PetscScalar alpha=1.0;

   /* BLASgemm expects fortan memory chunks, so the problem is formed using
   	 the matrix transpose formula [C]^T = [B]^T * [A]^T */

   BLASgemm_( &n, &n,
              &colB, &rowA, &colA,
              &alpha, B, &colB,
              A, &colA, &zero,
              C, &colB );
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

   FeVariable* rowVar=self->rowVariable;
   FeVariable* colVar=self->columnVariable;

   double* R = self->rotMat;
   double* tmp = self->tmpMat;

    for ( stiffnessMatrixTerm_I = 0 ; stiffnessMatrixTerm_I < stiffnessMatrixTermCount ; stiffnessMatrixTerm_I++ ) {
        stiffnessMatrixTerm = (StiffnessMatrixTerm*) Stg_ObjectList_At( self->stiffnessMatrixTermList, stiffnessMatrixTerm_I );
        StiffnessMatrixTerm_AssembleElement( stiffnessMatrixTerm, self, element_lI, sle, context, elStiffMatToAdd );
    }

  // if no rotation matrix term finish
  if( !self->rotMatTerm ) return;

  /* check if we need to PRE MULTIPLY with rotation matrix, [R] -
  only if rowVariable has non axis aligned BCs and the element is on the mesh boundary */
  if( rowVar->nonAABCs )
  {

     /*	Perform [tmp] = [R]^T * [elStiffMatToAdd]
         but do it with BLAS (fortran column major ordered) memory layout
         therefore compute: [tmp]^T = [elStiffMatToAdd]^T * [[R]^T]^T
      */

     int rowA = (self->nRowDofs > self->nColDofs) ? self->nRowDofs : self->nColDofs; // rows in [R]
     int colA = rowA;              // cols in [R]
     int colB = self->nColDofs;    // cols in [elStiffMatToAdd]

     PetscScalar one=1.0;
     PetscScalar zero=0.0;
     char n='N';
     char t='T';
     double *rubbish[27];
     int ii;

     // initialise [R] and [tmp] memory
     memset(R,0,rowA*rowA*sizeof(double));
     // set up 2D ptr for StiffnessMatrixTerm_AssembleElement
     for( ii=0; ii<rowA; ii++ ) {
       rubbish[ii] = &R[rowA*ii];
     }
     memset(tmp,0,rowA*rowA*sizeof(double));

     StiffnessMatrixTerm_AssembleElement( self->rotMatTerm, self, element_lI, sle, context, rubbish );

     // [tmp]^T = [elStiffMatToAdd]^T * [R]
     // only using BLASgemm_ [not blasMatrixMult()] because transposed matrices are used
     BLASgemm_( &n, &t, &colB, &rowA, &colA, &one,elStiffMatToAdd[0], &colB, R, &colA, &zero, tmp, &colB );

     // copy result into returned memory segment
     memcpy( elStiffMatToAdd[0], tmp, rowA*colB*sizeof(double) );
  }


  /* check if we need to POST MULTIPLY with rotation matrix -
    only if columnVariable has non axis aligned BCs and the element is on the mesh boundary */
  if( colVar->nonAABCs )
  {
     /*
        Perform [tmp] = [elStiffMatToAdd] * [R]
        but do it with BLAS (fortran column major ordered) memory layout
        therefore compute: [tmp]^T = [R]^T * [elStiffMatToAdd]^T
      */

     int rowA = self->nRowDofs; // rows in [elStiffMatToAdd]
     int colB = (self->nRowDofs > self->nColDofs) ? self->nRowDofs : self->nColDofs; // colB in [R]
     int colA = self->nColDofs;   // cols in [R]

     double *rubbish[27];
     int ii;

     // initialise [R] and [tmp] memory
     memset(R,0,rowA*rowA*sizeof(double));
     // set up 2D ptr for StiffnessMatrixTerm_AssembleElement
     for( ii=0; ii<rowA; ii++ ) {
       rubbish[ii] = &R[rowA*ii];
     }

     // TODO: for the stiffness matrix we already have this
     StiffnessMatrixTerm_AssembleElement( self->rotMatTerm, self, element_lI, sle, context, rubbish );
     memset(tmp,0,rowA*rowA*sizeof(double));

     // tmp = [elStiffMatToAdd] * [R]
     blasMatrixMult( elStiffMatToAdd[0], R, rowA, colB, colA, tmp );

     memcpy( elStiffMatToAdd[0], tmp, rowA*colB*sizeof(double) );
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
            MatMPIAIJSetPreallocation( self->matrix, 0, self->diagonalNonZeroIndices, 0, self->offDiagonalNonZeroIndices );
        else
            MatSeqAIJSetPreallocation( self->matrix, 0, self->diagonalNonZeroIndices );
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
//    Journal_Printf( stream, "Stiffness matrix: '%s'\n", self->name );
    Stream_Indent( stream );
//    Journal_Printf( stream, "Calculating number of nonzero entries...\n" );
    Stream_Indent( stream );

    rowVar = self->rowVariable;
    colVar = self->columnVariable ? self->columnVariable : rowVar;
    rowMesh = rowVar->feMesh;
    colMesh = colVar->feMesh;
    rowEqNum = self->rowEqNum;
    colEqNum = self->colEqNum;
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
            rowEq = rowEqNum->mapNodeDof2Eq[n_i][dof_i];

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
                        colEq = colEqNum->mapNodeDof2Eq[colNodes[n_j]][dof_j];

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

    Stream_UnIndent( stream );
    Stream_UnIndent( stream );

    Stg_Class_Delete( candColEqs );
}
