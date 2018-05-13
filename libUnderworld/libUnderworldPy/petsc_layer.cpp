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
  
  /* setup function */
  self->fn_viscosity = fn->getFunction(input.get());
}

void StokesModel_SetForceTerm(AppCtx* self,Fn::Function* fn) {
  PetscInt dim;
  DM       *dm = &(self->dm);

  /* create input */
  std::shared_ptr<IO_double> input = std::make_shared<IO_double>(3, FunctionIO::Vector);

  /* test if input it works */
  auto func = fn->getFunction(input.get());
  const FunctionIO* io = dynamic_cast<const FunctionIO*>(func(input.get()));

  /* error check */
  DMGetDimension(*dm, &dim);
  if( !io )
    throw std::invalid_argument("Provided function does not appear to return a valid result.");
  if( io->size() != dim ) {
      throw std::invalid_argument("Provided function appears to have a different dimensionality to the petsc problem");
  }
  
  /* setup function */
  self->fn_forceterm = fn->getFunction(input.get());
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
  
  /* create input */
  std::shared_ptr<IO_double> input = std::make_shared<IO_double>(3, FunctionIO::Vector);
  
  memcpy(input->data(), coords, 3*sizeof(double));
  
  const FunctionIO* output = debug_dynamic_cast<const FunctionIO*>(self->fn_forceterm(input.get()));
  // f0[0] = 0.;
  u[0] = output->at<double>(0);
  u[1] = output->at<double>(1);
  u[2] = output->at<double>(2);
#if 0
  PetscScalar circle;
  // circle geometry. If inside set gravity force, otherwise 0 force
  circle = pow(coords[0]-0.5,2)+pow(coords[1]-0.5,2)+pow(coords[2]-0.7,2);
  if (circle < pow(0.3,2) ) {
    u[0] = 1; 
  } else {
    u[0] = 0;
  }
#endif
  return 0;
}

#if 0
updateFunctions(dim, Nf, NfAux, u, a) {
    /* The idea here is to create a new input of the constant values,
     * that way the constants are up-to-date. Then call the functions for the rhs.
     * Note we still need the x[] (global coordinates to be used)
     */
 auto io_v = std::make_shared<IO_double>(dim, FunctionIO::Vector);
 auto io_p = std::make_shared<IO_double>(1, FunctionIO::Scalar);
 auto io_a0 = std::make_shared<IO_double>(a0_dim, FunctionIO::Vector);
 auto io_a1 = std::make_shared<IO_double>(a1_dim, FunctionIO::Vector);
}
#endif

void f0_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[])
{
  PetscScalar mag;

#if 0
  AppCtx *self = &petscApp;
  self->fn_v = std::shared_ptr<IO_double> input = std::make_shared<IO_double>(dim, FunctionIO::Vector);
  self->fn_p = std::shared_ptr<IO_double> input = std::make_shared<IO_double>(dim, FunctionIO::Scalar );
  self->fn_a0
  self->fn_a1
  v0 = FunctionIO(u[0])
  self->fn_v->value(0) = u[0];
  self->fn_v->value(1) = u[1];

  output->evaluate(input)
#endif

  /* the newton formulations means this is a negative of the f0 */
      /*
  mag = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
  f0[0] = a[0] * x[0]/mag;
  f0[1] = a[0] * x[1]/mag;
  f0[2] = a[0] * x[2]/mag;
  */
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

PetscErrorCode SetupProblem(DM dm, PetscDS prob, AppCtx *user)
{
  PetscInt                ids[2] = {1, 2};
  //PetscInt                id = 1;  // for DMLabel 'marker', '1' represents vertices + midpoints along the boundary
  PetscErrorCode          ierr;
  //const PetscInt          comp[3]   = {0,1,2}; /* scalar */

  PetscFunctionBeginUser;
  ierr = PetscDSSetResidual(prob, 0, f0_u, f1_u);CHKERRQ(ierr);
  ierr = PetscDSSetResidual(prob, 1, f0_p, f1_p);CHKERRQ(ierr);
  
  ierr = PetscDSSetJacobian(prob, 0, 0, NULL,  NULL,  NULL, j3_uu);CHKERRQ(ierr);
  ierr = PetscDSSetJacobian(prob, 0, 1, NULL,  NULL, j2_up,  NULL);CHKERRQ(ierr);
  ierr = PetscDSSetJacobian(prob, 1, 0, NULL, j1_pu,  NULL,  NULL);CHKERRQ(ierr);
  
  
  /* simple box
  ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, 
    "wall", "marker", 0, 0, NULL, (void (*)(void)) zero_vector, 1, &id, user);CHKERRQ(ierr);
  */
  ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, "wall", "marker", 0, 0, 0, (void (*)(void)) zero_vector, 2, ids, user);CHKERRQ(ierr);
    ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, "wall", "marker", 1, 0, 0, (void (*)(void)) zero_scalar, 2, ids, user);CHKERRQ(ierr);
  // ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, "mrJones", "wallTop", 
  //                           0, 0, NULL, /* field to constain and number of constained components */
  //                           (void (*)(void)) zero_scalar, 1, &ids[0], user);CHKERRQ(ierr);
  //                           
  // ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, "mrsSkywalker", "wallBottom", 
  //                           0, 0, NULL, /* field to constain and number of constained components */
  //                           (void (*)(void)) one_scalar, 1, &ids[0], user);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

PetscErrorCode SetupAux(DM dm, DM dmAux, AppCtx *user)
{
  PetscErrorCode (*matFuncs[2])(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nc, PetscScalar u[], void *ctx) = {force_vector, one_scalar};
  Vec            auxVec;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DMCreateGlobalVector(dmAux, &auxVec);CHKERRQ(ierr);
  ierr = DMProjectFunction(dmAux, 0.0, matFuncs, NULL, INSERT_ALL_VALUES, auxVec);CHKERRQ(ierr);

  ierr = DMSetFromOptions(dmAux);CHKERRQ(ierr);
  ierr = DMViewFromOptions(dmAux, NULL, "-dmAux_view");CHKERRQ(ierr);
  ierr = VecViewFromOptions(auxVec, NULL, "-aux_vec_view");CHKERRQ(ierr);
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
  PetscInt numAux = 2, f;

  PetscFunctionBeginUser;
  ierr = DMGetDimension(dm, &dim);CHKERRQ(ierr);
  const char auxNames[][2056] = {"force", "shutup_moresi"};
  const PetscInt auxSizes[] = {dim, 1};
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
  
  // /* create auxiliary field dynamically */
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
  ierr = PetscDSDestroy(&probAux);CHKERRQ(ierr);
  
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

PetscErrorCode ProcessOptions(MPI_Comm comm, AppCtx *user) {
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
  ierr = PetscOptionsString("-f", "Mesh filename to read", "stokesmodel", user->filename, user->filename, sizeof(user->filename), &flg);CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-u_petscspace_order", "1");CHKERRQ(ierr); // default linear trail function approximation
  ierr = PetscOptionsSetValue(NULL, "-p_petscspace_order", "0");CHKERRQ(ierr); // default linear trail function approximation
  ierr = PetscOptionsSetValue(NULL, "-aux_0_petscspace_order", "1");CHKERRQ(ierr); // default linear trail function approximation
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
#if 0
static PetscErrorCode CreateMesh(MPI_Comm comm, AppCtx *user, DM *dm)
{
  const char    *filename = user->filename;
  size_t         len;
  PetscErrorCode ierr;

  PetscFunctionBeginUser;
  ierr = PetscStrlen(filename, &len);CHKERRQ(ierr);
  if (!len) SETERRQ(comm, PETSC_ERR_ARG_WRONG, "Must supply a mesh filename");
  ierr = DMPlexCreateFromFile(comm, filename, PETSC_TRUE, dm);CHKERRQ(ierr);
  /* Distribute mesh over processes */
  {
    PetscPartitioner part;
    DM               pdm = NULL;

    ierr = DMPlexGetPartitioner(*dm, &part);CHKERRQ(ierr);
    ierr = PetscPartitionerSetFromOptions(part);CHKERRQ(ierr);
    ierr = DMPlexDistribute(*dm, 0, NULL, &pdm);CHKERRQ(ierr);
    if (pdm) {
      ierr = DMDestroy(dm);CHKERRQ(ierr);
      *dm  = pdm;
    }
  }
  /* Enable conversion to p4est */
  {
    char      convType[256];
    PetscBool flg;

    ierr = PetscOptionsBegin(comm, "", "Mesh conversion options", "DMPLEX");CHKERRQ(ierr);
    ierr = PetscOptionsFList("-dm_plex_convert_type","Convert DMPlex to another format","ex1",DMList,DMPLEX,convType,256,&flg);CHKERRQ(ierr);
    ierr = PetscOptionsEnd();
    if (flg) {
      DM dmConv;

      ierr = DMConvert(*dm,convType,&dmConv);CHKERRQ(ierr);
      if (dmConv) {
        ierr = DMDestroy(dm);CHKERRQ(ierr);
        *dm  = dmConv;
      }
    }
  }
  ierr = DMLocalizeCoordinates(*dm);CHKERRQ(ierr); /* needed for periodic */
  ierr = DMSetFromOptions(*dm);CHKERRQ(ierr);
  /* [O] The mesh is output to HDF5 using options */
  ierr = DMViewFromOptions(*dm, NULL, "-dm_view");CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#endif

AppCtx* StokesModel_Setup(int argc) {

    PetscFunctionBeginUser;
    AppCtx *user = &petscApp;
    DM *dm       = &(user->dm);
    SNES *snes   = &(user->snes);
    Vec u;
    Vec nullVec;
    MatNullSpace nullSpace;
    PetscErrorCode ierr;    
    MPI_Comm comm = PETSC_COMM_WORLD;

    PetscInt dim = 3;
    PetscBool use_simplices = PETSC_FALSE;

    //PetscInitialize( &argc, &argv, (char*)0, NULL );
    ProcessOptions(comm, user);

    ierr = SNESCreate(comm, snes);//CHKERRQ(ierr);
    
    const char    *filename = user->filename;

    /*
    DMPlexCreateBoxMesh(PETSC_COMM_WORLD, dim, 
                        use_simplices, // use_simplices, if not then tesor_cells
                        user->elements, NULL, NULL, // sizes, and the min and max coords  
                        NULL, PETSC_TRUE,dm);
                        */
    size_t len;
    PetscStrlen(filename, &len);
    if (len>0) {
        DMPlexCreateFromFile(comm, filename, PETSC_TRUE, dm);
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
    ierr = DMCreateGlobalVector(*dm, &u);//CHKERRQ(ierr);
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
      PetscErrorCode (*initialGuess[2])(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nf, PetscScalar *u, void* ctx) = {coord_vector, one_scalar};
      Vec              lu;
      
      /* Get global vector */
      ierr = DMCreateGlobalVector(*dm, &u);
      ierr = DMProjectFunction(*dm, 0.0, initialGuess, NULL, INSERT_VALUES, u);CHKERRQ(ierr);
      ierr = PetscObjectSetName((PetscObject) u, "Initial Solution");CHKERRQ(ierr); 
      ierr = VecViewFromOptions(u, NULL, "-initial_vec_view");CHKERRQ(ierr); 
      ierr = DMGetLocalVector(*dm, &lu);CHKERRQ(ierr);
      ierr = DMPlexInsertBoundaryValues(*dm, PETSC_TRUE, lu, 0.0, NULL, NULL, NULL);CHKERRQ(ierr);
      ierr = DMGlobalToLocalBegin(*dm, u, INSERT_VALUES, lu);CHKERRQ(ierr);
      ierr = DMGlobalToLocalEnd(*dm, u, INSERT_VALUES, lu);CHKERRQ(ierr);
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
