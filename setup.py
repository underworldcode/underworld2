#!/usr/bin/env python
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
Underworld
==========

Underworld is a python-friendly version of the Underworld geodynamics
code which provides a programmable and flexible front end to all the
functionality of the code running in a parallel HPC environment. This
gives significant advantages to the user, with access to the power of
python libraries for setup of complex problems, analysis at runtime,
problem steering, and coupling of multiple problems.

Underworld is integrated with the literate programming environment of
the jupyter notebook system for tutorials and as a teaching tool for
solid Earth geoscience.

Underworld is an open-source, particle-in-cell finite element code
tuned for large-scale geodynamics simulations. The numerical algorithms
allow the tracking of history information through the high-strain
deformation associated with fluid flow (for example, transport of the
stress tensor in a viscoelastic, convecting medium, or the advection of
fine-scale damage parameters by the large-scale flow). The finite
element mesh can be static or dynamic, but it is not contrained to move
in lock-step with the evolving geometry of the fluid. This hybrid approach
is very well suited to complex fluids which is how the solid Earth behaves
on a geological timescale.

.. note::

   To install ``underworld`` use::

     $ pip install scons numpy mpi4py h5py
     $ pip install underworld

.. tip::

  You can also install the in-development versions with::

    $ pip install scons numpy mpi4py h5py
    $ pip install git+https://github.com/underworldcode/underworld2@development

  To set the MPI compilers use the environmental variables ``MPICC``, ``MPICXX``, ``MPIF90``.

  To set the required PETSc, set the ``PETSC_DIR`` environment variable (or install the ``petsc`` Python package).

  Provide any ``Underworld`` ./configure options using the environmental variable ``UW_CONFIGURE_OPTIONS``.
    
"""
# NOTE:  This script has been adapted from the PETSc `setup.py` installation script.  

import sys, os
from setuptools import setup
from setuptools.command.install import install as _install
from distutils.util import get_platform, split_quoted
from distutils.spawn import find_executable
from distutils import log

metadata = {
    'provides' : ['underworld'],
    'zip_safe' : False,
    'install_requires' : ['scons','numpy','mpi4py>=1.2.2', 'h5py']
}

def config(prefix, dry_run=False):
    log.info('UW: configure')
    options = [
        '--prefix=' + prefix,
        '--with-debugging=0',
        ]
    try:
        import mpi4py
        conf = mpi4py.get_config()
        mpicc  = conf.get('mpicc')
        mpicxx = conf.get('mpicxx')
    except AttributeError:
        mpicc  = os.environ.get('MPICC')  or find_executable('mpicc')
        mpicxx = os.environ.get('MPICXX') or find_executable('mpicxx')
    if mpicc:
        options.append('--cc='+mpicc)
    if mpicxx:
        options.append('--cxx='+mpicxx)
    options.extend(split_quoted( os.environ.get('UW_CONFIGURE_OPTIONS', '')) )

    if 'PETSC_DIR' in os.environ:
        options.append('--petsc-dir='+os.environ['PETSC_DIR'])
    else:
        try:
            import petsc
            options.append('--petsc-dir='+petsc.get_config()['PETSC_DIR'])
        except:
            pass

    log.info('configure options:')
    for opt in options:
        log.info(' '*4 + opt)
    # Run UW configure
    if dry_run: return
    python = find_executable('python3')
    command = [python, './configure.py'] + options
    status = os.system(" ".join(command))
    if status != 0: raise RuntimeError(status)

def build(dry_run=False):
    log.info('Underworld: build')
    # Run UW build
    if dry_run: return
    python = find_executable('python3')
    command = [python, './compile.py']
    status = os.system(" ".join(command))
    if status != 0: raise RuntimeError(status)

def install(dry_run=False):
    log.info('Underworld: install')
    # Run UW installer
    if dry_run: return
    python = find_executable('python3')
    command = [python, 'scons.py install']
    status = os.system(" ".join(command))
    if status != 0: raise RuntimeError(status)

class context(object):
    def __init__(self):
        self.sys_argv = sys.argv[:]
        self.wdir = os.getcwd()
    def enter(self):
        del sys.argv[1:]
        pdir = os.path.join(os.getcwd(),'underworld/libUnderworld')
        os.chdir(pdir)
        return self
    def exit(self):
        sys.argv[:] = self.sys_argv
        os.chdir(self.wdir)

class cmd_install(_install):

    def initialize_options(self):
        _install.initialize_options(self)
        self.optimize = 1

    def finalize_options(self):
        _install.finalize_options(self)
        self.install_lib = self.install_platlib
        self.install_libbase = self.install_lib

    def run(self):
        prefix = os.path.abspath(self.install_lib)
        ctx = context().enter()
        try:
            config(prefix, self.dry_run)
            build(self.dry_run)
            install(self.dry_run)
        finally:
            ctx.exit()
        #
        self.outputs = []
        for dirpath, _, filenames in os.walk(prefix):
            for fn in filenames:
                self.outputs.append(os.path.join(dirpath, fn))
        #
        _install.run(self)

    def get_outputs(self):
        outputs = getattr(self, 'outputs', [])
        outputs += _install.get_outputs(self)
        return outputs

description = __doc__.split('\n')[1:-1]; del description[1:3]
classifiers = """
Development Status :: 5 - Production/Stable
Intended Audience :: Developers
Intended Audience :: Science/Research
License :: LGPL-3
Operating System :: POSIX
Programming Language :: C
Programming Language :: C++
Programming Language :: Fortran
Programming Language :: Python
Topic :: Scientific/Engineering
Topic :: Software Development :: Libraries
"""

if 'bdist_wheel' in sys.argv:
    sys.stderr.write("underworld: this package cannot be built as a wheel\n")
    sys.exit(1)

version = {}
with open("underworld/_version.py") as fp:
    exec(fp.read(), version)
setup(name='underworld',
      version=version['__version__'],
      description=description.pop(0),
      long_description='\n'.join(description),
      classifiers= classifiers.split('\n')[1:-1],
      keywords = ['Underworld', 'MPI', 'Geodynamics'],
      platforms=['POSIX'],
      license='BSD',

      url='https://github.com/underworldcode/underworld2',
      download_url="",

      author='Underworld Team',
    #   author_email='',
    #   maintainer='',
    #   maintainer_email='',

      packages = ['underworld'],
    #   package_dir = {'': 'config/pypi'},
      cmdclass={'install': cmd_install},
      **metadata)
