## Underworld install on MacOS 10.11.x

If this is a machine that is being upgraded from 10.10.x or below, then it seems easiest to nuke the homebrew installation and start afresh.

  `brew update`
  `brew remove --force $(brew list)`
  `brew doctor`

Everything other than installation of Xcode requires access to the terminal.

1) Install Xcode

(From app store)

Run it once to accept the licence, then quit

2) Install homebrew

Note, that `/usr/local` does not exist on a new machine and it can be tricky to create it. The details are [here on the homebrew github page](https://github.com/Homebrew/homebrew/blob/master/share/doc/homebrew/El_Capitan_and_Homebrew.md) and we recommend checking them carefully since things are likely to change. You may need to revisit these instructions after each system-software update.

`ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`

Now, to check if there is anything on the system than needs fixing before moving on to install packages, run:

`brew doctor`

3) Install dependencies via homebrew

`brew install python`
`brew install petsc`
`brew install libpng`
`brew install libxml2`
`brew install freeglut`
`brew install swig`

you may need to run

`brew tap homebrew/science`
`brew tap homebrew/x11`

4) Install python dependencies via pip

`pip install numpy`
`pip install scipy`
`pip install matplotlib `
`pip install jupyter`

5) Obtain u/w source

`git clone https://github.com/underworldcode/underworldcode.github.io.git ~/underworld2-2015.10.01`

6) Configure & Build

Set the environment variables for PETSC. `brew info PETSC` will tell you the value required for `$PETSC_DIR`. Use this value since the version in /usr/local/opt is incomplete.

`export PETSC_DIR=/usr/local/Cellar/petsc/3.6.1_3/real` (for example)

The following may be necessary:

`export HDF5_DIR=/usr/local/opt/hdf5`
`export LIBXML2_DIR=/usr/local/opt/libxml2`
`export LIBPNG_DIR=/usr/local/opt/libpng`
`export PYTHON_DIR=/usr/local/opt/python/Frameworks/Python.framework/Versions/Current`

`cd ~/underworld2-2015.10.01`
`cd libUnderworld`
`./configure.py`
`./scons.py`
`cd libUnderworldPy`
`./swigall.py`
`cd ..`
`./scons.py`

There will be many warnings

7) Did it work ?

Go back to the underworld directory

`cd ~/underworld2-2015.10.01`
`source UserGuide/updatePyPath.sh`

Fire up ipython and see if it works

`ipython`
`In [1]: import underworld`

You should see in response:

	Toolbox "StgDomain" found using ...
	Toolbox "StgFEM" found using ...
	Toolbox "PICellerator" found using ...
	Toolbox "Underworld" found using ...
	Toolbox "gLucifer" found using ...
	Toolbox "Solvers" found using ...

The paths on your system to the various toolboxes will be spat out in the preamble text "`found using ... `"
