Version Control
===============
* Create a new branch off the `development` branch for the new release. This will
  allow development on the dev branch to proceed unhindered by changes required
  for the release. Naming convention example`v2.13.x`
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
* Review `development_guidelines.md`.
* Review docstrings updates for deprecation warnings.
* Check for other DEPRECATE flags in the code.
* Check autocomplete to ensure no garbage has slipped in. Non
  user relevant objects should be made private so they don't appear in
  autocomplete suggestions.
* Review ``docs/`` for style, relevance and updates.
* Review ``docs/install_guides``, in particular ensure that guides 
  for Gadi & Magnus are up to date.
* Review docs generated at http://underworld2.readthedocs.io/
	- make sure builds are passing
	- review latest content
* Review top level `README.md`.
* Review `LICENSE.md`.
* Review `.zenodo.json` (and validate using `jsonlint`).
* Update the copyright information if necessary.
* Add new Binder link, and keep link to old Binder.
* Generate/update change log (`CHANGES.md`).
* Review cheat sheet contents.
* Increment version number within ``underworld/_version.py``
  (check `development_guidelines.md` for details on version numbering).
* Update `FROM` tag in top level (binder) Dockerfile.

Testing
=======
* Ensure 'test_long.py' script run to completion without raising an exception.
  Test on Docker/NCI/Pawsey. Note that parallel jobs within script may need to be
  executed by hand on NCI/Pawsey.
* Large parallel tests (NCI/Pawsey).
  Compare against previous for timing performance, and confirming expected convergence rates for SolDB3d.
  - IO=0, JOBS="1 2 4 6 8 10 12 14 16 18 20 24", BASE=16, ORDER=2, RTOL=1e-11, PENALTY=-1, MODEL=SOLDB3d
  - IO=0, JOBS="1 2 4 6 8 10 12 14 16 18 20 24", BASE=32, ORDER=1, RTOL=1e-6,  PENALTY=-1, MODEL=SOLH
  - IO=1, JOBS="1 2 4 6 8 10",                   BASE=32, ORDER=1, RTOL=1e-6,  PENALTY=-1, MODEL=SOLH
* Confirm Jenkins is running tests and tests are returning expected results. 
* Confirm Docker images are being generated correctly.
* Confirm metrics are being dispatched.

Creating the release
====================
* Create the release from within Github.
* Check `docker/docker.md` for docker image release information.
* Add tagged documentation version at http://underworld2.readthedocs.io/  
* Package for PyPi: `python setup.py sdist`
* Upload to PyPi: `twine upload dist/* -r pypi`
- The url and sha256 fields must be updated in the `conda/meta.yaml`. We want the url not the git_url and
  it should point to the tarball of the release.
  The `.github/workflow/conda_deploy.yaml` actions will have to be triggered by the release create process.
  For now the action build and push the conda package to our underworldcode channel.
  Once we are on conda-forge, the package will have to be maintained using a conda-forge feedstock.

Announce new version
====================
* via blog & facebook.

After the release
============
* Increment version number within ``underworld/_version.py`` on dev branch (eg 2.6.0-dev)
* Update `FROM` tag in top level (binder) Dockerfile to use dev images.
* Check `docker/docker.md` for docker related actions.


