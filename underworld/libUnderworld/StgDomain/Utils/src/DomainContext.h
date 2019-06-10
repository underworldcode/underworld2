/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Utils_DomainContext_h__
#define __StgDomain_Utils_DomainContext_h__
   
   /* Textual name of this class */
   extern const Type DomainContext_Type;
   
   #define __DomainContext \
      /* General info */ \
      __AbstractContext \
      \
      /* Virtual info */ \
      \
      /* DomainContext info */ \
      FieldVariable_Register* fieldVariable_Register; \
      Dimension_Index         dim; \
      unsigned                verticalAxis; \

   struct DomainContext { __DomainContext };

   DomainContext* _DomainContext_DefaultNew( Name name );
   
   DomainContext* DomainContext_New( 
      Name        name,
      double      start,
      double      stop,
      MPI_Comm    communicator,
      Dictionary* dictionary );
   
   /** Creation implementation / Virtual constructor */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define DOMAINCONTEXT_DEFARGS \
      ABSTRACTCONTEXT_DEFARGS

   #define DOMAINCONTEXT_PASSARGS \
      ABSTRACTCONTEXT_PASSARGS

   DomainContext* _DomainContext_New( DOMAINCONTEXT_DEFARGS );
   
   /** Initialisation implementation */
   void _DomainContext_Init( DomainContext* self );

   /* Virtual Functions ------------------------------------------------------------------------------------------------*/

   void _DomainContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data );
   
   /* Stg_Class_Delete implementation */
   void _DomainContext_Delete( void* context );
   void _DomainContext_Destroy( void* component, void* data );
   
   /* Print implementation */
   void _DomainContext_Print( void* context, Stream* stream );
   
   void _DomainContext_SetDt( void* context, double dt ) ;

#endif /* __DomainContext_h__*/

