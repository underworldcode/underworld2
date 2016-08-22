/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <assert.h>
#include <string.h>
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
const Type MatrixAssemblyTerm_RotationDof_Type = "MatrixAssemblyTerm_RotationDof";

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

void MatrixAssemblyTerm_RotationDof_SetRadialFn( void* _self, Fn::Function* fn ){
    MatrixAssemblyTerm_RotationDof*  self = (MatrixAssemblyTerm_RotationDof*)_self;

    // record fn to struct
    MatrixAssemblyTerm_RotationDof_cppdata* cppdata = (MatrixAssemblyTerm_RotationDof_cppdata*) self->cppdata;
    cppdata->rfn = fn;

    // setup fn
    FeMesh* mesh = (FeMesh*)self->geometryMesh;

    cppdata->input = std::make_shared<MeshCoordinate>((void*)mesh );
    cppdata->radialfunc = fn->getFunction(cppdata->input);

    // check output conforms
    std::shared_ptr<const IO_double> iodub = std::dynamic_pointer_cast<const IO_double>(cppdata->radialfunc(cppdata->input));
    /* TODO: Check this is a vector of doubles */
    if( !iodub )
        printf( "Hello" );
        //throw std::invalid_argument("MatrixAssemblyTerm routine expects functions to return 'double' type values.");
    if( iodub->size() != 1 )
        printf("Good Bye");
        //throw std::invalid_argument("MatrixAssemblyTerm matrix routine expects functions to scalar values.");
}

void MatrixAssemblyTerm_RotationDof_SetNormalFn( void* _self, Fn::Function* fn ){
    MatrixAssemblyTerm_RotationDof*  self = (MatrixAssemblyTerm_RotationDof*)_self;

    // record fn to struct
    MatrixAssemblyTerm_RotationDof_cppdata* cppdata = (MatrixAssemblyTerm_RotationDof_cppdata*) self->cppdata;
    cppdata->normalfn = fn;

    // setup fn
    FeMesh* mesh = (FeMesh*)self->geometryMesh;

    cppdata->input = std::make_shared<MeshCoordinate>((void*)mesh );
    cppdata->normalfunc = fn->getFunction(cppdata->input);

    // check output conforms
    std::shared_ptr<const IO_double> iodub = std::dynamic_pointer_cast<const IO_double>(cppdata->normalfunc(cppdata->input));
    /* TODO: Check this is a vector of doubles */
    if( !iodub )
        printf( "Hello" );
        //throw std::invalid_argument("MatrixAssemblyTerm routine expects functions to return 'double' type values.");
    if( iodub->size() != 1 )
        printf("Good Bye");
        //throw std::invalid_argument("MatrixAssemblyTerm matrix routine expects functions to scalar values.");
}

void _MatrixAssemblyTerm_RotationDof_Delete( void* matrixTerm ) {
    MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)matrixTerm;

    _StiffnessMatrixTerm_Delete( self );
}

void _MatrixAssemblyTerm_RotationDof_Print( void* matrixTerm, Stream* stream ) {
    MatrixAssemblyTerm_RotationDof* self = (MatrixAssemblyTerm_RotationDof*)matrixTerm;
    _StiffnessMatrixTerm_Print( self, stream );
    /* General info */
}

void* _MatrixAssemblyTerm_RotationDof_DefaultNew( Name name ) {
    /* Variables set in this function */
    SizeT                                                 _sizeOfSelf = sizeof(MatrixAssemblyTerm_RotationDof);
    Type                                                         type = MatrixAssemblyTerm_RotationDof_Type;
    Stg_Class_DeleteFunction*                                 _delete = _MatrixAssemblyTerm_RotationDof_Delete;
    Stg_Class_PrintFunction*                                   _print = _MatrixAssemblyTerm_RotationDof_Print;
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

    self->Ni = (double*)malloc(sizeof(double)*4);
    self->Mi = (double*)malloc(sizeof(double)*4);
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

   delete (MatrixAssemblyTerm_RotationDof_cppdata*)self->cppdata;

    _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}


void AXequalsY( StiffnessMatrix* a, SolutionVector* x, SolutionVector* y) {
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

  MatMult(Amat,X,Y);

  SolutionVector_UpdateSolutionOntoNodes( y );
  FeVariable_SyncShadowValues( y->feVariable );
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

   const double *normalVec, *radialVec;
   IArray       *inc = self->inc;
   double       crossproduct[3], mag;
   int          nNbr, *nbr, n_i, row_i, col_i, xx;


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

     // get the 'radial' units vector for the vertex
     std::shared_ptr<const IO_double> radial_fnout = debug_dynamic_cast<const IO_double>(cppdata->radialfunc(cppdata->input));
     radialVec = radial_fnout->data();

     // if vertex is ~0 mag we assume it is not to be rotated
     // and we apply the identity matrix
     mag = StGermain_VectorMagnitude((double*)radialVec, dim);
     if( mag < 0.5 ) {
       for( xx=0; xx<dim; xx++) {
         elStiffMat[row_i+xx][col_i+xx] = 1.0;
       }
       continue; // loop to next node
     }

     // get the normal unit vector
     std::shared_ptr<const IO_double> normal_fnout = debug_dynamic_cast<const IO_double>(cppdata->normalfunc(cppdata->input));
     normalVec = normal_fnout->data();

     StGermain_VectorCrossProduct(crossproduct, (double*)radialVec, (double*)normalVec);

     elStiffMat[row_i  ][col_i  ] = radialVec[0];  elStiffMat[row_i  ][col_i+1] = normalVec[0];  elStiffMat[row_i  ][col_i+2] = crossproduct[0];
     elStiffMat[row_i+1][col_i  ] = radialVec[1];  elStiffMat[row_i+1][col_i+1] = normalVec[1];  elStiffMat[row_i+1][col_i+2] = crossproduct[1];
     elStiffMat[row_i+2][col_i  ] = radialVec[2];  elStiffMat[row_i+2][col_i+1] = normalVec[2];  elStiffMat[row_i+2][col_i+2] = crossproduct[2];
   }
}
