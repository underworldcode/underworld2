/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <assert.h>
#include <sstream>
#include<cstdlib>

#include <mpi.h>
#include <petsc.h>

#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/MeshCoordinate.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>
#include <Underworld/Function/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/Function.hpp>

#include "MatrixAssemblyTerm_RotationDof.h"


/* Textual name of this class */
const Type MatrixAssemblyTerm_RotationDof_Type = (char*)"MatrixAssemblyTerm_RotationDof";

/* Creation implementation / Virtual constructor */
MatrixAssemblyTerm_RotationDof* _MatrixAssemblyTerm_RotationDof_New(  MatrixAssemblyTerm_RotationDof_DEFARGS  ) {
    MatrixAssemblyTerm_RotationDof* self;
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(MatrixAssemblyTerm_RotationDof) );
    self = (MatrixAssemblyTerm_RotationDof*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );

/* Virtual info */
    self->cppdata = (void*) new MatrixAssemblyTerm_RotationDof_cppdata;
    self->max_nElNodes_col = 0;
    self->max_nElNodes_row = 0;
    self->Ni = NULL;
    self->Mi = NULL;
    self->geometryMesh = NULL;

    return self;
}

void MatrixAssemblyTerm_RotationDof_SetE1Fn( void* _self, Fn::Function* fn ){
    MatrixAssemblyTerm_RotationDof*  self = (MatrixAssemblyTerm_RotationDof*)_self;
    FeMesh  *mesh = (FeMesh*)self->geometryMesh;
    unsigned dim  = Mesh_GetDimSize(mesh);

    MatrixAssemblyTerm_RotationDof_cppdata* cppdata = (MatrixAssemblyTerm_RotationDof_cppdata*) self->cppdata;
    // record fn to struct
    cppdata->input = std::make_shared<MeshCoordinate>((void*)mesh );
    cppdata->e1func = fn->getFunction(cppdata->input.get());

    // check output conforms
    const IO_double *testOutput = dynamic_cast<const IO_double*>(cppdata->e1func(cppdata->input.get()));
    if( !testOutput || testOutput->size() != dim ) {
      std::stringstream ss;
      ss << "Expected 'fn' for " << __func__ << " must be of length " << dim;
      throw std::invalid_argument(ss.str());
    }
}

void MatrixAssemblyTerm_RotationDof_SetE2Fn( void* _self, Fn::Function* fn ){
    MatrixAssemblyTerm_RotationDof*  self = (MatrixAssemblyTerm_RotationDof*)_self;
    FeMesh  *mesh = (FeMesh*)self->geometryMesh;
    unsigned dim  = Mesh_GetDimSize(mesh);

    MatrixAssemblyTerm_RotationDof_cppdata* cppdata = (MatrixAssemblyTerm_RotationDof_cppdata*) self->cppdata;
    // record fn to struct
    cppdata->input = std::make_shared<MeshCoordinate>((void*)mesh );
    cppdata->e2func = fn->getFunction(cppdata->input.get());

    // check output conforms
    const IO_double *testOutput = dynamic_cast<const IO_double*>(cppdata->e2func(cppdata->input.get()));
    if( !testOutput || testOutput->size() != dim ) {
      std::stringstream ss;
      ss << "Expected 'fn' for " << __func__ << " must be of length " << dim;
      throw std::invalid_argument(ss.str());
    }
}

void _MatrixAssemblyTerm_SetBNodes( void* _self, void* bNodes )
{
    MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)_self;

    if(!Stg_Class_IsInstance( bNodes, VariableCondition_Type ))
        throw std::invalid_argument("Provided 'indexSet' does not appear to be of 'IndexSet' type.");
    self->bNodes = (VariableCondition*)bNodes;
}

void _MatrixAssemblyTerm_RotationDof_Delete( void* matrixTerm ) {
    MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)matrixTerm;

    _StiffnessMatrixTerm_Delete( self );
}

void* _MatrixAssemblyTerm_RotationDof_DefaultNew( Name name ) {
    /* Variables set in this function */
    SizeT                                                 _sizeOfSelf = sizeof(MatrixAssemblyTerm_RotationDof);
    Type                                                         type = MatrixAssemblyTerm_RotationDof_Type;
    Stg_Class_DeleteFunction*                                 _delete = _MatrixAssemblyTerm_RotationDof_Delete;
    Stg_Class_PrintFunction*                                   _print = NULL;
    Stg_Class_CopyFunction*                                     _copy = NULL;
    Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _MatrixAssemblyTerm_RotationDof_DefaultNew;
    Stg_Component_ConstructFunction*                       _construct = _MatrixAssemblyTerm_RotationDof_AssignFromXML;
    Stg_Component_BuildFunction*                               _build = _MatrixAssemblyTerm_RotationDof_Build;
    Stg_Component_InitialiseFunction*                     _initialise = _MatrixAssemblyTerm_RotationDof_Initialise;
    Stg_Component_ExecuteFunction*                           _execute = _MatrixAssemblyTerm_RotationDof_Execute;
    Stg_Component_DestroyFunction*                           _destroy = _MatrixAssemblyTerm_RotationDof_Destroy;
    StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _MatrixAssemblyTerm_RotationDof_AssembleElement;
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return (void*)_MatrixAssemblyTerm_RotationDof_New(  MatrixAssemblyTerm_RotationDof_PASSARGS  );
}

void _MatrixAssemblyTerm_RotationDof_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
    MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)matrixTerm;
    /* Construct Parent */
    _StiffnessMatrixTerm_AssignFromXML( self, cf, data );

}

void _MatrixAssemblyTerm_RotationDof_Build( void* matrixTerm, void* data ) {
    MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)matrixTerm;
    _StiffnessMatrixTerm_Build( self, data );

    self->Ni = new double[27];
    self->Mi = new double[27];
    self->inc = IArray_New( );
}

void _MatrixAssemblyTerm_RotationDof_Initialise( void* matrixTerm, void* data ) {
    MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)matrixTerm;
    _StiffnessMatrixTerm_Initialise( self, data );
}

void _MatrixAssemblyTerm_RotationDof_Execute( void* matrixTerm, void* data ) {
    _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _MatrixAssemblyTerm_RotationDof_Destroy( void* matrixTerm, void* data ) {
    MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)matrixTerm;

    Stg_Class_Delete( self->inc );

    delete self->Ni;
    delete self->Mi;
    delete (MatrixAssemblyTerm_RotationDof_cppdata*)self->cppdata;

    _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}


void AXequalsY( StiffnessMatrix* a, SolutionVector* x, SolutionVector* y, Bool transpose ) {
  Mat Amat;
  Vec X, Y;
  PetscInt m,n,vecSize;
  PetscScalar *ptr;

  Amat = a->matrix;
  X    = x->vector;
  Y    = y->vector;

  MatGetSize(Amat,&m,&n);
  VecGetSize(X, &vecSize);
  VecGetArray(X,&ptr);
  VecRestoreArray(X, &ptr);

  if (transpose)
    MatMultTranspose(Amat,X,Y);
  else
    MatMult(Amat,X,Y);

  SolutionVector_UpdateSolutionOntoNodes( y );
  FeVariable_SyncShadowValues( y->feVariable );
}

void AXequalsX( StiffnessMatrix* a, SolutionVector* x, Bool transpose ) {
  Mat Amat;
  Vec X, Y;

  Amat = a->matrix;
  X    = x->vector;
  // create Y, duplicate vector of X
  VecDuplicate(X, &Y);
  VecCopy(X,Y);

  if (transpose)
    MatMultTranspose(Amat,X,Y);
  else
    MatMult(Amat,X,Y);

  VecCopy(Y, X);
  VecDestroy(&Y);

  SolutionVector_UpdateSolutionOntoNodes( x );
  FeVariable_SyncShadowValues( x->feVariable );
}

void _MatrixAssemblyTerm_RotationDof_AssembleElement(
   void*                                              matrixTerm,
   StiffnessMatrix*                                   stiffnessMatrix,
   Element_LocalIndex                                 lElement_I,
   SystemLinearEquations*                             sle,
   FiniteElementContext*                              context,
   double**                                           elStiffMat )
{
   MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)matrixTerm;
   MatrixAssemblyTerm_RotationDof_cppdata* cppdata = (MatrixAssemblyTerm_RotationDof_cppdata*)self->cppdata;

   FeVariable*    variable_row = stiffnessMatrix->rowVariable;
   FeMesh*        mesh         = ( self->geometryMesh ? self->geometryMesh : variable_row->feMesh );
   int            dim          = variable_row->dim;

   const IO_double *e1_fnout, *e2_fnout; // the unit vector uw function ptrs
   const double *e2Vec, *e1Vec;
   IArray       *inc = self->inc;
   double       crossproduct[3];
   int          nNbr, *nbr, n_i, row_i, col_i;


   // get this element's nodes, using IArray
   Mesh_GetIncidence( mesh, (MeshTopology_Dim)dim, (unsigned)lElement_I, MT_VERTEX, inc );
   nNbr = IArray_GetSize( inc );
   nbr = IArray_GetPtr( inc );

   // loop over element's nodes
   for( n_i=0 ; n_i<nNbr ; n_i++ ){
     cppdata->input->index() = nbr[n_i]; // get id of node
     // indexing into local stiffness matrix
     row_i = n_i*dim;
     col_i = n_i*dim;

     auto rawPtr = cppdata->input.get();
     // get the 'e1' units vector for the vertex
     e1_fnout = debug_dynamic_cast<const IO_double*>(cppdata->e1func(rawPtr));
     e1Vec = e1_fnout->data();

     // get the e2 unit vector
     e2_fnout = debug_dynamic_cast<const IO_double*>(cppdata->e2func(rawPtr));
     e2Vec = e2_fnout->data();

     if (dim == 2) {

       elStiffMat[row_i  ][col_i  ] = e1Vec[0];  elStiffMat[row_i  ][col_i+1] = e2Vec[0];
       elStiffMat[row_i+1][col_i  ] = e1Vec[1];  elStiffMat[row_i+1][col_i+1] = e2Vec[1];

    } else {
       // assume we can always calulate the 3rd basis vector from e2 x e2
       StGermain_VectorCrossProduct(crossproduct, (double*)e1Vec, (double*)e2Vec);

       elStiffMat[row_i  ][col_i  ] = e1Vec[0];  elStiffMat[row_i  ][col_i+1] = e2Vec[0];  elStiffMat[row_i  ][col_i+2] = crossproduct[0];
       elStiffMat[row_i+1][col_i  ] = e1Vec[1];  elStiffMat[row_i+1][col_i+1] = e2Vec[1];  elStiffMat[row_i+1][col_i+2] = crossproduct[1];
       elStiffMat[row_i+2][col_i  ] = e1Vec[2];  elStiffMat[row_i+2][col_i+1] = e2Vec[2];  elStiffMat[row_i+2][col_i+2] = crossproduct[2];
     }

   }
}
