/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_main_h__
#define __StGermain_main_h__

   /* Generate flattened XML. */
   void stgGenerateFlattenedXML( Dictionary* dictionary, Dictionary* sources, char* timeStamp );

	/* The StGermain main construction */
	Stg_ComponentFactory* stgMainConstruct( Dictionary* dictionary, Dictionary* sources, MPI_Comm communicator, void* _context );

	/* The StGermain main building and initialisation */
	void stgMainBuildAndInitialise( Stg_ComponentFactory* cf );

	/* Initialise the context, from a particular XML file. This saves the user manipulating
	   an IO_Handler and dictionary to get the data into the context. Useful for test code. */
	Stg_ComponentFactory* stgMainInitFromXML( char* xmlInputFilename, MPI_Comm communicator, void* _context );

	/* The StGermain main loop */
	void stgMainLoop( Stg_ComponentFactory* cf );

	/* The StGermain main destruction */
	void stgMainDestroy( Stg_ComponentFactory* cf );

	/* Add a toolbox to the "import" list in the dictionary */
	void stgImportToolbox( Dictionary* dictionary, char* toolboxName );
	
#endif /* __StGermain_main_h__ */
