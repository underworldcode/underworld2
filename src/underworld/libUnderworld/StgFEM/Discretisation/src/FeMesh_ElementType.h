/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_FeMesh_ElementType_h__
#define __StgFEM_Discretisation_FeMesh_ElementType_h__

   /** Textual name of this class */
   extern const Type FeMesh_ElementType_Type;

   /** Virtual function types */

   /** Class contents */
   #define __FeMesh_ElementType \
      /* General info */ \
      __Mesh_HexType \
      \
      /* Virtual info */ \
      \
      /* FeMesh_ElementType info */ \
      double local[3];

   struct FeMesh_ElementType { __FeMesh_ElementType };

   /*--------------------------------------------------------------------------------------------------------------------------
   ** Constructors
   */

   #ifndef ZERO
   #define ZERO 0
   #endif

   #define FEMESH_ELEMENTTYPE_DEFARGS \
      MESH_HEXTYPE_DEFARGS

   #define FEMESH_ELEMENTTYPE_PASSARGS \
      MESH_HEXTYPE_PASSARGS

   FeMesh_ElementType* FeMesh_ElementType_New();
   FeMesh_ElementType* _FeMesh_ElementType_New( FEMESH_ELEMENTTYPE_DEFARGS );
   void _FeMesh_ElementType_Init( FeMesh_ElementType* self );

   /*--------------------------------------------------------------------------------------------------------------------------
   ** Virtual functions
   */

   void _FeMesh_ElementType_Delete( void* hexType );
   void _FeMesh_ElementType_Print( void* hexType, Stream* stream );

   void FeMesh_ElementType_Update( void* hexType );
   Bool FeMesh_ElementType_ElementHasPoint( void* hexType, unsigned elInd, double* point, MeshTopology_Dim* dim, unsigned* ind );
   Bool FeMesh_ElementType_ElementHasPoint_ForIrregular( void* hexType, unsigned elInd, double* point, MeshTopology_Dim* dim, unsigned* ind );
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Public functions
   */

   /*--------------------------------------------------------------------------------------------------------------------------
   ** Private Member functions
   */
   void FeMesh_ElementTypeGetLocal( FeMesh_ElementType* self, double* xi );

#endif /* __StgFEM_Discretisation_FeMesh_ElementType_h__ */

