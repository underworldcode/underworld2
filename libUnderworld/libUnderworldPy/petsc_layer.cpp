/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include<stdio.h>
#include<sstream>
#include<petsc.h>
#include<petscsys.h>
#include<petscviewerhdf5.h>

#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/Constant.hpp>
#include "petsc_layer.hpp"

AppCtx petscApp;

void StokesModel_SetViscosityFn(AppCtx* self,Fn::Function* fn) {
  /* create input */
  std::shared_ptr<IO_double> input = std::make_shared<IO_double>(3, FunctionIO::Vector);
  
  /* test if input it works */
  auto func = fn->getFunction(input.get());
  const FunctionIO* io = dynamic_cast<const FunctionIO*>(func(input.get()));
  if( !io )
    throw std::invalid_argument("Provided function does not appear to return a valid result.");
  
  /* setup function, could I just give func */
  //self->fn_viscosity = fn->getFunction(input.get());
  self->fn_viscosity = func;
}

void StokesModel_SetForceTerm(AppCtx* self,Fn::Function* fn) {
  PetscInt dim;
  DM       *dm = &(self->dm);
  PetscDS  prob;
  PetscFE  fe;
  PetscSpace sp;

  // just to get the dof count of given equation unknown
  DMGetDS(*dm, &prob);
  PetscDSGetDiscretization(prob, 0, (PetscObject*)&fe);
  PetscFEGetBasisSpace(fe, &sp);
  PetscSpaceGetNumComponents(sp, &dim);
  
  /* create input */
  std::shared_ptr<IO_double> input = std::make_shared<IO_double>(3, FunctionIO::Vector);

  /* test if input it works */
  auto func = fn->getFunction(input.get());
  const FunctionIO* io = dynamic_cast<const FunctionIO*>(func(input.get()));

  /* error check */
  if( !io )
    throw std::invalid_argument("Provided function does not appear to return a valid result.");
  if( io->size() != dim ) {
      throw std::invalid_argument("Provided function appears to have a different dimensionality to the petsc problem");
  }
  
  /* setup function */
  self->fn_forceTerm = fn->getFunction(input.get());
}

PetscErrorCode zero_vector(PetscInt dim, PetscReal time, const PetscReal coords[], PetscInt Nf, PetscScalar *u, void *ctx)
{
  PetscInt d;
  for (d = 0; d < dim; ++d) u[d] = 0.0;
  return 0;
}

PetscErrorCode coord_vector(PetscInt dim, PetscReal time, const PetscReal coords[], PetscInt Nf, PetscScalar *u, void *ctx)
{
  PetscInt d;
  for (d = 0; d < dim; ++d) u[d] = coords[d];
  return 0;
}

PetscErrorCode one_scalar(PetscInt dim, PetscReal time, const PetscReal coords[], 
                                  PetscInt Nf, PetscScalar *u, void *ctx)
{
  u[0] = 1.0;
  return 0;
}

PetscErrorCode zero_scalar(PetscInt dim, PetscReal time, const PetscReal coords[], 
                                  PetscInt Nf, PetscScalar *u, void *ctx)
{
  u[0] = 0.0;
  return 0;
}

PetscErrorCode x_only_vector(PetscInt dim, PetscReal time, const PetscReal coords[], 
                                  PetscInt Nf, PetscScalar *u, void *ctx)
{
  u[0] = 1;
  u[1] = 1;
  u[2] = 1;
  return 0;
}

PetscErrorCode force_vector(PetscInt dim, PetscReal time, const PetscReal coords[], 
                                 PetscInt Nf, PetscScalar *u, void *ctx)
{

  AppCtx *self = &petscApp;
  PetscInt size,i; 
  /* create input */
  std::shared_ptr<IO_double> input = self->input;
  
  memcpy(input->data(), coords, 3*sizeof(double));
  
  const FunctionIO* output = debug_dynamic_cast<const FunctionIO*>(self->fn_forceTerm(input.get()));
  size = output->size(); //retrieve output size
  for( i = 0; i < size; i++ ) {
    u[i] = output->at<double>(i);
  }

  return 0;
}

void UpdateFunction( const double* vals, Fn::Constant* myconst ) {
    FunctionIO* funcIOptr = myconst->getFuncIO();
    memcpy(funcIOptr->dataRaw(), vals, funcIOptr->_dataSize*funcIOptr->size() );
} 

void prob_fnSetter( AppCtx* self, int id, Fn::Function* fn ) {
    self->prob_fn[id] = (Fn::Constant*)fn;
}

void aux_fnSetter( AppCtx* self, int id, Fn::Function* fn ) {
    self->aux_fn[id] = (Fn::Constant*)fn;
}

void f0_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[])
{
  PetscScalar mag;
  double v[20];
  AppCtx *self = &petscApp;
  std::shared_ptr<IO_double> input = self->input;
  int f_i;

  for( f_i = 0; f_i < Nf; f_i++ ) {
    UpdateFunction( u+uOff[f_i], self->prob_fn[f_i] );
  }
  /* create input */
  memcpy(input->dataRaw(), x, (input->_dataSize*input->size()) );
  const FunctionIO* output = debug_dynamic_cast<const FunctionIO*>(self->fn_viscosity(input.get()));

  /* the newton formulations means this is a negative of the f0 */
  f0[0] = a[0];
  f0[1] = a[1];
  f0[2] = a[2];
}

/* [P] The pointwise functions below describe all the problem physics */

/* gradU[comp*dim+d] = {u_x, u_y, v_x, v_y} or {u_x, u_y, u_z, v_x, v_y, v_z, w_x, w_y, w_z}
   u[Ncomp]          = {p} */
void f1_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f1[])
{
  const PetscInt Ncomp = dim;
  PetscInt       comp, d;

  for (comp = 0; comp < Ncomp; ++comp) {
    for (d = 0; d < dim; ++d) {
      /* f1[comp*dim+d] = 0.5*(gradU[comp*dim+d] + gradU[d*dim+comp]); */
      f1[comp*dim+d] = u_x[comp*dim+d];
    }
    f1[comp*dim+comp] -= u[Ncomp];
  }
}

/* gradU[comp*dim+d] = {u_x, u_y, v_x, v_y} or {u_x, u_y, u_z, v_x, v_y, v_z, w_x, w_y, w_z} */
void f0_p(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[])
{
  PetscInt d;
  f0[0] = 0.0;
  for (d = 0, f0[0] = 0.0; d < dim; ++d) f0[0] += u_x[d*dim+d];
}
void f1_p(PetscInt dim, PetscInt Nf, PetscInt NfAux,
          const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
          const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
          PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f1[])
{
  PetscInt d;
  for (d = 0; d < dim; ++d) f1[d] = 0.0;
}

/* < q, \nabla\cdot u >
   NcompI = 1, NcompJ = dim */
void j1_pu(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                  const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                  const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                  PetscReal t, PetscReal u_tShift, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar g1[])
{
  PetscInt d;
  for (d = 0; d < dim; ++d) g1[d*dim+d] = 1.0; /* \frac{\partial\phi^{u_d}}{\partial x_d} */
}

/* -< \nabla\cdot v, p >
    NcompI = dim, NcompJ = 1 */
void j2_up(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                  const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                  const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                  PetscReal t, PetscReal u_tShift, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar g2[])
{
  PetscInt d;
  for (d = 0; d < dim; ++d) g2[d*dim+d] = -1.0; /* \frac{\partial\psi^{u_d}}{\partial x_d} */
}

/* < \nabla v, \nabla u + {\nabla u}^T >
   This just gives \nabla u, give the perdiagonal for the transpose */
void j3_uu(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                  const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                  const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                  PetscReal t, PetscReal u_tShift, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar g3[])
{
  const PetscInt Nc = dim;
  PetscInt       c, d;

  for (c = 0; c < Nc; ++c) {
    for (d = 0; d < dim; ++d) {
      g3[((c*Nc+c)*dim+d)*dim+d] = 1.0;
    }
  }
}

static void stokes_momentum_J(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                                     const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                                     const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                                     PetscReal t, PetscReal u_tShift, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar g3[])
{
  const PetscReal nu  = 1.;//PetscExpReal(2.0*PetscRealPart(a[2])*x[0]);
  PetscInt        cI, d;

  for (cI = 0; cI < dim; ++cI) {
    for (d = 0; d < dim; ++d) {
      g3[((cI*dim+cI)*dim+d)*dim+d] += nu; /*g3[cI, cI, d, d]*/
      g3[((cI*dim+d)*dim+d)*dim+cI] += nu; /*g3[cI, d, d, cI]*/
    }
  }
}

/* 1/nu < q, I q >, Jp_{pp} */
static void massMatrix_invEta(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                                 PetscReal t, PetscReal u_tShift, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar g0[])
{
  const PetscReal nu = 1.;//PetscExpReal(2.0*PetscRealPart(a[2])*x[0]);
  g0[0] = 1.0/nu;
}

PetscErrorCode SetupProblem(DM dm, PetscDS prob, AppCtx *user)
{
  PetscInt                ids[2] = {1, 2}; // required for the cubed sphere reader matt implemented
  PetscErrorCode          ierr;
  const PetscInt          comp[3]   = {0,1,2}; /* scalar */

  PetscFunctionBeginUser;
  ierr = PetscDSSetResidual(prob, 0, f0_u, f1_u);CHKERRQ(ierr);
  ierr = PetscDSSetResidual(prob, 1, f0_p, f1_p);CHKERRQ(ierr);
  
  ierr = PetscDSSetJacobian(prob, 0, 0, NULL,  NULL,  NULL, j3_uu);CHKERRQ(ierr);
  ierr = PetscDSSetJacobian(prob, 0, 1, NULL,  NULL, j2_up,  NULL);CHKERRQ(ierr);
  ierr = PetscDSSetJacobian(prob, 1, 0, NULL, j1_pu,  NULL,  NULL);CHKERRQ(ierr);

#if 0
  ierr = PetscDSSetJacobianPreconditioner(prob, 0, 0, NULL, NULL, NULL, stokes_momentum_J);CHKERRQ(ierr);
  ierr = PetscDSSetJacobianPreconditioner(prob, 0, 1, NULL, NULL, j2_up, NULL);CHKERRQ(ierr);
  ierr = PetscDSSetJacobianPreconditioner(prob, 1, 0, NULL, j1_pu, NULL, NULL);CHKERRQ(ierr);
  ierr = PetscDSSetJacobianPreconditioner(prob, 1, 1, massMatrix_invEta, NULL, NULL, NULL);CHKERRQ(ierr);
#endif
  
  // no-slip dirichlet bc
  ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, 
          "wall", "marker", 0, 0, NULL, (void (*)(void)) zero_vector, 2, ids, user);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

PetscErrorCode SetupAux(DM dm, DM dmAux, AppCtx *user)
{
  PetscErrorCode (*matFuncs[3])(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nc, PetscScalar u[], void *ctx) = {force_vector, one_scalar, zero_vector};
  Vec            auxVec,gVec;
  PetscErrorCode ierr;

  PetscFunctionBegin;

  ierr = DMCreateLocalVector(dmAux, &auxVec);CHKERRQ(ierr);
  // create global to output only
  ierr = DMCreateGlobalVector(dmAux, &gVec);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) gVec, "auxiliary");CHKERRQ(ierr);

  // reload or create
  if ( user->reload==PETSC_TRUE ) {
    PetscViewer viewer;
    PetscViewerHDF5Open(PETSC_COMM_WORLD, "aux.h5", FILE_MODE_READ, &viewer);
    PetscViewerHDF5PushGroup(viewer, "/fields");
    VecLoad(gVec, viewer);
    ierr = PetscViewerHDF5PopGroup(viewer);CHKERRQ(ierr);
    ierr = PetscViewerDestroy(&viewer);CHKERRQ(ierr);
    DMGlobalToLocalBegin(dmAux, gVec, INSERT_VALUES, auxVec);
    DMGlobalToLocalEnd(dmAux, gVec, INSERT_VALUES, auxVec);
   } else {
    ierr = DMProjectFunctionLocal(dmAux, 0.0, matFuncs, NULL, INSERT_ALL_VALUES, auxVec);CHKERRQ(ierr);
    ierr = DMLocalToGlobalBegin( dmAux, auxVec, INSERT_VALUES, gVec );
    ierr = DMLocalToGlobalEnd( dmAux, auxVec, INSERT_VALUES, gVec );
    ierr = DMViewFromOptions( dmAux, NULL, "-dmAux_view");CHKERRQ(ierr);
    ierr = VecViewFromOptions(gVec, NULL, "-aux_vec_view");CHKERRQ(ierr);
   }

  ierr = VecDestroy(&gVec);CHKERRQ(ierr);
  // increases reference count
  ierr = PetscObjectCompose((PetscObject) dm, "A", (PetscObject) auxVec);CHKERRQ(ierr);
  ierr = VecDestroy(&auxVec);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

PetscErrorCode SetupDiscretization(DM dm, AppCtx *user)
{
  DM              cdm = dm;
  PetscFE         fe[2];
  PetscQuadrature q;
  PetscDS         prob;
  PetscInt        dim;
  PetscErrorCode  ierr;
  
  PetscFE *feAux  = NULL;
  PetscDS probAux = NULL;
  PetscInt numAux = 3, f;

  PetscFunctionBeginUser;
  ierr = DMGetDimension(dm, &dim);CHKERRQ(ierr);
  const char auxNames[][2056] = {"force", "shutup_moresi", "lm_velocity"};
  const PetscInt auxSizes[] = {dim, 1, dim};
  /* Create finite element */
  ierr = PetscFECreateDefault(dm, dim, dim, PETSC_FALSE, "u_", PETSC_DEFAULT, &fe[0]);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) fe[0], "velocity");CHKERRQ(ierr);
  ierr = PetscFEGetQuadrature(fe[0], &q);CHKERRQ(ierr);
  
  ierr = PetscFECreateDefault(dm, dim, 1, PETSC_FALSE, "p_", PETSC_DEFAULT, &fe[1]);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) fe[1], "pressure");CHKERRQ(ierr);
  ierr = PetscFESetQuadrature(fe[1], q);CHKERRQ(ierr);

  /* Set discretization and boundary conditions for each mesh */
  ierr = DMGetDS(dm, &prob);CHKERRQ(ierr);
  ierr = PetscDSSetDiscretization(prob, 0, (PetscObject) fe[0]);CHKERRQ(ierr);
  ierr = PetscDSSetDiscretization(prob, 1, (PetscObject) fe[1]);CHKERRQ(ierr);
  ierr = SetupProblem(dm, prob, user);CHKERRQ(ierr);
  
  /* create auxiliary field dynamically */
  if (numAux > 0) {
      PetscMalloc1( numAux, &feAux );
      // create another DS for aux
      PetscDSCreate(PetscObjectComm((PetscObject)dm), &probAux);
      for(f = 0; f < numAux; f++ ) {
        char prefix[PETSC_MAX_PATH_LEN];
      
        ierr = PetscSNPrintf(prefix, PETSC_MAX_PATH_LEN, "aux_%d_", f);CHKERRQ(ierr);
        ierr = PetscFECreateDefault( dm, dim, auxSizes[f], PETSC_FALSE, prefix, PETSC_DEFAULT, &feAux[f]);CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) feAux[f], auxNames[f]);CHKERRQ(ierr);
        ierr = PetscFESetQuadrature(feAux[f], q);CHKERRQ(ierr);
        ierr = PetscDSSetDiscretization(probAux, f, (PetscObject) feAux[f]);CHKERRQ(ierr);
      }
  }
    
  while (cdm) {
    DM coordDM;
    
    ierr = DMSetDS(cdm, prob);CHKERRQ(ierr);
    ierr = DMGetCoordinateDM(cdm,&coordDM);CHKERRQ(ierr);
    
    if ( feAux ) {
      DM dmAux;
      
      // clone topology
      ierr = DMClone(cdm, &dmAux);CHKERRQ(ierr); 
      // copy coords
      ierr = DMSetCoordinateDM(dmAux, coordDM);CHKERRQ(ierr);
      ierr = DMSetDS(dmAux, probAux);CHKERRQ(ierr);
      ierr = PetscObjectCompose((PetscObject) cdm, "dmAux", (PetscObject) dmAux);CHKERRQ(ierr);
      // function that defines aux field, and associates it with the dm
      ierr = DMDestroy(&dmAux);
    }
    ierr = DMGetCoarseDM(cdm, &cdm);CHKERRQ(ierr);
  }
  ierr = PetscFEDestroy(&fe[0]);CHKERRQ(ierr);
  ierr = PetscFEDestroy(&fe[1]);CHKERRQ(ierr);
  for( f=0; f<numAux; f++ ) {
    ierr = PetscFEDestroy(&feAux[f]);CHKERRQ(ierr);
  }
  if( probAux ) ierr = PetscDSDestroy(&probAux);CHKERRQ(ierr);
  
  {
    PetscObject  pressure;
    MatNullSpace nullSpacePres;

    ierr = DMGetField(dm, 1, &pressure);CHKERRQ(ierr);
    ierr = MatNullSpaceCreate(PetscObjectComm(pressure), PETSC_TRUE, 0, NULL, &nullSpacePres);CHKERRQ(ierr);
    ierr = PetscObjectCompose(pressure, "nullspace", (PetscObject) nullSpacePres);CHKERRQ(ierr);
    ierr = MatNullSpaceDestroy(&nullSpacePres);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

PetscErrorCode CreatePressureNullSpace(DM dm, AppCtx *user, Vec *v, MatNullSpace *nullSpace)
{
  Vec              vec;
  PetscErrorCode (*funcs[2])(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nf, PetscScalar *u, void* ctx) = {zero_vector, one_scalar};
  PetscErrorCode   ierr;

  PetscFunctionBeginUser;
  ierr = DMGetGlobalVector(dm, &vec);CHKERRQ(ierr);
  ierr = DMProjectFunction(dm, 0.0, funcs, NULL, INSERT_ALL_VALUES, vec);CHKERRQ(ierr);
  ierr = VecNormalize(vec, NULL);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) vec, "Pressure Null Space");CHKERRQ(ierr);
  ierr = VecViewFromOptions(vec, NULL, "-null_space_vec_view");CHKERRQ(ierr);
  ierr = MatNullSpaceCreate(PetscObjectComm((PetscObject) dm), PETSC_FALSE, 1, &vec, nullSpace);CHKERRQ(ierr);
  if (v) {
    ierr = DMCreateGlobalVector(dm, v);CHKERRQ(ierr);
    ierr = VecCopy(vec, *v);CHKERRQ(ierr);
  }
  ierr = DMRestoreGlobalVector(dm, &vec);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

PetscErrorCode ProcessOptionsStokes(MPI_Comm comm, AppCtx *user) {
  PetscInt dim;
  PetscBool flg;
  PetscErrorCode ierr;
  
  // set default elements
  user->elements[0] = 5;
  user->elements[1] = 5;
  user->elements[2] = 5; 
  dim=3;
  
  ierr = PetscOptionsBegin(comm, "", "Julian 3D Stokes test", PETSC_NULL);CHKERRQ(ierr); // must call before options
  user->filename[0] = '\0';
  // can't use '-f' for user->filename because jupyter steals it (!!!) 
  ierr = PetscOptionsString("-mesh_file", "Mesh filename to read", "stokesmodel", user->filename, user->filename, sizeof(user->filename), &flg);CHKERRQ(ierr);
  ierr = PetscOptionsGetBool(NULL,"","-r",&(user->reload),NULL);CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-u_petscspace_order", "1");CHKERRQ(ierr); // default linear trail function approximation
  ierr = PetscOptionsSetValue(NULL, "-p_petscspace_order", "0");CHKERRQ(ierr); // default linear trail function approximation
  ierr = PetscOptionsSetValue(NULL, "-aux_0_petscspace_order", "1");CHKERRQ(ierr); // default linear trail function approximation
  ierr = PetscOptionsSetValue(NULL, "-aux_1_petscspace_order", "1");CHKERRQ(ierr); // default linear trail function approximation
  ierr = PetscOptionsSetValue(NULL, "-aux_2_petscspace_order", "1");CHKERRQ(ierr); // default linear trail function approximation
  ierr = PetscOptionsIntArray("-elRes", "element count (default: 5,5,5)", "n/a", user->elements, &dim, NULL);CHKERRQ(ierr);
  // ierr = PetscOptionsSetValue(NULL, "-dm_plex_separate_marker", "");
 // ierr = PetscOptionsInsertFile(comm, NULL, "/home/julian/models/snippits/petsc_uw2/my.opts", PETSC_TRUE );
  ierr = PetscOptionsSetValue(NULL, "-pc_type", "jacobi"); CHKERRQ(ierr);
  /*
  ierr = PetscOptionsSetValue(NULL, "-pc_type", "fieldsplit"); CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-pc_fieldsplit_type", "schur"); CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-fieldsplit_velocity_pc_type", "mg");CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-fieldsplit_pressure_pc_type", "jacobi");CHKERRQ(ierr);
  */
  ierr = PetscOptionsHasName(NULL,NULL,"-dm_view",&flg);
  if (!flg) { ierr = PetscOptionsSetValue(NULL, "-dm_view", "hdf5:sol.h5");CHKERRQ(ierr); }
  ierr = PetscOptionsSetValue(NULL, "-sol_vec_view", "hdf5:sol.h5::append");CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-dmAux_view", "hdf5:aux.h5");CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-aux_vec_view", "hdf5:aux.h5::append");CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-snes_monitor", "");CHKERRQ(ierr);
  ierr = PetscOptionsEnd();
  return(0);
}

static void t1_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[])
{
  PetscInt d_i;
  for( d_i=0; d_i<dim; ++d_i ) f0[d_i] = u_x[d_i];
}

static void t0_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[])
{
  PetscScalar mag;
  double v[20];
  AppCtx *self = &petscApp;
  std::shared_ptr<IO_double> input = self->input;
  int f_i;

  for( f_i = 0; f_i < Nf; f_i++ ) {
    UpdateFunction( u+uOff[f_i], self->prob_fn[f_i] );
  }
  /* create input */
  memcpy(input->dataRaw(), x, (input->_dataSize*input->size()) );
  const FunctionIO* output = debug_dynamic_cast<const FunctionIO*>(self->fn_forceTerm(self->input.get()));

  f0[0] = output->at<double>(0);
}


PetscErrorCode PoissonSetupProblem(DM dm, PetscDS prob, AppCtx *user)
{
  PetscInt                ids[2] = {1, 2}; // required for the cubed sphere reader matt implemented
  PetscErrorCode          ierr;
  const PetscInt          comp[3]   = {0,1,2}; /* scalar */

  PetscFunctionBeginUser;
  ierr = PetscDSSetResidual(prob, 0, t0_u, t1_u);CHKERRQ(ierr);
  ierr = PetscDSSetJacobian(prob, 0, 0, NULL,  NULL,  NULL, j1_pu);CHKERRQ(ierr);

  ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, "mrJones", "marker", 
                             0, 0, NULL, /* field to constain and number of constained components */
                             (void (*)(void)) zero_scalar, 1, &ids[1], user);CHKERRQ(ierr);
                             
  ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, "mrsSkywalker", "marker", 
                             0, 0, NULL, /* field to constain and number of constained components */
                             (void (*)(void)) one_scalar, 1, &ids[0], user);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

// Can make a single SetupAux which dynamically takes the *user ptr and applies the coorect functions
PetscErrorCode PoissonSetupAux(DM dm, DM dmAux, AppCtx *user)
{
  PetscErrorCode (*matFuncs[1])(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nc, PetscScalar u[], void *ctx) = {force_vector};
  Vec            auxVec, gVec;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* Get the local vector, global doesn't work in parallel */
  ierr = DMCreateLocalVector(dmAux, &auxVec);CHKERRQ(ierr);
  ierr = DMProjectFunctionLocal(dmAux, 0.0, matFuncs, NULL, INSERT_ALL_VALUES, auxVec);CHKERRQ(ierr);

  ierr = DMSetFromOptions(dmAux);CHKERRQ(ierr);
  ierr = DMViewFromOptions(dmAux, NULL, "-dmAux_view");CHKERRQ(ierr);
  
  ierr = DMCreateGlobalVector(dmAux, &gVec);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) gVec, "auxiliary");CHKERRQ(ierr);
  ierr = VecViewFromOptions(gVec, NULL, "-aux_vec_view"); CHKERRQ(ierr);
  ierr = DMLocalToGlobalBegin( dmAux, auxVec, INSERT_VALUES, gVec );
  ierr = DMLocalToGlobalEnd( dmAux, auxVec, INSERT_VALUES, gVec );
#if 0
  /* load from lm hdf5 file */
  {
      PetscViewer viewer;
      PetscViewerHDF5Open(PETSC_COMM_WORLD, "aux.h5", FILE_MODE_READ, &viewer);
      PetscViewerHDF5PushGroup(viewer, "/fields");
      VecLoad(gVec, viewer);
//      ierr = PetscViewerHDF5PopGroup(viewer);CHKERRQ(ierr);
      ierr = PetscViewerDestroy(&viewer);CHKERRQ(ierr);
  }
#endif


  // increases reference count
  ierr = PetscObjectCompose((PetscObject) dm, "A", (PetscObject) auxVec);CHKERRQ(ierr);
  ierr = VecDestroy(&auxVec);CHKERRQ(ierr);
  ierr = VecDestroy(&gVec);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

PetscErrorCode PoissonProcessOptions(MPI_Comm comm, AppCtx *user) {
  PetscInt dim;
  PetscBool flg;
  PetscErrorCode ierr;
  
  // set default elements
  user->elements[0] = 5;
  user->elements[1] = 5;
  user->elements[2] = 5; 
  dim=3;
  
  ierr = PetscOptionsBegin(comm, "", "Julian 3D Poisson test", PETSC_NULL);CHKERRQ(ierr); // must call before options
  user->filename[0] = '\0';
  // can't use '-f' for user->filename because jupyter steals it (!!!) 
  ierr = PetscOptionsString("-mesh_file", "Mesh filename to read", "stokesmodel", user->filename, user->filename, sizeof(user->filename), &flg);CHKERRQ(ierr);
  //ierr = PetscOptionsSetValue(NULL, "-t_petscspace_order", "1");CHKERRQ(ierr); // default linear trail function approximation
  //ierr = PetscOptionsSetValue(NULL, "-aux_0_petscspace_order", "1");CHKERRQ(ierr); // default linear trail function approximation
  //ierr = PetscOptionsIntArray("-elRes", "element count (default: 5,5,5)", "n/a", user->elements, &dim, NULL);CHKERRQ(ierr);
  // ierr = PetscOptionsSetValue(NULL, "-dm_plex_separate_marker", "");
 // ierr = PetscOptionsInsertFile(comm, NULL, "/home/julian/models/snippits/petsc_uw2/my.opts", PETSC_TRUE );
  //ierr = PetscOptionsSetValue(NULL, "-pc_type", "amg"); CHKERRQ(ierr);
  /*
  ierr = PetscOptionsHasName(NULL,NULL,"-dm_view",&flg);
  if (!flg) { ierr = PetscOptionsSetValue(NULL, "-dm_view", "hdf5:sol.h5");CHKERRQ(ierr); }
  ierr = PetscOptionsSetValue(NULL, "-sol_vec_view", "hdf5:sol.h5::append");CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-dmAux_view", "hdf5:aux.h5");CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-aux_vec_view", "hdf5:aux.h5::append");CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-snes_monitor", "");CHKERRQ(ierr);
  */
  ierr = PetscOptionsEnd();
  return(0);
}

PetscErrorCode PoissonSetupDiscretization(DM dm, AppCtx *user)
{
  DM              cdm = dm;
  PetscFE         fe;
  PetscQuadrature q;
  PetscDS         prob;
  PetscInt        dim;
  PetscErrorCode  ierr;

  PetscFE *feAux  = NULL;
  PetscDS probAux = NULL;
  PetscInt numAux = 1, f;
  const char auxNames[][2056] = {"heating"};
  const PetscInt auxSizes[] = {1};

  PetscFunctionBeginUser;
  ierr = DMGetDimension(dm, &dim);CHKERRQ(ierr);
  /* Create finite element */
  ierr = PetscFECreateDefault(dm, dim, 1, PETSC_FALSE, "temperature_", PETSC_DEFAULT, &fe);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) fe, "temperature");CHKERRQ(ierr);
  ierr = PetscFEGetQuadrature(fe, &q);CHKERRQ(ierr);
  /* Set discretization and boundary conditions for each mesh */
  ierr = DMGetDS(dm, &prob);CHKERRQ(ierr);
  ierr = PetscDSSetDiscretization(prob, 0, (PetscObject) fe);CHKERRQ(ierr);
  ierr = PoissonSetupProblem(dm, prob, user);CHKERRQ(ierr);

  /* create auxiliary field dynamically */
  if (numAux > 0) {
      PetscMalloc1( numAux, &feAux );
      // create another DS for aux
      PetscDSCreate(PetscObjectComm((PetscObject)dm), &probAux);
      for(f = 0; f < numAux; f++ ) {
        char prefix[PETSC_MAX_PATH_LEN];
      
        ierr = PetscSNPrintf(prefix, PETSC_MAX_PATH_LEN, "aux_%d_", f);CHKERRQ(ierr);
        ierr = PetscFECreateDefault( dm, dim, auxSizes[f], PETSC_FALSE, prefix, PETSC_DEFAULT, &feAux[f]);CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) feAux[f], auxNames[f]);CHKERRQ(ierr);
        ierr = PetscFESetQuadrature(feAux[f], q);CHKERRQ(ierr);
        ierr = PetscDSSetDiscretization(probAux, f, (PetscObject) feAux[f]);CHKERRQ(ierr);
      }
  }

  while (cdm) {
    DM coordDM;

    ierr = DMSetDS(cdm, prob);CHKERRQ(ierr);
    ierr = DMGetCoordinateDM(cdm,&coordDM);CHKERRQ(ierr);

    if ( feAux ) {
      DM dmAux;
      
      // clone topology
      ierr = DMClone(cdm, &dmAux);CHKERRQ(ierr); 
      // copy coords
      ierr = DMSetCoordinateDM(dmAux, coordDM);CHKERRQ(ierr);
      ierr = DMSetDS(dmAux, probAux);CHKERRQ(ierr);
      ierr = PetscObjectCompose((PetscObject) cdm, "dmAux", (PetscObject) dmAux);CHKERRQ(ierr);
      // function that defines aux field, and associates it with the dm
      ierr = DMDestroy(&dmAux);
    }
    ierr = DMGetCoarseDM(cdm, &cdm);CHKERRQ(ierr);
  }

  ierr = PetscFEDestroy(&fe);CHKERRQ(ierr);
  for( f=0; f<numAux; f++ ) {
    ierr = PetscFEDestroy(&feAux[f]);CHKERRQ(ierr);
  }
  if( probAux ) ierr = PetscDSDestroy(&probAux);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

AppCtx* PoissonModel_Setup(char* rubbish) {
    PetscFunctionBeginUser;
    AppCtx *user = &petscApp;
    DM *dm       = &(user->dm);
    SNES *snes   = &(user->snes);
    Vec u, nullVec;
    MatNullSpace nullSpace;
    PetscErrorCode ierr;    
    MPI_Comm comm = PETSC_COMM_WORLD;
    PetscInt dim = 3;
    PetscBool use_simplices = PETSC_FALSE;
    user->reload = PETSC_FALSE;
    size_t len;
    const char    *filename = user->filename;

    // represent the FunctionIO for a coordinate
    user->input = std::make_shared<IO_double>(dim, FunctionIO::Vector);

    PoissonProcessOptions(comm, user);
    PetscStrlen(filename, &len);
    if(len > 0) {
       DMPlexCreateFromFile(comm, filename, PETSC_FALSE, dm);
    } else if ( rubbish != NULL ) {
       DMPlexCreateFromFile(comm, rubbish, PETSC_FALSE, dm);
    } else {
       DMPlexCreateBoxMesh(comm, dim, 
                            use_simplices, // use_simplices, if FALSE then tesor_cells
                            user->elements, NULL, NULL, // sizes, and the min and max coords  
                            NULL, PETSC_TRUE,dm); 
    }

    /* Distribute mesh over processes */
    {
      PetscPartitioner part;
      DM               pdm = NULL;

      ierr = DMPlexGetPartitioner(*dm, &part);//CHKERRQ(ierr);
      ierr = PetscPartitionerSetFromOptions(part);//CHKERRQ(ierr);
      ierr = DMPlexDistribute(*dm, 0, NULL, &pdm);//CHKERRQ(ierr);
      if (pdm) {
        ierr = DMDestroy(dm);//CHKERRQ(ierr);
        *dm  = pdm;
      }
    }
    ierr = DMLocalizeCoordinates(*dm);//CHKERRQ(ierr); /* needed for periodic only */
    ierr = DMSetFromOptions(*dm);//CHKERRQ(ierr);
    ierr = DMViewFromOptions(*dm, NULL, "-dm_view");//CHKERRQ(ierr);
    
    PoissonSetupDiscretization(*dm, NULL);
    /* Calculates the index of the 'default' section, should improve performance */
    ierr = DMPlexCreateClosureIndex(*dm, NULL); // REQUIRED for hdf5 output after fields have been set
    ierr = DMSetApplicationContext(*dm, user);//CHKERRQ(ierr);

    SNESCreate(comm, snes);
    ierr = SNESSetDM(*snes, *dm); //CHKERRQ(ierr);
    
    /* Sets the fem routines for boundary, residual and Jacobian point wise operations */
    ierr = DMPlexSetSNESLocalFEM(*dm, user, user, user);//CHKERRQ(ierr);
    /* Update SNES */
    ierr = SNESSetFromOptions(*snes);//CHKERRQ(ierr);
    ierr = SNESSetUp(*snes);
    ierr = SNESView(*snes, PETSC_VIEWER_STDOUT_WORLD);
    return user;
}

AppCtx* StokesModel_Setup(char *argv) {

    PetscFunctionBeginUser;
    AppCtx *user = &petscApp;
    DM *dm       = &(user->dm);
    SNES *snes   = &(user->snes);
    Vec u, nullVec;
    MatNullSpace nullSpace;
    PetscErrorCode ierr;    
    MPI_Comm comm = PETSC_COMM_WORLD;
    PetscInt dim = 3;
    PetscBool use_simplices = PETSC_FALSE;
    user->reload = PETSC_FALSE;

    //PetscInitialize( &argc, &argv, (char*)0, NULL );
    ProcessOptionsStokes(comm, user);

    user->input = std::make_shared<IO_double>(dim, FunctionIO::Vector);
    ierr = SNESCreate(comm, snes);//CHKERRQ(ierr);
    
    const char    *filename = user->filename;
    size_t len;
    PetscStrlen(filename, &len);
    if (len>0) {
        DMPlexCreateFromFile(comm, filename, PETSC_FALSE, dm);
    } else {
        DMPlexCreateBoxMesh(comm, dim, 
                            use_simplices, // use_simplices, if not then tesor_cells
                            user->elements, NULL, NULL, // sizes, and the min and max coords  
                            NULL, PETSC_TRUE,dm);
    }
    /* Distribute mesh over processes */
    {
      PetscPartitioner part;
      DM               pdm = NULL;

      ierr = DMPlexGetPartitioner(*dm, &part);//CHKERRQ(ierr);
      ierr = PetscPartitionerSetFromOptions(part);//CHKERRQ(ierr);
      ierr = DMPlexDistribute(*dm, 0, NULL, &pdm);//CHKERRQ(ierr);
      if (pdm) {
        ierr = DMDestroy(dm);//CHKERRQ(ierr);
        *dm  = pdm;
      }
    }
    ierr = DMLocalizeCoordinates(*dm);//CHKERRQ(ierr); /* needed for periodic only */
    ierr = DMSetFromOptions(*dm);//CHKERRQ(ierr);
    
     /* Make split wall labels - closer to uw feel must run with -dm_plex_separate_marker */
    // {
    //   const char *names[4] = {"wallBottom", "wallRight", "wallTop", "wallLeft"};
    //   PetscInt    ids[4]   = {1, 2, 3, 4};
    //   DMLabel     label;
    //   IS          is;
    //   PetscInt    f;
    // 
    //   for (f = 0; f < 4; ++f) {
    //     ierr = DMGetStratumIS(dm, "marker", ids[f],  &is);CHKERRQ(ierr);
    //     if (!is) continue;
    //     ierr = DMCreateLabel(dm, names[f]);CHKERRQ(ierr);
    //     ierr = DMGetLabel(dm, names[f], &label);CHKERRQ(ierr);
    //     if (is) {
    //       ierr = DMLabelInsertIS(label, is, 1);CHKERRQ(ierr);
    //     }
    //     ierr = ISDestroy(&is);CHKERRQ(ierr);
    //   }
    // }
    SetupDiscretization(*dm, NULL);
    /* [O] The mesh is output to HDF5 using options */
    ierr = DMViewFromOptions(*dm, NULL, "-dm_view");//CHKERRQ(ierr);
    ierr = DMSetApplicationContext(*dm, user);//CHKERRQ(ierr);
    ierr = SNESSetDM(*snes, *dm); //CHKERRQ(ierr);
    
    /* Calculates the index of the 'default' section, should improve performance */
    ierr = DMPlexCreateClosureIndex(*dm, NULL);//CHKERRQ(ierr);
    /* Sets the fem routines for boundary, residual and Jacobian point wise operations */
    ierr = DMPlexSetSNESLocalFEM(*dm, user, user, user);//CHKERRQ(ierr);
    ierr = CreatePressureNullSpace(*dm, user, &nullVec, &nullSpace);//CHKERRQ(ierr);
    /* Get global vector */
    //ierr = DMCreateGlobalVector(*dm, &u);//CHKERRQ(ierr);
    /* Update SNES */
    ierr = SNESSetFromOptions(*snes);//CHKERRQ(ierr);
    ierr = SNESSetUp(*snes);
    ierr = SNESView(*snes, PETSC_VIEWER_STDOUT_WORLD);

    return user;
}

int StokesModel_Solve(AppCtx* user) {
    DM *dm = &(user->dm);
    DM dmAux;
    SNES *snes = &user->snes;
    PetscErrorCode ierr;
    Vec u;

     /* Retrieve the auxiliary dm to set the aux field(s) */
     ierr = PetscObjectQuery((PetscObject)*dm, "dmAux", (PetscObject*)&dmAux);CHKERRQ(ierr);
     // function that defines aux field, and associates it with the dm
     ierr = SetupAux(*dm, dmAux, user);CHKERRQ(ierr);

    /* Solve and output*/
      PetscErrorCode (*initialGuess[2])(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nf, PetscScalar *u, void* ctx) = {zero_vector, zero_scalar};
      Vec              lu;
      
      /* Use global vector here */
      ierr = DMCreateGlobalVector(*dm, &u);
      ierr = DMProjectFunction(*dm, 0.0, initialGuess, NULL, INSERT_VALUES, u);CHKERRQ(ierr);
      ierr = PetscObjectSetName((PetscObject) u, "Initial Solution");CHKERRQ(ierr); 
      ierr = VecViewFromOptions(u, NULL, "-initial_vec_view");CHKERRQ(ierr); 
      ierr = DMGetLocalVector(*dm, &lu);CHKERRQ(ierr);
      ierr = DMPlexInsertBoundaryValues(*dm, PETSC_TRUE, lu, 0.0, NULL, NULL, NULL);CHKERRQ(ierr);
      ierr = DMGlobalToLocalBegin(*dm, u, INSERT_VALUES, lu);CHKERRQ(ierr);
      ierr = DMGlobalToLocalEnd(*dm, u, INSERT_VALUES, lu);CHKERRQ(ierr);
      ierr = PetscObjectSetName((PetscObject) lu, "Local Solution");CHKERRQ(ierr);
      ierr = VecViewFromOptions(lu, NULL, "-local_vec_view");CHKERRQ(ierr);
      ierr = DMRestoreLocalVector(*dm, &lu);CHKERRQ(ierr);
      ierr = PetscObjectSetName((PetscObject) u, "Solution");CHKERRQ(ierr);
      ierr = SNESSolve(*snes, NULL, u);CHKERRQ(ierr);
      ierr = VecViewFromOptions(u, NULL, "-sol_vec_view");CHKERRQ(ierr);
  //   {
	// /* Output dm and temperature solution */
  //     PetscViewer h5viewer;
  //     PetscViewerHDF5Open(PETSC_COMM_WORLD, "sol.h5", FILE_MODE_WRITE, &h5viewer);
  //     PetscViewerSetFromOptions(h5viewer);
  //     DMView(dm, h5viewer);
  //     PetscViewerDestroy(&h5viewer);
  // 
  //     PetscViewerHDF5Open(PETSC_COMM_WORLD, "sol.h5", FILE_MODE_APPEND, &h5viewer);
  //     PetscViewerSetFromOptions(h5viewer);
  //     VecView(u, h5viewer);
  //     PetscViewerDestroy(&h5viewer);
  //   }
	      
    //VecDestroy(&nullVec);
    //MatNullSpaceDestroy(&nullSpace);
    VecDestroy(&u);
    SNESDestroy(snes);
    DMDestroy(dm);
    //PetscFinalize(); Called by uw
    return 0;
}

int PoissonModel_Solve(AppCtx* user) {
    DM *dm = &(user->dm);
    DM dmAux;
    SNES *snes = &user->snes;
    PetscErrorCode ierr;
    Vec u;

     /* Retrieve the auxiliary dm to set the aux field(s) */
     ierr = PetscObjectQuery((PetscObject)*dm, "dmAux", (PetscObject*)&dmAux);CHKERRQ(ierr);
     // function that defines aux field, and associates it with the dm
     ierr = PoissonSetupAux(*dm, dmAux, user);CHKERRQ(ierr);

    /* Solve and output*/
      PetscErrorCode (*initialGuess[1])(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nf, PetscScalar *u, void* ctx) = {zero_scalar};
      Vec              lu;
      
      /* Use global vector here */
      ierr = DMCreateGlobalVector(*dm, &u);
      ierr = DMProjectFunction(*dm, 0.0, initialGuess, NULL, INSERT_VALUES, u);CHKERRQ(ierr);
      ierr = PetscObjectSetName((PetscObject) u, "Initial Solution");CHKERRQ(ierr); 
      ierr = VecViewFromOptions(u, NULL, "-initial_vec_view");CHKERRQ(ierr); 
      ierr = DMGetLocalVector(*dm, &lu);CHKERRQ(ierr);
      ierr = DMPlexInsertBoundaryValues(*dm, PETSC_TRUE, lu, 0.0, NULL, NULL, NULL);CHKERRQ(ierr);
      ierr = DMGlobalToLocalBegin(*dm, u, INSERT_VALUES, lu);CHKERRQ(ierr);
      ierr = DMGlobalToLocalEnd(*dm, u, INSERT_VALUES, lu);CHKERRQ(ierr);
      ierr = PetscObjectSetName((PetscObject) lu, "Local Solution");CHKERRQ(ierr);
      ierr = VecViewFromOptions(lu, NULL, "-local_vec_view");CHKERRQ(ierr);
      ierr = DMRestoreLocalVector(*dm, &lu);CHKERRQ(ierr);
      ierr = PetscObjectSetName((PetscObject) u, "Solution");CHKERRQ(ierr);
      ierr = SNESSolve(*snes, NULL, u);CHKERRQ(ierr);
      ierr = VecViewFromOptions(u, NULL, "-sol_vec_view");CHKERRQ(ierr);
	      
    //VecDestroy(&nullVec);
    //MatNullSpaceDestroy(&nullSpace);
    VecDestroy(&u);
    SNESDestroy(snes);
    DMDestroy(dm);
    //PetscFinalize(); Called by uw
    return 0;
}
