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
* Review docstrings udpates for deprication warnings.
* Review ``docs/`` for style, relevance and updates.
* Review ``docs/development/broken``, contents to be kept at a minimum.
* Review ``docs/development/unsupported``, contents to be kept at a minimum.
* Review docs generated at http://underworld2.readthedocs.io/
	- make sure builds are passing
	- review latest content

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
* Review completed issues.
* Generate and review change log.

Version numbers and tags
========================
* Increment version number within ``underworld/__init__.py``
* Update Dockerfiles:
** Within the `underworld_untested2` Dockerfile, ensure that the branch that
   is checked out is correct (either `development` or `master`).
** Within the `underworld_untested2` and `underworld2` Dockerfile, ensure that
   we inherit from the correct parent tagged Dockerfile. The git `development`
   branch should use the `dev` tag, while the `master` branch should use the 
   `latest` tag. 

Creating the release
====================
* Create the release from within Github. We will mark as pre-production
  while still in beta. 
* Also, create corresponding tags for the docker images:
** NOTE: dockerhub no longer appears to publish the digest ID in the 'build 
   details' tab.  But the digest isn't really need if you only need the 
   most recent 'development' (dev) or 'master' (latest) image, as you 
   can simply pull these directy, and then determine the image ID as below. 
** Once a new master/latest branch `underworld2` docker image is created on 
   dockerhub, find its digest ID. To do this, go to the `Build Details` tab
   on the Dockerhub page for `underworld2`, click on the `latest` tagged 
   build just created (it should be marked "Success"!), scroll to the very 
   bottom and you should find a sha256 ID. 
** Pull this image locally:
```
$ docker pull underworldcode/underworld2:latest@sha256:cbafa1d665b22445c3318b8567af1e7bfb8d5e49317a420debeba78343e76c8c
```
** Determine the image ID corresponding to this digest
```
$ docker images --digests
```
** Tag the release locally as required:
```
$ docker tag 6649e5e26534 underworldcode/underworld2:2.1.1b
```
** Push the tagged image
``` 
$ docker push underworldcode/underworld2:2.1.1b
```
** Add tagged documentation version at http://underworld2.readthedocs.io/  

Announce new version
====================
* via blog & facebook.


