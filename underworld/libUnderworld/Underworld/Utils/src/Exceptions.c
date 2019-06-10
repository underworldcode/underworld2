#include <stdio.h>
#include <fenv.h>

#include "Exceptions.h"

int Underworld_feclearexcept() {
  // request to clear all floating-point exceptions
  return feclearexcept(FE_ALL_EXCEPT);
}

int Underworld_fetestexcept() {
  /**
  Test if floating-point exceptions occured.
  This function will return 1 if an exception is currently in the
  floating-point environment; 0 otherwise. It will clear the environment also
  See http://en.cppreference.com/w/c/numeric/fenv.
  **/

  int exception_raised = 0;
  // exceptions to test
  const int fe_problem = FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID | FE_DIVBYZERO ;

  // test
  if(fetestexcept(fe_problem)) {
    exception_raised = 1;
  }

  // comment out possible reporting
  // if(fetestexcept(FE_DIVBYZERO))     printf(" FE_DIVBYZERO");
  // if(fetestexcept(FE_INVALID))       printf(" FE_INVALID");
  // if(fetestexcept(FE_OVERFLOW))      printf(" FE_OVERFLOW");
  // if(fetestexcept(FE_UNDERFLOW))     printf(" FE_UNDERFLOW");

  // clear floating-point exceptions environment
  feclearexcept(FE_ALL_EXCEPT);
  return exception_raised;
}
