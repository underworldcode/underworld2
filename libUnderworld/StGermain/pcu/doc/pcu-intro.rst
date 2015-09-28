.. _pcu-intro:

*****************************************
Introduction to PCU (Parallel C-Unit)
*****************************************

.. An introductory document about PCU.

**PCU** (Parallel C-Unit) is a C-Based unit testing framework.
In design, it is similar to the PyUnit unit testing framework for Python,
so that project's documentation may provide a useful introduction – see
http://pyunit.sourceforge.net/pyunit.html. It supports parallel testing,
and is able to trace and report and test failures to particular processors
in a multi-processor test.

Organisation of unit tests – Suites and Test Cases
==================================================

As with PyUnit, the core concept in PyUnit is that of a test `Suite`.
A Suite is a group of related Test Cases of a particular unit of your
software project.
Usually, the Suite will apply to a StGermain Component or Class.
For example, the `IndexSet` Component in `StGermain/Base/IO` has an
`IndexSetSuite` applied to it to test its functionality.

All Suites are comprised of one or more Test Cases, a `Setup` function
and a `Teardown` function.
Each Test Case should check one aspect of the functionality of the
unit of software it is testing.
In the common case of testing a StGermain Component, this will often be a
test that a particular method of the component works as documented.

Returning to the above example, one of the
IndexSetSuite's test cases is `IndexSetSuite_TestInsertion()`, which checks
that the `IndexSet_Add()` method works as advertised. It's important to
note that PCU has no strict rule that each test case
should test one single Component method though – as the test writer,
you can decide the Tests necessary for a component as you see fit.

.. Note::

   A Test Suite may also apply to a set of related functions that are not
   actually a Component,
   but this is likely to be unusual. There are several tests of this sort
   in StGermain/Base/Foundation.
   For more information about Classes and Components in StGermain,
   see `StGermain's documentation 
   <http://www.stgermainproject.org/documentation.html>`_.

.. _pcu-intro-workflow:

Layout of PCU tests in the source tree and build system
=======================================================

PCU has been integrated into the normal StGermain approach to organising
a project, and the `SCons <http://www.scons.org/>`_ build system that
manages this.

For example, in the StGermain `Base/Container` module, the source code for
the module is in the subdirectory `Base/Containter/src`,
and the tests will be in `Base/Container/tests`.

All files in the tests directory with the file name pattern
`*Suite.c` (e.g. `IndexSetSuite.c`) will be
considered as test suite files, and automatically compiled into the test
executables. It is no longer necessary to individually define
lists of test files in a config file, as it was using the VMake build
system.
