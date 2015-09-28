/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Utils_RBFManager_h__
#define __Underworld_Utils_RBFManager_h__

      /* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
      extern const Type RBFManager_Type;

      #define __RBFParticle \
         __GlobalParticle             \
         double density; 

         typedef struct { __RBFParticle } RBFParticle;

        /* RBFManager information */
        #define __RBFManager                                                                                    \
                /* Macro defining parent goes here - This means you can cast this class as its parent */      \
                __Stg_Component                                                                               \
                /* Virtual Info */                                                                            \
                PICelleratorContext*        context;                                                          \
                double                      particleMass;                                                     \
                Swarm*                      rbfParticleSwarm;                                                 \
                SwarmVariable*              particleDensity;                                                  \
                SwarmVariable*              particleCoordVariable;                                            \
                Dimension_Index             RBFdim;                                                           \
                double                      particleSupportRadius;

        struct RBFManager { __RBFManager };

        /*---------------------------------------------------------------------------------------------------------------------
        ** Constructors
        */

        #ifndef ZERO
        #define ZERO 0
        #endif

        #define RBFMANAGER_DEFARGS \
                STG_COMPONENT_DEFARGS

        #define RBFMANAGER_PASSARGS \
                STG_COMPONENT_PASSARGS

        RBFManager* _RBFManager_New(  RBFMANAGER_DEFARGS  );

        /* Stg_Class_Delete RBFManager implementation */
        void _RBFManager_Delete( void* RBFManager );
        void _RBFManager_Print( void* RBFManager, Stream* stream );
        #define RBFManager_Copy( self ) \
                (RBFManager*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
        #define RBFManager_DeepCopy( self ) \
                (RBFManager*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
        void* _RBFManager_Copy( void* RBFManager, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

        void* _RBFManager_DefaultNew( Name name ) ;
        void _RBFManager_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data ) ;
        void _RBFManager_Build( void* RBFManager, void* data ) ;
        void _RBFManager_Initialise( void* RBFManager, void* data ) ;
        void _RBFManager_Execute( void* RBFManager, void* data );
        void _RBFManager_Destroy( void* RBFManager, void* data ) ;
        void _RBFManager_Init(
            void*                  RBFManager,
            PICelleratorContext*   context,
            Dimension_Index        RBFdim,
            double                 particleSupportRadius,
            Swarm*                 rbfParticleSwarm );
        
        void RBFManager_CalculateParticleDensities( void* RBFManager );
        double RBFManager_2D_CubicSplineKernel( double distance, double h );
        double RBFManager_2D_CubicSplineDerivative( double distance, double projected_distance, double h );
        double RBFManager_3D_CubicSplineKernel( double distance, double h );
        double RBFManager_3D_CubicSplineDerivative( double distance, double projected_distance, double h );

        /*---------------------------------------------------------------------------------------------------------------------
        ** Private functions
        */

        /*---------------------------------------------------------------------------------------------------------------------
        ** Entry Point Hooks
        */

        /*---------------------------------------------------------------------------------------------------------------------
        ** Public functions
        */

#endif

