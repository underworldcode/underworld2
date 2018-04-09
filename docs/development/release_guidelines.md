Version Control
===============
* Create a new branch off the `development` branch for the new release. This will
  allow development on the dev branch to proceed unhindered by changes required
  for the release.
* Merge the `master` branch into the release branch immediately. This should 
  largely be a fast forward, except for changed version numbers (and Dockerfile
  related tags).
* Once the release is ready (as per details below), merge release branch back 
  into `development` to capture release related changes. This step is also useful
  to ensure all docker images are being built correctly at Dockerhub before
  merging into master. Remember to switch dev branch version numbers and 
  Dockerfiles back (as merge from master will change them). 
* Finally, once satisfied that docker images on dev are being built correctly, 
  merge the release branch (not the dev branch!) into `master`. This should be a 
  fast forward, otherwise something has gone wrong.
* The release branch may now be deleted.    

Review issue tracker 
====================
* Complete all issues tagged for current milestone 

Documentation review 
====================
* Review docstrings updates for deprication warnings.
* Review ``docs/`` for style, relevance and updates.
* Review ``docs/development/broken``, contents to be kept at a minimum.
* Review ``docs/development/unsupported``, contents to be kept at a minimum.
* Review docs generated at http://underworld2.readthedocs.io/
	- make sure builds are passing
	- review latest content
* Review top level README.md.
* Review lsheet contents.

Testing
=======
* Ensure 'run_tests.py' script run to completion without raising an exception.
* Test on docker, OSX, linux, NCI and Pawsey machines.
* Scaling tests on NCI and Pawsey machines.

Final Science/Production testing
================================
* High resolution production tests on raijin, magnus run to completion with 
  correct results. These should include models from ``docs/publications``, 
  record overall condition and walltime with optimised code.  TODO: include 
  detailed list of models, parameters and completion state.
* Ensure jenkins and docker are working correctly.

Document release
================
* Generate and review change log.

Version numbers and tags
========================
* Increment version number within ``underworld/__init__.py``
* Check `guidelines.md` for release numbering details.

Creating the release
====================
* Create the release from within Github. We will mark as pre-production
  while still in beta.
* Check `docker/docker.md` for docker image release information.
* Add tagged documentation version at http://underworld2.readthedocs.io/  

Announce new version
====================
* via blog & facebook.


After the release
============
* Check `docker/docker.md` for docker related actions.


