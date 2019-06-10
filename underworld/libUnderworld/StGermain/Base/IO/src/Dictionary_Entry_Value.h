/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_Dictionary_Entry_Value_h__
#define __StGermain_Base_IO_Dictionary_Entry_Value_h__

   /** The available formats which a dictionary can be written to output. */
   typedef enum _Encoding { Default, RawASCII, RawBinary } Encoding;

   /** \def __Dictionary_Entry_Value_List See Dictionary_Entry_Value_List */
   #define __Dictionary_Entry_Value_List \
      Dictionary_Entry_Value* first; \
      Dictionary_Entry_Value* last; \
      Index                   count; \
      Encoding                encoding;

   struct _Dictionary_Entry_Value_List { __Dictionary_Entry_Value_List };

   /* TODO */
   #if 0   
   #define __Dictionary_Entry_Value_Validator \
      union minVal { \
         unsigned int asUnsignedInt; \
         double asDouble; \
      }; \
      union maxVal { \
         unsigned int asUnsignedInt; \
         double asDouble; \
      }; \
       \
      Dictionary_Entry_Value_Validator type; \
      SetType* set; \
   struct _Dictionary_Entry_Value_Validator { __Dictionary_Entry_Value_Validator };
   #endif

   typedef union {
      char*                        typeString;
      double                       typeDouble;
      unsigned int                 typeUnsignedInt;
      int                          typeInt;
      unsigned long                typeUnsignedLong;
      Bool                         typeBool;
      Dictionary*                  typeStruct;
      Dictionary_Entry_Value_List* typeList;
      void*                        typeVoid; /* proposed... not in use yet */ \
   } Dictionary_Entry_Value_AsType;

   /** \def __Dictionary_Entry_Value See Dictionary_Entry_Value */
   #define __Dictionary_Entry_Value \
      Dictionary_Entry_Value_AsType as; \
      Dictionary_Entry_Value_Type   type; /**< The member of the union currently in use. */ \
      Dictionary_Entry_Value*       next; \

   /* TODO Dictionary_Entry_Value_Validator* validator; */
   struct _Dictionary_Entry_Value { __Dictionary_Entry_Value };
   
   /** Create a value from an unsigned int */
   Dictionary_Entry_Value* Dictionary_Entry_Value_FromUnsignedInt( unsigned int value );
   void Dictionary_Entry_Value_InitFromUnsignedInt( Dictionary_Entry_Value* self, unsigned int value );
   
   /** Create a value from an signed int */
   Dictionary_Entry_Value* Dictionary_Entry_Value_FromInt( int value );
   void Dictionary_Entry_Value_InitFromInt( Dictionary_Entry_Value* self, int value );
   
   /** Create a value from a double */
   Dictionary_Entry_Value* Dictionary_Entry_Value_FromDouble( double value );
   void Dictionary_Entry_Value_InitFromDouble( Dictionary_Entry_Value* self, double value );

   /** Create a value from an unsigned long */
   Dictionary_Entry_Value* Dictionary_Entry_Value_FromUnsignedLong( unsigned long value );
   void Dictionary_Entry_Value_InitFromUnsignedLong( Dictionary_Entry_Value* self, unsigned long value );
   
   /** Create a value from a string */
   Dictionary_Entry_Value* Dictionary_Entry_Value_FromString( const char* const value );
   void Dictionary_Entry_Value_InitFromString( Dictionary_Entry_Value* self, const char* const value );
   
   /** Create a dictionary entry value of the specified type from the specified string */
   Dictionary_Entry_Value* Dictionary_Entry_Value_FromStringTo( char* value, char type );
   
   /** Create a value from a boolean value */
   Dictionary_Entry_Value* Dictionary_Entry_Value_FromBool( Bool value );
   void Dictionary_Entry_Value_InitFromBool( Dictionary_Entry_Value* self, Bool value );
   
   /** Create a value from a struct, begining with an empty value. */
   Dictionary_Entry_Value* Dictionary_Entry_Value_NewStruct( void );
   void Dictionary_Entry_Value_InitNewStruct( Dictionary_Entry_Value* );
   
   /** Create a value from a struct. */
   Dictionary_Entry_Value* Dictionary_Entry_Value_FromStruct( void* dictionary );
   void Dictionary_Entry_Value_InitFromStruct( Dictionary_Entry_Value*, void* dictionary );
   
   /** Create a value from a list */
   Dictionary_Entry_Value* Dictionary_Entry_Value_NewList( void );
   void Dictionary_Entry_Value_InitNewList( Dictionary_Entry_Value* );
   
   /** Add a member into a struct, with a given value associated with the given key. */
   void Dictionary_Entry_Value_AddMember( Dictionary_Entry_Value*, Dictionary_Entry_Key, Dictionary_Entry_Value* );
   
   /** Add a member into a struct, with a given value associated with the given key from a given source file. */
   void Dictionary_Entry_Value_AddMemberWithSource(
      Dictionary_Entry_Value*, Dictionary_Entry_Key, Dictionary_Entry_Value*, Dictionary_Entry_Source );

   /** Add an element into a list, with a given value */
   void Dictionary_Entry_Value_AddElement( Dictionary_Entry_Value* self, Dictionary_Entry_Value* element );
   
   /** Add an element into a list, with a given value */
   void Dictionary_Entry_Value_AddElementWithSource(
      Dictionary_Entry_Value* self, Dictionary_Entry_Value* element, Dictionary_Entry_Source source );
   
   /** Deletes a Dictionary_Entry_Value */
   void Dictionary_Entry_Value_Delete( Dictionary_Entry_Value* self );
   
   /** Printing the semantic value of element in its current state */
   void Dictionary_Entry_Value_Print( Dictionary_Entry_Value* self, Stream* stream );
   
   /** Set a dictionary entry value to the given type and value */
   void Dictionary_Entry_Value_SetFrom( Dictionary_Entry_Value* self, void* value, char type );
   
   /** Set/Change value to an unsigned int */
   void Dictionary_Entry_Value_SetFromUnsignedInt( Dictionary_Entry_Value* self, unsigned int value );
   
   /** Set/Change value to an int */
   void Dictionary_Entry_Value_SetFromInt( Dictionary_Entry_Value* self, int value );
   
   /** Set/Change value to a double */
   void Dictionary_Entry_Value_SetFromDouble( Dictionary_Entry_Value* self, double value );
   
   /** Set/Change value to an unsigned long */
   void Dictionary_Entry_Value_SetFromUnsignedLong( Dictionary_Entry_Value* self, unsigned long value );

   /** Set/Change value to a string */
   void Dictionary_Entry_Value_SetFromString( Dictionary_Entry_Value* self, char* value );
   
   /** Set the value from a string, but keeping as the current type */
   void Dictionary_Entry_Value_SetFromStringKeepCurrentType( Dictionary_Entry_Value* self, char* string );
   
   /** Set/Change value to a Bool */
   void Dictionary_Entry_Value_SetFromBool( Dictionary_Entry_Value* self, Bool value );
   
   /** Set the value to be of struct type, but empty */
   void Dictionary_Entry_Value_SetNewStruct( Dictionary_Entry_Value* self );
   
   /** Set/Change value to be a a struct */
   void Dictionary_Entry_Value_SetFromStruct( Dictionary_Entry_Value* self, void* dictionary );
   
   /** Set the value to be of list type, begining with an empty list */
   void Dictionary_Entry_Value_SetNewList( Dictionary_Entry_Value* self );
   
   /** Obtain the value as an unsigned int */
   unsigned int Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Entry_Value* self );
   
   /** Obtain the value as an unsigned int */
   int Dictionary_Entry_Value_AsInt( Dictionary_Entry_Value* self );
   
   /** Obtain the value as a double */
   double Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value* self );

   /** Obtain the value as an unsigned long */
   unsigned long Dictionary_Entry_Value_AsUnsignedLong( Dictionary_Entry_Value* self );

   /** Obtain the value as a string */
   char* Dictionary_Entry_Value_AsString( Dictionary_Entry_Value* self );
   
   /** Obtain the value as a Bool */
   Bool Dictionary_Entry_Value_AsBool( Dictionary_Entry_Value* self );
   
   /** Obtain the value as a Dictionary */
   Dictionary* Dictionary_Entry_Value_AsDictionary( Dictionary_Entry_Value* self );
   
   /** get the first element out of a list */
   Dictionary_Entry_Value* Dictionary_Entry_Value_GetFirstElement( Dictionary_Entry_Value* self );
   
   /** get any element from a list (bear in mind whole list must be processed */
   Dictionary_Entry_Value* Dictionary_Entry_Value_GetElement( Dictionary_Entry_Value* self, Index index );
   
   /** Get the number of elements currently in a list */
   Index Dictionary_Entry_Value_GetCount( Dictionary_Entry_Value* self );
      
   /** Set the encoding type of a list */
   void Dictionary_Entry_Value_SetEncoding( Dictionary_Entry_Value* self, const Encoding encoding );
   
   /** Get the encoding type of a list */
   Encoding Dictionary_Entry_Value_GetEncoding( Dictionary_Entry_Value* self );

   /** Setting the value of a struct member */
   void Dictionary_Entry_Value_SetMember(
      Dictionary_Entry_Value* self, Dictionary_Entry_Key name, Dictionary_Entry_Value* member );
      
   /** Setting the value of a struct member from a source file */
   void Dictionary_Entry_Value_SetMemberWithSource(
      Dictionary_Entry_Value* self, Dictionary_Entry_Key name, Dictionary_Entry_Value* member, Dictionary_Entry_Source source );
   
   /** Getting/accessing members out of a struct */
   Dictionary_Entry_Value* Dictionary_Entry_Value_GetMember( Dictionary_Entry_Value* self, Dictionary_Entry_Key name );

   /** Compare two DEVs. Returns True if they are semantically identical. Default to a loose type check */
   #define Dictionary_Entry_Value_Compare( self, dev ) \
      ( Dictionary_Entry_Value_CompareFull( (self), (dev), False ) )

   Bool Dictionary_Entry_Value_CompareFull( Dictionary_Entry_Value* self, Dictionary_Entry_Value* dev, Bool strictTypeCheck );
   
   /** Copy a DEV. This means new memory will created for both the new DEV, and
    * any values it points to */   
   Dictionary_Entry_Value* Dictionary_Entry_Value_Copy(
      Dictionary_Entry_Value* self,
      Bool deep );
   
#endif /* __StGermain_Base_IO_Dictionary_Entry_Value_h__ */
