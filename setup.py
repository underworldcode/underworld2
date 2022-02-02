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
functionality of the code running in a parallel HPC environment.

.. note::

   To install ``underworld`` use::

     $ pip install underworld

.. tip::

  You can also install the in-development versions with::

    $ pip install git+https://github.com/underworldcode/underworld2@development

  To set the MPI compilers use the environmental variables ``MPICC``, ``MPICXX``, ``MPIF90``.

  To set the required PETSc, set the ``PETSC_DIR`` environment variable (or install the ``petsc`` Python package).

  Provide any ``Underworld`` ./configure options using the environmental variable ``UW_CONFIGURE_OPTIONS``.
    
"""
import sys, os
import shutil
import platform

import importlib

import subprocess

from typing import List
from pathlib import Path
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext



class CMakeExtension(Extension):
    """
    Custom setuptools extension that configures a CMake project.
    Args:
        name: The name of the extension.
        install_prefix: The path relative to the site-package directory where the CMake
            project is installed (typically the name of the Python package).
        disable_editable: Skip this extension in editable mode.
        source_dir: The location of the main CMakeLists.txt.
        cmake_build_type: The default build type of the CMake project.
        cmake_component: The name of component to install. Defaults to all
            components.
        cmake_depends_on: List of dependency packages containing required CMake projects.
    """

    def __init__(self,
                 name: str,
                 install_prefix: str,
                 disable_editable: bool = False,
                 cmake_configure_options: List[str] = (),
                 source_dir: str = str(Path(".").absolute()),
                 cmake_build_type: str = "Release",
                 cmake_component: str = None,
                 cmake_depends_on: List[str] = ()):

        super().__init__(name=name, sources=[])

        if not Path(source_dir).is_absolute():
            source_dir = str(Path(".").absolute() / source_dir)

        if not Path(source_dir).absolute().is_dir():
            raise ValueError(f"Directory '{source_dir}' does not exist")

        self.install_prefix = install_prefix
        self.cmake_build_type = cmake_build_type
        self.disable_editable = disable_editable
        self.cmake_depends_on = cmake_depends_on
        self.source_dir = str(Path(source_dir).absolute())
        self.cmake_configure_options = cmake_configure_options
        self.cmake_component = cmake_component


class BuildExtension(build_ext):
    """
    Setuptools build extension handler.
    It processes all the extensions listed in the 'ext_modules' entry.
    """

    def initialize_options(self):

        # Initialize base class
        build_ext.initialize_options(self)

        # Override define. This is supposed to pass C preprocessor macros, but we use it
        # to pass custom options to CMake.
        self.define = None

    def finalize_options(self):

        # Parse the custom CMake options and store them in a new attribute
        defines = self.define.split(";") if self.define is not None else []
        self.cmake_defines = [f"-D{define}" for define in defines]

        # Call base class
        build_ext.finalize_options(self)

    def run(self) -> None:
        """
        Process all the registered extensions executing only the CMakeExtension objects.
        """

        # Filter the CMakeExtension objects
        cmake_extensions = [e for e in self.extensions if isinstance(e, CMakeExtension)]

        if len(cmake_extensions) == 0:
            raise ValueError("No CMakeExtension objects found")

        # Check that CMake is installed
        if shutil.which("cmake") is None:
            raise RuntimeError("Required command 'cmake' not found")

        # Check that Ninja is installed
        if shutil.which("ninja") is None:
            raise RuntimeError("Required command 'ninja' not found")

        for ext in cmake_extensions:
            self.build_extension(ext)

    def build_extension(self, ext: CMakeExtension) -> None:
        """
        Build a CMakeExtension object.
        Args:
            ext: The CMakeExtension object to build.
        """

        if self.inplace and ext.disable_editable:
            print(f"Editable install recognized. Extension '{ext.name}' disabled.")
            return

        # Export CMAKE_PREFIX_PATH of all the dependencies
        for pkg in ext.cmake_depends_on:

            try:
                importlib.import_module(pkg)
            except ImportError:
                raise ValueError(f"Failed to import '{pkg}'")

            init = importlib.util.find_spec(pkg).origin
            BuildExtension.extend_cmake_prefix_path(path=str(Path(init).parent))

        # The ext_dir directory can be thought as a temporary site-package folder.
        #
        # Case 1: regular installation.
        #   ext_dir is the folder that gets compressed to make the wheel archive. When
        #   installed, the archive is extracted in the active site-package directory.
        # Case 2: editable installation.
        #   ext_dir is the in-source folder containing the Python packages. In this case,
        #   the CMake project is installed in-source.
        ext_dir = Path(self.get_ext_fullpath(ext.name)).parent.absolute()
        cmake_install_prefix = ext_dir / ext.install_prefix

        # CMake configure arguments
        configure_args = [
            "-GNinja",
            f"-DCMAKE_BUILD_TYPE={ext.cmake_build_type}",
            f"-DCMAKE_INSTALL_PREFIX:PATH={cmake_install_prefix}",
        ]

        # Extend the configure arguments with those passed from the extension
        configure_args += ext.cmake_configure_options

        # CMake build arguments
        build_args = [
            '--config', ext.cmake_build_type
        ]

        # CMake install target
        install_target = "install"

        if platform.system() == "Windows":

            configure_args += [
            ]

        elif platform.system() in {"Linux", "Darwin"}:

            configure_args += [
            ]

        else:
            raise RuntimeError(f"Unsupported '{platform.system()}' platform")

        # Parse the optional CMake options. They can be passed as:
        #
        # python setup.py build_ext -D"BAR=Foo;VAR=TRUE"
        # python setup.py bdist_wheel build_ext -D"BAR=Foo;VAR=TRUE"
        # python setup.py install build_ext -D"BAR=Foo;VAR=TRUE"
        # python setup.py install -e build_ext -D"BAR=Foo;VAR=TRUE"
        # pip install --global-option="build_ext" --global-option="-DBAR=Foo;VAR=TRUE" .
        #
        configure_args += self.cmake_defines

        # Get the absolute path to the build folder
        build_folder = str(Path('.').absolute() / f"{self.build_temp}_{ext.name}")

        # Make sure that the build folder exists
        Path(build_folder).mkdir(exist_ok=True, parents=True)

        # 1. Compose CMake configure command
        configure_command = \
            ['cmake', '-S', ext.source_dir, '-B', build_folder] + configure_args

        # 2. Compose CMake build command
        build_command = ['cmake', '--build', build_folder] + build_args

        # 3. Compose CMake install command
        install_command = ['cmake', '--install', build_folder]
        if ext.cmake_component is not None:
            install_command.extend(['--component', ext.cmake_component])

        print("")
        print("==> Configuring:")
        print(f"$ {' '.join(configure_command)}")
        print("")
        print("==> Building:")
        print(f"$ {' '.join(build_command)}")
        print("")
        print("==> Installing:")
        print(f"$ {' '.join(install_command)}")
        print("")

        # Call CMake
        subprocess.check_call(configure_command)
        subprocess.check_call(build_command)
        subprocess.check_call(install_command)

    @staticmethod
    def extend_cmake_prefix_path(path: str) -> None:

        abs_path = Path(path).absolute()

        if not abs_path.exists():
            raise ValueError(f"Path {abs_path} does not exist")

        if "CMAKE_PREFIX_PATH" in os.environ:
            os.environ["CMAKE_PREFIX_PATH"] = \
                f"{str(path)}:{os.environ['CMAKE_PREFIX_PATH']}"
        else:
            os.environ["CMAKE_PREFIX_PATH"] = str(path)



metadata = {
    'provides' : ['underworld'],
    'zip_safe' : False,
    'install_requires' : ['numpy>=1.22.1','mpi4py>=1.2.2', 'h5py', 'pint']
}
classifiers = """
Development Status :: 5 - Production/Stable
Intended Audience :: Developers
Intended Audience :: Science/Research
Operating System :: POSIX
Programming Language :: C
Programming Language :: C++
Programming Language :: Python
Topic :: Scientific/Engineering
Topic :: Software Development :: Libraries
"""

version = {}
with open("underworld/_version.py") as fp:
    exec(fp.read(), version)

with open("README.md", "r") as fh:
    long_description = fh.read()


setup(name='underworld',
    version=version['__version__'],
    description="Underworld is a python-friendly version of the Underworld geodynamics \
code which provides a programmable and flexible front end to all the \
functionality of the code running in a parallel HPC environment.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    classifiers= classifiers.split('\n')[1:-1],
    keywords = ['Underworld', 'MPI', 'Geodynamics'],
    platforms=['POSIX'],
    license='LGPL-3',

    url='https://github.com/underworldcode/underworld2',
    download_url="",

    author='Underworld Team',
    author_email='help@underworldcode.org',
    maintainer='Underworld Team',
    maintainer_email='help@underworldcode.org',
    include_package_data=False,
    packages=find_packages(),
    ext_modules=[
        CMakeExtension(name='libUnderworld',
                       install_prefix="underworld",
                       source_dir=str(Path("underworld/libUnderworld").absolute()),
                       cmake_configure_options=[
                           f"-DPython3_ROOT_DIR={Path(sys.prefix)}",
                           "-DCALL_FROM_SETUP_PY:BOOL=ON"]
                       ),
    ],
    cmdclass=dict(build_ext=BuildExtension),
    **metadata)
