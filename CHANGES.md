CHANGES: Underworld2
=======================

Release 2.6.1b [2018-10-31]
------------------
* Remove `warning` module visibility from top level underworld 
  module.

Release 2.6.0b [2018-10-18]
------------------
Enhancements:
* Removed submodules in favour of direct access.
* Petsc-3.9.x compatible.
* Support for nested SafeMaths operations.
* Improved function error messaging.

Release 2.5.1b [2018-04-13]
---------------------------
Documentation:
* Fixes for Sphinx API documentation generation.

Release 2.5.0b [2018-04-10]
---------------------------
New features:
* New 'long' type swarm variable.
* User provided post-solve callback functionality added.

Enhancements:
* Speed up (minimum 5x) for SwarmVariable reload.
* Petsc-3.8 compatible.
* Non root dockerfile now. 

Documentation:
* Viscoelastic tutorial, see ./docs/tutorials/
* Update raijin and magnus build instructions.

API Changes:
* New interface for a MeshVariable,  mesh.add_variable().
* New interface to Integrate, mesh.integrate().


Release 2.4.1b [2017-10-24]
----------------------------
*  Updating release to the correct version of lavavu


Release 2.4.0b [2017-10-23]
----------------------------
Documentation:
* Darcy flow models.
* New NCI Raijin build recipe.
* Many new models and tutorials.
* Cheatsheet added.

Enhancements:
* Docker usage no longer requires tokens.
* KSP convergence status now captured in python interface.
* `solve()` method now displays warnings for floating point over/underruns.
* Many general enhancements to visualisation.

New Features:
* New features for `min_max` function.
* solHA added.

API Changes:
* `min_max` function no longer accepts vector functions
  by default, with instead a fn_norm function having to  
  be provided by the user.
* Neumann condition now takes `indexSetsPerDof` parameter instead of `nodeIndexSet`.
  
  Bug fixes:
  * Catch out fortran ordered numpy arrays when using `evaluate()` method.
  * Parallel isosurface visualisation fixed.
  * Corrected sampling for FieldSampler visualisation object.
  

Release 2.3.0b [2017-07-07]
----------------------------
Enhancements:
* Improved swarm reload times for parallel simulations.
* Efficiency improvements for large proc count (>128) parallel simulations.
* Faster algorithms for swarms with deformed mesh.
* Many updates to visualisation routines. 
* Compressible Stokes general improvements.
* New example models and analytic tests.
* Addressed numerous memory leaks (some minor ones still exist).
* Improved error handling, documentation & testing.


New features:
* User provided pre-solve callback function.
* Updated or new vis objects: Isosurfaces, contours, discrete colourmap.

API Changes:
* Neumann condition change in user provided stress function.


Release 2.2.2b  [2017-03-08]
----------------------------
Bug fixes:
* Critical bug fixed where retrieving symmetric tensor in 3d returned 2d result.

Release 2.2.1b  [2017-02-14]
----------------------------
Documentation
* Updates to make api documentation available via read the docs: http://underworld2.readthedocs.io/

Release 2.2.0b  [2017-02-11]
----------------------------
Enhancements:
* Improved function evaluation efficiency.
* Handle numpy int types in index sets correctly.
* Webgl figure saving.
* General visualisation enhancements.
* Better testing for visualisation.
* petsc 3.7 compatibility

New Features:
* ParticleFound function. Allows for 'sparse' swarm operations.
* Figure copying functionality
* SurfaceOnMesh visualisation object

Documentation:
* HTML api documentation.
* Big docstring updates.

Deprecated:
* 'swarm' parameter now removed in favour of 'voronoi_swarm' parameter.

Release 2.1.2b  [2016-10-31]
----------------------------
* Bug fix for the stokes system's psuedo incompressibility


Release 2.1.1b  [2016-10-13]
----------------------------
* Improvements for docker usage.


Release 2.1.0b  [2016-10-11]
----------------------------

New Features:
* Shadow particles sync added.
* Finer solver execution control.
* Added XOR function.
* Can now describe functions using python tuples: fn = (fn1, fn2, fn3)
* Inline interactive visualisation.
* Better handling of systems without rendering capabilities. 
* gLucufer Surface renderer resolution setting.
* Parallel and recursive test runner.

Bug Fixes:
* General improvements to stability and error handling.
* MeshVariables can not be used with multiple SLE systems.
* glucifer deadlock when deleting from db in parallel. 
* glucifer DB situation where zero compression results in abort().
* glucifer valueRange fix.
* Fn.MinMax parallel fix.
* Issue saving swarms in parallel.

Documentation:
* Big update for User Guide.
* New examples added.
* Examples updated for better usage patterns.
* Updates for docstrings & doctests. 
* New & updated build recipes. 

Interface changes:
* SLE 'swarm' option renamed to 'voronoi_swarm'
* Conditions can be passed in directly now instead of in lists. 
* underworld.help function removed.


Release 2.0.3b2 [2016-06-08]
----------------------------

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
