/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef pcu_types_h
#define pcu_types_h

typedef struct pcu_source_t     pcu_source_t;
typedef struct pcu_test_t       pcu_test_t;
typedef struct pcu_suite_t      pcu_suite_t;
typedef struct pcu_listener_t   pcu_listener_t;
typedef struct pcu_textoutput_t pcu_textoutput_t;
typedef struct pcu_runner_t     pcu_runner_t;

typedef void (pcu_testfunc_t)   ( void* data );
typedef void (pcu_fixture_t)    ( void* data );
typedef void (pcu_suiteentry_t) ( pcu_listener_t* lsnr, pcu_suite_t* suite );
typedef void (pcu_testentry_t)  ( pcu_listener_t* lsnr, pcu_test_t* test );
typedef void (pcu_checkentry_t) ( pcu_listener_t* lsnr, pcu_source_t* src );
typedef void (pcu_runbegin_t)   ( pcu_listener_t* lsnr, int nsuites );
typedef void (pcu_runend_t)     ( pcu_listener_t* lsnr, int nsuites, int totalPasses, int totalTests );

#endif
