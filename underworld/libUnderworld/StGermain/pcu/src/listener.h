/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef pcu_listener_h
#define pcu_listener_h

struct pcu_listener_t {
      pcu_suiteentry_t* suitebegin;
      pcu_suiteentry_t* suiteend;
      pcu_testentry_t*  testbegin;
      pcu_testentry_t*  testend;
      pcu_checkentry_t* checkdone;
      pcu_runbegin_t*   runbegin;
      pcu_runend_t*     runend;
      void* data;
      char* projectName;
      int printdocs;
};

#endif
