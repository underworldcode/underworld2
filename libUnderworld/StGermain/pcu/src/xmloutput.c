/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
/* Needed for Mac Os X definition of PATH_MAX */
#include <limits.h>
#ifndef PATH_MAX
#include <sys/syslimits.h>
#endif
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlversion.h>

#include "types.h"
#include "listener.h"
#include "test.h"
#include "suite.h"
#include "xmloutput.h"
#include "test.h"
#include "source.h"
#include "suite.h"
#include "checks.h"
#include <setjmp.h>

extern int PCU_PRINT_DOCS;
extern jmp_buf pcu_rollback_env;

static const char*    DEFAULT_RESULTS_DIR = "test-results";
static const char*    RESULTS_FILE_PREFIX = "TEST-"; /* The same as the JUnit/Ant prefix */
static const xmlChar* XML_VERSION = (xmlChar*) "1.0";
static const char*    XML_NAMESPACE = "http://www.vpac.org/StGermain/PCU/Mar2010";

typedef struct {
   int               rank;
   char*             resultsDir;
   xmlDocPtr         currDoc; 
   xmlNsPtr          currNameSpace;
   xmlNodePtr        rootNode;   
} xmloutputdata_t;

void _pcu_xmloutput_printsuitestatus( pcu_listener_t* lsnr, pcu_suite_t* suite, int final );
void _pcu_xmloutput_printteststatus( pcu_listener_t* lsnr, pcu_suite_t* suite, char* testname, int final );
void _pcu_xmloutput_printsourcefailures( pcu_listener_t* lsnr, pcu_test_t* test, xmlNodePtr testXMLNode );
void _pcu_xmloutput_createXMLDocument( pcu_listener_t* lsnr, const char* rootNodeName );
void _pcu_xmloutput_prepareoutputdir( pcu_listener_t* lsnr );
void _pcu_xmloutput_checkcreatedir( const char* dirName );
int _pcu_xmloutput_checkdirexistswritable( const char* dirName );
void _pcu_xmloutput_cleanresults( const char* resultsDirName );

void pcu_xmloutput_suitebegin( pcu_listener_t* lsnr, pcu_suite_t* suite ) {
   _pcu_xmloutput_printsuitestatus( lsnr, suite, 0 );
   /*printstatus( lsnr, suite, 0 );*/
}

void pcu_xmloutput_suiteend( pcu_listener_t* lsnr, pcu_suite_t* suite ) {
   _pcu_xmloutput_printsuitestatus( lsnr, suite, 1 );
}

void pcu_xmloutput_testbegin( pcu_listener_t* lsnr, pcu_test_t* test ) {
}

void pcu_xmloutput_testend( pcu_listener_t* lsnr, pcu_test_t* test ) {
   _pcu_xmloutput_printteststatus( lsnr, test->suite, test->name, 0 );
}

void pcu_xmloutput_checkdone( pcu_listener_t* lsnr, pcu_source_t* src ) {
	if( !src->result )
      longjmp( pcu_rollback_env, 1);
}

void pcu_xmloutput_runbegin( pcu_listener_t* lsnr, int nsuites ) {
   /* Nothing needs to be done, since each suite is an independent file */
}

void pcu_xmloutput_runend( pcu_listener_t* lsnr, int nsuites, int totalPasses, int totalTests ) {
   /* Nothing needs to be done, since each suite is an independent file */
}

pcu_listener_t* pcu_xmloutput_create( const char* projectName, int printdocs, const char* outputDir ) {
   pcu_listener_t*   lsnr;
   xmloutputdata_t*  xmlData = NULL;

   lsnr = (pcu_listener_t*)malloc( sizeof(pcu_listener_t) );
   lsnr->suitebegin = pcu_xmloutput_suitebegin;
   lsnr->suiteend = pcu_xmloutput_suiteend;
   lsnr->testbegin = pcu_xmloutput_testbegin;
   lsnr->testend = pcu_xmloutput_testend;
   lsnr->checkdone = pcu_xmloutput_checkdone;
   lsnr->runbegin = pcu_xmloutput_runbegin;
   lsnr->runend = pcu_xmloutput_runend;
   assert( projectName );
   lsnr->projectName = strdup( projectName );
   assert( printdocs == 1 || printdocs == 0 );
   lsnr->printdocs = printdocs;
   lsnr->data = malloc( sizeof(xmloutputdata_t) );
   xmlData = ((xmloutputdata_t*)lsnr->data);
   MPI_Comm_rank( MPI_COMM_WORLD, &xmlData->rank );

   if ( outputDir ) {
      xmlData->resultsDir = strdup( outputDir ); 
   }
   else {
      /* Set to the Java/Ant default */
      xmlData->resultsDir = strdup( DEFAULT_RESULTS_DIR ); 
   }

   /* Set up output directory for results */
   _pcu_xmloutput_prepareoutputdir( lsnr );

   return lsnr;
}

void pcu_xmloutput_destroy( pcu_listener_t* lsnr ) {
   xmloutputdata_t*  xmlData = ((xmloutputdata_t*)lsnr->data);

   /* Don't free the xmlDocs here - will have already been done at end of each separate document */

   free( lsnr->projectName );

   if( xmlData ) {
      free( xmlData->resultsDir );
      free( xmlData );
   }   
   free( lsnr );
}

void _pcu_xmloutput_createXMLDocument( pcu_listener_t* lsnr, const char* rootNodeName ) {
   char*             nameSpaceString = NULL;
   xmloutputdata_t*  xmlData = ((xmloutputdata_t*)lsnr->data);
   
   nameSpaceString = (char*)malloc(sizeof(char) * 1000 );
   sprintf( nameSpaceString, "%s", XML_NAMESPACE );

   xmlData->currDoc = xmlNewDoc( XML_VERSION );
	assert( xmlData->currDoc );
	/* create root element */
   xmlData->rootNode = xmlNewNode( NULL, (xmlChar*)rootNodeName );
	assert( xmlData->rootNode );
	
	xmlData->currNameSpace = xmlNewNs( xmlData->rootNode, (xmlChar*) nameSpaceString, NULL );
	assert( xmlData->currNameSpace );
	xmlSetNs( xmlData->rootNode, xmlData->currNameSpace );
	
	xmlDocSetRootElement( xmlData->currDoc, xmlData->rootNode );

   free( nameSpaceString );
}

void _pcu_xmloutput_printsuitestatus( pcu_listener_t* lsnr, pcu_suite_t* suite, int final ) { 
   xmloutputdata_t*  xmlData = (xmloutputdata_t*)lsnr->data;

   if( xmlData->rank )
      return;

   if( !final ) {
      char        intString[100];
      xmlNodePtr  suiteNode = NULL;

      _pcu_xmloutput_createXMLDocument( lsnr, "testsuite" );
      suiteNode = xmlData->rootNode;
      xmlNewProp( suiteNode, (xmlChar*)"name", (xmlChar*)suite->name );
      xmlNewProp( suiteNode, (xmlChar*)"project", (xmlChar*)lsnr->projectName );
      xmlNewProp( suiteNode, (xmlChar*)"module", (xmlChar*)suite->moduleDir );
      sprintf( intString, "%i", suite->ntests );
      xmlNewProp( suiteNode, (xmlChar*)"tests", (xmlChar*)intString );
   }
   else {
      char* filename = NULL;
      char  intString[100];
      int   fileSize = 0;
      char* currDirName = (char*)malloc(sizeof(char)*PATH_MAX+1);

      xmlNodePtr  suiteNode = xmlData->rootNode;

      
      sprintf( intString, "%i", suite->ntests - suite->npassed );
      xmlNewProp( suiteNode, (xmlChar*)"failures", (xmlChar*)intString );
      /* Could also add some time-stamping info here if we build this in */

      /* change to results dir for writing */
      getcwd( currDirName, PATH_MAX );
      chdir( xmlData->resultsDir );
      chdir( lsnr->projectName );

      filename = (char*)malloc( sizeof(char) * (strlen(RESULTS_FILE_PREFIX)+strlen(suite->name)+4+1) );
      sprintf( filename, "%s%s.xml", RESULTS_FILE_PREFIX, suite->name );
      if ( (fileSize = xmlSaveFormatFile( filename, xmlData->currDoc, 1 )) <= 0 ) {
         printf( "Warning: failed to write dictionary contents to file %s.\n", filename );
      }
      free( filename );
      xmlFreeDoc( xmlData->currDoc );
      xmlData->currDoc = NULL;

      /* change back to default dir */
      chdir( currDirName );
      free( currDirName );
   }
}

void _pcu_xmloutput_printteststatus( pcu_listener_t* lsnr, pcu_suite_t* suite, char* testname, int final ) {
   xmloutputdata_t*  xmlData = (xmloutputdata_t*)lsnr->data;
   xmlNodePtr        newNode;

   if( xmlData->rank )
      return;

   newNode = xmlNewTextChild( xmlData->rootNode, xmlData->currNameSpace, (xmlChar*)"test", NULL );

   xmlNewProp( newNode, (xmlChar*)"name", (xmlChar*)testname );
   if (suite->curtest->globalresult) {
      xmlNewProp( newNode, (xmlChar*)"status", (xmlChar*)"pass" );
   }
   else { 
      xmlNewProp( newNode, (xmlChar*)"status", (xmlChar*)"failure" );
      _pcu_xmloutput_printsourcefailures( lsnr, suite->curtest, newNode );
   }   
   xmlNewProp( newNode, (xmlChar*)"description", (xmlChar*)suite->curtest->docString );
}

void _pcu_xmloutput_printsourcefailures( pcu_listener_t* lsnr, pcu_test_t* test, xmlNodePtr testXMLNode ) {
   xmloutputdata_t*  xmlData = (xmloutputdata_t*)lsnr->data;
   pcu_source_t*     src;
   char              intString[100];
   xmlNodePtr        srcXMLNode;
   xmlNodePtr        cDataMsgNode;

   if( xmlData->rank )
      return;

   src = test->srcs;
   while( src ) {
      if( !src->result ) {
         srcXMLNode = xmlNewChild( testXMLNode, xmlData->currNameSpace, (xmlChar*)"src", NULL );
         xmlNewProp( srcXMLNode, (xmlChar*)"type", (xmlChar*)src->type );
         xmlNewProp( srcXMLNode, (xmlChar*)"expression", (xmlChar*)src->expr );
         xmlNewProp( srcXMLNode, (xmlChar*)"file", (xmlChar*)src->file );
         sprintf( intString, "%i", src->line );
         xmlNewProp( srcXMLNode, (xmlChar*)"line", (xmlChar*)intString );
         sprintf( intString, "%i", src->rank );
         xmlNewProp( srcXMLNode, (xmlChar*)"rank", (xmlChar*)intString );
         if (src->msg) {
            /* The msg might include >, < etc which are XML reserved characters: prefer to use CDATA than
              encode these with backslashes etc */
            cDataMsgNode = xmlNewCDataBlock( xmlData->currDoc, (xmlChar*)src->msg, strlen(src->msg) );
            xmlAddChild( srcXMLNode, cDataMsgNode );
         }
      }
      src = src->next;
   }
}

void _pcu_xmloutput_prepareoutputdir( pcu_listener_t* lsnr ) {
   xmloutputdata_t*  xmlData = (xmloutputdata_t*)lsnr->data;
   char*             currDirName = (char*)malloc(sizeof(char)*PATH_MAX+1);

   if( xmlData->rank )
      return;

   _pcu_xmloutput_checkcreatedir( xmlData->resultsDir );

   getcwd( currDirName, PATH_MAX );
   chdir( xmlData->resultsDir );

   _pcu_xmloutput_checkcreatedir( lsnr->projectName );
   _pcu_xmloutput_cleanresults( lsnr->projectName );

   chdir( currDirName );

   free(currDirName);
}

void _pcu_xmloutput_checkcreatedir( const char* dirName ) {
   int               existsWritable = 0;

   existsWritable = _pcu_xmloutput_checkdirexistswritable( dirName );

   if ( -1 == existsWritable && (errno == ENOENT) ) {
         int   result = 0;

         /* If the directory doesn't exist, make an attempt to create it */
         result = mkdir( dirName, S_IRUSR | S_IWUSR | S_IXUSR );
         if (result != 0) {
            perror( "PCU XML output directory setup error, error in directory creation" );   
            exit(EXIT_FAILURE);
         }
   }
   else if ( -1 == existsWritable ) {
         perror( "PCU XML output directory setup error" );
         exit(EXIT_FAILURE);
   }
   /* Otherwise, the directory exists and we had no error */
}

int _pcu_xmloutput_checkdirexistswritable( const char* dirName ) {   
   struct stat       statBuf;
   int               sResult=0;

   sResult = lstat( dirName, &statBuf );

   if (0 == sResult) {
      /* A File/directory of that name exists and is valid... */

      if ( !S_ISDIR(statBuf.st_mode) ) {
         printf( "PCU XML Output creation error: given results directory %s exists, but is not a directory." 
            " Please check this and rename/delete the existing file, or specify a different output directory.\n",
            dirName );
         exit(EXIT_FAILURE);
      }
      else if ( 0 != access( dirName, W_OK ) ) {  
         /* We don't have permission to write output files in this directory */
         printf( "PCU XML Output creation error: given results directory %s exists, but the user executing the" 
            " program doesn't have write access to this directory.\n",
            dirName );
         exit(EXIT_FAILURE);
      }     
   }   
   
   /* If sResult is 0, dir exists and is writable, otherwise if -1 it doesn't exist or an error occurred */
   return sResult;
}

void _pcu_xmloutput_cleanresults( const char* resultsDirName ) {
   int            result=0;
   DIR*           resultsDir;
   struct dirent* dirEntry;
   char           filenameToDelete[10000];
   int            prefixLen=0;  

   resultsDir = opendir( resultsDirName );
   /* Error check to be safe */
   if (resultsDir == NULL) {
      perror( "PCU XML output directory setup error, error in directory opening for cleaning" );   
      exit(EXIT_FAILURE);
   }

   prefixLen = strlen( RESULTS_FILE_PREFIX );
   while ( (dirEntry = readdir( resultsDir )) ) {
      if ( 0 == strncmp( dirEntry->d_name, RESULTS_FILE_PREFIX, prefixLen ) ) {  
         sprintf( filenameToDelete, "%s/%s", resultsDirName, dirEntry->d_name );
         result = unlink( filenameToDelete );
         if ( 0 != result ) {
            perror( "PCU XML output directory setup error, Failure while deleting old results" ); 
            exit(EXIT_FAILURE);
         }
      }   
   }

   closedir( resultsDir );
}   

