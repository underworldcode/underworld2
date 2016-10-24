
import numpy as np
import underworld as uw
from underworld import function as fn

from .. import interfaces



class fault2D(interfaces.markerLine2D):
    """
    All the bits and pieces needed to define a fault surface (in 2D) from a marker line.
    """

    def __init__(self, mesh, velocityField, pointsX, pointsY, fthickness, fmu, fC, fID, insidePt=(0.0,0.0)):

        # Marker swarms are probably sparse, and on most procs will have nothing to do
        # if there are no particles (or not enough to compute what we need)
        # then set this flag and return appropriately. This can be checked once the swarm is
        # populated.

        self.mu = fmu
        self.C  = fC

        # Everything else is used to set up the interface
        super(fault2D, self).__init__(mesh, velocityField, pointsX, pointsY, fthickness, fID, insidePt)

        return

    ## Function to return the mu, C at the (local) coordinates of the given swarm

    def compute_marker_properties(self, coords):
        """
        Build a mask of values for points within the influence zone.
        """

        if self.empty:
            return np.empty((0,1)), np.empty(0, dtype="int")

        d, p   = self.kdtree.query( coords, distance_upper_bound=self.thickness )

        fpts = np.where( np.isinf(d) == False )[0]

        fmu = np.ones((coords.shape[0],1))
        fmu[fpts] = self.mu

        fC = np.ones((coords.shape[0],1)) * 1.0e99
        fC[fpts] = self.C

        return fmu, fC, fpts



# Global properties such as the rheology function definitions need to be defined through all the faults
# at once and so we bundle all of them into a fault collection and define those functions as methods of
# this class.

class fault_collection(list):
    '''
    Collection (list) of fault objects which together define the global rheology
    '''
    def __init__(self, fault_list=None):

        super(fault_collection, self).__init__()

        if fault_list != None:
            for fault in fault_list:
                if isinstance(fault, fault2D): # or isinstance(fault, fault3D)
                    super(fault_collection, self).append(fault)
                else:
                    print "Non fault object ", fault, " not added to collection"

        return

    def append(self, fault):
        '''

        '''
        if isinstance(fault, fault2D): # or isinstance(fault, fault3D)
            super(fault_collection, self).append(fault)
        else:
            print "Non fault object ", fault, " not added to collection"


    ## Note that this is strictly 2D  !
    def global_fault_strainrate_fns(self, velocityField, faultNormalVariable, proximityVariable):

        ## This is a quick / short cut way to find the resolved stress components.

        strainRateFn = fn.tensor.symmetric( velocityField.fn_gradient )
        directorVector = faultNormalVariable


        #  n1 * edot_11 * n1 +
        #  n2 * edot_21 * n1 +
        #  n1 * edot_21 * n2 +
        #  n2 * edot_22 * n2

    #     _edotn_SFn = ( directorVector[0] * strainRateFn[0] * directorVector[0] +
    #                    directorVector[1] * strainRateFn[2] * directorVector[0] + # Symmetry !!
    #                    directorVector[0] * strainRateFn[2] * directorVector[1] +
    #                    directorVector[1] * strainRateFn[1] * directorVector[1]
    #                 )


        ## "OPTIMIZED" VERSION

        _edotn_SFn = (        directorVector[0]**2 * strainRateFn[0]  +
                        2.0 * directorVector[1]    * strainRateFn[2] * directorVector[0] +
                              directorVector[1]**2 * strainRateFn[1]
                    )

        # any non-zero proximity requires the computation of the above

        _edotn_SFn_Map    = { 0: 0.0 }
        for f in self:
            _edotn_SFn_Map[f.ID] =  _edotn_SFn


        #  s1 = -n2
        #  s2 =  n1
        #
        # -n2 * edot_11 * n1 +
        #  n1 * edot_21 * n1 +
        # -n2 * edot_21 * n2 +
        #  n1 * edot_22 * n2

    #     _edots_SFn = ( -directorVector[1] * strainRateFn[0] * directorVector[0] +
    #                     directorVector[0] * strainRateFn[2] * directorVector[0] +
    #                    -directorVector[1] * strainRateFn[2] * directorVector[1] +
    #                     directorVector[0] * strainRateFn[1] * directorVector[1]
    #                 )

        ## "OPTIMIZED" VERSION


        _edots_SFn = (  directorVector[0] *  directorVector[1] *(strainRateFn[1] - strainRateFn[0]) +
                        strainRateFn[2] * (directorVector[0]**2 - directorVector[1]**2)
                     )


        _edots_SFn_Map = { 0: 1.0e-15 }

        for f in self:
            _edots_SFn_Map[f.ID] =  _edots_SFn


        edotn_SFn =     fn.branching.map( fn_key = proximityVariable,
                                                     mapping = _edotn_SFn_Map)


        edots_SFn =     fn.branching.map( fn_key = proximityVariable,
                                                     mapping = _edots_SFn_Map )


        return edotn_SFn, edots_SFn


    ## Note that this is strictly 2D  !
    def second_viscosity(self, firstViscosityFn, pressureField, edotn_SFn, edots_SFn, muVariable, CVariable):

        viscosityTI2_fn = fn.misc.min(firstViscosityFn*0.99999, fn.misc.max(0.0,
                          firstViscosityFn - (muVariable * (-edotn_SFn * firstViscosityFn + pressureField)  + CVariable) / (fn.math.abs(edots_SFn) + 1.0e-15)))

        return viscosityTI2_fn


    # Don't trust the pressure in mixed elements ... note, this doesn't really make use of
    # the fact that it is in a class ... that could be handled better (pswarm and psignedDistanceVariable)
    # could be internal to the class

    def fix_interface_pressure(self, pressureField, psignedDistanceVariable, pwidth, delta):

        from scipy.interpolate import Rbf as RBF

        vppts = np.where(np.logical_and(psignedDistanceVariable.data > pwidth, psignedDistanceVariable.data < delta))[0]
        ippts = np.where(np.logical_and(psignedDistanceVariable.data < pwidth, psignedDistanceVariable.data > 0.0 ))[0]

        Xs = pressureField.mesh.data[vppts,0]
        Ys = pressureField.mesh.data[vppts,1]
        Zs = pressureField.data[vppts]

        pinterp = RBF(Xs, Ys, Zs, epsilon=pwidth)

        iXs = pressureField.mesh.data[ippts,0]
        iYs = pressureField.mesh.data[ippts,1]

        pressureField.data[ippts,0] = pinterp(iXs, iYs)

        ## Other side

        vppts = np.where(np.logical_and(psignedDistanceVariable.data < -pwidth, psignedDistanceVariable.data > -delta ))[0]
        ippts = np.where(np.logical_and(psignedDistanceVariable.data > -pwidth, psignedDistanceVariable.data <  0.0  ))[0]

        Xs = pressureField.mesh.data[vppts,0]
        Ys = pressureField.mesh.data[vppts,1]
        Zs = pressureField.data[vppts]

        pinterp = RBF(Xs, Ys, Zs, epsilon=pwidth)

        iXs = pressureField.mesh.data[ippts,0]
        iYs = pressureField.mesh.data[ippts,1]

        pressureField.data[ippts,0] = pinterp(iXs, iYs)



    def rbf_pressure_to_mesh(self, mesh, meshPressureField, meshsignedDistanceVariable, pressureField, psignedDistanceVariable, pwidth, delta):

        from scipy.interpolate import Rbf as RBF
        from scipy.interpolate import Rbf as RBF

        vppts = np.where(np.logical_and(psignedDistanceVariable.data > pwidth, psignedDistanceVariable.data < delta))[0]
        ippts = np.where(np.logical_and(psignedDistanceVariable.data < pwidth, psignedDistanceVariable.data > 0.0 ))[0]

        Xs = pressureField.mesh.data[vppts,0]
        Ys = pressureField.mesh.data[vppts,1]
        Zs = pressureField.data[vppts]

        pinterpP = RBF(Xs, Ys, Zs, epsilon=pwidth*0.1)

        ## Other side

        vppts = np.where(np.logical_and(psignedDistanceVariable.data < -pwidth, psignedDistanceVariable.data > -delta ))[0]
        ippts = np.where(np.logical_and(psignedDistanceVariable.data > -pwidth, psignedDistanceVariable.data <  0.0  ))[0]

        Xs = pressureField.mesh.data[vppts,0]
        Ys = pressureField.mesh.data[vppts,1]
        Zs = pressureField.data[vppts]

        pinterpM = RBF(Xs, Ys, Zs, epsilon=pwidth*0.1)

        vppts = np.where(np.logical_and(meshsignedDistanceVariable.data > -pwidth, meshsignedDistanceVariable.data <  0.0  ))[0]
        vXs = meshPressureField.mesh.data[vppts,0]
        vYs = meshPressureField.mesh.data[vppts,1]
        print "Found ", len(vppts), " points in F minus range"

        meshPressureField.data[vppts,0] = pinterpM(vXs, vYs)

        vppts = np.where(np.logical_and(meshsignedDistanceVariable.data < pwidth, meshsignedDistanceVariable.data > 0.0 ))[0]

        vXs = meshPressureField.mesh.data[vppts,0]
        vYs = meshPressureField.mesh.data[vppts,1]
        print "Found ", len(vppts), " points in F plus range"


        meshPressureField.data[vppts,0] = pinterpP(vXs, vYs)


    def rbf_pressure_to_mesh_Monly(self, mesh, meshPressureField, meshsignedDistanceVariable, pressureField, psignedDistanceVariable, pwidth, delta):

            from scipy.interpolate import Rbf as RBF
            from scipy.interpolate import Rbf as RBF


            vppts = np.where(np.logical_and(psignedDistanceVariable.data < -pwidth, psignedDistanceVariable.data > -delta ))[0]
            ippts = np.where(np.logical_and(psignedDistanceVariable.data > -pwidth, psignedDistanceVariable.data <  0.0  ))[0]

            Xs = pressureField.mesh.data[vppts,0]
            Ys = pressureField.mesh.data[vppts,1]
            Zs = pressureField.data[vppts]

            pinterpM = RBF(Xs, Ys, Zs, epsilon=pwidth*0.1)

            vppts = np.where(np.logical_and(meshsignedDistanceVariable.data > -pwidth, meshsignedDistanceVariable.data <  0.0  ))[0]
            vXs = meshPressureField.mesh.data[vppts,0]
            vYs = meshPressureField.mesh.data[vppts,1]
            print "Found ", len(vppts), " points in F minus range"

            meshPressureField.data[vppts,0] = pinterpM(vXs, vYs)

    def rbf_pressure_to_mesh_Ponly(self, mesh, meshPressureField, meshsignedDistanceVariable, pressureField, psignedDistanceVariable, pwidth, delta):

            from scipy.interpolate import Rbf as RBF
            from scipy.interpolate import Rbf as RBF

            vppts = np.where(np.logical_and(psignedDistanceVariable.data > pwidth, psignedDistanceVariable.data < delta))[0]
            ippts = np.where(np.logical_and(psignedDistanceVariable.data < pwidth, psignedDistanceVariable.data > 0.0 ))[0]

            Xs = pressureField.mesh.data[vppts,0]
            Ys = pressureField.mesh.data[vppts,1]
            Zs = pressureField.data[vppts]

            pinterpP = RBF(Xs, Ys, Zs, epsilon=pwidth*0.1)


            vppts = np.where(np.logical_and(meshsignedDistanceVariable.data < pwidth, meshsignedDistanceVariable.data > 0.0 ))[0]
            vXs = meshPressureField.mesh.data[vppts,0]
            vYs = meshPressureField.mesh.data[vppts,1]
            print "Found ", len(vppts), " points in F plus range"

            meshPressureField.data[vppts,0] = pinterpP(vXs, vYs)


    def remove_boundary_values(self, mesh, meshPressureField, meshsignedDistanceVariable, pressureField, psignedDistanceVariable, pwidth, delta):

        from scipy.interpolate import Rbf as RBF
        from scipy.interpolate import Rbf as RBF


        vppts = np.where(np.logical_and(meshsignedDistanceVariable.data > -pwidth, meshsignedDistanceVariable.data <  0.0  ))[0]
        print "Found ", len(vppts), " points in F minus range"

        meshPressureField.data[vppts,0] = 0.0

        vppts = np.where(np.logical_and(meshsignedDistanceVariable.data < pwidth, meshsignedDistanceVariable.data > 0.0 ))[0]
        print "Found ", len(vppts), " points in F plus range"


        meshPressureField.data[vppts,0] = 0.0
