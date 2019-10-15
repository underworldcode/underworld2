Underworld Docker Schema
--------------------------------

https://hub.docker.com/u/underworldcode/

**base**:

    Inherits from a `Debian` image, and configures the software stack required by Underworld. This
    image is only built when explicitly triggered by a developer. It always pulls from the latest
    available `Debian` image. At the beginning of a new release cycle (ie, just after a release has been
    made), this image is regenerated from the latest available `Debian` image and will install the latest
    software versions from apt and pip.
    
**underworld2**:

    Inherits from `base`.
    This image is automatically built (tag:dev) whenever a commit is pushed to the Underworld Github
    repository (branch: development). It inherits from an SHA digest pinned `base` image so that the full 
    software stack is fixed within any release cycle. This also has the added benefit of recording the `base`
    image's SHA digest to the git repo, and therefore allowing the **exact** software stack to be
    reproduced (ie, pulled down from Docker Hub) at a later date if necessary. At the beginning
    of a release cycle, the `base` image is regenerated, and the `underworld2`  image is
    then pinned to the new version of the `base` image. This allows thorough testing by the dev
    team before the next release.
    Stable releases are built manually and pushed up to this registry with appropriate tags.
    
Refer to associated Dockerfiles for full implentation details.

Releases
-----------

Example, if `2.5` release:

** Tag the release locally:
```
$ docker tag 6649e5e26534 underworldcode/underworld2:2.5.0b
```
** Push the tagged image
```
$ docker login
$ docker push underworldcode/underworld2:2.5.0b
```



