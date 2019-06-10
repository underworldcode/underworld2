/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_Context_h__
#define __StgFEM_SLE_SystemSetup_Context_h__
   
   /* Textual name of this class */
   extern const Type FiniteElementContext_Type;

   extern const Name FiniteElementContext_EP_CalcDt;
   
   #define __FiniteElementContext \
      /* General info */ \
      __DomainContext \
      \
      /* Virtual info */ \
      \
      /* FiniteElementContext info */ \
      SystemLinearEquationList* slEquations; \
      double                    prevTimestepDt; \
      Bool                      limitTimeStepIncreaseRate; \
      double                    maxTimeStepIncreasePercentage; \
      double                    maxTimeStepSize; \
      double                    dtGran; \
      EntryPoint*               calcDtEP;

   struct FiniteElementContext { __FiniteElementContext };

   /* Constructors ----------------------------------------------------------------------------------------------------------*/
   
   /** Constructor */
   void* FiniteElementContext_DefaultNew( Name name );

   FiniteElementContext* FiniteElementContext_New( 
      Name        name,
      double      start,
      double      stop,
      MPI_Comm    communicator,
      Dictionary* dictionary );
   
   /** Creation implementation / Virtual constructor */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define FINITEELEMENTCONTEXT_DEFARGS \
      DOMAINCONTEXT_DEFARGS

   #define FINITEELEMENTCONTEXT_PASSARGS \
      DOMAINCONTEXT_PASSARGS

   FiniteElementContext* _FiniteElementContext_New( FINITEELEMENTCONTEXT_DEFARGS );
   
   /** Initialisation implementation */
   void  _FiniteElementContext_Init( FiniteElementContext* self );

   /* Virtual Functions -----------------------------------------------------------------------------------------------------*/
   
   /* Stg_Class_Delete implementation */
   void _FiniteElementContext_Delete( void* context );
   
   /* Destroy implementation */
   void _FiniteElementContext_Destroy( void* component, void* data );

   /* Print implementation */
   void _FiniteElementContext_Print( void* context, Stream* stream );   
   
   /* Construct EntryPoint hook */
   void _FiniteElementContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data );
   
   /* Build EntryPoint hook */
   void _FiniteElementContext_Build( void* context );
   
   /* Initialise EntryPoint hook */
   void _FiniteElementContext_Initialise( void* context );
   
   /* Solve EntryPoint hook */
   void _FiniteElementContext_Solve( void* context );

   /* PostSolve EntryPoint hook: for things such as updating the Dt */
   void _FiniteElementContext_PostSolve( void* context ) ;
   
   /* Dt related functions */
   /** Function to assign the dt mediated by the abstract context to use for time integration
      in the current step - usually just the last timestep's dt */
   void _FiniteElementContext_SetDt( void* context, double dt );
   /** Function to pass the last-calculated dt back to the context. If loading from checkpoint,
      will load the dt from timeInfo file */
   double _FiniteElementContext_GetDt( void* context );

   /** Function to calculate the new dt based on the solution just obtained, which
     * will be used next timestep. Calls as entry point of a corresponding name,
     * which each SLE can add a hook to based on its own criterion */
   double FiniteElementContext_CalcNewDt( void* context ) ;

   /* Public functions ------------------------------------------------------------------------------------------------------*/
   
   void FiniteElementContext_AddSLE_Func( void* context, void* sle );

   #define FiniteElementContext_AddSLE_Macro( self, sle ) \
      Stg_ObjectList_Append( (self)->slEquations, sle )

   #ifdef MACRO_AS_FUNC
      #define FiniteElementContext_AddSLE FiniteElementContext_AddSLE_Func
   #else
      #define FiniteElementContext_AddSLE FiniteElementContext_AddSLE_Macro
   #endif
   #define AddSLE FiniteElementContext_AddSLE
   
   SystemLinearEquations* FiniteElementContext_GetSLE_Func( void* context, Name sleName );
   #define FiniteElementContext_GetSLE_Macro( self, sleName ) \
      ((SystemLinearEquations*)Stg_ObjectList_Get( (self)->slEquations, sleName ))
   #ifdef MACRO_AS_FUNC
      #define FiniteElementContext_GetSLE FiniteElementContext_GetSLE_Func
   #else
      #define FiniteElementContext_GetSLE FiniteElementContext_GetSLE_Macro
   #endif
   #define GetSLE FiniteElementContext_GetSLE

   /* Private functions -----------------------------------------------------------------------------------------------------*/

#endif /* __StgFEM_SLE_SystemSetup_Context_h__ */

