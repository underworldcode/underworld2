CHANGES: Underworld2
=======================

Release 2.9.0b []
---------------------------
New:
* Experimental `pip install` functionality.
* Move to Python 3.7, from Python 3.5.
 
API changes:
* `glucifer` module moved inside `underworld` and 
   renamed `visualisation`.
   Access as `import underworld.visualisation`
* `GLUCIFER_USE_XVFB` -> `UW_USE_XVFB`.
* --prefix=/SOME/DIR now installs to /SOME/DIR/underworld
* `test_basic.py` no longer sets PYTHONPATH

Release 2.8.1b [2019-09-02]
---------------------------
Documentation:
* Fix broken live documentation generator.
* Fix broken and incomplete docstrings.
* Update advection-diffusion test, see `./docs/test/2DCosineHillAdvection.ipynb`. 
* Update README.md to mention pip. 

Release 2.8.0b [2019-07-18]
---------------------------
New:
* MOVE TO PYTHON3. PYTHON2 NO LONGER SUPPORTED.
* New Examples & tests.
* New Semi-Lagrangian Crank Nicolson advection diffusion scheme.
* Collective H5 read/write for faster parallel IO operations. 
* Arbitrary coordinate `evaluate()` for swarm data using nearest 
  neighbouring (kdtree) algorithm. See function section of user guide.
* XDMF for subMesh element types.
* Async test runner. 

Enhancements:
* Move all mpi functionality into `uw.mpi` submodule. 
* Non-parallel h5py is now sufficient for UW, although 
  parallel h5py is recommended for larger parallel simulations.
* Integral returns numpy array results (as opposed to Py natives).
* `GlobalSpaceFillerLayout` deprecated.
* Other tweaks & bug fixes.

Documentation:
* Many class docstrings updates.
* Function user guide section updated. 
* Improved error messaging in parallel.
* Stampede2 build & usage instructions. 
* Update for NCI/Raijin build instructions.

Release 2.7.1b [2019-02-19]
---------------------------
Documentation:
* Dockerfile for TACC/Stampede2
* New build instructions for NCI/Raijin.
* Add dev Binder to README.md

Release 2.7.0b [2019-02-06]
---------------------------

New functionality: 
* Timing module for high level profiling.
* Scaling module for numerical value scaling.
* Index set objects can now directly index numpy arrays.
* `show_grid()` image rendering.

Changes:
* glucifer Surface defaults to use onMesh=True.
* added `data` handle to swarm as shortcut to particle coordinates data.
* Adding aliases for mesh vertex sets (left,right,etc)
* Removed `unsupported` subrepo.
* Docker images switch to MPICH for Shifter compatibility at magnus.

Documentation:
* Examples reorganised.
* All examples now include quantitative tests.
* Moved many examples to models library repo.
    https://github.com/underworldcode/model_library
* Added new example, 08_Uplift_TractionBCs.ipynb.
* Moved benchmarks and tutorials to models library repo.
* Relocated defunct models:
    https://github.com/underworldcode/model_graveyard
* Big overhaul of user guide.
* User guide rendered to html for Readthedocs page.
* Updates for docstrings and doctests.
* Improved error messaging.

Fixes:
* Fix incorrect particle outside domain issue #335
* Other stability improvements.

Release 2.6.1b [2018-10-31]
---------------------------
* Remove `warning` module visibility from top level underworld 
  module.

Release 2.6.0b [2018-09-27]
---------------------------
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
