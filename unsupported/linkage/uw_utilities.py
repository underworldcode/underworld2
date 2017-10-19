# Utilities to handle Underworld mesh and swarm variables fields...
# Romain Beucher June 2017
# romain.beucher@unimelb.edu.au
import underworld as uw
import numpy as np
from mpi4py import MPI

comm = MPI.COMM_WORLD

def get_UW_velocities(surfacePoints, velocityField):

    local_top_vy = np.zeros_like(surfacePoints)
    global_top_vy = np.zeros_like(surfacePoints)
    local_counts = np.zeros_like(surfacePoints)
    global_counts = np.zeros_like(surfacePoints)

    for i, coords in enumerate(surfacePoints):
        try:
            local_top_vy[i] = velocityField.evaluate(np.array([coords]))[0]
            local_counts[i] += 1
        except ValueError:
            pass

    # reduce local arrays into global_array
    comm.Allreduce(local_top_vy, global_top_vy)
    comm.Allreduce(local_counts, global_counts)
       
    global_top_vy /= global_counts

    # Calculate and return sep velocities
    return global_top_vy
