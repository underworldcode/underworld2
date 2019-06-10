/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Utils_types_h__
#define __StgDomain_Utils_types_h__

	typedef struct DofLayout			DofLayout;
	typedef struct FieldVariable			FieldVariable;
	typedef struct FieldVariable_Register		FieldVariable_Register;
	typedef struct DomainContext		DomainContext;
	typedef struct SobolGenerator            SobolGenerator;
	typedef struct TimeIntegrand            TimeIntegrand;
	typedef struct TimeIntegrator            TimeIntegrator;

	/* Degrees of Freedom */
	typedef unsigned int				Dof;
	typedef Index                       Bit_Index;
	
#endif /* __StgDomain_Utils_types_h__ */
