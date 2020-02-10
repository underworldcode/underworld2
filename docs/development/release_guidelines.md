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
* Review this document.
* Review `guidelines.md`.
* Review docstrings updates for deprecation warnings.
* Check for other DEPRECATE flags in the code.
* Check autocomplete to ensure no garbage has slipped in. Non
   user relevant objects should be made private so they don't appear in
   autocomplete suggestions.
* Review ``docs/`` for style, relevance and updates.
* Review docs generated at http://underworld2.readthedocs.io/
	- make sure builds are passing
	- review latest content
* Review top level `README.md`.
* Review `LICENSE.md`.
* Review `.zenodo.json`.
* Update the copyright information if necessary.
** Add new Binder link, and keep link to old Binder.
* Generate/update change log (`CHANGES.md`).
* Review cheat sheet contents.
* Increment version number within ``underworld/_version.py``
  (check `guidelines.md` for details on version numbering).
* Update `FROM` tag in top level (binder) Dockerfile.

Testing
=======
* Ensure 'run_tests.py' script run to completion without raising an exception.
* Test on
** docker
** OSX
** linux
** NCI
** Pawsey
* Scaling tests on NCI and Pawsey machines.
* High resolution production tests on Raijin, Magnus run to completion with 
  correct results.
* Ensure jenkins and docker are working correctly.
* Ensure metrics are being dispatched.

Creating the release
====================
* Create the release from within Github.
* Check `docker/docker.md` for docker image release information.
* Add tagged documentation version at http://underworld2.readthedocs.io/  
* Package for PyPi: `python setup.py sdist`
* Upload to PyPi: `twine upload dist/* -r pypi`

Announce new version
====================
* via blog & facebook.

After the release
============
* Increment version number within ``underworld/_version.py`` on dev branch (eg 2.6.0-dev)
* Check `docker/docker.md` for docker related actions.


