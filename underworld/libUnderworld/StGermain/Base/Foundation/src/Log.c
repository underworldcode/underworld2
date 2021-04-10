/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <mpi.h>

#ifndef STGERMAIN_LOGFILE_NAME
#define STGERMAIN_LOGFILE_NAME "stgermain.log"
#endif 

/* empty out the file */
void _init_stg_log_printf( const char *name )
{
        FILE *fp;
        char err[128];
        int rank, length;
        char mname[BUFSIZ];


        sprintf( err, name );
        fp = fopen( err, "w" );

		/* It's useful to have a safe fallback here */ 
		
		if(fp==NULL) {
			fp = stderr;
		}	

        /* write header for log file */
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );
        MPI_Get_processor_name( mname, &length );

        fprintf( fp, "=== StGermain Parallel log ===\n" );
        fprintf( fp, "=== Generated on %s, by rank %d ===\n", mname, rank );
		
		if(fp != stderr)
        	fclose( fp );
}

void _stg_log_write_filename( char name[] )
{
	int rank;
	
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    sprintf( name, "%s.p%d" , STGERMAIN_LOGFILE_NAME, rank ); 
}


void stg_log_printf( const char *format, ... )
{
        FILE *fp;
        char err[128];
        va_list ap;
        static int beenhere = 0;


	_stg_log_write_filename( err );

        if( beenhere == 0 ) {
                _init_stg_log_printf(err);
        }

	
        fp = fopen( err, "a" );
		if(fp==NULL) {
			fp = stderr;
		}	

        va_start(ap,format);
        vfprintf( fp, format, ap );
        va_end(ap);

		if(fp != stderr)
        	fclose(fp);

        beenhere = 1;
}

FILE* _get_file_pointer_log_printf( void )
{
	FILE *fp;
	char err[128];
	
	
	_stg_log_write_filename( err );
	fp = fopen( err, "a" );
	if(fp==NULL) {
		fp = stderr;
	}
	
	return fp;
}

void stg_profile_EntryPoint( char ep_name[], char hk_name[], double time )
{ 
	int len_ep, len_hk;
	int max_length = 50;
	int i;
	FILE *fp;
	int cnt;
	
	stg_log_printf( "ExecutionTime for - " );
	
	len_ep = strlen( ep_name );
	len_ep = len_ep + 13;
	cnt = len_ep;
	if( len_ep < max_length ) {
		fp = _get_file_pointer_log_printf();
		
		fprintf( fp, "%s (EntryPoint)", ep_name );
		for( i=0; i<(max_length-(len_ep) ); i++ ) {
			fprintf( fp, " ");
			cnt++;
		}
		
		fprintf( fp, " : %s (Hook)", hk_name );
		len_hk = strlen( hk_name );
		for( i=0; i<(max_length-len_hk); i++ ) {
			fprintf( fp, " ");
		}
		fprintf( fp, " time= %6.6e (sec)\n", time );
		fclose( fp );
	}
	else {
		/* do an unformatted */
		stg_log_printf( "%s (EntryPnt) : %s (Hook) - time = %.6g\n", ep_name, hk_name, time );
	}
}


void stg_profile_Func( char func_name[], double time )
{
        int len_ep;
        int max_length = 50;
        int i;
        FILE *fp;
        int cnt;

        stg_log_printf( "ExecutionTime for - " );

        len_ep = strlen( func_name );
        len_ep = len_ep + 11;
        cnt = len_ep;
        if( len_ep < max_length ) {
                fp = _get_file_pointer_log_printf();

                fprintf( fp, "%s (Function)", func_name );
                for( i=0; i<(2*max_length+10-(len_ep) ); i++ ) {
                        fprintf( fp, " ");
                        cnt++;
                }

                fprintf( fp, " time= %6.6e (sec)\n", time );
				if(fp != stderr)
                	fclose( fp );
        }
        else {
                /* do an unformatted */
                stg_log_printf( "%s (Function) - time = %.6g\n", func_name, time );
        }
}




