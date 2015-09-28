## Underworld install on MacOS 10.10.3 from new

Everything other than installation of Xcode requires access to the terminal.

1) Install Xcode

(From app store)

Run it once to accept licence or

`sudo xcodebuild -license`

2) Install homebrew

`ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`

3) Install dependencies via homebrew

`brew install python`
`brew install petsc`
`brew install libpng`
`brew install libxml2`
`brew install mercurial`
`brew install freeglut`

you may need to run

`brew tap homebrew/science`
`brew tap homebrew/x11`

4) Install python dependencies via pip

`pip install numpy`
`pip install scipy`
`pip install matplotlib `
`pip install ipython`
`pip install pyzmq`
`pip install jinga2`
`pip install tornado`
`pip install jsonschema`
`pip install terminado # provides a shell via the ipython notebook`

5) Obtain u/w source

`hg clone https://lmoresi@bitbucket.org/underworldproject/underworld2 ~/underworld2-2015.07.07`

6) Configure & Build

`export HDF5_DIR=/usr/local/opt/hdf5`
`export LIBXML2_DIR=/usr/local/opt/libxml2`
`export LIBPNG_DIR=/usr/local/opt/libpng`
`export PYTHON_DIR=/usr/local/opt/python/Frameworks/Python.framework/Versions/Current`

`cd ~/underworld2-2015.07.07`
`cd libUnderworld`
`./configure.py`
`./scons.py`

There will be many warnings

7) Did it work ?

Go back to the underworld directory

`cd ~/underworld2-2015.07.07`
`source InputFiles/updatePyPath.sh`

Fire up ipython and see if it works

`ipython`
`In [1]: import underworld`

You should see in response:

	StGermain Framework. Copyright (C) 2003-2005 VPAC.
	Toolbox "StgDomain" found using ~/underworld2-2015.07.07/libUnderworld/build/lib/StgDomain_Toolboxmodule.dylib
	StgDomain (Domain Library). Copyright (C) 2003-2007 VPAC.
	Toolbox "StgFEM" found using ~/underworld2-2015.07.07/libUnderworld/build/lib/StgFEM_Toolboxmodule.dylib
	StgFEM (Finite Element Framework). Copyright (C) 2003-2005 VPAC.
	Toolbox "PICellerator" found using ~/underworld2-2015.07.07/libUnderworld/build/lib/PICellerator_Toolboxmodule.dylib
	PICellerator (Particle-In-Cell Framework). Copyright (C) 2005-2010 VPAC & Monash University.
	Toolbox "Underworld" found using ~/underworld2-2015.07.07/libUnderworld/build/lib/Underworld_Toolboxmodule.dylib
	Underworld (Geodynamics Framework). Copyright (C) 2005-2010 Monash University.
	Toolbox "gLucifer" found using ~/underworld2-2015.07.07/libUnderworld/build/lib/gLucifer_Toolboxmodule.dylib
	gLucifer (Visualisation Framework). Copyright (C) 2005-2010 Monash University.
	Toolbox "ImportersToolbox" found using ~/underworld2-2015.07.07/libUnderworld/build/lib/ImportersToolbox_Toolboxmodule.dylib
	ImportersToolbox. Copyright (C) 2012 Monash University.
