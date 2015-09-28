/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_Context_h__
#define __Underworld_Utils_Context_h__
   
   /* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
   extern const Type UnderworldContext_Type;
   
   #define __UnderworldContext \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __PICelleratorContext \
      /* Virtual functions go here */ \

   struct UnderworldContext { __UnderworldContext };

   /* Constructors ----------------------------------------------------------------------------------------------------*/
   
   /** Constructor */
   void* _UnderworldContext_DefaultNew( Name name );
   
   UnderworldContext* UnderworldContext_New( 
      Name        name,
      double      start,
      double      stop,
      MPI_Comm    communicator,
      Dictionary* dictionary );

   
   /** Private Constructor: This will accept all the virtual functions for this class as arguments. */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define UNDERWORLDCONTEXT_DEFARGS \
      PICELLERATORCONTEXT_DEFARGS

   #define UNDERWORLDCONTEXT_PASSARGS \
      PICELLERATORCONTEXT_PASSARGS

   UnderworldContext* _UnderworldContext_New( UNDERWORLDCONTEXT_DEFARGS );
   
   /** Initialisation implementation */
   void _UnderworldContext_Init( UnderworldContext* self );

   /* Virtual Functions -----------------------------------------------------------------------------------------------*/

   void _UnderworldContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data );

   /* Stg_Class_Delete implementation */
   void _UnderworldContext_Delete( void* context );
   
   /* Destroy implmentation  */
   void _UnderworldContext_Destroy( void* component, void* data );

   /* Print implementation */
   void _UnderworldContext_Print( void* context, Stream* stream );
   
   /* Public functions -----------------------------------------------------------------------------------------------*/
   void _UnderworldContext_GenerateOptionsFile( UnderworldContext* context );
   
#endif

