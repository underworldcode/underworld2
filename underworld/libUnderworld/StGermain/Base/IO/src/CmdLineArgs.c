/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <string.h>

#include "Base/Foundation/Foundation.h"


void stgRemoveCmdLineArg( int* argc, char** argv[], int index ) {
	if( index > 0 && index < *argc ) {
		Index i;
		void* tmpPtr = (*argv)[index];

		for( i = index; i < *argc - 1; i++ ) {
			(*argv)[i] = (*argv)[i+1];
		}

		(*argv)[*argc-1] = (char*)tmpPtr;
		*argc -= 1;
	}
}


char* stgParseHelpCmdLineArg( int* argc, char** argv[] ) {
	Index                   arg_I;

	/* Loop over all the arguments from command line and reads all arguments of form "--help topic" */
	for( arg_I = 1 ; arg_I < *argc; arg_I++ ) {
		char*                   valueString = 0;
		char*                   argumentString = (*argv)[arg_I];
		const char*             preceedingString = "--help";
		unsigned int            preceedingStringLength = strlen( preceedingString );

		/* Check is string has preceeding string is "--help" if not then continue in loop */
		if( strncmp( preceedingString, argumentString , preceedingStringLength ) != 0 ) {
			continue;
		}
		if( strlen( argumentString ) != strlen( preceedingString ) ) {
			continue;
		}
		if( arg_I >= (*argc - 1) ) {
			continue;
		}

		valueString = StG_Strdup( (*argv)[arg_I+1] );
		stgRemoveCmdLineArg( argc, argv, arg_I ); /* first argument: --help */
		stgRemoveCmdLineArg( argc, argv, arg_I ); /* second argument: topic */
		return valueString;		
	}

	return 0;
}

char* stgParseListCmdLineArg( int* argc, char** argv[] ) {
	Index                   arg_I;

	/* Loop over all the arguments from command line and reads all arguments of form "--help topic" */
	for( arg_I = 1 ; arg_I < *argc; arg_I++ ) {
		char*                   valueString = 0;
		char*                   argumentString = (*argv)[arg_I];
		const char*             preceedingString = "--list";
		unsigned int            preceedingStringLength = strlen( preceedingString );

		/* Check is string has preceeding string is "--list" if not then continue in loop */
		if( strncmp( preceedingString, argumentString , preceedingStringLength ) != 0 ) {
			continue;
		}
		if( strlen( argumentString ) != strlen( preceedingString ) ) {
			continue;
		}
		if( arg_I >= (*argc - 1) ) { /* "--list" is the last commandline argument */
			valueString = StG_Strdup( "" );
			stgRemoveCmdLineArg( argc, argv, arg_I ); /* first argument: --list */
			return valueString;
		}

		valueString = StG_Strdup( (*argv)[arg_I+1] );
		stgRemoveCmdLineArg( argc, argv, arg_I ); /* first argument: --list */
		stgRemoveCmdLineArg( argc, argv, arg_I ); /* second argument: topic */
		return valueString;		
	}

	return 0;
}

char* stgParseListAllCmdLineArg( int* argc, char** argv[] ) {
	Index                   arg_I;

	/* Loop over all the arguments from command line and reads all arguments of form "--help topic" */
	for( arg_I = 1 ; arg_I < *argc; arg_I++ ) {
		char*                   valueString = 0;
		char*                   argumentString = (*argv)[arg_I];
		const char*             preceedingString = "--list-all";
		unsigned int            preceedingStringLength = strlen( preceedingString );

		/* Check is string has preceeding string is "--list" if not then continue in loop */
		if( strncmp( preceedingString, argumentString , preceedingStringLength ) != 0 ) {
			continue;
		}
		if( strlen( argumentString ) != strlen( preceedingString ) ) {
			continue;
		}
		if( arg_I >= (*argc - 1) ) { /* "--list" is the last commandline argument */
			valueString = StG_Strdup( "" );
			stgRemoveCmdLineArg( argc, argv, arg_I ); /* first argument: --list */
			return valueString;
		}

		valueString = StG_Strdup( (*argv)[arg_I+1] );
		stgRemoveCmdLineArg( argc, argv, arg_I ); /* first argument: --list */
		stgRemoveCmdLineArg( argc, argv, arg_I ); /* second argument: topic */
		return valueString;		
	}

	return 0;
}

static void deleteInputPathItem( void* ptr ) {
   /* Correct way to delete result of stgParseInputPathCmdLineArg items */
   Memory_Free( ptr );
}

Stg_ObjectList* stgParseInputPathCmdLineArg( int* argc, char** argv[] ) {
	Index                   arg_I;
	Stg_ObjectList*		allInputPaths = Stg_ObjectList_New();

	/* Loop over all the arguments from command line and reads all arguments of form "--help topic" */
	for( arg_I = 1 ; arg_I < *argc; arg_I++ ) {
		char*                   valueString = 0;
		char*                   argumentString = (*argv)[arg_I];
		const char*             preceedingString = "--inputPath";
		unsigned int            preceedingStringLength = strlen( preceedingString );

		/* Check is string has preceeding string is "--inputPath" if not then continue in loop */
		if( strncmp( preceedingString, argumentString , preceedingStringLength ) != 0 ) {
			continue;
		}
		if( strlen( argumentString ) <= (strlen( preceedingString ) + 1) ) {
			/* i.e. it has = sign (maybe) but not an input path itself */
			continue;
		}
		if( strncmp( "=", &argumentString[preceedingStringLength], 1 ) != 0 ) {
			/* i.e. no = sign */
			continue;
		}

		valueString = StG_Strdup( &(*argv)[arg_I][preceedingStringLength+1] );
		stgRemoveCmdLineArg( argc, argv, arg_I ); /* name=value: --inputPath=??? */
		Stg_ObjectList_Append( allInputPaths, Stg_ObjectAdaptor_NewOfPointer( valueString, 0, True, False, deleteInputPathItem, 0, 0 ) );
	}

	return allInputPaths;
}



