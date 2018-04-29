/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __petsc_layer_h__
#define __petsc_layer_h__

/* Define the AppCtx data structure */
typedef struct {
  DM dm;
  SNES snes;
  Fn::Function::func fn_source;
  
  PetscInt elements[3];
}AppCtx;

typedef int Simple;

/** Would be nice to get all the mesh coordinates, in python move it */


/** important python functions */
void theMadness(AppCtx* some,Fn::Function* fn);
AppCtx* SetupDiscretization(DM dm, AppCtx *user);
AppCtx* SetupModel(int x);
PetscErrorCode SolveModel(AppCtx*);
std::string DSGetFieldInfo(AppCtx *user);

/** standard functions */
PetscErrorCode SetupProblem(DM dm, PetscDS prob, AppCtx *user);
PetscErrorCode ProcessOptions(MPI_Comm comm, AppCtx *user);

PetscErrorCode zero_scalar(PetscInt dim, PetscReal time, const PetscReal coords[],
                                 PetscInt Nf, PetscScalar *u, void *ctx);
PetscErrorCode fn_x(PetscInt dim, PetscReal time, const PetscReal coords[],
                                 PetscInt Nf, PetscScalar *u, void *ctx);                                 
PetscErrorCode one_scalar(PetscInt dim, PetscReal time, const PetscReal coords[],
                                 PetscInt Nf, PetscScalar *u, void *ctx);
void f0_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[]);
void g3_uu(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                  const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                  const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                  PetscReal t, PetscReal u_tShift, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar g3[]);
void f1_u(PetscInt dim, PetscInt Nf, PetscInt NfAux,
                 const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
                 const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
                 PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[]);
#endif
