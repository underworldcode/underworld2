/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_types_h__
#define __StGermain_Base_IO_types_h__
	
	/* Dicitonary internal types */
	typedef char*				Dictionary_Entry_Key;
	typedef char*				Dictionary_Entry_Source;
	typedef char*				Dictionary_Entry_Units;
	typedef enum {
		Dictionary_Entry_Value_Type_String,
		Dictionary_Entry_Value_Type_Double,
		Dictionary_Entry_Value_Type_UnsignedInt,
		Dictionary_Entry_Value_Type_Int,
		Dictionary_Entry_Value_Type_UnsignedLong,
		Dictionary_Entry_Value_Type_Bool,
		Dictionary_Entry_Value_Type_Struct,
		Dictionary_Entry_Value_Type_List,
		Dictionary_Entry_Value_Type_VoidPtr, /* proposed... not in use yet */
		Dictionary_Entry_Value_Num_Types
	} Dictionary_Entry_Value_Type;

	/* Base types/classes */
	typedef struct _Dictionary_Entry_Value_Validator	Dictionary_Entry_Value_Validator;
	typedef struct _Dictionary_Entry_Value_List		Dictionary_Entry_Value_List;
	typedef struct _Dictionary_Entry_Value			Dictionary_Entry_Value;
	typedef struct _Dictionary_Entry			Dictionary_Entry;
	typedef struct _Dictionary				Dictionary;
	
	typedef struct _IO_Handler	IO_Handler;
	typedef struct _XML_IO_Handler	XML_IO_Handler;

	typedef unsigned int	JournalLevel;

	/* Journal class*/
	typedef struct File		File;
	typedef struct CFile			CFile;
	typedef struct MPIFile			MPIFile;
	typedef struct JournalTypedStream	JournalTypedStream;
	typedef struct Journal			Journal;	
	typedef struct Stream			Stream;
	typedef struct CStream			CStream;
	typedef struct MPIStream		MPIStream;
	typedef struct BinaryStream	BinaryStream;
	
	typedef struct StreamFormatter		StreamFormatter;
	typedef struct StreamFormatter_Buffer	StreamFormatter_Buffer;
	typedef struct LineFormatter		LineFormatter;
	typedef struct RankFormatter		RankFormatter;
	typedef struct IndentFormatter		IndentFormatter;

#endif /* __StGermain_Base_IO_types_h__ */
