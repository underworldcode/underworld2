/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Utils_FieldVariable_h__
#define __StgDomain_Utils_FieldVariable_h__

   /* Textual name of this class */
   extern const Type FieldVariable_Type;
   
   /*
    * Enumerated type to allow the user to know the result of an effort to interpolate the value of a
    * FieldVariable at a particular point.
    * Note that the order in the enum has been chosen for backward-compatibility for functions that now
    * return one of these instead of a Bool: 0 (false) meant other proc, 1 (true) meant local
    */
   typedef enum InterpolationResult {
      OTHER_PROC = 0, /* The value wasn't interpolated, as the requested co-ord is on another processor */
      LOCAL = 1, /* The value was successfully interpolated, in local space. */
      SHADOW, /* The value was successfully interpolated, in shadow space. */
      OUTSIDE_GLOBAL /* The value wasn't interpolated, as the requested co-ord is outside the entire field */
   } InterpolationResult;
   
   /* Mapping to convert an interpolation result to a string for debug purposes */
   extern const char* InterpolationResultToStringMap[4];
   
   typedef InterpolationResult (FieldVariable_InterpolateValueAtFunction)
      ( void* fieldVariable, Coord coord, double* value );

   typedef double (FieldVariable_GetValueFunction) ( void* fieldVariable );
   typedef void (FieldVariable_CacheValuesFunction) ( void* fieldVariable );
   typedef void (FieldVariable_GetCoordFunction) ( void* fieldVariable, Coord min, Coord max );
   
   /** FieldVariable contents */
   #define __FieldVariable \
      /* General info */ \
      __Stg_Component \
      \
      DomainContext*                            context; \
      /* Virtual info */ \
      FieldVariable_InterpolateValueAtFunction* _interpolateValueAt; \
      FieldVariable_GetValueFunction*           _getMinGlobalFieldMagnitude; \
      FieldVariable_GetValueFunction*           _getMaxGlobalFieldMagnitude; \
      FieldVariable_CacheValuesFunction*        _cacheMinMaxGlobalFieldMagnitude; \
      FieldVariable_GetCoordFunction*           _getMinAndMaxLocalCoords; \
      FieldVariable_GetCoordFunction*           _getMinAndMaxGlobalCoords; \
      \
      /* Member info */ \
      ExtensionManager*                         extensionMgr; \
      Index                                     fieldComponentCount; \
      Dimension_Index                           dim; \
      MPI_Comm                                  communicator; \
      FieldVariable_Register*                   fieldVariable_Register; \
      Bool                                      isCheckpointedAndReloaded; \
      /* Bool to determine whether the field variable should be saved on the saveData entry point */ \
      Bool                                      isSavedData; \
      int                                       cachedTimestep; \
      double                                    magnitudeMin; \
      double                                    magnitudeMax; \
      char*                                     o_units; /* output units string */ \
      Bool                                      useCacheMaxMin;

   struct FieldVariable { __FieldVariable };   

   /* General Virtual Functions */
   #define FieldVariable_Copy( self ) \
      (FieldVariable*)Stg_Class_Copy( self, NULL, False, NULL, NULL )

   /* Creation implementation */
   FieldVariable* _FieldVariable_DefaultNew( Name name );

   FieldVariable* FieldVariable_New( 
      Name                    name,
      DomainContext*          context,
      Index                   fieldComponentCount,
      Dimension_Index         dim,
      Bool                    isCheckpointedAndReloaded,
      MPI_Comm                communicator,
      FieldVariable_Register* fieldVariable_Register ) ;
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define FIELDVARIABLE_DEFARGS \
      STG_COMPONENT_DEFARGS, \
      FieldVariable_InterpolateValueAtFunction* _interpolateValueAt, \
      FieldVariable_GetValueFunction*           _getMinGlobalFieldMagnitude, \
      FieldVariable_GetValueFunction*           _getMaxGlobalFieldMagnitude, \
      FieldVariable_CacheValuesFunction*        _cacheMinMaxGlobalFieldMagnitude, \
      FieldVariable_GetCoordFunction*           _getMinAndMaxLocalCoords, \
      FieldVariable_GetCoordFunction*           _getMinAndMaxGlobalCoords

   #define FIELDVARIABLE_PASSARGS \
      STG_COMPONENT_PASSARGS, \
      _interpolateValueAt,         \
      _getMinGlobalFieldMagnitude, \
      _getMaxGlobalFieldMagnitude, \
      _cacheMinMaxGlobalFieldMagnitude, \
      _getMinAndMaxLocalCoords,    \
      _getMinAndMaxGlobalCoords  

   FieldVariable* _FieldVariable_New( FIELDVARIABLE_DEFARGS );

   /** Member initialisation implementation */
   void _FieldVariable_Init( 
      FieldVariable*          self, 
      DomainContext*          context,
      Index                   fieldComponentCount, 
      Dimension_Index         dim,
      Bool                    isCheckpointedAndReloaded,
      char*                   o_units,
      MPI_Comm                communicator, 
      FieldVariable_Register* fV_Register,
      Bool                    useCacheMaxMin ) ;
   
   void _FieldVariable_Delete( void* fieldVariable ) ;

   void _FieldVariable_Print( void* _fieldVariable, Stream* stream ) ;

   void _FieldVariable_AssignFromXML( void* fieldVariable, Stg_ComponentFactory* cf, void* data ) ;

   void _FieldVariable_Build( void* fieldVariable, void* data ) ;

   void _FieldVariable_Execute( void* fieldVariable, void* data ) ;

   void _FieldVariable_Destroy( void* fieldVariable, void* data ) ;

   void _FieldVariable_Initialise( void* fieldVariable, void* data ) ;
   
   /* Copy */
   void* _FieldVariable_Copy( void* fieldVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
   
   /* 
    * Interface to determine the value of the field at a specific co-ordinate in space.
    * Return status determines whether operation was successful - i.e. whether the given
    * coordinate was valid, and within the processor's local domain.
    */
   InterpolationResult FieldVariable_InterpolateValueAt( void* fieldVariable, Coord coord, double* value );

   /* Interface for finding the minimum field value */
   double FieldVariable_GetMinGlobalFieldMagnitude( void* fieldVariable );

   /* Interface for finding the maximum field value */
   double FieldVariable_GetMaxGlobalFieldMagnitude( void* fieldVariable );

   /* Interface for finding the minimum field value, forcing usage of cached value */
   double FieldVariable_GetCachedMinGlobalFieldMagnitude( void* fieldVariable );

   /* Interface for finding the maximum field value, forcing usage of cached value */
   double FieldVariable_GetCachedMaxGlobalFieldMagnitude( void* fieldVariable );

   /* Get field magnitude values and save them */
   void FieldVariable_CacheMinMaxGlobalFieldMagnitude( void* fieldVariable );

   /* Interface for finding the boundary of the spatial region this processor is holding info on */
   void FieldVariable_GetMinAndMaxLocalCoords( void* fieldVariable, Coord min, Coord max ) ;

   void FieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord min, Coord max ) ;

   void _FieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord min, Coord max ) ;

   /* 
    * Interface for setting the units string. If the string already exists nothing happens, else
    * an array of chars is allocated and the 2nd arg is copied into the array.
    */
   void FieldVariable_SetUnits( void* fieldVariable, char* o_units );

   /* Return a pointer to the fieldVariable units string: either a valid char* address or NULL*/
   char* FieldVariable_GetUnits( void* fieldVariable );

#endif /* __StgDomain_Utils_FieldVariable_h__ */

