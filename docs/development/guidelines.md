Directory Structure
===================

   * libUnderworld            - Backend stack of tools (Underworld, StgFEM, etc).
      * libUnderworldPy       - Swig generated wrappers for backend as well as auxiliary implementations (such as StGermain_Tools).
   * underworld               - Underworld python modules.
   * glucifer                 - gLucifer python modules.
   * docs                     - Various documentation including this file.
     * development            - Documents relevant to developers.
       * broken               - Input files & tests which are broken or require review/testing
     * tests                  - Python type scripts which are mainly for regression testing underworld.
     * examples               - Python type example scripts for users.
     * user_guide             - Jupyter notebook based user guide
     * publications           - Models in published work
   * utils                    - Utility files.

Documents (docs)
=========

Various user & developer domain documents and models. All models should run 
successfully against corresponding version of Underworld. 

Tests (docs/tests)
-----
These are more developer domain scripts, but advanced users might also find them useful.
For example, more thorough analytic tests might be housed here.

Examples (docs/examples)
---------
Examples generally demonstrate complete underworld model construction and usage.

Naming convention for example files:

```
a_b_textual_name.ipynb (.py)
```

where

a: Loose indication of difficulty level (1=novice, 3=advanced)
b: Unique & persistent integer identifier for example

Example: `2_04_Analytic Solutions.ipynb`


User Guide (docs/user_guide)
----------
The user guide takes a more focussed look at various aspects of underworld
model construction. Jupyter notebooks are utilised to allow users to directly
interact with and modify content (temporarily). Where possible, visualisation
should be constructed using glucifer.


Publications (docs/publications)
------------ 
Models explicitly concerned with reproduced published results are housed here. 
  


Coding Style
============

API Documentation
-----------------
Largely follow the numpy/scipy docstring conventions:
`https://github.com/numpy/numpy/blob/master/doc/HOWTO_DOCUMENT.rst.txt#docstring-standard`

Note that class constructors should be document at the class level docstring,
not the `__init__` method docstring.


Whitespace
----------
PEP 8
http://legacy.python.org/dev/peps/pep-0008/

also check YAPF for python, and clang-format for c

Most importantly:
Spaces should be used for tabs.
Indent levels should be 4 spaces deep.

autopep8 tool can be used to tidy when necessary:
https://pypi.python.org/pypi/autopep8/

This command seems to give good results (not overly aggressive):
`autopep8 -v -i -r  --ignore E201,E202,E501,E221,E251 .`


Naming Conventions
-----------------
module_name, package_name, ClassName, function_name, method_name, ExceptionName, propertyName
GLOBAL_CONSTANT_NAME, globalVarName, instanceVarName, functionParameterName, localVarName

An exception to the above naming convention are classes within the Function module. These
classes are generally (from the user interface perspective) utilised as functions, and
therefore they use the 'function_name' style naming. This is also for consistency with
other python modules (such as 'math' and 'numpy').


License
=======
See LICENSE.md file in project top directory


Version Control
===============

We follow the Gitflow Workflow:
https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow/

Features branches should be created for anything besides quick fixes. Once ready for
publishing to the development branch, a pull request should be created and another
team member nominated to review the changes. Pull request should only be merged
once the following criterion are met:
   1. Any new functionality is sufficiently tested (via doctest or other means).
   2. Any new functionality is sufficiently documented.
   3. All tests pass.


Bug reporting
=============

Use the github issue tracker! Changes relating to issue tracker issues should register
which issue they relate to in the commit log.

Version Numbering
=================
Underworld follows PEP440 for versioning:

`X.Y.Z`

where

X = Major version number. This will be '2' for the foreseeable future.
Y = Minor version number. This will increment major feature releases, or with scheduled
    releases (such as quarterly releases).  Unlike SemVer, changes to interface
    may occur with minor version increments.
Z = Micro/Patch version. Backwards compatible bug fixes.

The version number may also be appended with pre-release type designations, for
example 2.0.0b.

Development software will be have the 'dev' suffix, so 2.0.0-dev, and should 
represent the version the development is working towards, so 2.1.0-dev is working
towards release 2.1.0. 

Testing
=======

The 'run_tests.py' script (in the utils directory) can be used to execute the 
underworld notebooks and python scripts. The 'run_tests.py' script simply checks 
that input notebooks/scripts run to completion without raising an exception. 

Continuous integration is performed via a Jenkins continuous integration system.

Jenkins server address:
http://128.250.120.238:9080

For further details about Jenkins testing, please refer "Guide to jenkins 
testing" (stored within the private Underworld google documents folder).


TODO for this document:
======================
* API changes  & deprections
