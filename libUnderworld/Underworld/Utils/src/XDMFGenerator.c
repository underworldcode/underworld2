/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

#include "XDMFGenerator.h"

#include <stdlib.h>
#include <string.h>

#ifndef MASTER
   #define MASTER 0
#endif

const Type XDMFGenerator_Type = "XDMFGenerator";

void XDMFGenerator_GenerateAll( void* _context ) {
   UnderworldContext*   context    = (UnderworldContext*)_context;
   Stream*              stream;

   /** only the MASTER process writes to the file.  other processes send information to the MASTER where required **/
   if(context->rank == MASTER) {
      Bool                 fileOpened;
      Stream*              errorStream  = Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  );
      char*                 filename;
      char*                outputPathString;

      /** Create Stream **/
      stream = Journal_Register( InfoStream_Type, (Name)"XDMFOutputFile"  );

      /** Set auto flush on stream **/
      Stream_SetAutoFlush( stream, True );

      outputPathString = Context_GetCheckPointWritePrefixString( context );

      /** Get name of XDMF schema file **/
      Stg_asprintf( &filename, "%sXDMF.%05d.xmf", outputPathString, context->timeStep );

      /** Init file, always overwriting any existing **/
      fileOpened = Stream_RedirectFile( stream, filename );
      Journal_Firewall( fileOpened, errorStream,
            "Could not open file %s. Possibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );

      /** write header information **/
      _XDMFGenerator_WriteHeader( context, stream );

      /** Write all (checkpointed) FeVariable field information  **/
      _XDMFGenerator_WriteFieldSchema( context, stream );

      /** Write all (checkpointed) FeVariable field information  **/
      if ( ( context->checkpointEvery != 0 ) && ( context->timeStep % context->checkpointEvery == 0 ) ||
           ( context->checkpointAtTimeInc && ( context->currentTime >= context->nextCheckpointTime ) ) )
         _XDMFGenerator_WriteSwarmSchema( context, stream);

      /** writes footer information and close file/stream **/
      _XDMFGenerator_WriteFooter( context, stream );

      /** close the file **/
      Stream_CloseFile( stream );

      /** update file lists for temporal collections **/
      _XDMFGenerator_UpdateTemporalFileLists( context );

      /** add two files which allow users to load only fields, or only swarms **/
      _XDMFGenerator_FieldsOrSwarmsOnlyFiles( context );


      Memory_Free( filename );
      Memory_Free( outputPathString );


   } else {
      /** other process send information about swarms populations to MASTER **/
      if ( ( context->checkpointEvery != 0 ) && ( context->timeStep % context->checkpointEvery == 0 ) ||
           ( context->checkpointAtTimeInc && ( context->currentTime >= context->nextCheckpointTime ) ) )
         _XDMFGenerator_SendInfo( context );
   }
}

void _XDMFGenerator_WriteFieldSchema( UnderworldContext* context, Stream* stream ) {
   FieldVariable*       fieldVar = NULL;
   FeVariable*          feVar    = NULL;
   Mesh*                mesh     = NULL;
   FeMesh*              feMesh   = NULL;
   unsigned             nDims;
   unsigned             totalVerts;
   Index                maxNodes;
   Index                elementGlobalSize;
   Element_GlobalIndex  gElement_I;
   Index                var_I = 0;
   Bool                 saveCoords  = Dictionary_GetBool_WithDefault( context->dictionary, (Dictionary_Entry_Key)"saveCoordsWithFields", False  );
   Stream*              errorStream = Journal_Register( Error_Type, (Name)CURR_MODULE_NAME );
   Name                 variableType = NULL;
   Name                 topologyType = NULL;
   unsigned             componentCount = LiveComponentRegister_GetCount(stgLiveComponentRegister);
   unsigned             compI;
   Stg_Component*       stgComp;
   char*                prefixGuy;
   if (strcmp("",context->checkPointPrefixString))
      Stg_asprintf( &prefixGuy, "%s." , context->checkPointPrefixString );
   else
      Stg_asprintf( &prefixGuy, "" );

   /** search for entire live component register for feMesh types  **/
   for( compI = 0 ; compI < componentCount ; compI++ ){
      stgComp = LiveComponentRegister_At( stgLiveComponentRegister, compI );
      /* check that component is of type FeMesh, and that its element family is linear */
      if ( Stg_Class_IsInstance( stgComp, FeMesh_Type ) && !strcmp( ((FeMesh*)stgComp)->feElFamily, "linear" ) && ((Mesh*)stgComp)->isCheckpointedAndReloaded ) {
         mesh   = (  Mesh*)stgComp;
         feMesh = (FeMesh*)stgComp;

         nDims             = Mesh_GetDimSize( mesh );
         totalVerts        = Mesh_GetGlobalSize( mesh, 0 );
         elementGlobalSize = FeMesh_GetElementGlobalSize(mesh);

         /* get connectivity array size */
         if (mesh->nElTypes == 1)
            maxNodes = FeMesh_GetElementNodeSize( mesh, 0);
         else {
            /* determine the maximum number of nodes each element has */
            maxNodes = 0;
            for ( gElement_I = 0 ; gElement_I < FeMesh_GetElementGlobalSize(mesh); gElement_I++ ) {
               unsigned numNodes;
               numNodes = FeMesh_GetElementNodeSize( mesh, gElement_I);
               if( maxNodes < numNodes ) maxNodes = numNodes;
            }
         }
         /** now write all the xdmf geometry info **/
         /**----------------------- START GEOMETRY   ------------------------------------------------------------------------------------------------------------------- **/
         if(         maxNodes == 4 ){
            Stg_asprintf( &topologyType, "Quadrilateral" );
         } else if ( maxNodes == 8  ) {
            Stg_asprintf( &topologyType, "Hexahedron" );
         } else {
            Journal_DPrintf( errorStream, "\n\n Error: number of element nodes %u not supported by XDMF generator...\n should be 4 (2D quadrilateral) "
                                    "or should be 8 (3D hexahedron). \n\n", maxNodes );
            Stg_asprintf( &topologyType, "UNKNOWN_POSSIBLY_ERROR" );
         }
         /** first create the mesh which is applicable to the checkpointed fevariables **/
                                 Journal_Printf( stream, "   <Grid Name=\"FEM_Mesh_%s\">\n\n", feMesh->name);
                                 Journal_Printf( stream, "      <Time Value=\"%g\" />\n\n", context->currentTime );
         /** now print out topology info, only quadrilateral elements are supported at the moment **/
                                 Journal_Printf( stream, "         <Topology Type=\"%s\" NumberOfElements=\"%u\"> \n", topologyType, elementGlobalSize );
                                 if( feMesh->isDeforming == 1 ) {
                                    Journal_Printf( stream, "            <DataItem Format=\"HDF\" DataType=\"Int\"  Dimensions=\"%u %u\">%sMesh.%s.%05d.h5:/connectivity</DataItem>\n", elementGlobalSize, maxNodes, prefixGuy, feMesh->name, context->timeStep );
                                 } else {
                                    Journal_Printf( stream, "            <DataItem Format=\"HDF\" DataType=\"Int\"  Dimensions=\"%u %u\">%sMesh.%s.%05d.h5:/connectivity</DataItem>\n", elementGlobalSize, maxNodes, prefixGuy, feMesh->name, 0 );
                                 }
                                 Journal_Printf( stream, "         </Topology>\n\n" );
                                 Journal_Printf( stream, "         <Geometry Type=\"XYZ\">\n" );

         Stg_asprintf( &variableType, "NumberType=\"Float\" Precision=\"8\"" );
         if(         nDims == 2 ){
            /** note that for 2d, we feed back a quasi 3d array, with the 3rd Dof zeroed.  so in effect we always work in 3d.
                this is done because paraview in particular seems to do everything in 3d, and if you try and give it a 2d vector
                or array, it complains.... and hence the verbosity of the following 2d definitions**/
                                 Journal_Printf( stream, "            <DataItem ItemType=\"Function\"  Dimensions=\"%u 3\" Function=\"JOIN($0, $1, 0*$1)\">\n", totalVerts );
                                 Journal_Printf( stream, "               <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"XCoords\">\n", totalVerts );
                                 Journal_Printf( stream, "                  <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 0 1 1 %u 1 </DataItem>\n", totalVerts );
                                 if( feMesh->isDeforming == 1 ) {
                                    Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u 2\">%sMesh.%s.%05d.h5:/vertices</DataItem>\n", variableType, totalVerts, prefixGuy, feMesh->name,  context->timeStep );
                                 } else {
                                    Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u 2\">%sMesh.%s.%05d.h5:/vertices</DataItem>\n", variableType, totalVerts, prefixGuy, feMesh->name,  0 );
                                 }
                                 Journal_Printf( stream, "               </DataItem>\n" );
                                 Journal_Printf( stream, "               <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"YCoords\">\n", totalVerts );
                                 Journal_Printf( stream, "                  <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 1 1 1 %u 1 </DataItem>\n", totalVerts );
                                 if( feMesh->isDeforming == 1 ) {
                                    Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u 2\">%sMesh.%s.%05d.h5:/vertices</DataItem>\n", variableType, totalVerts, prefixGuy, feMesh->name, context->timeStep );
                                 } else {
                                    Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u 2\">%sMesh.%s.%05d.h5:/vertices</DataItem>\n", variableType, totalVerts, prefixGuy, feMesh->name, 0 );
                                 }
                                 Journal_Printf( stream, "               </DataItem>\n" );
                                 Journal_Printf( stream, "            </DataItem>\n" );
         } else if ( nDims == 3 ) {
            /** in 3d we simply feed back the 3d hdf5 array, nice and easy **/
                                 if( feMesh->isDeforming == 1 ) {
                                    Journal_Printf( stream, "            <DataItem Format=\"HDF\" %s Dimensions=\"%u 3\">%sMesh.%s.%05d.h5:/vertices</DataItem>\n", variableType, totalVerts, prefixGuy, feMesh->name, context->timeStep );
                                 } else {
                                    Journal_Printf( stream, "            <DataItem Format=\"HDF\" %s Dimensions=\"%u 3\">%sMesh.%s.%05d.h5:/vertices</DataItem>\n", variableType, totalVerts, prefixGuy, feMesh->name, 0 );
                                 }
         } else {
            Journal_DPrintf( errorStream, "\n\n Error: Mesh vertex location is not of dofCount 2 or 3.\n\n" );
         }
                                 Journal_Printf( stream, "         </Geometry>\n\n" );
         /**----------------------- FINISH GEOMETRY  ------------------------------------------------------------------------------------------------------------------- **/


      /** now write FeVariable data **/
      for ( var_I = 0; var_I < context->fieldVariable_Register->objects->count; var_I++ ) {
         fieldVar = FieldVariable_Register_GetByIndex( context->fieldVariable_Register, var_I );

         if ( Stg_Class_IsInstance( fieldVar, FeVariable_Type ) ) {
            feVar = (FeVariable*)fieldVar;
            if ( (feVar->isCheckpointedAndReloaded && (context->checkpointEvery != 0) && (context->timeStep % context->checkpointEvery == 0))  ||
                 (feVar->isCheckpointedAndReloaded && (context->checkpointAtTimeInc && (context->currentTime >= context->nextCheckpointTime))) ||
                 (feVar->isSavedData               && (context->saveDataEvery != 0) && (context->timeStep % context->saveDataEvery   == 0)) ){
               FeMesh* feVarMesh = NULL;
               /** check what type of generator was used to know where elementMesh is **/
               if( Stg_Class_IsInstance( feVar->feMesh->generator, C0Generator_Type))        feVarMesh = (FeMesh*)((C0Generator*)feVar->feMesh->generator)->elMesh;
               if( Stg_Class_IsInstance( feVar->feMesh->generator, CartesianGenerator_Type)) feVarMesh = feVar->feMesh;
               if( !strcmp( feVar->feMesh->generator->type, "ProjectionGenerator")) feVarMesh = feVar->feMesh; // hack to correctly use the ProjectionGenerator_Type
               /** make sure that the fevariable femesh is the same as that used above for the geometry definition, if so proceed **/
               if( feVarMesh == feMesh ){
                     Name   centering = NULL;
                     Index  offset = 0;
                     Index  meshSize = Mesh_GetGlobalSize( feVar->feMesh, 0 );
                     Index  dofCountIndex;
                     Index  dofAtEachNodeCount;

               /**----------------------- START ATTRIBUTES ------------------------------------------------------------------------------------------------------------------- **/
                     /** if coordinates are being stored with feVariable, account for this **/
                     if( saveCoords) offset = nDims;
                     /** all feVariables are currently stored as doubles **/
                     Stg_asprintf( &variableType, "NumberType=\"Float\" Precision=\"8\"" );
                     /** determine whether feVariable data is cell centered (like Pressure), or on the nodes (like Velocity) **/
                     if(        meshSize == elementGlobalSize ){
                        Stg_asprintf( &centering, "Cell" );
                     } else if( meshSize == totalVerts ) {
                        Stg_asprintf( &centering, "Node" );
                     } else {
                        /* unknown/unsupported type */
                        Stg_asprintf( &centering, "UNKNOWN_POSSIBLY_ERROR" );
                     }
                     /** how many degrees of freedom does the fevariable have? **/
                     dofAtEachNodeCount = feVar->fieldComponentCount;
                     if (        dofAtEachNodeCount == 1 ) {
                                 Journal_Printf( stream, "         <Attribute Type=\"Scalar\" Center=\"%s\" Name=\"%s\">\n", centering,  feVar->name);
                                 Journal_Printf( stream, "            <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" >\n", meshSize );
                                 Journal_Printf( stream, "               <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u 1 </DataItem>\n", offset, meshSize );
                                 Journal_Printf( stream, "               <DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s%s.%05d.h5:/data</DataItem>\n", variableType, meshSize, (offset + dofAtEachNodeCount), prefixGuy, feVar->name, context->timeStep);
                                 Journal_Printf( stream, "            </DataItem>\n" );
                                 Journal_Printf( stream, "         </Attribute>\n\n" );
                     } else if ( dofAtEachNodeCount == 2 ){
                        /** note that for 2d, we feed back a quasi 3d array, with the 3rd Dof zeroed.  so in effect we always work in 3d.
                            this is done because paraview in particular seems to do everything in 3d, and if you try and give it a 2d vector
                            or array, it complains.... and hence the verbosity of the following 2d definitions **/
                                 Journal_Printf( stream, "         <Attribute Type=\"Vector\" Center=\"%s\" Name=\"%s\">\n", centering,  feVar->name);
                                 Journal_Printf( stream, "            <DataItem ItemType=\"Function\"  Dimensions=\"%u 3\" Function=\"JOIN($0, $1, 0*$1)\">\n", meshSize );
                                 Journal_Printf( stream, "               <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"XValue\">\n", meshSize );
                                 Journal_Printf( stream, "                  <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u 1 </DataItem>\n", offset, meshSize );
                                 Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s%s.%05d.h5:/data</DataItem>\n", variableType, meshSize, (offset + dofAtEachNodeCount), prefixGuy, feVar->name, context->timeStep);
                                 Journal_Printf( stream, "               </DataItem>\n" );
                                 Journal_Printf( stream, "               <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"YValue\">\n", meshSize );
                                 Journal_Printf( stream, "                  <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u 1 </DataItem>\n", (offset+1), meshSize );
                                 Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s%s.%05d.h5:/data</DataItem>\n", variableType, meshSize, (offset + dofAtEachNodeCount), prefixGuy, feVar->name, context->timeStep);
                                 Journal_Printf( stream, "               </DataItem>\n" );
                                 Journal_Printf( stream, "            </DataItem>\n" );
                                 Journal_Printf( stream, "         </Attribute>\n\n" );
                     } else if ( dofAtEachNodeCount == 3 ) {
                        /* Vector header in 3D, vector header in 2D (though it should be a tensor) */
                                 Journal_Printf( stream, "         <Attribute Type=\"Vector\" Center=\"%s\" Name=\"%s\">\n", centering,  feVar->name);
                                 Journal_Printf( stream, "            <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 3\" >\n", meshSize );
                                 Journal_Printf( stream, "               <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u 3 </DataItem>\n", offset, meshSize );
                                 Journal_Printf( stream, "               <DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s%s.%05d.h5:/data</DataItem>\n", variableType, meshSize, (offset + dofAtEachNodeCount), prefixGuy, feVar->name, context->timeStep);
                                 Journal_Printf( stream, "            </DataItem>\n" );
                                 Journal_Printf( stream, "         </Attribute>\n\n" );
                     } else if ( dofAtEachNodeCount == 6 && nDims== 3 ) {
                        /* Symmetric Tensor in 3D */
                                    Journal_Printf( stream, "         <Attribute Type=\"Tensor6\" Center=\"%s\" Name=\"%s\">\n", centering,  feVar->name);
                                    Journal_Printf( stream, "            <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u %u\" >\n", meshSize, dofAtEachNodeCount );
                                    Journal_Printf( stream, "               <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u %u </DataItem>\n", offset, meshSize, dofAtEachNodeCount );
                                    Journal_Printf( stream, "               <DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s%s.%05d.h5:/data</DataItem>\n", variableType, meshSize, (offset + dofAtEachNodeCount), prefixGuy, feVar->name, context->timeStep);
                                    Journal_Printf( stream, "            </DataItem>\n" );
                                    Journal_Printf( stream, "         </Attribute>\n\n" );
                     } else if ( dofAtEachNodeCount == 9 && nDims==3 ) {
                           /* Full Tensor in 3D */
                                    Journal_Printf( stream, "         <Attribute Type=\"Tensor\" Center=\"%s\" Name=\"%s\">\n", centering,  feVar->name);
                                    Journal_Printf( stream, "            <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u %u\" >\n", meshSize, dofAtEachNodeCount );
                                    Journal_Printf( stream, "               <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u %u </DataItem>\n", offset, meshSize, dofAtEachNodeCount );
                                    Journal_Printf( stream, "               <DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s%s.%05d.h5:/data</DataItem>\n", variableType, meshSize, (offset + dofAtEachNodeCount), prefixGuy, feVar->name, context->timeStep);
                                    Journal_Printf( stream, "            </DataItem>\n" );
                                    Journal_Printf( stream, "         </Attribute>\n\n" );
                     }
                     else {
                        /** where there are more than 3 components, we write each one out as a scalar **/
                        for(dofCountIndex = 0 ; dofCountIndex < dofAtEachNodeCount ; ++dofCountIndex){
                                 Journal_Printf( stream, "         <Attribute Type=\"Scalar\" Center=\"%s\" Name=\"%s-Component-%u\">\n", centering,  feVar->name, dofCountIndex);
                                 Journal_Printf( stream, "            <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" >\n", meshSize );
                                 Journal_Printf( stream, "               <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u 1 </DataItem>\n", (offset+dofCountIndex), meshSize );
                                 Journal_Printf( stream, "               <DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s%s.%05d.h5:/data</DataItem>\n", variableType, meshSize, (offset + dofAtEachNodeCount), prefixGuy, feVar->name, context->timeStep);
                                 Journal_Printf( stream, "            </DataItem>\n" );
                                 Journal_Printf( stream, "         </Attribute>\n" );
                        }
                                 Journal_Printf( stream, "\n" );
                     }
               /**----------------------- END ATTRIBUTES   ------------------------------------------------------------------------------------------------------------------- **/
               if(centering) {
                  Memory_Free( centering );
                  centering = NULL;
               }
               }
               }
         }
      }
                                 Journal_Printf( stream, "   </Grid>\n\n" );
      if(variableType){ 
         Memory_Free( variableType );
         variableType = NULL;
      }
      if(topologyType){
         Memory_Free( topologyType );
         topologyType=NULL;
      }
      }
   }
   Memory_Free(prefixGuy);
}

void _XDMFGenerator_WriteSwarmSchema( UnderworldContext* context, Stream* stream ) {
   Swarm_Register* swarmRegister = Swarm_Register_GetSwarm_Register();
   Index           swarmCount;
   Index           swarmcountindex;
   Index           variablecount;
   Index           dofCountIndex;
   Index           swarmParticleLocalCount;
   Index           countindex;
   Index           ii;
   Swarm*          currentSwarm;
   SwarmVariable*  swarmVar;
   Name            swarmVarName;
   Name            variableType = NULL;
   Name            filename_part = NULL;
   Stream*         errorStream  = Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  );
   const int       FINISHED_WRITING_TAG = 100;
   MPI_Status      status;
   char*                prefixGuy;
   if (strcmp("",context->checkPointPrefixString))
      Stg_asprintf( &prefixGuy, "%s." , context->checkPointPrefixString );
   else
      Stg_asprintf( &prefixGuy, "" );

   /** get total number of different swarms **/
   swarmCount  = swarmRegister->swarmList->count;

   /** parse swarm list, checking which are actually stored to HDF5.  **/
   /** We assume that all processes have the same number of swarms on them, with the same swarms checkpointed**/
   for(swarmcountindex = 0; swarmcountindex < swarmCount; ++swarmcountindex){
      currentSwarm  = Swarm_Register_At( swarmRegister, swarmcountindex );

      if ( currentSwarm->isSwarmTypeToCheckPointAndReload != True )
         continue;

      swarmParticleLocalCount = currentSwarm->particleLocalCount;
      /** first create a grid collection which will contain the collection of swarms from each process.
          there will be one of these collections for each swarm (not swarmvariable) that is checkpointed **/
                              Journal_Printf( stream, "   <Grid Name=\"%s\" GridType=\"Collection\">\n\n", currentSwarm->name );
                              Journal_Printf( stream, "      <Time Value=\"%g\" />\n\n", context->currentTime );

      Stg_asprintf( &filename_part, "" );
      for (ii = 0 ; ii < context->nproc ; ++ii) {

         /** get the number of particles in each swarm from each process **/
         if (ii != MASTER        ) (void)MPI_Recv( &swarmParticleLocalCount, 1, MPI_INT, ii, FINISHED_WRITING_TAG, context->communicator, &status );
         if (context->nproc != 1 ) Stg_asprintf( &filename_part, ".%uof%u", (ii + 1), context->nproc );

         /** first write all the MASTER procs swarm info **/
         if (swarmParticleLocalCount != 0) {
                              Journal_Printf( stream, "      <Grid Name=\"%s_proc_%u\">\n\n", currentSwarm->name, ii );

            /** now write all the xdmf geometry info **/
            /**----------------------- START GEOMETRY   ------------------------------------------------------------------------------------------------------------------- **/
                              Journal_Printf( stream, "         <Topology Type=\"POLYVERTEX\" NodesPerElement=\"%u\"> </Topology>\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "         <Geometry Type=\"XYZ\">\n" );
            Stg_asprintf( &swarmVarName, "%s-Position", currentSwarm->name );
            swarmVar = SwarmVariable_Register_GetByName( currentSwarm->swarmVariable_Register, swarmVarName );
            if (!swarmVar)
               Journal_DPrintf( errorStream, "\n\n Error: Could not find required Position SwarmVariable. \n\n" );
            Memory_Free( swarmVarName );

            /** check what precision it Position variable is stored at **/
            if(        swarmVar->variable->dataTypes[0] == Variable_DataType_Int ){
               Journal_DPrintf( errorStream, "\n\n Error: Position variable can not be of type Int. \n\n" );
            } else if ( swarmVar->variable->dataTypes[0] == Variable_DataType_Char){
               Journal_DPrintf( errorStream, "\n\n Error: Position variable can not be of type Char. \n\n" );
            } else if ( swarmVar->variable->dataTypes[0] == Variable_DataType_Float ){
               Stg_asprintf( &variableType, "NumberType=\"Float\" Precision=\"4\"" );
            } else {
               Stg_asprintf( &variableType, "NumberType=\"Float\" Precision=\"8\"" );
            }

            if(         swarmVar->dofCount == 2 ){
               /** note that for 2d, we feed back a quasi 3d array, with the 3rd Dof zeroed.  so in effect we always work in 3d.
                   this is done because paraview in particular seems to do everything in 3d, and if you try and give it a 2d vector
                   or array, it complains.... and hence the verbosity of the following 2d definitions**/
                              Journal_Printf( stream, "            <DataItem ItemType=\"Function\"  Dimensions=\"%u 3\" Function=\"JOIN($0, $1, 0*$1)\">\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "               <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"XCoords\">\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "                  <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 0 1 1 %u 1 </DataItem>\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u 2\">%s%s.%05d%s.h5:/Position</DataItem>\n", variableType, swarmParticleLocalCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part );
                              Journal_Printf( stream, "               </DataItem>\n" );
                              Journal_Printf( stream, "               <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"YCoords\">\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "                  <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 1 1 1 %u 1 </DataItem>\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u 2\">%s%s.%05d%s.h5:/Position</DataItem>\n", variableType, swarmParticleLocalCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part );
                              Journal_Printf( stream, "               </DataItem>\n" );
                              Journal_Printf( stream, "            </DataItem>\n" );
            } else if ( swarmVar->dofCount == 3 ) {
               /** in 3d we simply feed back the 3d hdf5 array, nice and easy **/
                              Journal_Printf( stream, "            <DataItem Format=\"HDF\" %s Dimensions=\"%u 3\">%s%s.%05d%s.h5:/Position</DataItem>\n", variableType, swarmParticleLocalCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part );
            } else {
               Journal_DPrintf( errorStream, "\n\n Error: Position SwarmVariable is not of dofCount 2 or 3.\n\n" );
            }
                              Journal_Printf( stream, "         </Geometry>\n\n" );
            /**----------------------- FINISH GEOMETRY  --------------------------------------------------------------------------------------------------------- **/


            /** now write all the swarm attributes.. ie all the checkpointed swarmVariables **/

            variablecount = currentSwarm->swarmVariable_Register->objects->count;
            for(countindex = 0; countindex < variablecount; ++countindex){
               swarmVar = SwarmVariable_Register_GetByIndex( currentSwarm->swarmVariable_Register, countindex );
               if( swarmVar->isCheckpointedAndReloaded ) {
            /**----------------------- START ATTRIBUTES --------------------------------------------------------------------------------------------------------- **/
                  if(         swarmVar->variable->dataTypes[0] == Variable_DataType_Int ){
                     Stg_asprintf( &variableType, "NumberType=\"Int\"" );
                  } else if ( swarmVar->variable->dataTypes[0] == Variable_DataType_Char){
                     Stg_asprintf( &variableType, "NumberType=\"Char\"" );
                  } else if ( swarmVar->variable->dataTypes[0] == Variable_DataType_Float ){
                     Stg_asprintf( &variableType, "NumberType=\"Float\" Precision=\"4\"" );
                  } else {
                     Stg_asprintf( &variableType, "NumberType=\"Float\" Precision=\"8\"" );
                  }
                  if (        swarmVar->dofCount == 1 ) {
                              Journal_Printf( stream, "         <Attribute Type=\"Scalar\" Center=\"Node\" Name=\"%s\">\n", swarmVar->name);
                              Journal_Printf( stream, "            <DataItem Format=\"HDF\" %s Dimensions=\"%u 1\">%s%s.%05d%s.h5:/%s</DataItem>\n", variableType, swarmParticleLocalCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part, swarmVar->name + strlen(currentSwarm->name)+1 );
                              Journal_Printf( stream, "         </Attribute>\n\n" );
                  } else if ( swarmVar->dofCount == 2 ){
                     /** note that for 2d, we feed back a quasi 3d array, with the 3rd Dof zeroed.  so in effect we always work in 3d.
                         this is done because paraview in particular seems to do everything in 3d, and if you try and give it a 2d vector
                         or array, it complains.... and hence the verbosity of the following 2d definitions **/
                              Journal_Printf( stream, "         <Attribute Type=\"Vector\" Center=\"Node\" Name=\"%s\">\n", swarmVar->name);
                              Journal_Printf( stream, "            <DataItem ItemType=\"Function\"  Dimensions=\"%u 3\" Function=\"JOIN($0, $1, 0*$1)\">\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "               <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"XValue\">\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "                  <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 0 1 1 %u 1 </DataItem>\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u 2\">%s%s.%05d%s.h5:/%s</DataItem>\n", variableType, swarmParticleLocalCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part, swarmVar->name + strlen(currentSwarm->name)+1 );
                              Journal_Printf( stream, "               </DataItem>\n" );
                              Journal_Printf( stream, "               <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" Name=\"YValue\">\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "                  <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 1 1 1 %u 1 </DataItem>\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "                  <DataItem Format=\"HDF\" %s Dimensions=\"%u 2\">%s%s.%05d%s.h5:/%s</DataItem>\n", variableType, swarmParticleLocalCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part, swarmVar->name + strlen(currentSwarm->name)+1 );
                              Journal_Printf( stream, "               </DataItem>\n" );
                              Journal_Printf( stream, "            </DataItem>\n" );
                              Journal_Printf( stream, "         </Attribute>\n\n" );
                  } else if ( swarmVar->dofCount == 3 ) {
                     /** in 3d we simply feed back the 3d hdf5 array, nice and easy **/
                              Journal_Printf( stream, "         <Attribute Type=\"Vector\" Center=\"Node\" Name=\"%s\">\n", swarmVar->name);
                              Journal_Printf( stream, "            <DataItem Format=\"HDF\" %s Dimensions=\"%u 3\">%s%s.%05d%s.h5:/%s</DataItem>\n", variableType, swarmParticleLocalCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part, swarmVar->name + strlen(currentSwarm->name)+1 );
                              Journal_Printf( stream, "         </Attribute>\n\n" );
                  } else {
                     /** where there are more than 3 components, we write each one out as a scalar **/
                     for(dofCountIndex = 0 ; dofCountIndex < swarmVar->dofCount ; ++dofCountIndex){
                              Journal_Printf( stream, "         <Attribute Type=\"Scalar\" Center=\"Node\" Name=\"%s-Component-%u\">\n", swarmVar->name, dofCountIndex);
                              Journal_Printf( stream, "            <DataItem ItemType=\"HyperSlab\" Dimensions=\"%u 1\" >\n", swarmParticleLocalCount );
                              Journal_Printf( stream, "               <DataItem Dimensions=\"3 2\" Format=\"XML\"> 0 %u 1 1 %u 1 </DataItem>\n", dofCountIndex, swarmParticleLocalCount );
                              Journal_Printf( stream, "               <DataItem Format=\"HDF\" %s Dimensions=\"%u %u\">%s%s.%05d%s.h5:/%s</DataItem>\n", variableType, swarmParticleLocalCount, swarmVar->dofCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part, swarmVar->name + strlen(currentSwarm->name)+1 );
                              Journal_Printf( stream, "            </DataItem>\n" );
                              Journal_Printf( stream, "         </Attribute>\n" );
                     }
                              Journal_Printf( stream, "\n" );
                  }
            /**----------------------- END ATTRIBUTES   --------------------------------------------------------------------------------------------------------- **/
               }

            }

            /**----------------------- START OWNING ELEMENT ----------------------------------------------------------------------------------------------------- **/
            if( IsChild( ((Stg_Component*)currentSwarm->cellLayout)->type, ElementCellLayout_Type )){
                              Stg_asprintf( &variableType, "NumberType=\"Int\"" );
                              Journal_Printf( stream, "         <Attribute Type=\"Scalar\" Center=\"Node\" Name=\"OwningElement\">\n");
                              Journal_Printf( stream, "            <DataItem Format=\"HDF\" %s Dimensions=\"%u 1\">%s%s.%05d%s.h5:/OwningElement</DataItem>\n", variableType, swarmParticleLocalCount, prefixGuy, currentSwarm->name, context->timeStep, filename_part );
                              Journal_Printf( stream, "         </Attribute>\n\n" );
            }
            /**----------------------- END OWNING ELEMENT ----------------------------------------------------------------------------------------------------------- **/

                              Journal_Printf( stream, "      </Grid>\n\n" );

         }

      }

                              Journal_Printf( stream, "   </Grid>\n\n" );

   if(variableType){
      Memory_Free( variableType );
      variableType=NULL;
   }
   if(filename_part){
      Memory_Free( filename_part );
      filename_part=NULL;
   }
   }

   Memory_Free( prefixGuy );

}

void  _XDMFGenerator_WriteHeader( UnderworldContext* context, Stream* stream ) {

   /** Print XDMF header info **/
                              Journal_Printf( stream, "<?xml version=\"1.0\" ?>\n" );
                              /*Journal_Printf( stream, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n" ); */  /*  currently seems unable to validate document type */
                              Journal_Printf( stream, "<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\">\n" );
                              Journal_Printf( stream, "\n" );
                              Journal_Printf( stream, "<Domain>\n" );
                              Journal_Printf( stream, "\n" );

}


void _XDMFGenerator_WriteFooter( UnderworldContext* context, Stream* stream ) {

                              Journal_Printf( stream, "</Domain>\n" );
                              Journal_Printf( stream, "\n" );
                              Journal_Printf( stream, "</Xdmf>\n" );
                              Journal_Printf( stream, "\n" );

}

void _XDMFGenerator_SendInfo( UnderworldContext* context ) {
   Swarm_Register* swarmRegister = Swarm_Register_GetSwarm_Register();
   Index           swarmCount;
   Index           swarmcountindex;
   Index           swarmParticleLocalCount;
   Swarm*          currentSwarm;
   const int       FINISHED_WRITING_TAG = 100;

   /** get total number of different swarms **/
   swarmCount  = swarmRegister->swarmList->count;

   /** parse swarm list, checking which are actually stored to HDF5.  **/
   /** We assume that all processes have the same number of swarms on them, with the same swarms checkpointed**/
   for(swarmcountindex = 0; swarmcountindex < swarmCount; ++swarmcountindex){
      currentSwarm  = Swarm_Register_At( swarmRegister, swarmcountindex );

      if ( currentSwarm->isSwarmTypeToCheckPointAndReload != True )
         continue;

      swarmParticleLocalCount = currentSwarm->particleLocalCount;
      MPI_Ssend( &swarmParticleLocalCount, 1, MPI_INT, MASTER, FINISHED_WRITING_TAG, context->communicator );
   }
}

void XDMFGenerator_GenerateTemporalTopLevel( void* _context, void* ignored ) {
   UnderworldContext*   context    = (UnderworldContext*)_context;
   Stream*              stream;

   /** only the MASTER process creates and writes these files **/
   if(context->rank == MASTER) {
      Bool                 fileOpened;
      Stream*              errorStream  = Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  );
      char*                filename;
      char*                outputPathString;
      FILE*                file;
      Bool                 fileExists;
      /** Create Stream **/
      stream = Journal_Register( InfoStream_Type, (Name)"XDMFOutputTemporalTopLevelFiles"  );

      /** Set auto flush on stream **/
      Stream_SetAutoFlush( stream, True );

      outputPathString = StG_Strdup(context->checkpointWritePath);//Context_GetCheckPointWritePrefixStringTopLevel( context );

      /** Set filename for temporal file containing both fields and swarms **/
      Stg_asprintf( &filename, "%s/XDMF.temporalAll.xmf", outputPathString );
      /** Init file, always overwriting any existing **/
      fileOpened = Stream_RedirectFile( stream, filename );
      Journal_Firewall( fileOpened, errorStream,
            "Could not open file %s. Possibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );
      Memory_Free( filename );
      _XDMFGenerator_WriteHeader( context, stream );
      Journal_Printf( stream, "   <xi:include href=\"XDMF.FilesField.xdmf\" xpointer=\"xpointer(//Xdmf/Grid)\"/>\n" );
      Journal_Printf( stream, "   <xi:include href=\"XDMF.FilesSwarm.xdmf\" xpointer=\"xpointer(//Xdmf/Grid)\"/>\n\n" );
      _XDMFGenerator_WriteFooter( context, stream );
      Stream_CloseFile( stream );
   
      /** Set filename for temporal file containing only fields **/
      Stg_asprintf( &filename, "%s/XDMF.temporalFields.xmf", outputPathString );
      /** Init file, always overwriting any existing **/
      fileOpened = Stream_RedirectFile( stream, filename );
      Journal_Firewall( fileOpened, errorStream,
            "Could not open file %s. Possibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );
      Memory_Free( filename );
      _XDMFGenerator_WriteHeader( context, stream );
      Journal_Printf( stream, "   <xi:include href=\"XDMF.FilesField.xdmf\" xpointer=\"xpointer(//Xdmf/Grid)\"/>\n\n" );
      _XDMFGenerator_WriteFooter( context, stream );
      Stream_CloseFile( stream );
      
      /** Set filename for temporal file containing only swarms **/
      Stg_asprintf( &filename, "%s/XDMF.temporalSwarms.xmf", outputPathString );
      /** Init file, always overwriting any existing **/
      fileOpened = Stream_RedirectFile( stream, filename );
      Journal_Firewall( fileOpened, errorStream,
            "Could not open file %s. Possibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );
      Memory_Free( filename );
      _XDMFGenerator_WriteHeader( context, stream );
      Journal_Printf( stream, "   <xi:include href=\"XDMF.FilesSwarm.xdmf\" xpointer=\"xpointer(//Xdmf/Grid)\"/>\n\n" );
      _XDMFGenerator_WriteFooter( context, stream );
      Stream_CloseFile( stream );
      
      /** Set filename for file containing list of XDMF files, for fields **/
      Stg_asprintf( &filename, "%s/XDMF.FilesField.xdmf", outputPathString );
      /** If the following files do not exist, or we are not restarting a job, create/recreate **/
      if ( (file = fopen(filename,"r") ) == NULL ){
         fileExists = False;
      } else {
         fileExists = True;
         fclose(file);
      }
      if ( !fileExists || !(context->loadFromCheckPoint)  ){
         /** if does not exists, then we create blank version **/
         fileOpened = Stream_RedirectFile( stream, filename );
         Memory_Free( filename );
         /** Print XDMF header info **/
         Journal_Printf( stream, "<?xml version=\"1.0\" ?>\n" );
         /* Journal_Printf( stream, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n" ); */  /*  currently seems unable to validate document type */
         Journal_Printf( stream, "<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\">\n" );
         Journal_Printf( stream, "<Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"FEM_Mesh_Fields\">\n" );
         Journal_Printf( stream, "</Grid>\n" );
         Journal_Printf( stream, "</Xdmf>\n" );
         Stream_CloseFile( stream );
      } else
         Memory_Free( filename );

      /** Set filename for file containing list of XDMF files, for fields **/
      Stg_asprintf( &filename, "%s/XDMF.FilesSwarm.xdmf", outputPathString );
      /** If the following files do not exist, or we are not restarting a job, create/recreate **/
      if ( (file = fopen(filename,"r") ) == NULL ){
         fileExists = False;
      } else {
         fileExists = True;
         fclose(file);
      }
      if ( !fileExists || !(context->loadFromCheckPoint)  ){
         /** if does not exists, then we create blank version **/
         fileOpened = Stream_RedirectFile( stream, filename );
         Memory_Free( filename );
         /** Print XDMF header info **/
         Journal_Printf( stream, "<?xml version=\"1.0\" ?>\n" );
         /* Journal_Printf( stream, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n" ); */  /*  currently seems unable to validate document type */
         Journal_Printf( stream, "<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\">\n" );
         Journal_Printf( stream, "<Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"FEM_Swarms\">\n" );
         Journal_Printf( stream, "</Grid>\n" );
         Journal_Printf( stream, "</Xdmf>\n" );
         Stream_CloseFile( stream );
      } else
         Memory_Free( filename );
      /** close the file **/
      Memory_Free( outputPathString );

   } 

}


void _XDMFGenerator_UpdateTemporalFileLists( UnderworldContext* context ){

   if(context->rank == MASTER) {
      FILE*                fileOpened;
      Stream*              errorStream  = Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  );
      char*                filename;
      char*                outputPathString;
      char*                prefixGuy;
      if (strcmp("",context->checkPointPrefixString))
         Stg_asprintf( &prefixGuy, "%s." , context->checkPointPrefixString );
      else
         Stg_asprintf( &prefixGuy, "" );

      Swarm_Register* swarmRegister = Swarm_Register_GetSwarm_Register();
      Swarm* currentSwarm=NULL;
      Index swarmcountindex, recordCount, swarmCount=swarmRegister->swarmList->count;

      outputPathString = StG_Strdup(context->checkpointWritePath);// Context_GetCheckPointWritePrefixStringTopLevel( context );

      /** Get name of XDMF Field file list file **/
      Stg_asprintf( &filename, "%s/XDMF.FilesField.xdmf", outputPathString );

      /** check if file exists.. file should always exist at this point **/
      Journal_Firewall( Stg_FileExists( filename ), errorStream,
            "Could not find file %s. This file should have been created already.\nPossibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );
      
      /** Init file, always overwriting any existing **/
      fileOpened = fopen( filename, "r+");
      Journal_Firewall( (fileOpened!=NULL), errorStream,
            "Could not open file %s. Possibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );
      Memory_Free( filename );
      
      /** rewind to before file footer */
      fseek( fileOpened, -16, SEEK_END);
      
      /** write current filename */
      fprintf( fileOpened, "    <xi:include href=\"%sXDMF.%05d.xmf\" xpointer=\"xpointer(//Xdmf/Domain/Grid[1])\"/>\n", prefixGuy, context->timeStep );

      /** now rewrite footer */
      fprintf( fileOpened, "</Grid>\n" );
      fprintf( fileOpened, "</Xdmf>\n" );

      /** close file */
      fclose( fileOpened );

      
      /** Get name of XDMF Swarm file list file **/
      Stg_asprintf( &filename, "%s/XDMF.FilesSwarm.xdmf", outputPathString );

      /** check if file exists.. file should always exist at this point **/
      Journal_Firewall( Stg_FileExists( filename ), errorStream,
            "Could not find file %s. This file should have been created already.\nPossibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );
      
      /** Init file, always overwriting any existing **/
      fileOpened = fopen( filename, "r+");
      Journal_Firewall( (fileOpened!=NULL), errorStream,
            "Could not open file %s. Possibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );
      Memory_Free( filename );
      
      /** rewind to before file footer */
      fseek( fileOpened, -16, SEEK_END);

      /** for each swarm we are checkpointing we create an entry starting from Grid[2]: BIG ASSUMPTION */
      recordCount=0; // var to remember offset from 2
      for(swarmcountindex = 0; swarmcountindex < swarmCount; ++swarmcountindex){
         currentSwarm  = Swarm_Register_At( swarmRegister, swarmcountindex );

         if ( currentSwarm->isSwarmTypeToCheckPointAndReload == True ) {
            /* due to bug in paraview, can't viz a unstructured grid with 1 point */ 
            if( currentSwarm->particleLocalCount > 1 ) {
               fprintf( fileOpened, "    <xi:include href=\"%sXDMF.%05d.xmf\" xpointer=\"xpointer(//Xdmf/Domain/Grid[%d])\"/>\n", prefixGuy, context->timeStep, 2+recordCount );
            }
            recordCount++;
         }
      }

      /** now rewrite footer */
      fprintf( fileOpened, "</Grid>\n" );
      fprintf( fileOpened, "</Xdmf>\n" );

      /** close file */
      fclose( fileOpened );
      
      Memory_Free( outputPathString );
      Memory_Free( prefixGuy );
      
   }

}


void _XDMFGenerator_FieldsOrSwarmsOnlyFiles( UnderworldContext* context ){

   if(context->rank == MASTER) {
      Stream*              stream;
      Bool                 fileOpened;
      Stream*              errorStream  = Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  );
      char*                 filename;
      char*                outputPathString;

      Swarm_Register* swarmRegister = Swarm_Register_GetSwarm_Register();
      Swarm* currentSwarm=NULL;
      Index swarmcountindex, recordCount, swarmCount=swarmRegister->swarmList->count;

      /** Create Stream **/
      stream = Journal_Register( InfoStream_Type, (Name)"XDMFOutputFile_Field"  );

      /** Set auto flush on stream **/
      Stream_SetAutoFlush( stream, True );

      outputPathString = Context_GetCheckPointWritePrefixString( context );

      /** Get name of XDMF schema file **/
      Stg_asprintf( &filename, "%sXDMF.Fields.%05d.xmf", outputPathString, context->timeStep );

      /** Init file, always overwriting any existing **/
      fileOpened = Stream_RedirectFile( stream, filename );
      Journal_Firewall( fileOpened, errorStream,
            "Could not open file %s. Possibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );

      _XDMFGenerator_WriteHeader( context, stream );
      Journal_Printf( stream, "  <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"FEM_Mesh_Fields\">\n" );
      Journal_Printf( stream, "     <xi:include href=\"XDMF.%05d.xmf\" xpointer=\"xpointer(//Xdmf/Domain/Grid[1])\"/>\n", context->timeStep );
      Journal_Printf( stream, "  </Grid>\n\n" );
      _XDMFGenerator_WriteFooter( context, stream );
      Stream_CloseFile( stream );

      /** close the file **/
      Stream_CloseFile( stream );
      Memory_Free( filename );
      
      /** now for swarm **/
      /** Create Stream **/
      stream = Journal_Register( InfoStream_Type, (Name)"XDMFOutputFile_Swarm"  );

      /** Set auto flush on stream **/
      Stream_SetAutoFlush( stream, True );

      outputPathString = Context_GetCheckPointWritePrefixString( context );

      /** Get name of XDMF schema file **/
      Stg_asprintf( &filename, "%sXDMF.Swarms.%05d.xmf", outputPathString, context->timeStep );

      /** Init file, always overwriting any existing **/
      fileOpened = Stream_RedirectFile( stream, filename );
      Journal_Firewall( fileOpened, errorStream,
            "Could not open file %s. Possibly directory %s does not exist or is not writable.\n"
            "Check 'checkpointWritePath' in input file.", filename, outputPathString );

      _XDMFGenerator_WriteHeader( context, stream );
      Journal_Printf( stream, "  <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"Swarms\">\n" );

      /** for each swarm we are checkpointing we create an entry starting from Grid[2]: BIG ASSUMPTION */
      recordCount=0; // var to remember offset from 2
      for(swarmcountindex = 0; swarmcountindex < swarmCount; ++swarmcountindex){
         currentSwarm  = Swarm_Register_At( swarmRegister, swarmcountindex );

         if ( currentSwarm->isSwarmTypeToCheckPointAndReload == True ) {
            /* due to bug in paraview, can't viz a unstructured grid with 1 point */ 
            if( currentSwarm->particleLocalCount > 1 ) {
               Journal_Printf( stream, "     <xi:include href=\"XDMF.%05d.xmf\" xpointer=\"xpointer(//Xdmf/Domain/Grid[%d])\"/>\n", context->timeStep, 2+recordCount );
            }

            recordCount++;
         }
      }

      Journal_Printf( stream, "  </Grid>\n\n" );
      _XDMFGenerator_WriteFooter( context, stream );
      Stream_CloseFile( stream );

      /** close the file **/
      Stream_CloseFile( stream );
      Memory_Free( filename );
      
   }
}



