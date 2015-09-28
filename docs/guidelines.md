

Design
======
User Interface?
Naming convections - for classes, instances, input parameters?
Class overview?


Directory Structure
===================

* libUnderworld            - contains backend stack of tools (Underworld, StgFEM, etc)
 * libUnderworldPy         - contains swig generated wrappers for backend as well as auxiliary implementations (such as StGermain_Tools)
* underworld               - python only directory structure containing python frontend routines and supporting scripts
* docs                     - contains various documentation including this file
* InputFiles               - contains various examples input files



Coding Style
============

Documentation
-------------
Largely follow the numpy/scipy documentation conventions. 
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

An exception to the above naming convention are classes within the Function module.  These
classes are generally (from the user interface perspective) utilised as functions, and 
therefore they use the 'function_name' style naming.  This is also for consistency with
other python modules (such as 'math' and 'numpy'). 


* Comments?
* Error handling?
 * Reporting?
 * Checking / exceptions?


License 
=======



Development
===========
* Versions?
* Development Workflow?
* Bug fixes & reporting?
* New Features / Redesigned interface or API?
* Release candidates?
* Distribution?

Testing
=======

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
