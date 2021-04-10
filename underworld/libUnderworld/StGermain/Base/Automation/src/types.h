/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Automation_types_h__
#define __StGermain_Base_Automation_types_h__

typedef struct stgComponent stgComponent;
typedef struct Comm Comm;

   /* types/classes */
   typedef struct Stg_Component                 Stg_Component;
   typedef struct Stg_ComponentRegisterElement  Stg_ComponentRegisterElement;
   typedef struct Stg_ComponentRegister         Stg_ComponentRegister;
   typedef struct LiveComponentRegister         LiveComponentRegister;
   typedef struct Stg_ComponentFactory          Stg_ComponentFactory;

   typedef struct HierarchyTable                HierarchyTable;
   typedef struct Stg_CallGraph                 Stg_CallGraph;

   typedef void* (Stg_Component_Constructor) ( Name );
   typedef void (Stg_Component_LoadFromDict) ( void*, Dictionary*, Dictionary*, Stg_ObjectList* );


#endif /* __StGermain_Base_Automation_types_h__ */
