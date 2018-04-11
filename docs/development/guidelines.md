Directory Structure
==============

This list is not exhaustive.

   * libUnderworld              - Backend stack of tools (Underworld, StgFEM, etc).
      * libUnderworldPy       - Swig generated wrappers for backend as well as auxiliary implementations (such as StGermain_Tools).
   * underworld                  - Underworld python modules.
   * glucifer                        - gLucifer python modules.
   * docs                            - Various documentation including this file.
     * development             - Documents relevant to developers.
     * tests                          - Python type scripts which are mainly for regression testing underworld.
     * examples                  - Python type example scripts for users.
     * user_guide                - Jupyter notebook based user guide
     * publications              - Models in published work
   * utils                             - Utility files.
  


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

For further details about Jenkins testing, please refer "Guide to jenkins 
testing" (stored within the private Underworld google documents folder).


API DEPRECATIONS
================

Deprecations can be either hard or soft. Hard deprecations should raise exceptions, 
while soft deprecations should simply provide a warning. In either case a message should inform
the user of the change (and the new parameter if appropriate).  

Developers should mark deprecations with a comment such as:
'# DEPRECATION, remove v2.5.0'
indicating which version of the code should entirely remove the code. Generally,
this will be the version after the next release. So, if we are currently working
towards v2.4.0, when release, v2.4.0 will feature the deprecation warning
(as the next release will provide the users with a grace period).  

An example in the code for a warning might look like this

```python
# DEPRECATION, remove v2.5.0
if flux != None:
    import warnings
    warnings.warn("DEPRECATION: The 'flux' parameter in the NeumannCondition " +
    "class has been replaced with 'fn_flux'. In the coming release 'flux' will be deprecated "+
    "please update your python code.")
```




TODO for this document:
======================
* API changes  & deprections
