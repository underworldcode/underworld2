.. _pcu-write:

************************
Writing a PCU Test Suite
************************

This section outlines the process of writing a PCU suite.

.. Note::
   All the tests files need to follow the "\*Suite.c" naming convention.
   For example the test suite of the `IndexSet` component in StGermain is called
   `IndexSetSuite.c`. See the :ref:`pcu-intro-workflow` section for more
   on this.

Structure of a PCU Suite file \– Tests, Data Structures, & Setup and Teardown
=============================================================================

.. highlight:: c

A PCU test suite is broken down into one or more individual test cases, which
will be separate functions in your PCU suite C file. Generally, test cases of a component will need
to set up common data structures, and to make this easier PCU follows the PyUnit approach of
using Setup and Teardown functions so this work only needs to be done once.

The basic structure of how a PCU suite is defined and run is:

* PCU does any general setup required, such as calling Init() on each
  StGermain framework being used, and starting MPI;
* Then, for each test case defined in a PCU file:

  * The Suite's defined Setup() function is called, saving results on a
    struct to be passed to each test case;
  * One of the test cases is run, and results recorded;
  * The Suite's defined Teardown() function is called to
    clean up any allocated data.

* Summary results are printed;
* PCU does any finalisation required, such as closing off MPI.

It's important to note in the above that the `Setup()` and `Teardown()`
functions are called before and
after each test case, not before and after the whole set of test cases
(once again following PyUnit).
Thus, each test case should be independent of the other test cases and be
prepared to work on 'fresh' data structures allocated by the Setup() function.
To see how this maps onto the layout of an actual file, see the
:ref:`pcu-appendix-template-suite` section of the appendix where a template
header and C file are provided.

PCU_Check functions available
=============================

PCU provides a library of functions to perform checks on required conditions throughout your test
cases. These are documented below.

.. function:: pcu_check_true( expr )

   Check that the given Boolean expression evaluates as True.
   This is the most general-purpose check, and is useful if none
   of the more specific ones below are appropriate.

.. function:: pcu_check_gt( a, b )

   Check that numerical value 'a' is greater than 'b'.

.. function:: pcu_check_lt( a, b )

   Check that numerical value 'a' is less than 'b'.

.. function:: pcu_check_ge( a, b ) 

   Check that numerical value 'a' is greater than or equal to 'b'.

.. function:: pcu_check_le( a, b )
   
   Check that numerical value 'a' is less than or equal to 'b'.

.. function:: pcu_check_streq( strA, strB )
  
   Check that the strings strA and strB are entirely equal
   (match). The function will check that both strings passed in
   are non-null first. If they don't match, the actual strings will
   be printed out.

.. function:: pcu_check_fileEq( fnameA, fnameB )

   Check that the files described by fnameA and fnameB
   (filename strings) both exist, and match. This is thus similar
   to doing a diff on the files. More description below in the
   section describing input & expected files.

.. function:: pcu_check_noassert( stmnt )
   
   Checks that the code statement stmnt executes correctly
   without producing an assert statement.

.. function:: pcu_check_assert( stmnt )

   Executes the code statement stmnt, and checks that it
   produces a C assert() statement. The test will continue
   running either way. This is useful for checking that a function
   successfully catches bad input. Important: the code statement
   in question must use the pcu_assert() macro instead of the
   regular C assert() statement for this check to work
   successfully.

.. Note::

   (The check code resides in pcu/src/checks.[c|h] if you wish to examine them,
   or add a new check).

It's important to note that in parallel tests, a `pcu_check()` will be run
on each processor that executes that line of code. So if you only want a
particular processor to execute a `check()`, you'll need to use
an if() statement based on processor rank to control this. 
pcu_checks are not collective, and there's no requirement for all
processors that run a particular test case to execute every check within the
case. For example, you may choose to only use processor 0 to check required
output files were created correctly, to avoid potential problems involved
in parallel file I/O.

Accessing Input & Expected files using PCU_Filename
===================================================

While PCU encourages you to do algorithmic and arithmetic testing within the test case code itself,
there are times when it's more clearer, easier and more maintainable to use expected files. There are
also times when there is a lot of input required that it's necessary to use some sort of input file with
a test, in addition to the preparation done in the `Setup()` function.

To handle these situations, PCU provides the pcu_filename functions. It's important to note that
PCU tests run in the build tree created by SCons, not the source tree – so you need to use these
pcu_filename functions to access required files.

Firstly, input and expected required by a test should be put in the input and expected 
subdirectories of the tests module you are working on, respectively. All files in those directories will
be 'installed' (copied) into the build tree when you run SCons on the project.

Then within your actual test suite code, you can run the following functions to translate a direct
filename into the full path to access the file::

   void pcu_filename_input( const char* const inputFileName, char* fullPathFileName );
   void pcu_filename_expected( const char* const expectedFileName, char* fullPathFileName );

In the case of both functions, the first argument is the name of the file you wish to load, for example
“testJournal.xml”. The second argument must be a buffer large enough to receive the full relative
path of the file you require.

It is most convenient to make this a statically allocated array of size
`PCU_PATH_MAX`. However,
if you wish to dynamically allocate the buffer you can use the following
functions to calculate the exact size required.::

   unsigned pcu_filename_expectedLen( const char* expectedFileName );
   unsigned pcu_filename_inputLen( const char* inputFileName );

These will return the correct number of characters to use.
Once you've retreived the correct filename, you can operate on it in any
way you would for a normal filename. For example, if it's a StGermain input
XML file, you could use the `IO_Handler_ReadAllFromFile()`
function (see the `Doxygen reference
<https://csd.vpac.org/doxygen-v1.4.1/dd/d61/IO__Handler_8h_source.html>`_)
to read its contents into a Dictionary. In the case of expected
files, the :func:`pcu_check_fileEq` macro documented above is very useful
to check that a file created as part of the test case
matches an expected file you've saved and loaded using
:func:`pcu_filename_expected()`.
