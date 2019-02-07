Numerical methods - background
==============================


Description
-----------

Underworld is a Lagrangian integration point finite element code. This is a modernization of the original particle-in-cell concept from the 1960s in which a structured mesh and an unstructured particle swarm co-exist. The mesh is used to solve diffusion-dominated parts of the problem and the particle swarm is used to track advected quantities. In the finite element context, the mapping from mesh to particles is through the usual basis functions of the elements and the mapping from particles to mesh is through the integration scheme used to build up the stiffness matrices etc.

The applications of the method are mainly in modelling of complex fluids where very large strains occur but the material also has a memory of the entire strain / strain-rate history. In geosciences this occurs due to the visco-elasticity of rocks at lithospheric temperature and their tendency to develop fabric (lattice preferred orientation and stress/strain-dependent grain size). Problems with material interfaces which undergo severe distortion during the deformation are also naturally handled by this method provided there is no slip on the interface.

Background
----------

The method has been published in detail in Moresi et al (2002, 2003)[1]. These papers dealt exclusively with 2D applications but in recent years, we have introduced a number of improvements in the method to enable us to scale the problem to 3D. For example we developed a fast discrete Voronoi method to compute the integration weights of the particle-to-mesh mapping efficiently [2]. We have also concentrated on extremely robust solvers / preconditioners which are necessary because the material variations and geometrical complexity are both large and unpredictable at the start of the simulation.

The benefit of this approach is associated with the separation of the computational mesh from the swarm of points which track the history. This allows us to retain a much more structured computational mesh than the deformation / material history would otherwise allow. We can take full advantage of the most efficient geometrical multigrid solvers and there is no need to preserve structure during any remeshing operations we undertake (for example if we do need to track a free surface or an internal interface). Although there are several complexities introduced by enforcing this separation, we find that the benefits, for our particular class of problems, are significant.

Implementation and parallelism
------------------------------

Underworld is implemented using the StGermain framework . This provides the essential infrastructure to manage i/o, meshes, particle swarms, finite element operations, in a parallel (domain decomposition, message passing) environment. The numerical solvers are based around the PETSc software suite which focuses on delivering good parallel scalability. Good scalability results have been achieved for over 10000 core simulations.


[1]:
    Moresi, L. N., F. Dufour, and H. B. Muhlhaus (2003), A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials, Journal of Computational Physics, 184(2), 476–497, doi:10.1016/S0021-9991(02)00031-1.

[2]:
    Velić, M., D. A. May, and L. N. Moresi (2009), A fast robust algorithm for computing discrete voronoi diagrams, Journal of Mathematical Modelling and …, doi:10.1007/s10852-008-9097-6.
    
[3]
    L. Moresi, F. Dufour, and H. B. Muhlhaus. A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials. Journal Of Computational Physics, 184:476–497, 2003.

