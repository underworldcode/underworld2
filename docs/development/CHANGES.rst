=======================
CHANGES: Underworld2
=======================

Release 2.0.3b2 [2016-06-08]
==========================

* Pseudo Compressibility python wrapped.
* Neumann boundary conditions for Stokes & SSHeat & AdvectionDiffusion (SUPG!)
* Exposed population control mechanisms. Zero population control now occurs
  without using the uw.swarm.PopulationControl object.
* Bug: Potentially critical bug in the integration to material swarm mapping fixed.
* Glucifer/LavaVu graphics 'Store' workflows introduced.
* Dockers upgraded to use ipyparallel where possible.
* Global evaluation for functions (experimental!)
* Exposed non-linear tolerance.
* Deprecated various old interface options.
* PICIntegrationSwarm -> VoronoiIntegrationSwarm
* Viscoelasticity implementation and example.
* 'deform_swarm' context manager for particle relocations.
* Auto-repopulate now occurs for voronoi swarms (except for the uw.utils.Integral class)
* SUPG source terms.
* MeshVariable_Projection method introduced.
* Various bug fixes.
* Various documentation improvements.
* Various testing improvements.
* Various other minor impementations.