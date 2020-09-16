Geodynamics - mathematical background
=====================================

The simplest template set of equations for solid-Earth dynamics cover mass, momentum and heat conservation in a highly viscous fluid allowing for additional effects due to elasticity and plasticity. The Stokes momentum equation neglects inertia but includes an additional term on the right hand side that represents stress history associated with an explicit treatment of viscoelasticity.[1] [2] [3]

.. math::

    \begin{equation}
        \tau_{ij,j} - p_{,i} = \rho\left( T, C, \cdots \right) - \tau^{\delta t}_{ij,j}
        \label{eq:stokes-momentum}
    \end{equation}

:math:`\tau` is the deviatoric stress, :math:`p` represents the pressure, :math:`\rho` is density, :math:`T` is the temperature, :math:`C` is a concentration intended to represent changes in composition.

At pressures in planetary interiors, silicate minerals are weakly compressible and this is generally considered as a perturbation to an incompressible flow ignoring bulk viscosity and only considering the long-term elastic resistance to volume change. For the purposes of explaining the formulation, the incompressible constraint equation on the velocity, :math:`u` is sufficient.

.. math::

    \begin{equation}
        u_{i,i} = 0
        \label{eq:stokes-incompressibility}
    \end{equation}

The thermal evolution of the system expresses the balance between heat transport by fluid motion, thermal diffusion and internal heat generation. Additional terms can be included to account for heating due to viscous dissipation, for example, but do not change the overall character of the conservation equation.  

.. math::

    \begin{equation}
        T_{,t} - u_i T_{,i} = \left(\kappa T_{,i} \right)_{,i} + Q_T
        \label{eq:adv-diffusion-thermal}
    \end{equation}

The most significant feature of this system is the spontaneous appearance of boundary layers where horizontal advection and vertical diffusion are approximately balanced. By contrast, compositional variations are characterised by a much smaller, usually negligible, rate of diffusion:

.. math::

    \begin{equation}
        C_{,t} - u_i C_{,i} =  Q_C
        \label{eq:adv-compositional}
    \end{equation}

The thermal and compositional variations couple to the momentum equation through their effect on density. The Boussinesq approximation [6], accounts for the buoyancy forces while neglecting the associated volume change allowing us to assume incompressibility. If the non-diffusive, compositional variation represents a smoothly varying concentration, then the density can be written as [4]

.. math::

    \begin{equation}
        \rho = \rho_0 (1-\alpha \Delta T) (1-\alpha_C \Delta C)
    \end{equation}

In the case where C represents a state with discrete steps (e.g.\ a phase change or immiscible fluids), it is common to let :math:`\rho_0` take discrete values and assume :math:`\alpha_C=0`.

The final requirement is a constitutive relationship for the momentum equation that links the stress to the velocity unknown. Rheology is one of the defining aspects of the dynamics of the mantle, particularly in the cooler parts of the upper boundary layer where elasticity, non-linearity, and brittle behaviour plays a significant role. A general constitutive law can be expressed as:

.. math::

    \begin{equation}
        \frac{\dot{\tau_{ij}} }{\mu} + \frac{\tau_{ij}}{\eta} +
        \lambda \Lambda_{ijkl} \tau_{kl} =
        \frac{\partial u_i}{\partial x_j} + \frac{\partial u_j}{\partial x_i}
        \label{eq:viscoelasticplastic-const-law}
    \end{equation}

where :math:`\mu` is the elastic shear modulus and :math:`\eta` is the shear viscosity (both of which may vary with temperature and composition). :math:`\Lambda` is a structural tensor that represents the orientation of the plastic deformation relative to the applied stress and :math:`\lambda` is a scalar multiplier that is computed to satisfy the stress conditions at yield [5]. Typically, :math:`\eta` varies by several tens of orders of magnitude over the typical temperature ranges expected between the Earth's surface and interior. The orientation tensor and the yield stress are usually modelled to include a simple damage evolution that relates to the work expended in deforming the material at yield.



[1]:
    Moresi, L. N., F. Dufour, and H. B. Muhlhaus (2002), Mantle convection modeling with viscoelastic/brittle lithosphere: Numerical methodology and plate tectonic modeling, Pure And Applied Geophysics, 159(10), 2335–2356, doi:10.1007/s00024-002-8738-3.

[2]:
    Moresi, L. N., F. Dufour, and H. B. Muhlhaus (2003), A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials, Journal of Computational Physics, 184(2), 476–497, doi:10.1016/S0021-9991(02)00031-1.

[3]:
    Farrington, R. J., L. N. Moresi, and F. A. Capitanio (2014), The role of viscoelasticity in subducting plates, Geochemistry, Geophysics, Geosystems, 15(11), 4291–4304, doi:10.1002/2014GC005507.

[4]:
    van Keken, P. E., S. D. King, H. Schmeling, U. R. Christensen, D. Neumeister, and M. P. Doin (1997), A comparison of methods for the modeling of thermochemical convection, J. Geophys. Res., 102(B10), 22,477–22,495.

[5]:
    L. Moresi, H. B. Muhlhaus, V. Lemiale, and D. A. May (2007), Incompressible viscous formulations for deformation and yielding of the lithosphere, Geological Society London Special Publications, 282(1), 457–472, doi:10.1144/SP282.19.

[6]:
    Boussinesq, J. (1903), The ́orie analytique de la chaleur, Vol. 2, Gauthier-Villars, Paris (Reproduction Bibliothe ́que Nationale de France, 1995).