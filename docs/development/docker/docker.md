Underworld Docker Schema
--------------------------------

https://hub.docker.com/u/underworldcode/

**lavavu**: 

    This Dockerfile generates the software stack required by `lavavu`. The resulting image can operate
    stand alone to run `lavavu` within `Jupyter`. This image should be rebuilt after a new release is published.  

**petsc**: 

    This Dockerfile generates the software stack required by `petsc`. It includes `MPI`. The resulting image can operate
    stand alone to, for example, run `petsc4py`. This image should be rebuilt after a new release is published.  

**base**:

    This Dockerfile generates the software stack required by Underworld. The corresponding image is only built 
    when explicitly triggered by a developer. At the beginning of a new release cycle (ie, just after a release has been
    made), this image is regenerated rebuilt to update software stack. The `lavavu` and `petsc` images should
    be rebuilt first. 
    
**underworld2**:

    Inherits from `base`.
    This image is automatically built (tag:dev) whenever a commit is pushed to the Underworld Github
    repository (branch: development). It inherits from an SHA digest pinned `base` image so that the full 
    software stack is fixed within any release cycle. This also has the added benefit of recording the `base`
    image's SHA digest to the git repo, and therefore allowing the **exact** software stack to be
    reproduced (ie, pulled down from Docker Hub) at a later date if necessary. At the beginning
    of a release cycle, the `base` image is regenerated, and the `underworld2` image is
    then pinned to the new version of the `base` image. This allows thorough testing by the dev
    team before the next release.

    Stable releases are built manually and pushed up to this registry with appropriate tags.
    
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



