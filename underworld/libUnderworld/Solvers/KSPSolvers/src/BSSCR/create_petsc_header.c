/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include "common-driver-utils.h"


/*
./FullIndef-driver-basic.app on a darwin8.1 named sushi.maths.monash.edu.au with 2 processors, by dmay Mon Dec  3 18:52:03 2007
Using Petsc Release Version 2.3.2, Patch 10, Wed Mar 28 19:13:22 CDT 2007 HG revision: d7298c71db7f5e767f359ae35d33cab3bed44428
*/

PetscErrorCode BSSCR_GeneratePetscHeader_for_file( FILE *fd, MPI_Comm comm )
{
        PetscErrorCode ierr;
        char           version[256];
        char           arch[50], hostname[64], username[16], pname[PETSC_MAX_PATH_LEN], date[64];
        int            size, rank;
        char           *_dir;

        ierr = PetscGetArchType(arch, 50);CHKERRQ(ierr);
        ierr = PetscGetHostName(hostname, 64);CHKERRQ(ierr);
        ierr = PetscGetUserName(username, 16);CHKERRQ(ierr);
        ierr = PetscGetProgramName(pname, PETSC_MAX_PATH_LEN);CHKERRQ(ierr);
        ierr = PetscGetDate(date, 64);CHKERRQ(ierr);
        ierr = PetscGetVersion(&version,256);CHKERRQ(ierr);
        ierr = PetscGetPetscDir( (const char**)&_dir ); CHKERRQ(ierr);

        MPI_Comm_size( comm, &size );
	MPI_Comm_rank( comm, &rank );
	if( rank != 0 ) PetscFunctionReturn(0);

        if( size == 1 ) {
                fprintf( fd,"## %s on a %s named %s with %d processor, by %s %s\n", pname, arch, hostname, size, username, date);
        } else {
                fprintf( fd,"## %s on a %s named %s with %d processors, by %s %s\n", pname, arch, hostname, size, username, date);
        }
        fprintf( fd, "## Using %s, installed at %s \n", version, _dir );


        PetscFunctionReturn(0);
}

PetscErrorCode BSSCR_GeneratePetscHeader_for_viewer( PetscViewer viewer )
{
        PetscErrorCode ierr;
        char           version[256];
        char           arch[50], hostname[64], username[16], pname[PETSC_MAX_PATH_LEN], date[64];
        int            size;
        char           *_dir;
	MPI_Comm       comm;


        ierr = PetscGetArchType(arch, 50);CHKERRQ(ierr);
        ierr = PetscGetHostName(hostname, 64);CHKERRQ(ierr);
        ierr = PetscGetUserName(username, 16);CHKERRQ(ierr);
        ierr = PetscGetProgramName(pname, PETSC_MAX_PATH_LEN);CHKERRQ(ierr);
        ierr = PetscGetDate(date, 64);CHKERRQ(ierr);
        ierr = PetscGetVersion(&version,256);CHKERRQ(ierr);
        ierr = PetscGetPetscDir( (const char**)&_dir ); CHKERRQ(ierr);

	PetscObjectGetComm( (PetscObject)viewer, &comm );
        MPI_Comm_size( comm, &size );

        if( size == 1 ) {
                PetscViewerASCIIPrintf( viewer,"## %s on a %s named %s with %d processor, by %s %s\n", pname, arch, hostname, size, username, date);
        } else {
                PetscViewerASCIIPrintf( viewer,"## %s on a %s named %s with %d processors, by %s %s\n", pname, arch, hostname, size, username, date);
        }
        PetscViewerASCIIPrintf( viewer, "## Using %s, installed at %s \n", version, _dir );


        PetscFunctionReturn(0);
}
