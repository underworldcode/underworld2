/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef pcu_source_h
#define pcu_source_h

struct pcu_source_t {
      int result;
      char* type;
      char* file;
      int line;
      char* expr;
      char* msg;
      pcu_test_t* test;
      int rank;
      pcu_source_t* next;
};

void pcu_source_init( pcu_source_t* src );
pcu_source_t* pcu_source_create( int result, const char* type,
				 const char* file, int line,
				 const char* expr, const char* msg,
				 pcu_test_t* test );
int pcu_source_getPackLen( pcu_source_t* src );
void pcu_source_pack( pcu_source_t* src, void* buf );
void pcu_source_unpack( pcu_source_t* src, void* buf );
void pcu_source_clear( pcu_source_t* src );

#endif
