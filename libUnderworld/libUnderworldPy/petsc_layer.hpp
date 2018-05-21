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
typedef enum {RUN_ANALYTIC_SIMPLE, RUN_ANALYTIC_FREE_SLIP, RUN_CONVECTION} RunType;

typedef struct {
  DM dm;
  SNES snes;
  MatNullSpace nullSpace;
  Fn::Function::func fn_forceTerm;
  Fn::Function::func fn_viscosity;
  Fn::Constant *prob_fn[2];
  Fn::Constant *aux_fn[10];
  std::shared_ptr<IO_double> input;
  char filename[2048];
  RunType runType;
  PetscInt elements[3];
  PetscBool reload;
}AppCtx;

typedef int Simple;

/** Would be nice to get all the mesh coordinates, in python move it */

void prob_fnSetter( AppCtx* self, int id, Fn::Function* fn );
void StokesModel_SetViscosityFn(AppCtx* self,Fn::Function* fn); // not enabled in c yet
void StokesModel_SetForceTerm(AppCtx*,Fn::Function*);
int StokesModel_Solve(AppCtx*);
AppCtx* StokesModel_Setup(char*);
AppCtx* PoissonModel_Setup(char*);
int PoissonModel_Solve(AppCtx* user);
//std::string DSGetFieldInfo(AppCtx *user);
#endif
