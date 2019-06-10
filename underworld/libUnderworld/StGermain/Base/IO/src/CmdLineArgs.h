/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StGermain_Base_IO_CmdLineArgs_h__
#define __StGermain_Base_IO_CmdLineArgs_h__
	
	void stgRemoveCmdLineArg( int* argc, char** argv[], int index );
	char* stgParseHelpCmdLineArg( int* argc, char** argv[] );
	char* stgParseListCmdLineArg( int* argc, char** argv[] );
	char* stgParseListAllCmdLineArg( int* argc, char** argv[] );
	Stg_ObjectList* stgParseInputPathCmdLineArg( int* argc, char** argv[] );

#endif /* __StGermain_Base_IO_CmdLineArgs_h__ */
