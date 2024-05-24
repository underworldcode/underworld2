/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_Init_h__
#define __StGermain_Base_IO_Init_h__
	
	extern const Name    LiveDebugName;
	/** A global stream for interactive debugging through GDB. Note that the 
	    code shouldn't send output to this stream */
	extern Stream*       LiveDebug;

	extern Stream* stgErrorStream;

	Bool      BaseIO_Init( int* argc, char** argv[] );
	
#endif /* __StGermain_Base_IO_Init_h__ */



