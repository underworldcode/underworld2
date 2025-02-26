/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_ToolboxesManager_h__
#define __StGermain_Base_Extensibility_ToolboxesManager_h__
   
   /* Textual name of this class */
   extern const Type ToolboxesManager_Type;

   /* Toolboxes info */
   #define __ToolboxesManager \
      /* General info */ \
      __ModulesManager \
      \
      /* Virtual info */ \
      \
      /* Toolboxes info */ \
      Stg_ObjectList *initTB; /* list of initialised toolboxes */ \
      int*           argc; \
      char***        argv;
      
   struct ToolboxesManager { __ToolboxesManager };
   
    /** Define a global list of plugin directories*/
     extern Stg_ObjectList*  pluginDirectories;   

   /* Create a new Toolboxes */
   ToolboxesManager* ToolboxesManager_New( int* argc, char*** argv );
   
   /* Creation implementation / Virtual constructor */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define TOOLBOXESMANAGER_DEFARGS \
      MODULESMANAGER_DEFARGS, \
      int*    argc, \
      char*** argv

   #define TOOLBOXESMANAGER_PASSARGS \
      MODULESMANAGER_PASSARGS, \
      argc, \
      argv

   ToolboxesManager* _ToolboxesManager_New( TOOLBOXESMANAGER_DEFARGS );

   /* Initialisation implementation */
   void _ToolboxesManager_Init( void* toolboxesManager, int* argc, char*** argv );
   
   /* Stg_Class_Delete implementation */
   void _ToolboxesManager_Delete( void* toolboxesManager );
   
   /* Print implementation */
   void _ToolboxesManager_Print( void* toolboxesManager, Stream* stream );
   
   /** Get the toolbox list from the dictionary */
   Dictionary_Entry_Value* _ToolboxesManager_GetToolboxesList( void* toolboxesManager, void* dictionary );
   
   /** Exactly what to do to load the toolbox */
   Bool _ToolboxesManager_LoadToolbox( void* toolboxesManager, Module* toolbox );

   /** Exactly what to do to unload the toolbox */
   Bool _ToolboxesManager_UnloadToolbox( void* toolboxesManager, Module* toolbox );

   Bool _ToolboxesManager_CheckContext( void* toolboxesManager, Dictionary_Entry_Value* modulesVal, unsigned int entry_I, Name contextName );

   Name _ToolboxesManager_GetModuleName( void* toolboxesManager, Dictionary_Entry_Value* moduleVal, unsigned int entry_I );

   #define ToolboxesManager_Submit ModulesManager_Submit

   /** This exists to handle the case where a module is linked into a binary and the user attempts to module load the module too. 
      Its expected at modules will check to see if they have been inited already before doing initialisation work in the init 
      function. */
   Bool ToolboxesManager_IsInitialised( void* toolboxesManager, char* label );

#endif /* __StGermain_Base_Extensibility_ToolboxesManager_h__ */

