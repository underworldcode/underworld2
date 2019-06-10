/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* \file
**  Role:
**   Provides an interface (and certain key functions) for reading from/ writing to files with useful information.
**   Files are read to/from the dictionary provided by the user.
**
** Assumptions:
**
** Comments:
**   When writing files, I should change the interface to add an option to update, rather than overwrite, files.
**
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Base_IO_IO_Handler_h__
#define __StGermain_Base_IO_IO_Handler_h__
   
   /* Function pointer interface for inherited classes to use */
   typedef void ( IO_Handler_DeleteFunction ) ( void* io_handler );
   typedef void ( IO_Handler_PrintFunction ) ( void* io_handler );
   typedef Bool ( IO_Handler_ReadAllFromFileFunction ) (
      void*       io_handler, 
      const char* filename,
      Dictionary* dictionary,
      Dictionary* sources );
   typedef Bool ( IO_Handler_ReadAllFromBufferFunction ) (
      void*       io_handler,
      const char* buffer,
      Dictionary* dictionary,
      const char* tag );
   typedef Bool ( IO_Handler_WriteAllToFileFunction ) (
      void*       io_handler,
      const char* filename,
      Dictionary* dictionary,
      Dictionary* sources );
   typedef Bool ( IO_Handler_DictSetAddValueFunction )
      ( void* io_handler, const char* filename, Dictionary* dictionary, Dictionary_MergeType mergeType );
   
   /* Textual name for IO_Handler class */
   extern const Type IO_Handler_Type;
   
   /* Default merge type for IO_Handler instantiations to use when inserting into dictionary */
   extern const Dictionary_MergeType IO_Handler_DefaultMergeType;
   /* Default merge type for IO_Handler instantiations to use when inserting children entries
    * (ie of lists, structs) into dictionary */
   extern const Dictionary_MergeType IO_Handler_DefaultChildrenMergeType;

   /* \def __IO_Handler See IO_Handler */
   #define __IO_Handler \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      IO_Handler_ReadAllFromFileFunction*            _readAllFromFile; \
      IO_Handler_ReadAllFromBufferFunction*          _readAllFromBuffer; \
      IO_Handler_WriteAllToFileFunction*             _writeAllToFile; \
      \
      /* IO_Handler info */ \
      Dictionary*                                    currDictionary; \
      Dictionary*                                    currSources; \
      char*                                          resource; \
      char*                                          currPath; \
      char*                                          schema; \
      int                                            validate;

   struct _IO_Handler { __IO_Handler };
   
   /* No "IO_Handler_New" and "IO_Handler_Init" as this is an abstract class */
   
   /* Creation implementation */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define IO_HANDLER_DEFARGS \
      STG_CLASS_DEFARGS, \
      IO_Handler_ReadAllFromFileFunction*            _readAllFromFile, \
      IO_Handler_ReadAllFromBufferFunction*          _readAllFromBuffer, \
      IO_Handler_WriteAllToFileFunction*             _writeAllToFile

   #define IO_HANDLER_PASSARGS \
      STG_CLASS_PASSARGS, \
      _readAllFromFile, \
      _readAllFromBuffer, \
      _writeAllToFile            

   IO_Handler* _IO_Handler_New( IO_HANDLER_DEFARGS );
   
   /* Initialisation implementation */
   void _IO_Handler_Init( IO_Handler* self );
   
   /* Stg_Class_Delete implementation */
   extern void _IO_Handler_Delete( void* io_handler ); 
   
   /* Print current information */
   extern void _IO_Handler_Print( void* io_handler, Stream* stream );
   
   /* Read a dictionary entry of a given name from file */
   extern Bool IO_Handler_ReadAllFromFile(
      void*       io_handler,
      const char* filename,
      Dictionary* dictionary,
      Dictionary* sources );
      
   /* Read a dictionary entry of a given name from buffer */
   extern Bool IO_Handler_ReadAllFromBuffer( void* io_handler, const char* buffer, Dictionary* dictionary, const char* tag );
   
   /* Write a given dictionary entry to file */
   /* TODO: option to overwrite / update */
   extern Bool IO_Handler_WriteAllToFile(
      void*       io_handler,
      const char* filename,
      Dictionary* dictionary,
      Dictionary* sources );
   
   /* Set/add entry to dictionary given parent */
   extern Dictionary_Entry_Value* IO_Handler_DictSetAddValue( 
      void*                   io_handler, 
      Dictionary_Entry_Key    name,
      char*                   newValue, 
      char                    newDictValType, 
      Dictionary_Entry_Value* parent, 
      Dictionary_MergeType    mergeType );

   /* Set/add entry to dictionary given parent */
   extern Dictionary_Entry_Value* IO_Handler_DictSetAddValueWithSource( 
      void*                   io_handler, 
      Dictionary_Entry_Key    name,
      char*                   newValue, 
      char                    newDictValType, 
      char*                   newUnits, 
      Dictionary_Entry_Value* parent, 
      Dictionary_MergeType    mergeType,
      Dictionary_Entry_Source source );

   /* Finds all references to files in command line args with extension ".xml" and reads them with the IO
   handler. Returns the number of files successfully read. */
   Index IO_Handler_ReadAllFilesFromCommandLine(
      void*       ioHandler,
      int         argc,
      char*       argv[],
      Dictionary* dictionary,
      Dictionary* sources );

   /* Runs IO_Handler_ReadAllFilesFromCommandLine and Dictionary_ReadAllParamFromCommandLine.
   Returns the number of files successfully read. */
   Index IO_Handler_ReadAllFromCommandLine(
      void*       ioHandler,
      int         argc,
      char*       argv[],
      Dictionary* dictionary,
      Dictionary* sources );
   
#endif /* __StGermain_Base_IO_IO_Handler_h__ */

