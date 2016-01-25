Directory Structure
===================

* libUnderworld            - Backend stack of tools (Underworld, StgFEM, etc).
** libUnderworldPy         - Swig generated wrappers for backend as well as auxiliary implementations (such as StGermain_Tools).
* underworld               - Underworld python modules.
* glucifer                 - gLucifer python modules.
* docs                     - Various documentation including this file.
** development             - Documents relevant to developers.
*** broken                 - Input files & tests which are broken.
** tests                   - Python type scripts which are mainly for regression testing underworld.
** examples               - Python type example scripts for users. 
** user_guide              - IPython notebook based user guide
* utils                    - Utility files.

Documents
=========

Tests (docs/tests)
-----
These are more developer domain scripts, but advanced users might also find them useful.
For example, more thorough analytic tests might be housed here.

All files here are read-only and should pass tests at all times.

Examples (docs/examples)
---------
Examples generally demonstrate complete underworld model construction and usage.

Naming convention for example files:

a_b_textual_name.ipynb (.py)

where

a: Loose indication of difficulty level (1=novice, 3=advanced)
b: Unique & persistent integer identifier for example

Example: 2_04_Analytic Solutions.ipynb

All files here are read-only and should pass tests at all times.

User Guide (docs/user_guide)
----------
The user guide takes a more focussed look at various aspects of underworld
model construction. Ipython notebooks are utilised to allow users to directly
interact with and modify content (temporarily). Where possible, visualisation 
should be constructed using glucifer. 

Chapter outline:

01_GettingStarted.ipynb
02_Meshing.ipynb
03_FEVariables.ipynb
04_Swarms.ipynb
05_Functions.ipynb
06_Systems.ipynb
07_Utilities.ipynb
08_Visualisation.ipynb
09_Checkpointing.ipynb
10_Parallelism.ipynb

All files here are read-only and should pass tests at all times.


Coding Style
============

API Documentation
-----------------
Largely follow the numpy/scipy docstring conventions:
https://github.com/numpy/numpy/blob/master/doc/HOWTO_DOCUMENT.rst.txt#docstring-standard


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
autopep8 -v -i -r  --ignore E201,E202,E501,E221,E251 .


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

X.Y.Z

where

X = Major version number. This will be '2' for the foreseeable future. 
Y = Minor version number. This will increment major feature releases, or with scheduled 
    releases (such as quarterly releases).  Unlike SemVer, changes to interface 
    may occur with minor version increments. 
Z = Micro/Patch version. Backwards compatible bug fixes.

The version number may also be appended with pre-release type designations, for
example 2.0.0b.

Development software will be have the 'dev' suffix, so 2.0.0-dev

Testing
=======

The 'testNotebook.py' script (in the utils directory) can be used to execute ipython 
notebooks and check that they run to completion without issue. 

Jenkins continuous testing system - see http://jenkins-ci.org/
(followed setup instructions from https://wiki.jenkins-ci.org/display/JENKINS/Installing+Jenkins+on+Ubuntu )

Backend setup:
On a virtual machine we have at - https://130.56.248.95:8080
JENKINS_HOME=/mnt/scratch/testing_platform/jenkins

Script to run server is = /etc/init.d/jenkins
configuration file = /etc/default/jenkins

To start/stop/restart the server use
sudo /etc/init.d/jenkins {start|stop|restart}

Front end setup:
goto https://130.56.248.95:8080 and log in with 
username = banana
password = https://www.youtube.com/watch?v=InsspuvAmBs

Using the front-end one can configure the build system, security feature, email notifications etc.
(Note: I’ve never been able to get the email notification happening because the virtual machines have some problem communicating out of them - i suspect port blocking by nectar guys)


Useful Link
===========
http://matplotlib.org/devel/gitwash/git_development.html
https://github.com/numpy/numpy/blob/master/doc/HOWTO_DOCUMENT.rst.txt
http://docs.scipy.org/doc/numpy/dev/index.html


TODO for this document:
======================
* Releases
* New Features /  API changes  / deprections
* Distribution?
