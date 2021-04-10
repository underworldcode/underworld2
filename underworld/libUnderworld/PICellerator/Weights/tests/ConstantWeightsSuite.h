/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <stdio.h>
#include <stdlib.h>

#include "pcu/pcu.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>

#ifndef PICellerator_ConstantWeightsSuite_h
#define PICellerator_ConstantWeightsSuite_h

void constantFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value );
void linearFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value );
void shapeFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value );
void polynomialFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value );
void quadraticFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value );
void exponentialFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value );
void exponentialInterface( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value );
void circleInterface( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value );

void WeightsSuite_TestElementIntegral( PICelleratorContext* context, Name funcName, Index count,
	 double meanTolerance, double stdDevTolerance, double expectedMean, double expectedStdDev );

void ConstantWeightsSuite( pcu_suite_t* suite );

#endif
