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
#include <Underworld/Function/FEMCoordinate.hpp>
#include <Underworld/Function/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/Function.hpp>

#include "MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni.h"


/* Textual name of this class */
const Type MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Type = "MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni";

/* Creation implementation / Virtual constructor */
MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_New(  MATRIXASSEMBLYTERM_NA__NB__FN_DEFARGS  ) {
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* self;
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni) );
    self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );

/* Virtual info */
    self->cppdata = (void*) new MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_cppdata;
    self->max_nElNodes = 0;
    self->Ni = NULL;
    self->geometryMesh = NULL;

    return self;
}

void MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_SetFn( void* _self, Fn::Function* fn ){
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*  self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*)_self;

    // record fn to struct
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_cppdata* cppdata = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_cppdata*) self->cppdata;
    cppdata->fn = fn;

    // setup fn
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)swarm->mesh, localCoord);
    cppdata->func = fn->getFunction(cppdata->input.get());

    // check output conforms
    const IO_double* io = dynamic_cast<const IO_double*>(cppdata->func(cppdata->input.get()));
    if( !io )
        throw std::invalid_argument("MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni routine expects functions to return 'double' type values.");
    if( io->size() != self->stiffnessMatrix->rowVariable->fieldComponentCount ) {
        std::stringstream ss;
        ss << "MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni term expects function to return array of size " << self->stiffnessMatrix->rowVariable->fieldComponentCount << ".\n";
        ss << "Provided function returns array of size " << io->size() << ".";
        throw std::invalid_argument( ss.str() );
    }
}

void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Delete( void* matrixTerm ) {
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*)matrixTerm;

    _StiffnessMatrixTerm_Delete( self );
}

void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Print( void* matrixTerm, Stream* stream ) {
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*)matrixTerm;
    _StiffnessMatrixTerm_Print( self, stream );
    /* General info */
}

void* _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_DefaultNew( Name name ) {
    /* Variables set in this function */
    SizeT                                                 _sizeOfSelf = sizeof(MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni);
    Type                                                         type = MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Type;
    Stg_Class_DeleteFunction*                                 _delete = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Delete;
    Stg_Class_PrintFunction*                                   _print = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Print;
    Stg_Class_CopyFunction*                                     _copy = NULL;
    Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_DefaultNew;
    Stg_Component_ConstructFunction*                       _construct = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_AssignFromXML;
    Stg_Component_BuildFunction*                               _build = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Build;
    Stg_Component_InitialiseFunction*                     _initialise = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Initialise;
    Stg_Component_ExecuteFunction*                           _execute = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Execute;
    Stg_Component_DestroyFunction*                           _destroy = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Destroy;
    StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_AssembleElement;
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return (void*)_MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_New(  MATRIXASSEMBLYTERM_NA__NB__FN_PASSARGS  );
}

void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*)matrixTerm;
    /* Construct Parent */
    _StiffnessMatrixTerm_AssignFromXML( self, cf, data );
}

void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Build( void* matrixTerm, void* data ) {
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*)matrixTerm;
    _StiffnessMatrixTerm_Build( self, data );

    self->Ni = (double*)malloc(sizeof(double)*4);
}

void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Initialise( void* matrixTerm, void* data ) {
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*)matrixTerm;
    _StiffnessMatrixTerm_Initialise( self, data );
}

void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Execute( void* matrixTerm, void* data ) {
    _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Destroy( void* matrixTerm, void* data ) {
    MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*)matrixTerm;

   delete (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_cppdata*)self->cppdata;

    _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}

/* DEPRECATED
void jZeroMem( void *buf, int c, size_t size ) {
    // just for dubugging
    memset( buf, c, size );
}
*/

PetscErrorCode AXequalsX( StiffnessMatrix* a, SolutionVector* x, Bool transpose ) {
  Mat Amat;
  Vec X, Y;
  PetscErrorCode ierr;

  PetscFunctionBeginUser;
  Amat = a->matrix;
  X    = x->vector;
  // create Y, duplicate vector of X
  VecDuplicate(X, &Y);
  VecCopy(X,Y);

  if (transpose) {
    ierr = MatMultTranspose(Amat,X,Y);
  } else {
    ierr = MatMult(Amat,X,Y);
  }
  // check for non-zero error code manually
  Journal_Firewall(ierr==0, NULL, (char *)"Error in AXequalsX(), see terminal command line\n");

  VecCopy(Y, X);
  VecDestroy(&Y);

  SolutionVector_UpdateSolutionOntoNodes( x );
  PetscFunctionReturn(0);
}

void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_AssembleElement(
   void*                                              matrixTerm,
   StiffnessMatrix*                                   stiffnessMatrix,
   Element_LocalIndex                                 lElement_I,
   SystemLinearEquations*                             sle,
   FiniteElementContext*                              context,
   double**                                           elStiffMat )
{
  
   MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* self = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni*)matrixTerm;
   Swarm*                              swarm        = self->integrationSwarm;
   FeVariable*                         variable     = stiffnessMatrix->rowVariable;
   Particle_InCellIndex                cParticle_I, cellParticleCount;
   int                                 dim = stiffnessMatrix->dim;

   IntegrationPoint*                   intPoint;
   ElementType*                        elementType;
   const double                        *fn_vector;
   double                              *xi, *Ni; 
   double                              detJac, weight, localNormal[3], factor;
   int                                 nodesPerEl, cell_I, dofPerNode, row, col, rowNode_I, colNode_I, g_I, d_i, n_i;

   MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_cppdata* cppdata = (MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_cppdata*)self->cppdata;
   debug_dynamic_cast<ParticleInCellCoordinate*>(cppdata->input->localCoord())->index() = lElement_I;  // set the elementId as the owning cell for the particleCoord
   cppdata->input->index() = lElement_I;

   FeMesh*                 geometryMesh = ( self->geometryMesh ? self->geometryMesh : variable->feMesh );
   ElementType*            geometryElementType;

   /* DEPRECATED
   factor = 1;
   if(factor == 1) { memset(&elStiffMat[0][0],0, 512); }
   */

   /* Set the element type */
   geometryElementType = FeMesh_GetElementType( geometryMesh, lElement_I );

   elementType = FeMesh_GetElementType( variable->feMesh, lElement_I );
   nodesPerEl  = elementType->nodeCount;
   dofPerNode  = variable->fieldComponentCount;

   // allocate shape function array Ni
   if( nodesPerEl > self->max_nElNodes ) {
      if( self->Ni) free(self->Ni);
      self->Ni = (double*)AllocArray(double, nodesPerEl );
      self->max_nElNodes = nodesPerEl;
   }
   Ni = self->Ni;

   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      debug_dynamic_cast<ParticleInCellCoordinate*>(cppdata->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
      /* get integration point information */
      intPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
      xi = intPoint->xi;
      weight = intPoint->weight;

      /* evaluate shape function, surface normal and jacobian determinant */
      ElementType_EvaluateShapeFunctionsAt( elementType, xi, Ni );
      ElementType_SurfaceNormal( elementType, lElement_I, dim, xi, localNormal );
      detJac = ElementType_SurfaceJacobianDeterminant( geometryElementType, geometryMesh, lElement_I, xi, dim, localNormal );

      // /* evaluate function */
      const IO_double* funcout = debug_dynamic_cast<const IO_double*>(cppdata->func(cppdata->input.get()));
      fn_vector = funcout->data();

      factor = weight*detJac;

        /* 
          per edge evaluation as per equation 19 - B.J.P Kaus et al. 2010
          where: 
            fn_vector is user input for, theta * pho * dT * g[]
            localNormal is calculated from the border gauss swarm, n
            Ni are the shape functions

        elStiffMat[row  ][col  ] = fn_vector[0] * localNormal[0] * Ni[rowNode_I] * Ni[colNode_I]
        elStiffMat[row  ][col+1] = fn_vector[0] * localNormal[1] * Ni[rowNode_I] * Ni[colNode_I]
        elStiffMat[row+1][col  ] = fn_vector[1] * localNormal[0] * Ni[rowNode_I] * Ni[colNode_I]
        elStiffMat[row+1][col+1] = fn_vector[1] * localNormal[1] * Ni[rowNode_I] * Ni[colNode_I]
        */

      /* The following is an assumption for 2D model testing eq.21.
       * Using 2D linear quad element (non deformed)
       * The resulting matrix will only have diagonal entries as per Kauss et al. FSSA2*/

      /*
      */
      double Ai, Bi, fem;
      for( rowNode_I = 0; rowNode_I < nodesPerEl ; rowNode_I++ ) {

        Ai = Ni[rowNode_I]; // row shape funcs
        row = rowNode_I * dofPerNode;

        for( colNode_I = 0; colNode_I < nodesPerEl; colNode_I++ ) {
          //if( colNode_I != rowNode_I ) continue; //skip
          
          Bi = Ni[colNode_I]; // col shape funcs
          col = colNode_I * dofPerNode;

          fem = Ai * Bi * factor; // build a fem factor

          elStiffMat[row  ][row  ] += fn_vector[0] * localNormal[0] * fem;
          // Remove cross terms
          // elStiffMat[row  ][col+1] += fn_vector[0] * localNormal[1] * fem;
          // elStiffMat[row+1][col  ] += fn_vector[1] * localNormal[0] * fem;
          elStiffMat[row+1][row+1] += fn_vector[1] * localNormal[1] * fem;
        }
      }

      /* build full stiffness matrix */
      /*
      for ( rowNode_I = 0; rowNode_I < nodesPerEl ; rowNode_I++ ) {
        for ( colNode_I = 0; colNode_I < nodesPerEl; colNode_I++ ) {

          for( g_I = 0; g_I < dim; g_I++ ) {
            row = rowNode_I*dofPerNode + g_I;

            for( n_I = 0; n_I < dim; n_I++ ) {
              col = colNode_I*dofPerNode + n_I;
              
              if(row != col) { continue; }
              elStiffMat[row][col] += factor * fn_vector[g_I] * localNormal[n_I] * Ni[rowNode_I] * Ni[colNode_I];
            }
          }
        }
      }
      */
   }
}
