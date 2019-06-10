/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef pcu_test_h
#define pcu_test_h

struct pcu_test_t {
	char* name;
 	pcu_suite_t* suite;
 	pcu_testfunc_t* func;
 	pcu_test_t* next;

	int globalresult;
	int nsrcs;
	pcu_source_t* srcs;
	pcu_source_t* lastSrc;
	char* docString;
};

/** Associate a documentation string with the current test (must be called from within a test function) */
void pcu_docstring( const char* docString );

/** Associate a documentation string with a test. Expected this will be called from function above */
void pcu_test_doc( pcu_test_t* test, const char* docString );

void pcu_test_run( pcu_test_t* test, pcu_listener_t** lsnrs, int nlsnrs );
pcu_source_t* pcu_test_addSource( pcu_test_t* test, pcu_source_t* src );

#endif
