from scipy.spatial import cKDTree as kdTree
import numpy as np

def nn_evaluation(_fromCoords, _toCoords, n=1, weighted=False):

    """
    This function provides nearest neighbour information for uw swarms,
    given the "_toCoords", which could be the .data handle (coordinates) of a mesh or a different swarm,
    this function returns the indices of the n nearest neighbours in "_fromCoords" (will usually be swarm.particleCoordinates.data )
    it also returns the inverse-distance weights if weighted=True.

    The function works in parallel, if the example below is followed


    Usage
    ------------
    #get the n indexes, weights and distances
    ix, weights, d = nn_evaluation(swarm.particleCoordinates.data, toSwarm.particleCoordinates.data, n=n, weighted=False)

    #apply to the 'toSwarm' variable in a parallel-safe way
    if len(weights): #parallel safety
        toSwarmVar.data[:,0] =  np.average(fromSwarmVar.evaluate(fromSwarm)[:,0][ix], weights=weights, axis=len((weights.shape)) - 1)

    """

    if len(_toCoords) > 0: #this is required for safety in parallel

        tree = kdTree(_fromCoords)
        d, ix = tree.query(_toCoords, n)
        if n == 1:
            weights = np.ones(_toCoords.shape[0])
        elif not weighted:
            weights = np.ones((_toCoords.shape[0], n))*(1./n)
        else:
            weights = (1./d[:])/(1./d[:]).sum(axis=1)[:,None]
        return ix,  weights, d
    else:
        return  np.empty(0, dtype="int"),  np.empty(0, dtype="int"), np.empty(0, dtype="int")
