/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_ProvidedSystems_StokesFlow_types_h__
#define __StgFEM_SLE_ProvidedSystems_StokesFlow_types_h__
	
	/* types/classes */
	typedef struct Stokes_SLE                   Stokes_SLE;
	typedef struct Stokes_SLE_UzawaSolver       Stokes_SLE_UzawaSolver;
	typedef struct Stokes_SLE_PenaltySolver     Stokes_SLE_PenaltySolver;
	typedef struct UzawaPreconditionerTerm      UzawaPreconditionerTerm;

	/* output streams: initialised in Init() */
	extern Stream* StgFEM_SLE_ProvidedSystems_StokesFlow_Debug;

#endif /* __StgFEM_SLE_ProvidedSystems_StokesFlow_types_h__ */
