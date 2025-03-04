.. _pcu-run:

***********
Running PCU
***********

This section outlines the process of running PCU tests.

Running the tests using SCons
=============================

To run the entire set of unit tests for your set of StGermain projects,
run “scons check-unit” in the root directory of the your codebase [#f1]_.
E.g. in the case of stgUnderworld, you'd run the command from the
stgUnderworld directory, and not the Underworld sub-directory.

You should see output something like the following::

   scons: done reading SConscript files.
   scons: Building targets ...
   /home/jrevote/Documents/Projects/stgUnderworld/stgUnderworld_Syke/build/tests/testStGermain
   StGermain Framework revision 1b4df20976bf. Copyright (C) 2003-2005 VPAC.
   -----------------------------------------------------------
   [PCU] Project StGermain, running 54 test suites:
   -----------------------------------------------------------
   ------------------------------------------------------------------------
   [PCU] Testing 'CommonRoutinesSuite':
   [PCU]       Test Case: 'CommonRoutinesSuite_TestLMS', Passes: (1/5)
   [PCU]       Test Case: 'CommonRoutinesSuite_TestStringIsNumeric', Passes: (2/5)
   [PCU]       Test Case: 'CommonRoutinesSuite_TestStringIsEmpty', Passes: (3/5)
   [PCU]       Test Case: 'CommonRoutinesSuite_TestStG_RoundDoubleToNDecimalPlaces', Passes: (4/5)
   [PCU]       Test Case: 'CommonRoutinesSuite_TestStG_RoundDoubleToNSigFigs', Passes: (5/5)
   [PCU] Status: PASSED
   ------------------------------------------------------------------------
   [PCU] Testing 'MemMonitorSuite':
   [PCU] Status: PASSED
   ------------------------------------------------------------------------
   [PCU] Testing 'MemoryReportSuite':
   [PCU] Status: PASSED
   ------------------------------------------------------------------------
   [PCU] Testing 'MemorySuite':
   [PCU]       Test Case: 'MemorySuite_Test2DArray', Passes: (1/8)
   [PCU]       Test Case: 'MemorySuite_Test3DArray', Passes: (2/8)
   [PCU]       Test Case: 'MemorySuite_Test4DArray', Passes: (3/8)
   [PCU]       Test Case: 'MemorySuite_Test2DArrayAs1D', Passes: (4/8)
   [PCU]       Test Case: 'MemorySuite_Test3DArrayAs1D', Passes: (5/8)
   [PCU]       Test Case: 'MemorySuite_Test4DArrayAs1D', Passes: (6/8)
   [PCU]       Test Case: 'MemorySuite_Test2DComplexArray', Passes: (7/8)
   [PCU]       Test Case: 'MemorySuite_Test3DComplexArray', Passes: (8/8)
   [PCU] Status: PASSED

... and so on, concluding with a total number of tests and failures::

   -----------------------------------------------------------
   [PCU] Total Passes: (205/205)
   -----------------------------------------------------------

Running the tests for each component (i.e. StGermain)
=====================================================

Tests can also be ran per component by executing their corresponding test* 
executables that exists in the build/tests directory of your root checkout.

For example to run all the test cases under StGermain::

   cd build/tests
   ./testStGermain

This will run all test suites for StGermain.

If you just wanted to run a specific test suite, you can pass the suite name
as an argument to the test* executable, for example::

   cd build/tests
   ./testStGermain JournalSuite

This will run the "JournalSuite" test suite and produce the following output to
the terminal::

   StGermain Framework revision 1b4df20976bf. Copyright (C) 2003-2005 VPAC.
   -----------------------------------------------------------
   [PCU] Project StGermain, running 1 test suites:
   -----------------------------------------------------------
   ------------------------------------------------------------------------
   [PCU] Testing 'JournalSuite':
   [PCU]       Test Case: 'JournalSuite_TestRegister', Passes: (1/10)
   [PCU]       Test Case: 'JournalSuite_TestRegister2', Passes: (2/10)
   [PCU]       Test Case: 'JournalSuite_TestPrintBasics', Passes: (3/10)
   [PCU]       Test Case: 'JournalSuite_TestPrintfL', Passes: (4/10)
   [PCU]       Test Case: 'JournalSuite_TestDPrintf', Passes: (5/10)
   [PCU]       Test Case: 'JournalSuite_TestPrintChildStreams', Passes: (6/10)
   [PCU]       Test Case: 'JournalSuite_TestReadFromDictionary', Passes: (7/10)
   [PCU]       Test Case: 'JournalSuite_TestPrintString_WithLength', Passes: (8/10)
   [PCU]       Test Case: 'JournalSuite_TestShortcuts', Passes: (9/10)
   [PCU]       Test Case: 'JournalSuite_TestFirewall', Passes: (10/10)
   [PCU] Status: PASSED
   -----------------------------------------------------------
   [PCU] Total Passes: (10/10)
   -----------------------------------------------------------

.. rubric:: Footnotes

.. [#f1] The PCU unit tests are also set up to run as part of the default
   `scons check` command, which will also run certain system tests.
