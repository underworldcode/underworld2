/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StGermain_Base_IO_PathUtils_h__
#define __StGermain_Base_IO_PathUtils_h__


/** Note that this function is designed as call-by-reference to modify fullPath to the found
 * full path of the chosen file.
 * WARNING: because this function uses fopen() to check the file's existence, then it
 * shouldn't be called by multiple processors simultaneously */
void FindFile( char* fullPath, const char* filename, const char* searchPaths );

Bool FindFileInPathList( char* fullPath, char* filename, char** searchPaths, Index searchPathsSize );

void PathJoin( char* path, unsigned count, ... );

void PathClean( char* outPath, char* inPath );

char* ExpandEnvironmentVariables( char* string );

char* ParentDirectory( const char* path );

Bool Stg_CreateDirectory( const char* path );

Bool Stg_FileExists( const char* path );

Bool Stg_DirectoryExists( const char* path );

#endif /* __StGermain_Base_IO_PathUtils_h__ */
