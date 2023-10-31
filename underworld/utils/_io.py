##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##

"""
This module contains helper functionality for h5 IO in Underworld.

Note that you may set the `UW_IO_PATTERN` environment variable to force the
required behaviour:

0 - auto
1 - sequential
2 - collective

You may alternatively set the `PATTERN` module variable directly.
"""


import underworld as uw
import h5py
from mpi4py import MPI
import os
PATTERN = int(os.getenv('UW_IO_PATTERN', 0))

class h5File(uw.mpi.call_pattern):
    """
    This class provides a context manager for h5py file lifecycle management. The
    implementation here differs in that it automatically performs sequential/collective
    read/write within the manager block, depending on the configuration of the available
    h5py package, and/or the user requested write pattern.

    Important: Remember that all code written within the manager's block should be sequential
    safe!! It can be easy to accidentally including global operations (for example
    `self.swarm.particleGlobalCount`) that may work correctly when called collectively, but
    will cause a deadlock when called sequentially.

    Parameters
    ----------
    args: list
        Arguments to pass through to h5py.File constructor.
    kwargs: dict
        Keyword args to pass through to h5py.File constructor.
    """

    def __init__(self, *args, **kwargs):
        global PATTERN
        # figure if we need to run collective or sequential
        h5py_mpi = h5py.get_config().mpi
        if h5py_mpi:
            pattern = 'collective'
        else:
            if (uw.mpi.size>1) and (uw.mpi.rank==0):
                import warnings
                warnings.warn("H5py not available in parallel mode. Read/write will be " \
                              "performed sequentially. Note that this may be slow for " \
                              "parallel simulations.")
            pattern = 'sequential'
            if PATTERN==2:
                import warnings
                warnings.warn("Collective IO not possible as h5py not available in parallel mode. Switching to sequential.")
                PATTERN=1

        if PATTERN != 0:
            if PATTERN==1:
                pattern = "sequential"
            elif PATTERN==2:
                pattern = "collective"
            else:
                raise ValueError("PATTERN must be set to 0:auto, 1:sequential or 2:collective.")

        # Record these for later. Note we shouldn't
        # open file here, as may need to open/close
        # sequentially
        self.args = args
        self.kwargs = kwargs
        self.pattern = pattern

        super(h5File, self).__init__(pattern=pattern)

    def __enter__(self):
        # Call parent __enter__.  This will effect the
        # sequential/collective behaviour.
        super(h5File, self).__enter__()

        h5py_mpi = h5py.get_config().mpi and (self.pattern=="collective")
        if h5py_mpi:
            self.kwargs.update( {"driver": 'mpio', "comm": MPI.COMM_WORLD} )
        else:
            # If writing sequential, non-root procs should use `append` mode.
            if uw.mpi.rank != 0:
                self.kwargs.update( {"mode": 'a'} )

        ## check if File exists if we aren't writing to disk
        if self.kwargs["mode"] != 'w' and not os.path.exists(self.kwargs["name"]):
            fname = self.kwargs["name"]
            raise RuntimeError(f"Can't open file \' {fname} \' ")

        self.h5f = h5py.File(*self.args, **self.kwargs)

        return self.h5f

    def __exit__(self, *args):
        # close file, then send signal to next proc via parent (if
        # running sequentially)
        self.h5f.close()
        super(h5File, self).__exit__(*args)
        # add barrier to be safe
        uw.mpi.barrier()


class _PatchedDataset(h5py.Dataset):
    @property
    def collective(self):
        class _dummy_manager():
            def __enter__(self):
                pass

            def __exit__(self, *args):
                pass
        return _dummy_manager()


def h5_require_dataset(h5f, name, *args, **kwargs):
    """
    This function either creates uses an existing file
    dataset where compatible, or creates a dataset of the
    required size/type.

    It also adds the `collective` context manager to  non-parallel
    datasets. In this scenario, the mananger does nothing and simply
    exists to allow the calling code to be written identically.

    Parameters
    ----------
    h5f: h5py.File
        The h5py File object to add the dataset to.
    name: str
        Name for the dataset.
    args: list
        Arguments to pass through to h5py.Dataset constructor.
    kwargs: dict
        Keyword args to pass through to h5py.Dataset constructor.

    """

    # Try and use an existing dataset to avoid
    # having to reallocate space on disk. If that
    # fails, create
    try:
        dset = h5f.require_dataset(name, *args, **kwargs)
    except TypeError:
        del h5f[name]
        dset = h5f.create_dataset(name, *args, **kwargs)

    # Also, so minimise branching in our code, let's add
    # a `collective` context manager that does nothing
    # in serial. We do this by using a patched version
    # of the class
    if not hasattr(dset, "collective") or (h5f.driver!="mpio"):
        dset.__class__ = _PatchedDataset
    return dset

def h5_get_dataset(h5f, name):
    """
    This function retrieves a dataset.

    It also adds the `collective` context manager to  non-parallel
    datasets. In this scenario, the mananger does nothing and simply
    exists to allow the calling code to be written identically.

    Parameters
    ----------
    h5f: h5py.File
        The h5py File object to retrieve dataset from.
    name: str
        Name of the required dataset.
    """
    dset = h5f[name]
    # Also, so minimise branching in our code, let's add
    # a `collective` context manager that does nothing
    # in serial. We do this by using a patched version
    # of the class
    if not hasattr(dset, "collective") or (h5f.driver!="mpio"):
        dset.__class__ = _PatchedDataset
    return dset
