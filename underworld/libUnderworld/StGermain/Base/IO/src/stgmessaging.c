/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "mpi.h"

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Dictionary.h"
#include "Dictionary_Entry.h"
#include "Dictionary_Entry_Value.h"
#include "Journal.h"
#include "File.h"
#include "CFile.h"
#include "Stream.h"
#include "CStream.h"
#include "MPIStream.h"
#include "StreamFormatter.h"


int Stg_Messaging_GetRank( MPI_Comm comm ) {
	int rank;
	MPI_Comm_rank( comm, &rank );
	return rank;
}

int Stg_MPI_Send( char* file, int line, void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Send: tag = %d, count = %d, datatype = %d, dest = %d\n", file, line, Stg_Messaging_GetRank( comm ), tag, count, datatype, dest );
	return MPI_Send( buf, count, datatype, dest, tag, comm );
}

int Stg_MPI_Ssend( char* file, int line, void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Ssend: tag = %d, count = %d, datatype = %d, dest = %d\n", file, line, Stg_Messaging_GetRank( comm ), tag, count, datatype, dest );
	return MPI_Ssend( buf, count, datatype, dest, tag, comm );
}
int Stg_MPI_Isend( char* file, int line, void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Isend: tag = %d, count = %d, datatype = %d, dest = %d\n", file, line, Stg_Messaging_GetRank( comm ), tag, count, datatype, dest );
	return MPI_Isend( buf, count, datatype, dest, tag, comm, request );
}
int Stg_MPI_Recv( char* file, int line, void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Recv: tag = %d, count = %d, datatype = %d, source = %d\n", file, line, Stg_Messaging_GetRank( comm ), tag, count, datatype, source );
	return MPI_Recv( buf, count, datatype, source, tag, comm, status );
}
int Stg_MPI_Irecv( char* file, int line, void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Irecv: tag = %d, count = %d, datatype = %d, source = %d\n", file, line, Stg_Messaging_GetRank( comm ), tag, count, datatype, source );
	return MPI_Irecv( buf, count, datatype, source, tag, comm, request );
}
int Stg_MPI_Reduce ( char* file, int line, void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Reduce: count = %d, datatype = %d\n", file, line, Stg_Messaging_GetRank( comm ), count, datatype );
	return MPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm );
}
int Stg_MPI_Allreduce ( char* file, int line, void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Allreduce: count = %d, datatype = %d\n", file, line, Stg_Messaging_GetRank( comm ), count, datatype );
	return MPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm );
}
int Stg_MPI_Gather ( char* file, int line, void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Gather: count = %d, datatype = %d\n", file, line, Stg_Messaging_GetRank( comm ), sendcnt, sendtype );
	return MPI_Gather( sendbuf, sendcnt, sendtype, recvbuf, recvcount, recvtype, root, comm );
}
int Stg_MPI_Allgather ( char* file, int line, void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	Journal_Printf( stream, "%s %d, rank %d MPI_Allgather: count = %d, datatype = %d\n", file, line, Stg_Messaging_GetRank( comm ), sendcount, sendtype );
	return MPI_Allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm );

}

int Stg_MPI_Wait ( char* file, int line, MPI_Request *request, MPI_Status *status ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	int result =  MPI_Wait( request, status );
	Journal_Printf( stream, "%s %d, rank %d MPI_Wait:",
		file, line, Stg_Messaging_GetRank( MPI_COMM_WORLD ) );
	if ( MPI_STATUS_IGNORE != (status) ) {
		Journal_Printf( stream, " tag = %d, source = %d", status->MPI_TAG, status->MPI_SOURCE );
	}
	Journal_Printf( stream, "\n" );
	return result;
}


int Stg_MPI_Test ( char* file, int line, MPI_Request *request, int *flag, MPI_Status *status ) {
	Stream* stream = Journal_Register( Info_Type, "mpi" );
	int result = MPI_Test( request, flag, status );
	if ( *flag ) {
		Journal_Printf( stream, "%s %d, rank %d MPI_Test: result = %d",
			file, line, Stg_Messaging_GetRank( MPI_COMM_WORLD ), result );
		if ( MPI_STATUS_IGNORE != (status) ) {
			Journal_Printf( stream, ", tag = %d, source = %d", status->MPI_TAG, status->MPI_SOURCE );
		}
		Journal_Printf( stream, "\n" );
	}

	return result;
}


