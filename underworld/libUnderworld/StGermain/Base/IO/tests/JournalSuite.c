/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mpi.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "JournalSuite.h"


void JournalSuite_Setup( JournalSuiteData* data ) {
   data->comm = MPI_COMM_WORLD;  
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );

   /* This is where we'll save the Journal, to be restored after the test, and create one for testing */
   data->savedJournal = stJournal;
   stJournal = Journal_New();
   Journal_SetupDefaultTypedStreams();

   /* For testing, we want our custom Journal to output to saved files */
   Stg_asprintf( &data->testStdOutFilename, "./testStdOut-%d.txt", data->rank );
   Stg_asprintf( &data->testStdErrFilename, "./testStdErr-%d.txt", data->rank );

   stJournal->stdOut = CFile_New();
   stJournal->stdErr = CFile_New();
   File_Write( stJournal->stdOut, data->testStdOutFilename );
   File_Write( stJournal->stdErr, data->testStdErrFilename );

   Stream_SetFile( Journal_GetTypedStream(Info_Type), stJournal->stdOut );
   Stream_SetPrintingRank( Journal_GetTypedStream(Info_Type), STREAM_ALL_RANKS );
   Stream_SetFile( Journal_GetTypedStream(Debug_Type), stJournal->stdOut );
   Stream_SetPrintingRank( Journal_GetTypedStream(Debug_Type), STREAM_ALL_RANKS );
   Stream_SetFile( Journal_GetTypedStream(Dump_Type), stJournal->stdOut );
   Stream_SetPrintingRank( Journal_GetTypedStream(Dump_Type), STREAM_ALL_RANKS );
   Stream_SetFile( Journal_GetTypedStream(Error_Type), stJournal->stdErr );
   Stream_SetPrintingRank( Journal_GetTypedStream(Error_Type), STREAM_ALL_RANKS );
   /* We don't want the rank formatting stuff interefering with tests unnecessarily */
   Stream_ClearCustomFormatters( Journal_GetTypedStream(Error_Type) );

   data->testStdOutFile = fopen( data->testStdOutFilename, "r" );
   data->testStdErrFile = fopen( data->testStdErrFilename, "r" );
}

void JournalSuite_Teardown( JournalSuiteData* data ) {
   /* Delete temporary Journal, then restore the regular one */
   Journal_Delete();
   stJournal = data->savedJournal;

   fclose( data->testStdOutFile );
   fclose( data->testStdErrFile );
   remove( data->testStdOutFilename );
   remove( data->testStdErrFilename );
}

void JournalSuite_TestRegister( JournalSuiteData* data ) {
   Journal* testJournal;
   Stream* myInfo;
   Stream* myDebug;
   Stream* myDump;
   Stream* myError;
   Stream* allNew;   /* Will use for testing a non-standard type stream */
   
   /* We want to test properties of the "real" journal in this test. Thus save & restore our testing one */   
   testJournal = stJournal;
   stJournal = data->savedJournal;

   myInfo = Journal_Register( Info_Type, (Name)"MyInfo"  );
   myDebug = Journal_Register( Debug_Type, (Name)"MyDebug"  );
   myDump = Journal_Register( Dump_Type, (Name)"MyDump"  );
   myError = Journal_Register( Error_Type, (Name)"MyError"  );
   allNew = Journal_Register( "New_Type", (Name)"allNew"  );

   /* Check the streams themselves were created properly */
   /* Including Louis' requirement that they default to have printingRank 0 */
   pcu_check_streq( myInfo->name, "MyInfo" );
   pcu_check_true( myInfo->_parent == Journal_GetTypedStream( Info_Type ) );
   pcu_check_true( myInfo->_children->count == 0 );
   pcu_check_true( 0 == Stream_GetPrintingRank( myInfo ));
   pcu_check_streq( myDebug->name, "MyDebug" );
   pcu_check_true( myDebug->_parent == Journal_GetTypedStream( Debug_Type ) );
   pcu_check_true( myDebug->_children->count == 0 );
   pcu_check_true( 0 == Stream_GetPrintingRank( myDebug ));
   pcu_check_streq( myDump->name, "MyDump" );
   pcu_check_true( myDump->_parent == Journal_GetTypedStream( Dump_Type ) );
   pcu_check_true( myDump->_children->count == 0 );
   pcu_check_streq( myError->name, "MyError" );
   pcu_check_true( myError->_parent == Journal_GetTypedStream( Error_Type ) );
   pcu_check_true( myError->_children->count == 0 );
   pcu_check_true( STREAM_ALL_RANKS == Stream_GetPrintingRank( myError ));
   pcu_check_true( myError->_formatterCount == 1 );
   pcu_check_true( myError->_formatter[0]->type == RankFormatter_Type );
   pcu_check_streq( allNew->name, "allNew" );
   pcu_check_true( allNew->_parent == Journal_GetTypedStream( "New_Type" ) );
   pcu_check_true( allNew->_children->count == 0 );
   pcu_check_true( STREAM_ALL_RANKS == Stream_GetPrintingRank( allNew ));

   /* Now check they were inserted in Journal hierarchy correctly */
   pcu_check_true( Stg_ObjectList_Get( Journal_GetTypedStream(Info_Type)->_children, (Name)"MyInfo" ) == myInfo );
   pcu_check_true( Stg_ObjectList_Get( Journal_GetTypedStream(Debug_Type )->_children, "MyDebug" ) == myDebug );
   pcu_check_true( Stg_ObjectList_Get( Journal_GetTypedStream(Dump_Type)->_children, (Name)"MyDump" ) == myDump );
   pcu_check_true( Stg_ObjectList_Get( Journal_GetTypedStream(Error_Type )->_children, "MyError" ) == myError );
   pcu_check_true( Stg_ObjectList_Get( Journal_GetTypedStream("New_Type")->_children, (Name)"allNew" ) == allNew  );

   /* Ok, restore the testing journal */
   stJournal = testJournal;
}


void JournalSuite_TestRegister2( JournalSuiteData* data ) {
   Stream* register2Stream;
   Stream* register2Test;
   
   register2Stream = Journal_Register2( Info_Type, "Component", "Instance" );
   register2Test   = Journal_Register( Info_Type, (Name)"Component.Instance"    );

   pcu_check_true( register2Stream == register2Test );
}


void JournalSuite_TestPrintBasics( JournalSuiteData* data ) {
   Stream*     myInfo;
   Stream*     myDebug;
   Stream*     myDump;
   Stream*     myError;
   #define     MAXLINE 1000
   char        outLine[MAXLINE];

   /* Check as is expected - see Base/IO/src/Init.c . Important for later tests */
   pcu_check_true( Stream_IsEnable( Journal_GetTypedStream(Info_Type)) == True );
   pcu_check_true( Stream_IsEnable( Journal_GetTypedStream(Debug_Type)) == False );
   pcu_check_true( Stream_IsEnable( Journal_GetTypedStream(Dump_Type)) == False );
   pcu_check_true( Stream_IsEnable( Journal_GetTypedStream(Error_Type)) == True  ) ;

   myInfo = Journal_Register( InfoStream_Type, (Name)"MyInfo" );
   myDebug = Journal_Register( DebugStream_Type, (Name)"MyDebug"  );
   myDump = Journal_Register( Dump_Type, (Name)"MyDump"  );
   myError = Journal_Register( ErrorStream_Type, (Name)"MyError"  );

   Journal_Printf( myInfo, "%s\n", "HELLOInfo" );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "HELLOInfo\n" );
   Journal_Printf( myDebug, "%s\n", "WORLDDebug" );
   pcu_check_true( NULL == fgets( outLine, MAXLINE, data->testStdErrFile ));
   Journal_Printf( myDump, "%s\n", "HELLODump" );
   pcu_check_true( NULL == fgets( outLine, MAXLINE, data->testStdOutFile ));
   Journal_Printf( myError, "%s\n", "WORLDError" );
   rewind( data->testStdErrFile );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdErrFile ));
   pcu_check_streq( outLine, "WORLDError\n" );

   Journal_Enable_NamedStream( Info_Type, "MyInfo", False );
   Journal_Printf( myInfo, "%s\n", "HELLOInfo2" );
   pcu_check_true( NULL == fgets( outLine, MAXLINE, data->testStdOutFile ));

   Journal_Enable_TypedStream( Dump_Type, True );
   Journal_Enable_NamedStream( Dump_Type, "MyDump", True );
   Journal_Printf( myDump, "%s\n", "HELLODump2" );
   /* This stream should have auto-flush set to false. Check first, then flush and check again */
   pcu_check_true( Journal_GetTypedStream(Dump_Type)->_autoFlush == False );
   pcu_check_true( NULL == fgets( outLine, MAXLINE, data->testStdOutFile ));
   rewind( data->testStdOutFile );
   Stream_Flush( Journal_GetTypedStream(Dump_Type) );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "HELLOInfo\n" );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "HELLODump2\n" );

   
   pcu_check_streq( outLine, "HELLODump2\n" );
   stJournal->enable = False;
   Journal_Printf( myDump, "%s\n", "HELLODump3" );
   pcu_check_true( NULL == fgets( outLine, MAXLINE, data->testStdOutFile ));
}


void JournalSuite_TestPrintfL( JournalSuiteData* data ) {
   Stream* myStream;
   #define     MAXLINE 1000
   char        outLine[MAXLINE];

   myStream = Journal_Register( InfoStream_Type, (Name)"myComponent" );
   Journal_PrintfL( myStream, 1, "Hello\n" );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "Hello\n" );
   /* We should get a blank line, since level 2 printing not enabled by default */
   Journal_PrintfL( myStream, 2, "Hello\n" );
   pcu_check_true( NULL ==fgets( outLine, MAXLINE, data->testStdOutFile ));
   /* Now enable level 2, and try again */
   Stream_SetLevel( myStream, 2 );
   Journal_PrintfL( myStream, 2, "Hello\n" );
   rewind( data->testStdOutFile );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "Hello\n" );
}


void JournalSuite_TestDPrintf( JournalSuiteData* data ) {
   Stream*     myInfo;
   #define     MAXLINE 1000
   char        outLine[MAXLINE];

   myInfo = Journal_Register( InfoStream_Type, (Name)"MyInfo" );
   Journal_DPrintf( myInfo, "DPrintf\n" );
   #ifdef DEBUG
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "DPrintf\n" );
   #else
   pcu_check_true( NULL == fgets( outLine, MAXLINE, data->testStdOutFile ));
   #endif
}


void JournalSuite_TestPrintChildStreams( JournalSuiteData* data ) {
   Stream*     myStream;
   Stream*     childStream1;
   Stream*     childStream2;
   #define     MAXLINE 1000
   char        outLine[MAXLINE];

  /* Make sure the hierarchy works*/
   myStream = Journal_Register( InfoStream_Type, (Name)"myComponent" );
   childStream1 = Stream_RegisterChild( myStream, "child1" );
   childStream2 = Stream_RegisterChild( childStream1, "child2" );

   Journal_Printf( myStream, "0 no indent\n" );
   Stream_IndentBranch( myStream );
   Journal_Printf( childStream1, "1 with 1 indent\n" );
   Stream_IndentBranch( myStream );
   Journal_Printf( childStream2, "2 with 2 indent\n" );
   Stream_UnIndentBranch( myStream );
   Journal_Printf( childStream2, "2 with 1 indent\n" );
   Stream_UnIndentBranch( myStream );
   Journal_Printf( childStream1, "1 with no indent\n" );
   Journal_Printf( childStream2, "2 with no indent\n" );

   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "0 no indent\n" );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "\t1 with 1 indent\n" );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "\t\t2 with 2 indent\n" );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "\t2 with 1 indent\n" );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "1 with no indent\n" );
   pcu_check_true(         fgets( outLine, MAXLINE, data->testStdOutFile ));
   pcu_check_streq( outLine, "2 with no indent\n" );
}


void JournalSuite_TestReadFromDictionary( JournalSuiteData* data ) {
   Dictionary* testDict = Dictionary_New();
   Stream*     infoTest1;
   Stream*     infoTest2;
   Stream*     debugTest1;
   Stream*     debugTest2;
   Stream*     dumpTest1;
   Stream*     dumpTest2;
   Stream*     newTest1;
   Stream*     newTest2;
   Stream*     fileTest1;
   Stream*     fileTest2;
   Stream*     propTest1;
   Stream*     propTest2;
   const char* testNewTypeFilename1 = "./testJournal-out1.txt";
   const char* testNewTypeFilename2 = "./testJournal-out2.txt";

   infoTest1 = Journal_Register( Info_Type, (Name)"test1"  );
   infoTest2 = Journal_Register( Info_Type, (Name)"test2"  );
   debugTest1 = Journal_Register( Debug_Type, (Name)"test1"  );
   debugTest2 = Journal_Register( Debug_Type, (Name)"test2"  );
   dumpTest1 = Journal_Register( Dump_Type, (Name)"test1"  );
   dumpTest2 = Journal_Register( Dump_Type, (Name)"test2"  );

   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal.debug.test1", Dictionary_Entry_Value_FromBool(  True ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal.dump.test1", Dictionary_Entry_Value_FromBool(  True ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal.info.test2", Dictionary_Entry_Value_FromBool( False ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal.info.test2", Dictionary_Entry_Value_FromBool( False ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal.info.test1.new1", Dictionary_Entry_Value_FromBool(  True ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal.info.test1.new2", Dictionary_Entry_Value_FromBool(  False ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal-level.info.test1.new1", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal.newtype", Dictionary_Entry_Value_FromBool(  True ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal-file.newtype", Dictionary_Entry_Value_FromString( testNewTypeFilename1 ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal-file.newtype.other", Dictionary_Entry_Value_FromString( testNewTypeFilename2 ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal-rank.info.propertiestest1", Dictionary_Entry_Value_FromUnsignedInt( 0 ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal-rank.info.propertiestest2", Dictionary_Entry_Value_FromUnsignedInt( 5 ) );
   Dictionary_Add( testDict, (Dictionary_Entry_Key)"journal-autoflush.info.propertiestest1", Dictionary_Entry_Value_FromBool( False ));

   Journal_ReadFromDictionary( testDict );

   pcu_check_true( True == debugTest1->_enable );
   pcu_check_true( True == dumpTest1->_enable );
   pcu_check_true( False == infoTest2->_enable  );

   newTest1 = Journal_Register( Info_Type, (Name)"test1.new1"  );
   newTest2 = Journal_Register( Info_Type, (Name)"test1.new2" );
   pcu_check_true( True == newTest1->_enable );
   pcu_check_true( False == newTest2->_enable );
   pcu_check_true( 3 == newTest1->_level );
   
   /* Just do the rest of this test with 1 proc to avoid parallel I/O problems */
   if ( data->rank==0  ) {
      FILE*       testNewTypeFile1;
      FILE*       testNewTypeFile2;
      #define     MAXLINE 1000
      char        outLine[MAXLINE];

      /* We do actually need to do some printing to the newtype streams, as the filename isn't stored
       *  on the CFile or File struct*/
      fileTest1 = Journal_Register( "newtype", (Name)"hello"  );
      fileTest2 = Journal_Register( "newtype", (Name)"other"  );
      Journal_Printf( fileTest1, "yay!\n" );
      Journal_Printf( fileTest2, "double yay!\n" );
      Stream_Flush( fileTest1 );
      Stream_Flush( fileTest2 );
      testNewTypeFile1 = fopen( testNewTypeFilename1, "r" );
      testNewTypeFile2 = fopen( testNewTypeFilename2, "r" );
      pcu_check_true(         fgets( outLine, MAXLINE, testNewTypeFile1 ));
      pcu_check_streq( outLine, "yay!\n" );
      pcu_check_true(         fgets( outLine, MAXLINE, testNewTypeFile2 ));
      pcu_check_streq( outLine, "double yay!\n" );

      propTest1 = Journal_Register( Info_Type, (Name)"propertiestest1"  );
      propTest2 = Journal_Register( Info_Type, (Name)"propertiestest2" );
      pcu_check_true( 0 == Stream_GetPrintingRank( propTest1 ));
      pcu_check_true( 5 == Stream_GetPrintingRank( propTest2 ));
      pcu_check_true( False == Stream_GetAutoFlush( propTest1 ));

      fclose( testNewTypeFile1 );
      fclose( testNewTypeFile2 );
      remove( testNewTypeFilename1 );
      remove( testNewTypeFilename2 );
   }
   Stg_Class_Delete( testDict );
}


void JournalSuite_TestPrintString_WithLength( JournalSuiteData* data  ) {
   Stream*      myStream    = Journal_Register( Info_Type, (Name)"TestStream" );
   char*        string        = "helloWorldHowDoYouDo";
   int          char_I;
   const char*  stringLengthTestFilename = "testJournalPrintStringWithLength.txt" ;
   char         expectedFilename[PCU_PATH_MAX];

   /* Just do this test with rank 0 */
   if (data->rank != 0 ) return;

   Stream_RedirectFile( myStream, stringLengthTestFilename );

   for ( char_I = -1 ; char_I < 25 ; char_I++ ) {
      Journal_PrintString_WithLength( myStream, string, char_I );
      Journal_Printf( myStream, "\n" );
   }

   pcu_filename_expected( stringLengthTestFilename, expectedFilename );
   pcu_check_fileEq( stringLengthTestFilename, expectedFilename ); 

   remove( stringLengthTestFilename );
}


void JournalSuite_TestShortcuts( JournalSuiteData* data ) {
   Stream*      myStream    = Journal_Register( Info_Type, (Name)"TestStream"  );
   char*        string        = "helloWorldHowDoYouDo";
   double       doubleValue   = 3142e20;
   double       floatValue    = 2.173425;
   int          intValue      = 3;
   unsigned int uintValue     = 3980;
   char         charValue     = 'V';
   double       doubleArray[] = { 10.23, 393.1, -89, 1231 };        
   Index        uintArray[]   = { 10, 2021, 231, 2, 3, 4, 55 };
   const char*  shortcutTestFilename = "./testJournalPrintShortcuts.txt" ;
   char         expectedFilename[PCU_PATH_MAX];

   /* Testing String Printing Shortcuts */
   /* Just do this test with rank 0 */
   if (data->rank != 0 ) return;

   Stream_RedirectFile( myStream, shortcutTestFilename );

   Journal_PrintString( myStream, string );
   Journal_PrintValue( myStream, doubleValue );
   Journal_PrintValue( myStream, floatValue );
   Journal_PrintValue( myStream, intValue );
   Journal_PrintValue( myStream, uintValue );
   Journal_PrintChar(  myStream, charValue );
   Journal_PrintArray( myStream, doubleArray, 4 );
   Journal_PrintArray( myStream, uintArray, 7 );

   pcu_filename_expected( shortcutTestFilename, expectedFilename );
   pcu_check_fileEq( shortcutTestFilename, expectedFilename ); 

   remove( shortcutTestFilename );
}


void JournalSuite_TestFirewall( JournalSuiteData* data ) {
   Stream*      myInfo = NULL;

   myInfo = Journal_Register( Info_Type, (Name)"MyInfo" );

   stJournal->firewallProducesAssert = True;

   /* We expect nothing to happen on this first run - in effect the test would "fail" if an uncaught assert( )
    *  terminated the program */   
   Journal_Firewall( 1, myInfo, "Firewall" );
   /* We can use pcu_check_assert to make sure a pcu_assert is generated. This is actually quite important
    *  as many other tests rely on this functionality. */   
	stJournal->firewallProtected = False;
   pcu_check_assert( Journal_Firewall( 1 == 0, myInfo, "Firewall" ) );
	stJournal->firewallProtected = True;
}


void JournalSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, JournalSuiteData );
   pcu_suite_setFixtures( suite, JournalSuite_Setup, JournalSuite_Teardown );
   pcu_suite_addTest( suite, JournalSuite_TestRegister );
   pcu_suite_addTest( suite, JournalSuite_TestRegister2 );
   pcu_suite_addTest( suite, JournalSuite_TestPrintBasics );
   pcu_suite_addTest( suite, JournalSuite_TestPrintfL );
   pcu_suite_addTest( suite, JournalSuite_TestDPrintf );
   pcu_suite_addTest( suite, JournalSuite_TestPrintChildStreams );
   pcu_suite_addTest( suite, JournalSuite_TestReadFromDictionary );
   pcu_suite_addTest( suite, JournalSuite_TestPrintString_WithLength );
   pcu_suite_addTest( suite, JournalSuite_TestShortcuts );
   pcu_suite_addTest( suite, JournalSuite_TestFirewall );
}


