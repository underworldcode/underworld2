/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <gLucifer/Base/DrawingObject.h>

#ifndef __lucPlot_h__
#define __lucPlot_h__

/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
extern const Type lucPlot_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucPlot                    \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__lucDrawingObject                  \
		/* Virtual functions go here */           \
		/* Other info */                          \
		char*                outputPath;        \
		char*                dataFileName;        \
		char*                colourMapFileName;        \
      unsigned int         columns;             \
      unsigned int         columnX;          \
      unsigned int         columnY;          \
	  unsigned int         columnZ;          \
      unsigned int         skipRows;         \
      double               coordZ;           \
      char*                labelX;           \
      char*                labelY;           \
	char*                labelZ;           \
      Bool                 lines;            \
      Bool                 points;           \
      Bool                 bars;             \
	unsigned int         ticksX;          \
	unsigned int         ticksY;          \
	unsigned int         ticksZ;          \
	Bool                 flipY;            \
	Bool            setX; \
	Bool            setY; \
	Bool            setZ; \
	double			minX;	\
	double           minY;	\
	double           minZ;	\
	double			maxX;	\
	double           maxY;	\
	double           maxZ;	\
      unsigned int         pointSize;          \
	Bool                 colourMapFromFile;            \
      /* Axis data values */                 \
      double*              dataX;            \
      double*              dataY;            \
	double*              dataZ;            \
      unsigned int         rows;          \


struct lucPlot
{
   __lucPlot
};

/** Private Constructor: This will accept all the virtual functions for this class as arguments. */

#ifndef ZERO
#define ZERO 0
#endif

#define LUCPLOT_DEFARGS \
                LUCDRAWINGOBJECT_DEFARGS

#define LUCPLOT_PASSARGS \
                LUCDRAWINGOBJECT_PASSARGS

lucPlot* _lucPlot_New(  LUCPLOT_DEFARGS  );

void _lucPlot_Delete( void* drawingObject ) ;
void _lucPlot_Print( void* drawingObject, Stream* stream ) ;

/* 'Stg_Component' implementations */
void* _lucPlot_DefaultNew( Name name ) ;
void _lucPlot_AssignFromXML( void* drawingObject, Stg_ComponentFactory* cf, void* data );
void _lucPlot_Build( void* drawingObject, void* data ) ;
void _lucPlot_Initialise( void* drawingObject, void* data ) ;
void _lucPlot_Execute( void* drawingObject, void* data );
void _lucPlot_Destroy( void* drawingObject, void* data ) ;

void lucPlot_ReadDataFile( void* drawingObject );
void _lucPlot_Draw( void* drawingObject, lucDatabase* database, void* _context );

#endif

