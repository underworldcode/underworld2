# LavaVu #

The acronym stands for: **L**ightweight, **A**utomatable  **V**isualisation & **A**nalysis **V**iewing **U**tility, but the name is also a reference to its roots as a viewer for geophysical simulations (ie: the viewing of simulated lava). It's also a unique enough name that anyone should hopefully be able to find the repository with google.

The project sprang out of the gLucifer framework (https://underworldproject.org/hg/gLucifer) for visualising geodynamics simulations when the OpenGL visualisation code was separated from the sampling code. It has become a more general purpose tool in recent years and has been until recently uninspiringly named "gLucifer Viewer".
GLucifer itself (which is currently being reworked to include python integration) continues as a sampling tool for Underworld simulations, the output of which are visualisation databases that can be viewed in LavaVu.

Basically it's a scriptable 3D visualisation tool, it can correctly render opaque and transparent points and surfaces and volume render by GPU ray-marching. There are also features for drawing vector and tensor fields and tracers (streamlines).

It is capable of producing publication quality high res images and 4D movie output from time varying data sets as well as HTML5 3D visualisations in WebGL.

It uses its own native data format called GLDB based on SQLite for compact storage and fast loading but can read a number of other data formats for import. Even more formats have been supported with python scripts to write the GLDB database.

Control is via a simple verbose scripting language and mouse/keyboard shortcuts but a more user-friendly GUI is being developed to control the viewer from a web browser via a built in web server.

A CAVE mode is provided by utilising Omegalib (http://github.com/uic-evl/omegalib) to allow use in Virtual Reality and Immersive Visualisation facilities, such as the CAVE2 at Monash (http://monash.edu/mivp).

### What is this repository for? ###

This is the public source code repository for all development on the project.
Currently I am the sole developer but contributions are welcome.

### How do I get set up? ###

The simplest way to get started on a Unix based OS is clone this repository and build from source:
(You will need to first install Mercurial http://mercurial.selenic.com/downloads)

```
#!shell script

  hg clone https://bitbucket.org/underworldproject/lavavu
  cd lavavu
  make -j4
```


If all goes well it should build the software and the viewer will be built and ready to run under ./bin, try running with:
  bin/LavaVu

TODO:

Dependencies required - Mercurial, otherwise base build should not require any

Releases: prebuilt versions for Windows, Mac, Linux

### Who do I talk to? ###

* Owen Kaluza (at) monash.edu

WIll be adding documentation to the Wiki https://bitbucket.org/underworldproject/lavavu/wiki/Home

TODO: 
License, include/update documentation