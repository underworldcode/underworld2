/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_types_h__
#define __StgFEM_Discretisation_types_h__
   
   /* FE types/classes */
   typedef struct FeMesh_Algorithms         FeMesh_Algorithms;
   typedef struct FeMesh_ElementType        FeMesh_ElementType;
   typedef struct ElementType               ElementType;
   typedef struct ElementType_Register      ElementType_Register;
   typedef struct ConstantElementType       ConstantElementType;
   typedef struct LinearElementType         LinearElementType;
   typedef struct BilinearElementType       BilinearElementType;
   typedef struct TrilinearElementType      TrilinearElementType;
   typedef struct Biquadratic               Biquadratic;
   typedef struct LinearTriangleElementType LinearTriangleElementType;
   typedef struct dQ12DElType               dQ12DElType;
   typedef struct dQ13DElType               dQ13DElType;
   typedef struct FiniteElement_Element     FiniteElement_Element;
   typedef struct FeMesh                    FeMesh;
   typedef struct C0Generator               C0Generator;
   typedef struct C2Generator               C2Generator;
   typedef struct P1Generator               P1Generator;
   typedef struct Inner2DGenerator          Inner2DGenerator;
   typedef struct dQ1Generator              dQ1Generator;
   typedef struct LinkedDofInfo             LinkedDofInfo;
   typedef struct FeEquationNumber          FeEquationNumber;
   typedef struct FeVariable                FeVariable;
   typedef struct Triquadratic              Triquadratic;
   typedef struct BilinearInnerElType       BilinearInnerElType;
   typedef struct TrilinearInnerElType      TrilinearInnerElType;
   typedef struct IrregularMeshGaussLayout  IrregularMeshGaussLayout;

   /* Types for clarity */
   typedef Index Iteration_Index;

   /* Stuff for FeEquationNumber class */
   typedef Node_GlobalIndex     Node_RemappedGlobalIndex;
   typedef Index                RemappedNodeInfo_Index;
   typedef int                  Dof_EquationNumber;
   typedef Dof_EquationNumber*  Dof_EquationNumbers;
   typedef Dof_EquationNumbers* Dof_EquationNumbersList;

   /* Basic types for ElementRegister */
   typedef Index ElementType_Index;

   /* Output streams: initialised in Init() */
   /* 
    * Let's give this library responsiblity for setting up the StgFEM stream, 
    * as it's the first to be compiled...  
    */
   extern Stream* StgFEM_Debug;
   extern Stream* StgFEM_Warning;

   extern Stream* StgFEM_Discretisation_Debug;

#endif /* __StgFEM_Discretisation_types_h__ */
