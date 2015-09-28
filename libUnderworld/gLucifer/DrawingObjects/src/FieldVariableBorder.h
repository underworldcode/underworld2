/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __lucFieldVariableBorder_h__
#define __lucFieldVariableBorder_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucFieldVariableBorder_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucFieldVariableBorder \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucDrawingObject \
		/* Virtual functions go here */ \
		/* Other info */\
		FieldVariable*                                     fieldVariable;          \

struct lucFieldVariableBorder
{
   __lucFieldVariableBorder
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCFIELDVARIABLEBORDER_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCFIELDVARIABLEBORDER_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucFieldVariableBorder* _lucFieldVariableBorder_New(  LUCFIELDVARIABLEBORDER_DEFARGS  );

void _lucFieldVariableBorder_Delete( void* drawingObject ) ;
void _lucFieldVariableBorder_Print( void* drawingObject, Stream* stream ) ;
void* _lucFieldVariableBorder_Copy( void* drawingObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap) ;

/* 'Stg_Component' implementations */
void* _lucFieldVariableBorder_DefaultNew( Name name ) ;
void _lucFieldVariableBorder_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucFieldVariableBorder_Build( void* drawingObject, void* data ) ;
void _lucFieldVariableBorder_Initialise( void* drawingObject, void* data ) ;
void _lucFieldVariableBorder_Execute( void* drawingObject, void* data );
void _lucFieldVariableBorder_Destroy( void* drawingObject, void* data ) ;

void _lucFieldVariableBorder_Draw( void* drawingObject, lucDatabase* database, void* _context ) ;

#endif

