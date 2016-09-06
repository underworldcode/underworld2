Review issue tracker 
====================
* Complete all issues tagged for current milestone 

Documentation review 
====================
* Review docstrings udpates.
* Review ``doc/`` for style, relevance and updates.
* Review ``doc/development/broken``, contents to be kept at a minimum.
* Review ``doc/development/unsupported``, contents to be kept at a minimum.

Testing
=======
* Ensure 'run_tests.py' script run to completion without raising an exception.
* Test on docker, OSX, linux, NCI and Pawsey machines.

Tag release candidate 
=====================
* Increment version number
* Update ``underworld/__init__.py``

Final Science/Production testing
================================
* High resolution production tests on raijin, magnus run to completion with correct results. These should include models from ``doc/publications``, record overall condition and walltime with optimised code.  TODO: include detailed list of models, parameters and completion state.
* Ensure jenkins and docker are working correctly.

Document release
================
* Review completed issues.
* Generate and review change log.
* Master branch, docker to be built --with-debugging=0. 

Announce new version
====================
* via blog & facebook.


