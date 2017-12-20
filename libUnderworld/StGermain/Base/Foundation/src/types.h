/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Foundation_types_h__
#define __StGermain_Base_Foundation_types_h__

   /*
    * #ifdef barrier for Bool type. Added 07-Sep-2004 -- Alan
    *
    * Change for Cecile so that she may procede with gLucifier work.
    * This was causing problems when compiling StGermain with gLucifier/vtk using g++.
    * It seems to suggest that this line was trying to redefine the int primitive.
    *
    * Grep searches reveal that neither gLucifier or vtk use the "Bool" word.
    *
    * TODO: Find a more elegant way to fix this issue.
    */
   #ifndef Bool   
   typedef enum Bool { False=0, True=1 } Bool;
   #endif

   typedef unsigned char stgByte;
   typedef void* (NewFunc)();
   typedef void (ConstructFunc)( void* );
   typedef Bool (IsSuperFunc)( const char* );

   typedef struct NewClass NewClass;
   typedef struct NewObject NewObject;

   typedef void* Pointer; /**< General Pointer. */
   typedef unsigned long Stg_Word; /**< A word on the cpu architecture */
   typedef unsigned long ArithPointer; /**< Used to do arithmetic on a pointer. Usually a long. */
   typedef unsigned long SizeT; /**< Size of memory allocations. */
   typedef unsigned int Index; /**< Generic array index. Positive, not constrained. */

   /* Generic types */
   /* Base types/classes */   
   typedef struct _Stg_Class Stg_Class;
      
   typedef struct _Stg_Object Stg_Object;
   typedef Stg_Object* Stg_ObjectPtr;
   typedef Index Stg_Object_Index;
   
   typedef struct _Stg_ObjectAdaptor Stg_ObjectAdaptor;

   typedef struct NamedObject_Register NamedObject_Register;
   typedef struct _Stg_ObjectList Stg_ObjectList;

   typedef char* Name; /**< A string used for names. */
   #define Name_Invalid 0 /**< An invalid or empty name. */
   
   /** Unique ID of a type/class. (runtime-proc persistance). 
    * Pointer used as ID. Must point to a valid string. */
   typedef char* Type;
   #define Type_Invalid 0 /**< An invalid or empty type. */

   /** Used to indicate whether an allocation is for a global variable or deleted locally.
    ** Currently used in NamedObject.
    **/
   typedef enum AllocationType {
      GLOBAL,
      NON_GLOBAL
   } AllocationType;


   /** Enum to tell the replace function whether to delete the replaced object or not. */
   typedef enum ReplacementOption {
      KEEP,
      DELETE
   } ReplacementOption;
   
#endif /* __StGermain_Base_Foundation_types_h__ */
