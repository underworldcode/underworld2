CHANGES: Underworld2
=======================

Release 2.2.2b  [2016-03-08]
----------------------------
Bug fixes:
* Critical bug fixed where retrieving symmetric tensor in 3d returned 2d result.

Release 2.2.1b  [2016-02-14]
----------------------------
Documentation
* Updates to make api documentation available via read the docs: http://underworld2.readthedocs.io/

Release 2.2.0b  [2016-02-11]
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
