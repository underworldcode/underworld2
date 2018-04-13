Underworld Docker Schema
--------------------------------

https://hub.docker.com/u/underworldcode/

Underworld docker images are created across three distinct layers. The basic breakdown
is as follows:

**base**:

    Inherits from a `Debian` image, and configures the software stack required by Underworld. This
    image is only built when explicitly triggered by a developer. It always pulls from the latest
    available `Debian` image. At the beginning of a new release cycle (ie, just after a release has been
    made), this image is regenerated from the latest available `Debian` image and will install the latest
    software versions from apt and pip.
    
**underworld2_untested**:

    Inherits from `base`, downloads Underworld from Github and compiles. This image is
    automatically built whenever a commit is pushed to the Underworld Github
    repository. It inherits from an SHA digest pinned `base` image so that the full software
    stack is fixed within any release cycle. This also has the added benefit of recording the `base`
    image's SHA digest to the git repo, and therefore allowing the **exact** software stack to be
    reproduced (ie, pulled down from Docker Hub) at a later date  if necessary. At the beginning
    of a release cycle, the `base` image is regenerated, and the `underworld2_untested`  image is
    then pinned to the new version of the `base` image. This allows thorough testing by the dev
    team before the next release.
    
**underworld2**:

    Inherits from underworld2_untested, runs tests. This image is built whenever a new
    `underworld2_untested` image is generated successfully. If the tests fails, the image
    is not created, and the developer should then launch the associated `underworld2_untested`
    image to debug the issue.

Refer to associated Dockerfiles for full implentation details.


Releases
-----------

All docker images are built against the `development` git repository branch.  At release time,
automated builds are generated against the release candidate branch as well (remember to update
the Dockerfiles!).

When ready for release, it is best to pull the lastest candidate branch image from `underworld2_untested`
(as it is not possible to tell between the different `underworld2` builds, and some result from `dev` branch
and others are due to the release branch images).  Once pulled, the tests should be run locally, and assuming
success the image should be tagged with the required release number, and then pushed to the `underworld2`
Docker Hub repository.

Example, if `2.5` release:

** Pull newly created 'latest' image:
```
$ docker pull underworldcode/underworld2_untested:v2.5
```
** Launch into the docker image and run required tests. Once happy,
   determine the corresponding image ID:
```
$ docker images
```
** Tag the release locally:
```
$ docker tag 6649e5e26534 underworldcode/underworld2:2.5.0b
```
** Push the tagged image
```
$ docker login
$ docker push underworldcode/underworld2:2.5.0b
```

Magnus images should also be tested and pushed with the `magnus` suffixed release tag.



Magnus
---------

We currently maintain independent images which are compatibile with the Magnus Shifter system.  These
are an all in one Dockerfile build for simplicity. We may consolidate the magnus and standard images in the future. 


