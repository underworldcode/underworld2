/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StgFEM_Discretisaton_PETScErrorChecking_h__
#define __StgFEM_Discretisaton_PETScErrorChecking_h__
	
	#define CheckPETScError( errorFlag ) \
		do {								\
			if ( ! errorFlag ) { 				\
				Stream* myNewErrorStream = Journal_Register( Error_Type, CURR_MODULE_NAME );	\
				CHKERRCONTINUE(errorFlag); 			\
				Journal_Firewall( ! errorFlag , myNewErrorStream, "Function %s failed.\n", __func__ );	\
			}						\
		} while (0)

#endif	/* __StgFEM_Discretisaton_PETScErrorChecking_h__ */
