/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_Dictionary_Entry_h__
#define __StGermain_Base_IO_Dictionary_Entry_h__
   
   /** \def __Dictionary_Entry See Dictionary_Entry */
   #define __Dictionary_Entry  \
      Dictionary_Entry_Key    key; \
      Dictionary_Entry_Value* value; \
      Dictionary_Entry_Units  units; \
      Dictionary_Entry_Source source;

   struct _Dictionary_Entry { __Dictionary_Entry };

   /** Constant value used for Dictionary_Entry source when no source file was specified */
   #define NO_SOURCEFILE "created_in_code"
   
   /** Create a new Dictionary_Entry (assumes ownership of the value) */
   Dictionary_Entry* Dictionary_Entry_New( Dictionary_Entry_Key key, Dictionary_Entry_Value* value );

   /** Create a new Dictionary_Entry with a source file (assumes ownership of the value) */
   Dictionary_Entry* Dictionary_Entry_NewWithSource( Dictionary_Entry_Key key, Dictionary_Entry_Value* value, Dictionary_Entry_Source source );

   /** Initialise a Dictionary_Entry (assumes ownership of the value) */
   void Dictionary_Entry_Init( Dictionary_Entry* self, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
   
   /** Destroy a Dictionary_Entry instance */
   void Dictionary_Entry_Delete( Dictionary_Entry* self );
   
   /** Print the contents of a Dictionary_Entry construct */
   void Dictionary_Entry_Print( Dictionary_Entry* self, Stream* stream );

   /** Compare a Dictionary_Entry to a key */
   Bool Dictionary_Entry_Compare( Dictionary_Entry* self, Dictionary_Entry_Key key );

   /** Compare a Dictionary_Entry to a key, and source */
   Bool Dictionary_Entry_CompareWithSource( Dictionary_Entry* self, Dictionary_Entry_Key key, Dictionary_Entry_Source source );

   /** Set/Replace an entry's value (assumes ownership of the value) */
   void Dictionary_Entry_Set( Dictionary_Entry* self, Dictionary_Entry_Value* value );
   
   /** Set/Replace an entry's value with new source file (assumes ownership of the value). */
   void Dictionary_Entry_SetWithSource( Dictionary_Entry* self, Dictionary_Entry_Value* value, Dictionary_Entry_Source source );

   /** Set an entry's source. */
   void Dictionary_Entry_SetSource( Dictionary_Entry* self, Dictionary_Entry_Source source );
   
   /** Get an entry's value */
   Dictionary_Entry_Value* Dictionary_Entry_Get( Dictionary_Entry* self );

   /** Get an entry's originalSource */
   Dictionary_Entry_Source Dictionary_Entry_GetSource( Dictionary_Entry* self );
   
#endif /* __StGermain_Base_IO_Dictionary_Entry_h__ */


