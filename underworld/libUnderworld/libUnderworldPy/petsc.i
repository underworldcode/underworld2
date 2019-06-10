/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */

%module (package="underworld.libUnderworld.libUnderworldPy") petsc

%{
/* Includes the header in the wrapper code */
/* #include <petscext.h> */
#include <petscsys.h>
#include <petscviewer.h>
#include <petscvec.h>

extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
}

%}


%inline %{

void SetVec(Vec vec, double alpha){
  VecSet( vec, alpha );
}

void OptionsInsertString(char * string){
    PetscOptionsInsertString(string);
}

void OptionsPrint(){
//     PetscOptionsView(PETSC_VIEWER_STDOUT_WORLD);
#if ( (PETSC_VERSION_MAJOR >= 3) && (PETSC_VERSION_MINOR >= 2 ) )
    PetscOptionsView(PETSC_VIEWER_STDOUT_WORLD);
#else
    PetscOptionsPrint(PETSC_STDOUT);
#endif
}

void OptionsClear(){
    PetscOptionsClear();
}

void OptionsSetValue(const char iname[],const char value[]){
    PetscOptionsSetValue(iname, value);
}

void OptionsClearValue(const char iname[]){
    /* option name must being with a - */
    PetscOptionsClearValue(iname);
}

void OptionsInsertFile(const char file[]){
    PetscOptionsInsertFile(PETSC_COMM_WORLD,file,PETSC_TRUE);
}

%}
