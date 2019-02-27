##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module contains routines related to parallel operation via
the Message Passing Interface (MPI).

Attributes
----------
comm : mpi4py.MPI.Intracomm
    The MPI communicator.
rank : int
    The rank of the current process.
size : int
    The size of the pool of processes.

"""

import underworld as _uw
from mpi4py import MPI as _MPI
comm = _MPI.COMM_WORLD
size = comm.size
rank = comm.rank

def barrier():
    """
    Creates an MPI barrier. All processes wait here for others to catch up.

    """
    comm.Barrier()

class call_pattern():
    """
    This context manager calls the code within its block using the
    specified calling pattern.

    Parameters
    ----------
    pattern: str
        'collective': Each process calls the block of code simultaneously.
        'sequential': Processes call block of code in order of rank.

    Example
    -------
    This example is redundant as it will only run with a single process.
    However, where run in parallel, you should expect the outputs to be
    ordered according to process rank. Note also that for deterministic
    printing in parallel, and you may need to run Python unbuffered
    (`mpirun -np 4 python -u yourscript.py`, for example).

    >>> import underworld as uw
    >>> with uw.mpi.call_pattern(pattern="sequential"):
    ...     print("My rank is {}".format(uw.mpi.rank))
    My rank is 0

    """
    def __init__(self, pattern="collective", returnobj=None):
        if not isinstance(pattern,str):
            raise TypeError("`pattern` parameter must be of type `str`")
        pattern = pattern.lower()
        if pattern not in ("collective", "sequential"):
            raise ValueError("`pattern` must take values `collective` or `sequential`.")
        self.pattern = pattern
        self.returnobj = returnobj

    def __enter__(self):
        if self.pattern == "sequential":
            if rank!=0:
                comm.recv(source=rank-1, tag=333)
        return self.returnobj

    def __exit__(self, *args):
        if self.pattern == "sequential":
            dest = rank+1
            if dest<comm.size:
                comm.send(None, dest=rank+1, tag=333)



