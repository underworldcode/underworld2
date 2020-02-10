/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef pcu_runner_h
#define pcu_runner_h

typedef enum { PCU_RUNNER_FAILS, PCU_RUNNER_ALLPASS } PCU_Runner_Status;

extern int PCU_PRINT_DOCS;

void pcu_runner_init( int argc, char* argv[] );
void pcu_runner_finalise();
PCU_Runner_Status pcu_runner_run();
void _pcu_runner_addSuite( const char* name,
			   void (initfunc)( pcu_suite_t* ),
            const char* moduleDir );
void pcu_runner_associateListener( pcu_listener_t* lsnr );

#define pcu_runner_addSuite( suite, initfunc, moduleDir )  \
   _pcu_runner_addSuite( #suite, initfunc, #moduleDir )

#endif
