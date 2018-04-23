/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include<stdio.h>
#include<petsc.h>
#include<petscviewerhdf5.h>

#include <Underworld/Function/Function.hpp>
#include "petsc_layer.hpp"

AppCtx petscApp;

/** TODO: make the first arg the AppCtx, and pass it in with python */
void theMadness(AppCtx* self,Fn::Function* fn) {
  // AppCtx *self = (AppCtx*)_self;
  // AppCtx *self = &petscApp;
  
  /* create input */
  std::shared_ptr<IO_double> input = std::make_shared<IO_double>(2, FunctionIO::Vector);
  
  /* test if input it works */
  auto func = fn->getFunction(input.get());
  const FunctionIO* io = dynamic_cast<const FunctionIO*>(func(input.get()));
  if( !io )
    throw std::invalid_argument("Provided function does not appear to return a valid result.");
  
  /* setup function */
  self->fn_source = fn->getFunction(input.get());
}
#if 0
#endif

PetscErrorCode ProcessOptions(MPI_Comm comm, AppCtx *user) {
  PetscInt dim;
  PetscErrorCode ierr;
  
  // set default elements
  user->elements[0] = 4;
  user->elements[1] = 4; 
  dim=2;
  ierr = PetscOptionsBegin(comm, "", "Julian 2D Poisson test", PETSC_NULL); // must call before options
  ierr = PetscOptionsSetValue(NULL, "-temperature_petscspace_order", "1"); // default linear trail function approximation
  ierr = PetscOptionsSetValue(NULL, "-dm_plex_separate_marker", ""); // must use
  ierr = PetscOptionsIntArray("-elRes", "element count (default: 4,4)", "n/a", user->elements, &dim, NULL);
  ierr = PetscOptionsEnd();
  return(0);
}

// PetscErrorCode SetupModel(int x) 
AppCtx* SetupModel(int x) 
{
  PetscFunctionBeginUser;
  AppCtx *user = &petscApp;
  DM *dm = &user->dm;
  SNES *snes = &user->snes;
  PetscInt size;
  Vec u;
  Vec gVec, lVec, xy;
  PetscErrorCode ierr;

    PetscInt dim = 2;
    PetscInt elements[2] = {4,3};
  ProcessOptions(PETSC_COMM_WORLD, user);


    ierr = SNESCreate(PETSC_COMM_WORLD, snes); 
    ierr = DMPlexCreateHexBoxMesh(PETSC_COMM_WORLD, 2, user->elements,
      DM_BOUNDARY_NONE,DM_BOUNDARY_NONE,DM_BOUNDARY_NONE, dm );
    // PetscReal max[2]= {4,3};
    // DMPlexCreateBoxMesh(PETSC_COMM_WORLD, 2,
    //                     PETSC_FALSE, // use_simplices, if not then tesor_cells
    //                     elements, NULL, max, // sizes, and the min and max coords  
    //                     NULL, PETSC_TRUE,&dm);
    // user->dm = dm;
    /* Distribute mesh over processes */
    {
      PetscPartitioner part;
      DM               pdm = NULL; 

      ierr = DMPlexGetPartitioner(*dm, &part);
      ierr = PetscPartitionerSetFromOptions(part);
      ierr = DMPlexDistribute(*dm, 0, NULL, &pdm);
      if (pdm) {
        ierr = DMDestroy(dm);
        dm  = &pdm;
      }
    }
    ierr = DMLocalizeCoordinates(*dm); /* needed for periodic only */
    ierr = DMSetFromOptions(*dm);

     /* Make split wall labels - closer to uw feel must run with -dm_plex_separate_marker */
      const char *names[4] = {"wallBottom", "wallRight", "wallTop", "wallLeft"};
      PetscInt    ids[4]   = {1, 2, 3, 4};
      DMLabel     label;
      IS          is;
      PetscInt    f;

      for (f = 0; f < 4; ++f) {
        ierr = DMGetStratumIS(*dm, "marker", ids[f],  &is);
        if (!is) continue;
        ierr = DMCreateLabel(*dm, names[f]);
        ierr = DMGetLabel(*dm, names[f], &label);
        if (is) {
          ierr = DMLabelInsertIS(label, is, 1);
        }
        ierr = ISDestroy(&is);
      }
    
    SetupDiscretization(*dm, NULL);
    /* [O] The mesh is output to HDF5 using options */
    ierr = DMViewFromOptions(*dm, NULL, "-dm_view");
    ierr = DMSetApplicationContext(*dm, user);
    ierr = SNESSetDM(*snes, *dm); 
    // user->snes = snes;

    /* Calculates the index of the 'default' section, should improve performance */
    ierr = DMPlexCreateClosureIndex(*dm, NULL);
    /* Sets the fem routines for boundary, residual and Jacobian point wise operations */
    ierr = DMPlexSetSNESLocalFEM(*dm, user, user, user);

    // Simple xx;
    // PetscFunctionReturn(xx);
    return user;
}

int SolveModel(AppCtx *user) {
    // AppCtx *user = &petscApp;
    DM *dm = &user->dm;
    SNES *snes = &user->snes;
    
    PetscErrorCode ierr;
    Vec u;
    
    /* Get global vector */
    ierr = DMCreateGlobalVector(*dm, &u);
    /* Update SNES */
    ierr = SNESSetFromOptions(*snes);
    // ierr = SNESView(snes, PETSC_VIEWER_STDOUT_WORLD);

    /* Solve */
    {
      PetscErrorCode (*initialGuess[1])(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nf, PetscScalar *u, void* ctx) = {fn_x};
      Vec              lu;

      ierr = DMProjectFunction(*dm, 0.0, initialGuess, NULL, INSERT_VALUES, u);
      ierr = PetscObjectSetName((PetscObject) u, "Initial Solution");
      ierr = VecViewFromOptions(u, NULL, "-initial_vec_view");
      ierr = DMGetLocalVector(*dm, &lu);
      ierr = DMPlexInsertBoundaryValues(*dm, PETSC_TRUE, lu, 0.0, NULL, NULL, NULL);
      ierr = DMGlobalToLocalBegin(*dm, u, INSERT_VALUES, lu);
      ierr = DMGlobalToLocalEnd(*dm, u, INSERT_VALUES, lu);
      ierr = VecViewFromOptions(lu, NULL, "-local_vec_view");
      ierr = DMRestoreLocalVector(*dm, &lu);
      ierr = PetscObjectSetName((PetscObject) u, "Solution");
      ierr = SNESSolve(*snes, NULL, u);
      ierr = VecViewFromOptions(u, NULL, "-sol_vec_view");
    }
    {
	/* Output dm and temperature solution */
      PetscViewer h5viewer;
      PetscViewerHDF5Open(PETSC_COMM_WORLD, "sol.h5", FILE_MODE_WRITE, &h5viewer);
      PetscViewerSetFromOptions(h5viewer);
      DMView(*dm, h5viewer);
      PetscViewerDestroy(&h5viewer);
  
      PetscViewerHDF5Open(PETSC_COMM_WORLD, "sol.h5", FILE_MODE_APPEND, &h5viewer);
      PetscViewerSetFromOptions(h5viewer);
      VecView(u, h5viewer);
      PetscViewerDestroy(&h5viewer);
    }

    VecDestroy(&u);
    SNESDestroy(snes);
    DMDestroy(dm);
    // PetscFinalize();
    return 0;
#if 0
#endif
}


PetscErrorCode zero_scalar(PetscInt dim, PetscReal time, const PetscReal coords[],
                                  PetscInt Nf, PetscScalar *u, void *ctx)
{
  u[0] = 0.0;
  return 0;
}

PetscErrorCode fn_x(PetscInt dim, PetscReal time, const PetscReal coords[],
                                  PetscInt Nf, PetscScalar *u, void *ctx)
{
  u[0] = coords[0];
  return 0;
}

PetscErrorCode one_scalar(PetscInt dim, PetscReal time, const PetscReal coords[],
                                 PetscInt Nf, PetscScalar *u, void *ctx)
{
  u[0] = 1.0;
  return 0;
}


void f0_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[])
{
  AppCtx *self = &petscApp;
  
  /* create input */
  std::shared_ptr<IO_double> input = std::make_shared<IO_double>(dim, FunctionIO::Vector);
  
  memcpy(input->data(), x, dim*sizeof(double));
  
  const FunctionIO* output = debug_dynamic_cast<const FunctionIO*>(self->fn_source(input.get()));
  // f0[0] = 0.;
  f0[0] = output->at<double>(0);
}

void f1_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[])
{
  PetscInt d_i;
  for( d_i=0; d_i<dim; ++d_i ) f0[d_i] = u_x[d_i];
}

/* < \nabla v, \nabla u + {\nabla u}^T >
   This just gives \nabla u, give the perdiagonal for the transpose */
void g3_uu(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                  const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                  const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                  PetscReal t, PetscReal u_tShift, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar g3[])
{
  PetscInt d_i;

  for ( d_i=0; d_i<dim; ++d_i ) { g3[d_i*dim+d_i] = 1.0; }
}

PetscErrorCode SetupProblem(DM dm, PetscDS prob, AppCtx *user)
{
  const PetscInt          comp   = 0; /* scalar */
  PetscInt                ids[4] = {1,2,3,4};
  PetscErrorCode          ierr;

  PetscFunctionBeginUser;
  ierr = PetscDSSetResidual(prob, 0, f0_u, f1_u);
  ierr = PetscDSSetJacobian(prob, 0, 0, NULL, NULL,  NULL,  g3_uu);

  ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, "mrJones", "wallTop",
                            0, 0, NULL, /* field to constain and number of constained components */
                            (void (*)(void)) zero_scalar, 1, &ids[0], user);

  ierr = PetscDSAddBoundary(prob, DM_BC_ESSENTIAL, "mrsSkywalker", "wallBottom",
                            0, 0, NULL, /* field to constain and number of constained components */
                            (void (*)(void)) one_scalar, 1, &ids[0], user);

  PetscFunctionReturn(0);
}


AppCtx* SetupDiscretization(DM dm, AppCtx *user)
{
  DM              cdm = dm;
  PetscFE         fe;
  PetscQuadrature q;
  PetscDS         prob;
  PetscInt        dim;
  PetscErrorCode  ierr;

  PetscFunctionBeginUser;
  ierr = DMGetDimension(dm, &dim);
  /* Create finite element */
  ierr = PetscFECreateDefault(dm, dim, 1, PETSC_FALSE, "temperature_", PETSC_DEFAULT, &fe);
  ierr = PetscObjectSetName((PetscObject) fe, "temperature");
  ierr = PetscFEGetQuadrature(fe, &q);
  /* Set discretization and boundary conditions for each mesh */
  ierr = DMGetDS(dm, &prob);
  ierr = PetscDSSetDiscretization(prob, 0, (PetscObject) fe);
  ierr = SetupProblem(dm, prob, user);
  while (cdm) {
    ierr = DMSetDS(cdm, prob);
    ierr = DMGetCoarseDM(cdm, &cdm);
  }
  ierr = PetscFEDestroy(&fe);
  PetscFunctionReturn(user);
}

    // AppCtx petscApp;
    
// int main(int argc, char**argv) {
// int SetupModel(int x) {
