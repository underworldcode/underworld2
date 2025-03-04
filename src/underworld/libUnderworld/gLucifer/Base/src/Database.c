/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <stdio.h>
#include <float.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>

#include "Base.h"

#include "types.h"

/* Process: fork/exec/wait */
#include <unistd.h>
#include <sys/wait.h>

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

#include <gLucifer/DrawingObjects/src/DrawingObjects.h>

#define MAX_QUERY_LEN 4096
char SQL[MAX_QUERY_LEN];

const Type lucDatabase_Type = "lucDatabase";

lucDatabase* _lucDatabase_New(  LUCDATABASE_DEFARGS  )
{
   lucDatabase*    self;

   /* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
   assert( _sizeOfSelf >= sizeof(lucDatabase) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (lucDatabase*) _Stg_Component_New(  STG_COMPONENT_PASSARGS  );
   
   self->filename  = NULL;
   self->db        = NULL;
   self->db2       = NULL;
   self->memdb     = NULL;
   self->vfs       = NULL;
   self->timeStep  = -1;

   return self;
}

void* _lucDatabase_DefaultNew( Name name )
{
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof( lucDatabase );
   Type                                                      type = lucDatabase_Type;
   Stg_Class_DeleteFunction*                              _delete = _lucDatabase_Delete;
   Stg_Class_PrintFunction*                                _print = NULL;
   Stg_Class_CopyFunction*                                  _copy = NULL;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _lucDatabase_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _lucDatabase_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _lucDatabase_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _lucDatabase_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _lucDatabase_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _lucDatabase_Destroy;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return _lucDatabase_New(  LUCDATABASE_PASSARGS  );
}

void _lucDatabase_Init( 
   lucDatabase*         self,
   AbstractContext*     context,
   lucDrawingObject**   drawingObjectList,
   DrawingObject_Index  drawingObjectCount,
   int                  deleteAfter,
   Bool                 splitTransactions,
   Bool                 compressed,
   Bool                 singleFile,
   char*                filename,
   char*                vfs,
   Bool                 viewonly )
{
   DrawingObject_Index object_I;

   self->isConstructed = True;
   self->context = (DomainContext*)context;
   self->deleteAfter = deleteAfter;
   self->splitTransactions = splitTransactions;

   self->compressed = compressed;
   self->singleFile = singleFile;
   self->filename = StG_Strdup(filename);
   
   if (vfs && strlen(vfs)) self->vfs = StG_Strdup(vfs);
   self->viewonly = viewonly;
   self->drawingObjects = NamedObject_Register_New();

   for ( object_I = 0 ; object_I < drawingObjectCount ; object_I++ )
      NamedObject_Register_Add( self->drawingObjects, drawingObjectList[ object_I ] );

   /* Set minZ = maxZ for 2d models */

   /* Create geometry data stores */
   lucGeometryType type;
   lucGeometryDataType data_type;
   for (type=lucMinType; type<lucMaxType; type++)
      for (data_type=lucMinDataType; data_type<lucMaxDataType; data_type++)
         self->data[type][data_type] = lucGeometryData_New(data_type);
   for (type=lucMinType; type<lucMaxType; type++)
   {
      self->labels[type] = NULL;
      self->label_lengths[type] = 0;
   }

   /* Set bin_path */
   char binpath[MAX_PATH];
   int pos;
   /* Strip lib from libpath */
   strncpy(binpath, LIB_DIR, MAX_PATH-1);
   pos = strlen(binpath)-4;
   if (pos > 0) 
   {
      binpath[pos] = 0;
      sprintf(self->bin_path, "%s/lavavu", binpath);
   }
   
   if(self->context){
      self->rank         = self->context->rank;
      self->nproc        = self->context->nproc;
      self->communicator = self->context->communicator;
   } else {
      self->communicator = MPI_COMM_WORLD;
      MPI_Comm_rank( self->communicator, &self->rank );
      MPI_Comm_size( self->communicator, &self->nproc );
   }

}

lucDatabase* lucDatabase_New(
   AbstractContext*  context,
   int               deleteAfter,
   Bool              splitTransactions,
   Bool              compressed,
   Bool              singleFile,
   char*             filename,
   char*             vfs)
{
   lucDatabase* self = (lucDatabase*)_lucDatabase_DefaultNew("database");
   _lucDatabase_Init(self, context, NULL, 0, deleteAfter, splitTransactions, compressed, singleFile, filename, vfs, False);
   return self;
}

void _lucDatabase_Delete( void* database )
{
   lucDatabase* self = (lucDatabase*)database;

   /* Delete geometry data stores */
   lucGeometryType type;
   lucGeometryDataType data_type;
   for (type=lucMinType; type<lucMaxType; type++)
      for (data_type=lucMinDataType; data_type<lucMaxDataType; data_type++)
         if (self->data[type][data_type]) lucGeometryData_Delete(self->data[type][data_type]);
   for (type=lucMinType; type<lucMaxType; type++)
      if (self->labels[type]) Memory_Free(self->labels[type]);

   if (self->db) sqlite3_close(self->db);
   if (self->db2) sqlite3_close(self->db2);
   if (self->memdb) sqlite3_close(self->memdb);

   if (self->filename) Memory_Free(self->filename);
   if (self->vfs) Memory_Free(self->vfs);
   

   _Stg_Component_Delete( self );
}

void _lucDatabase_AssignFromXML( void* database, Stg_ComponentFactory* cf, void* data )
{
   lucDatabase* self = (lucDatabase*) database ;
   AbstractContext* context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
   DrawingObject_Index drawingObjectCount;
   lucDrawingObject**  drawingObjectList;

   /* Optional global drawing object list, can provide to database instead of windows+viewports */
   drawingObjectList = Stg_ComponentFactory_ConstructByList( cf, self->name, (Dictionary_Entry_Key)"DrawingObject", Stg_ComponentFactory_Unlimited, lucDrawingObject, False, &drawingObjectCount, data);

   _lucDatabase_Init( self, context, 
      drawingObjectList,
      drawingObjectCount,
      Stg_ComponentFactory_GetInt( cf, self->name, (Dictionary_Entry_Key)"deleteAfter", 0),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"splitTransactions", True),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"compressed", True),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"singleFile", True),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"filename", NULL),
      Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"vfs", NULL),
      Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"viewonly", False  )
      );
}

void _lucDatabase_Build( void* database, void* data ) {}

void _lucDatabase_Initialise( void* database, void* data ) {}

void _lucDatabase_Execute( void* database, void* data ) 
{
   /* This begins the transaction, if drawing objects assigned
    * to the database, they are also output here, otherwise
    * they will be output by their parent window:
    * Window_Execute(Display) -> Viewport->Draw */
   lucDatabase* self = (lucDatabase*)database;
   float currentTime = 0.0;
   if (self->context)
   {
      self->timeStep = self->context->timeStep;
      currentTime    = self->context->currentTime;
   }
   
   if (self->context && !self->context->vis) return;

   NamedObject_Register* dr = self->drawingObjects;
   Index objectCount = dr->objects->count;
   Index object_I;
   lucDrawingObject* object;

   if (self->rank == 0)
   {
      if (!self->db)
      {
         /* Open and create database */
         lucDatabase_OpenDatabase(self);
      }

      /* Output own object settings */
      for ( object_I = 0 ; object_I < objectCount ; object_I++ )
      {
         object = (lucDrawingObject*)NamedObject_Register_GetByIndex( dr, object_I );
         lucDatabase_OutputDrawingObject(self, object);
      }
   }

   /* Call setup on drawing objects (if any) !This must be called on all procs! */
   for ( object_I = 0 ; object_I < objectCount ; object_I++ )
   {
      object = (lucDrawingObject*)NamedObject_Register_GetByIndex( dr, object_I );
      /* Ensure setup has been called to prepare for rendering (used to set position information) */
      if ( !object->disabled)
      {
         object->_setup( object, self, self->context );
         object->needsToCleanUp = True;
         object->needsToDraw = True;
      }
   }

   if (self->rank == 0)
   {
      /* Multi-file database setup */
      if (!self->singleFile)
      {
         if (!self->memdb)
         {
            /* Copy structure to template file */
            sqlite3_open(":memory:", &self->memdb);
            lucDatabase_BackupDb(self->db, self->memdb);
            /* Remove any existing data in the memory copy (can only be one timestep) */
            lucDatabase_IssueSQL(self->memdb, "delete from geometry");
            lucDatabase_IssueSQL(self->memdb, "delete from timestep");
         }
 
         if (self->timeStep > 0)
         {
            /* Attach db for new timestep */
            lucDatabase_AttachDatabase(self);
            /* Copy base table structure */
            lucDatabase_BackupDb(self->memdb, self->db2);
         }
      }

      /* Do each timestep database output in a single transaction 
       * (defaults to on, fastest option but possibly will require more memory) */
      if (!self->splitTransactions)
         Journal_Firewall(lucDatabase_BeginTransaction(self), lucError, "Begin transaction failed! %s '%s'.\n", self->type, self->name );

      /* If a data window is set, delete expired geometry */
      if (self->deleteAfter > 0 )
      {
         int deleteEnd = self->timeStep - self->deleteAfter - 1;
         if (deleteEnd >= 0)
            lucDatabase_DeleteGeometry(self, -1, deleteEnd);
      }

      /* Remove any geometry at current timestep before insertion */
      lucDatabase_DeleteGeometry(self, self->timeStep, self->timeStep);

      /* Enter timestep in database */
      /* Write and update timestep */
      snprintf(SQL, MAX_QUERY_LEN, "insert into timestep (id, time, properties) values (%d, %g, '%s')", self->timeStep, currentTime, "");
      /*printf("%s\n", SQL);*/
      if (!lucDatabase_IssueSQL(self->db, SQL)) return;
      /* Also write to attached db */
      if (!self->singleFile && self->timeStep > 0)
        if (!lucDatabase_IssueSQL(self->db2, SQL)) return;
   }

   /* If we have global list of drawing objects to output, process them here */
   for ( object_I = 0 ; object_I < objectCount ; object_I++ )
   {
      object = (lucDrawingObject*)NamedObject_Register_GetByIndex( dr, object_I );
      lucDrawingObject_Draw( object, self, self->context );
      lucDatabase_OutputGeometry(self, object->id);

      if (object->needsToCleanUp )
         object->_cleanUp( object );
   }
}

void _lucDatabase_Destroy( void* database, void* data ) { }

void lucDatabase_Dump(void* database)
{
   lucDatabase* self = (lucDatabase*)database;

   /* Commit the full timestep transaction */
   if (!self->splitTransactions)
   {
      double wtime = MPI_Wtime();
      lucDatabase_Commit(self);
      wtime = MPI_Wtime() - wtime;
      Journal_Printf(lucDebug, "    Transaction took %f seconds\n", wtime);
   }
}

void lucDatabase_OutputDrawingObject(lucDatabase* self, lucDrawingObject* object)
{
   /* Save the object */
   if (!object->id) /* Not already written */
   {
      snprintf(SQL, MAX_QUERY_LEN, "insert into object (name, properties) values ('%s_%s', ?)", object->type, object->name); 

     /* Prepare statement... */
     sqlite3_stmt* statement;
     if (sqlite3_prepare_v2(self->db, SQL, -1, &statement, NULL) != SQLITE_OK)
        Journal_Printf(lucError, "SQL prepare error: (%s) %s\n", SQL, sqlite3_errmsg(self->db));
     /* Setup text data for insert */
     else if (sqlite3_bind_text(statement, 1, object->properties, strlen(object->properties), SQLITE_STATIC) != SQLITE_OK)
        Journal_Printf(lucError, "SQL bind error: %s\n", sqlite3_errmsg(self->db));
     /* Execute statement */
     else if (sqlite3_step(statement) != SQLITE_DONE )
        Journal_Printf(lucError, "SQL step error: (%s) %s\n", SQL, sqlite3_errmsg(self->db));
     else
       sqlite3_finalize(statement);

      /* Save object id */
      object->id = sqlite3_last_insert_rowid(self->db);
      Journal_Printf(lucDebug, "      Drawing object: %s %s id %d\n", object->name, object->type, object->id);

      /* Save colour maps */
      if (object->colourMap)
         lucDatabase_OutputColourMap(self, object->colourMap, object, lucColourValueData);

      /* Would probably be nice to extend opacity & RGB maps to base object, for now check type first */
      if (object->type == lucSwarmViewer_Type || object->type == lucSwarmVectors_Type || 
          object->type == lucSwarmShapes_Type || object->type == lucSwarmRGBColourViewer_Type)
      {
         /* Only RGB viewer supports these for now */
         if (object->type == lucSwarmRGBColourViewer_Type)
         {
            lucSwarmRGBColourViewer* srgbobj = (lucSwarmRGBColourViewer*)object;
            if (srgbobj->colourRedVariable)
              lucDatabase_OutputColourMap(self, srgbobj->redColourMap, object, lucRedValueData);
            if (srgbobj->colourGreenVariable)
               lucDatabase_OutputColourMap(self, srgbobj->greenColourMap, object, lucGreenValueData);
            if (srgbobj->colourBlueVariable)
               lucDatabase_OutputColourMap(self, srgbobj->blueColourMap, object, lucBlueValueData);
         }
      }
   }
}

void lucDatabase_OutputColourMap(lucDatabase* self, lucColourMap* colourMap, lucDrawingObject* object, lucGeometryDataType type)
{
   /* Save colourMap */
   if (!colourMap->id) /* Not already written */
   {
      snprintf(SQL, MAX_QUERY_LEN, "insert into colourmap (name, properties) values ('%s', ?)", colourMap->name);
      /*printf("%s\n", SQL);*/
      //if (!lucDatabase_IssueSQL(self->db, SQL)) return;

      /* Prepare statement... */
      sqlite3_stmt* statement;
      if (sqlite3_prepare_v2(self->db, SQL, -1, &statement, NULL) != SQLITE_OK)
         Journal_Printf(lucError, "SQL prepare error: (%s) %s\n", SQL, sqlite3_errmsg(self->db));
      /* Setup text data for insert */
      else if (sqlite3_bind_text(statement, 1, colourMap->properties, strlen(colourMap->properties), SQLITE_STATIC) != SQLITE_OK)
         Journal_Printf(lucError, "SQL bind error: %s\n", sqlite3_errmsg(self->db));
      /* Execute statement */
      else if (sqlite3_step(statement) != SQLITE_DONE )
         Journal_Printf(lucError, "SQL step error: (%s) %s\n", SQL, sqlite3_errmsg(self->db));
      else
         sqlite3_finalize(statement);

      /* Save id */
      colourMap->id = sqlite3_last_insert_rowid(self->db);

      Journal_Printf(lucDebug, "         ColourMap: %s, id %d\n", colourMap->name, colourMap->id);
   }

   /* Add reference for object */
   Journal_Printf(lucDebug, "         Linking colourMap: %s to object %s\n", colourMap->name, object->name);
   /* Link object & colour map */
   snprintf(SQL, MAX_QUERY_LEN, "insert into object_colourmap (object_id, colourmap_id, data_type) values (%d, %d, %d)", object->id, colourMap->id, type); 
   /*printf("%s\n", SQL);*/
   if (!lucDatabase_IssueSQL(self->db, SQL)) return;
}

void lucDatabase_ClearGeometry(lucDatabase* self)
{
   /* Clear existing render objects */
   lucGeometryType type;
   lucGeometryDataType data_type;
   for (type=lucMinType; type<lucMaxType; type++)
      for (data_type=lucMinDataType; data_type<lucMaxDataType; data_type++)
         lucGeometryData_Clear(self->data[type][data_type]);
   for (type=lucMinType; type<lucMaxType; type++)
      if (self->labels[type]) self->labels[type][0] = 0;
}

void lucDatabase_OutputGeometry(lucDatabase* self, int object_id)
{
   lucGeometryType type;
   lucGeometryDataType data_type;
   unsigned int procs = self->nproc;
   unsigned int bytes = 0, outbytes = 0;
   double time, gtotal = 0, wtotal = 0;
   //Journal_Printf(lucDebug, "gLucifer: writing geometry to database ...\n");

   /* Write geometry to database */
   time = MPI_Wtime();
   if (self->rank > 0 || !self->splitTransactions || lucDatabase_BeginTransaction(self))
   {
      for (type=lucMinType; type<lucMaxType; type++)
      {
         //printf("%d Writing geometry %d procs, type %d\n", self->context->rank, procs, type);
         time = MPI_Wtime();
         if (procs > 1)
         {
            for (data_type=lucMinDataType; data_type<lucMaxDataType; data_type++)
               lucDatabase_GatherGeometry(self, type, data_type);
            lucDatabase_GatherLabels(self, type);
         }
         if (self->rank > 0) continue;
         gtotal += MPI_Wtime() - time;

         /* Loop through all geometry types and write to database */
         for (data_type=lucMinDataType; data_type<lucMaxDataType; data_type++)
         {
            lucGeometryData* data = self->data[type][data_type];
            bytes += data->count * sizeof(float);
            if (self->labels[type]) data->labels = self->labels[type];
            outbytes += lucDatabase_WriteGeometry(self, 0, type, data_type, object_id, data);
         }
      }

      /* Commit transaction */
      if (self->rank == 0 && self->splitTransactions)
      {
         time = MPI_Wtime();
         lucDatabase_Commit(self);
         wtotal += MPI_Wtime() - time;
      }
   }

   if (bytes > 0)
   {
      if (self->nproc > 1)
         Journal_Printf(lucDebug, "    Gather data, took %f sec\n", gtotal);
      if (self->rank == 0 && self->splitTransactions)
         Journal_Printf(lucDebug, "    Transaction took %f seconds\n", wtotal);
      Journal_Printf(lucInfo, "    %.3f kb of geometry data saved, %.3f kb written.\n", bytes/1000.0f, outbytes/1000.0f);
   }

   /* Clear all objects */
   lucDatabase_ClearGeometry(self);
}

/*** 
 * Utility function to prepare for gathering variable length data from all procs to root
 *  count:   local count
 *  counts:  per proc counts return array
 *  offsets: per proc offsets return array
 *  returns total elements to be received on root
 ***/
int lucDatabase_GatherCounts(lucDatabase* self, int count, int* counts, int* offsets)
{
   /* Get the count on each proc */
   int p, total = 0;
   (void)MPI_Gather(&count, 1, MPI_INT, counts, 1, MPI_INT, 0, self->communicator);

   /* Now we have count per proc, calculate offsets and total */
   if (self->rank == 0)
   {
      for (p=0; p<self->nproc; p++) /* Get offset */
      {
         offsets[p] = p==0 ? 0 : offsets[p-1] + counts[p-1];
      }
      total = offsets[self->nproc-1] + counts[self->nproc-1];
      /* All values already on root? no gather required */
      //if (total == count) total = 0;
   }

   /* Return total elements to be received */
   MPI_Bcast(&total, 1, MPI_INT, 0, self->communicator);
   return total;
}

void lucDatabase_GatherGeometry(lucDatabase* self, lucGeometryType type, lucGeometryDataType data_type)
{
   lucGeometryData* block = self->data[type][data_type];
   /* Get the count on each proc */
   int *counts = NULL, *offsets = NULL;
   int p, total = 0;
   float *data = NULL;
   if (self->rank == 0)
   {
      counts = Memory_Alloc_Array(int, self->nproc, "counts");
      offsets = Memory_Alloc_Array(int, self->nproc, "offsets");
   }

   total = lucDatabase_GatherCounts(self, block->count, counts, offsets);

   if (total > 0)
   {
      if (self->rank == 0)
         data = Memory_Alloc_Array(float, total, "FloatData");

      (void)MPI_Gatherv(block->data, block->count, MPI_FLOAT, data, counts, offsets, MPI_FLOAT, 0, self->communicator);

      /* Reduce to get minimum & maximum from all procs */
      float min, max;
      float bmin[3], bmax[3];
      int width = 0;
      MPI_Reduce( &block->minimum, &min, 1, MPI_FLOAT, MPI_MIN, 0, self->communicator );
      MPI_Reduce( &block->maximum, &max, 1, MPI_FLOAT, MPI_MAX, 0, self->communicator );
      MPI_Reduce( block->min, &bmin, 3, MPI_FLOAT, MPI_MIN, 0, self->communicator );
      MPI_Reduce( block->max, &bmax, 3, MPI_FLOAT, MPI_MAX, 0, self->communicator );
      memcpy(block->min, bmin, sizeof(float) * 3);
      memcpy(block->max, bmax, sizeof(float) * 3);
      if (data_type == lucVertexData)
         MPI_Reduce( &block->width, &width, 1, MPI_INT, MPI_SUM, 0, self->communicator );
      if (self->rank == 0)
      {
         //Journal_Printf(lucDebug, "Gathered %d values, took %f sec\n", total, MPI_Wtime() - time);
         /* Add new data on master */
         for (p=1; p<self->nproc; p++)
         {
            if (counts[p] == 0) continue;

            //printf("[%d - %d] Reading %d values at displacement %d from proc %d MIN %f MAX %f WIDTH %d\n", type, data_type, counts[p], offsets[p], p, min, max, width);
            if (data_type == lucVertexData)
               block->width = width;   //Summed width
            lucGeometryData_Read(block, counts[p] / block->size, &data[offsets[p]]);
         }
         //printf("count %d, \n", block->count);
         lucGeometryData_Setup(block, min, max);
      }
   }

   if (data) Memory_Free(data);
   Memory_Free(counts);
   Memory_Free(offsets);
}

void lucDatabase_GatherLabels(lucDatabase* self, lucGeometryType type)
{
   /* Get the count on each proc */
   int p;
   int *counts = NULL, *offsets = NULL;
   int total = 0;
   char *data = NULL;
   if (self->rank == 0)
   {
      counts = Memory_Alloc_Array(int, self->nproc, "counts");
      offsets = Memory_Alloc_Array(int, self->nproc, "offsets");
   }
   int length = self->labels[type] ? strlen(self->labels[type])+1 : 0;
   total = lucDatabase_GatherCounts(self, length, counts, offsets);

   /* Gather labels */
   if (total > 0)
   {
      if (self->rank == 0)
         data = Memory_Alloc_Array(char, total, "LabelData");

      (void)MPI_Gatherv(self->labels[type], length, MPI_CHAR, data, counts, offsets, MPI_CHAR, 0, self->communicator);
      if (self->rank == 0)
      {
         /* Add new data on master */
         for (p=1; p<self->nproc; p++)/* Get displacements */
         {
            if (counts[p] == 0) continue;
            lucDatabase_AddLabel(self, type, &data[offsets[p]]);
         }
      }
   }

   if (data) Memory_Free(data);
   Memory_Free(counts);
   Memory_Free(offsets);
}

/* Direct write functions to enter data into geom data store */
void lucDatabase_AddGridVertices(lucDatabase* self, int n, int width, float* data)
{
   int height = n / width;
   self->data[lucGridType][lucVertexData]->width = width;
   self->data[lucGridType][lucVertexData]->height = height;
   lucDatabase_AddVertices(self, n, lucGridType, data);
}

void lucDatabase_AddGridVertex(lucDatabase* self, int width, int height, float* data)
{
   /* Adds a single grid vertex, setting width and height */
   self->data[lucGridType][lucVertexData]->width = width;
   self->data[lucGridType][lucVertexData]->height = height;
   lucDatabase_AddVertices(self, 1, lucGridType, data);
}

void lucDatabase_AddVertices(lucDatabase* self, int n, lucGeometryType type, float* data)
{
   //Detects bounding box by checking each vertex x,y,z
   float* min = self->data[type][lucVertexData]->min;
   float* max = self->data[type][lucVertexData]->max;
   int p;
   for (p=0; p < n*3; p++)
   {
      int d = p % 3;
      if (data[p] > max[d]) max[d] = data[p];
      if (data[p] < min[d]) min[d] = data[p];
   }

   lucGeometryData_Read(self->data[type][lucVertexData], n, data);
}

void lucDatabase_AddVerticesWidth(lucDatabase* self, int n, lucGeometryType type, int width, float* data)
{
   self->data[type][lucVertexData]->width = width;
   lucDatabase_AddVertices(self, n, type, data);
}

void lucDatabase_AddNormals(lucDatabase* self, int n, lucGeometryType type, float* data)
{
   lucGeometryData_Read(self->data[type][lucNormalData], n, data);
}

void lucDatabase_AddNormal(lucDatabase* self, lucGeometryType type, XYZ norm)
{
   float normal[3] = {norm[0], norm[1], norm[2]};
   lucGeometryData_Read(self->data[type][lucNormalData], 1, normal);
}

void lucDatabase_AddVectors(lucDatabase* self, int n, lucGeometryType type, float min, float max, float* data)
{
   lucGeometryData_Read(self->data[type][lucVectorData], n, data);
   lucGeometryData_Setup(self->data[type][lucVectorData], min, max);
}

void lucDatabase_AddValues(lucDatabase* self, int n, lucGeometryType type, lucGeometryDataType data_type, lucColourMap* colourMap, float* data)
{
   /* Used for colour or colour component mapping to values */
   lucGeometryData_Read(self->data[type][data_type], n, data);

   if (!colourMap) return;

   /* Set colour map parameters */
   lucGeometryData_Setup(self->data[type][data_type], colourMap->minimum, colourMap->maximum);
}

void lucDatabase_AddVolumeSlice(lucDatabase* self, int width, int height, float* corners, lucColourMap* colourMap, float* data)
{
   /* Output corner vertices */
   lucDatabase_AddVertices(self, 2, lucVolumeType, corners);
   self->data[lucVolumeType][lucVertexData]->width = width;
   self->data[lucVolumeType][lucVertexData]->height = height;
   lucDatabase_AddValues(self, width*height, lucVolumeType, lucColourValueData, colourMap, data);
}

void lucDatabase_AddIndex(lucDatabase* self, lucGeometryType type, unsigned int index)
{
   /* Convert a uint to a float and write value */
   union
   {
      float val;
      unsigned int idx;
   } fidx;
   fidx.idx = index;
   lucGeometryData_Read(self->data[type][lucIndexData], 1, &fidx.val);
}

void lucDatabase_AddRGBA(lucDatabase* self, lucGeometryType type, float opacity, lucColour* colour)
{
   /* Convert a colour to a float and write value */
   union
   {
      float val;
      char rgba[4];
   } fcolour;

   if (opacity >= 0) 
      fcolour.rgba[3] = 255 * opacity;
   else
      fcolour.rgba[3] = 255 * colour->opacity;
   fcolour.rgba[0] = 255 * colour->red;
   fcolour.rgba[1] = 255 * colour->green;
   fcolour.rgba[2] = 255 * colour->blue;

   lucGeometryData_Read(self->data[type][lucRGBAData], 1, &fcolour.val);
}

void lucDatabase_AddTexCoord(lucDatabase* self, lucGeometryType type, float x, float y)
{
   float coord[3] = {x, y};
   lucGeometryData_Read(self->data[type][lucTexCoordData], 1, coord);
}

void lucDatabase_AddLabel(lucDatabase* self, lucGeometryType type, char* label)
{
   if (!self->labels[type] || strlen(self->labels[type]) + strlen(label) + 2 > self->label_lengths[type])
   {
      /* No label memory allocated yet or more required */
      self->labels[type] = Memory_Realloc_Array(self->labels[type], char, self->label_lengths[type] + MAX_QUERY_LEN);
      self->label_lengths[type] += MAX_QUERY_LEN;
      if (self->label_lengths[type] == MAX_QUERY_LEN) self->labels[type][0] = 0;
   }
   if (self->labels[type][0] != 0) strcat(self->labels[type], "\n");
   strcat(self->labels[type], label);
}

lucGeometryData* lucGeometryData_New(lucGeometryDataType data_type)
{
   lucGeometryData* self = Memory_Alloc(lucGeometryData, "Geometry data block");
   self->data = NULL;
   self->labels = NULL;
   self->allocated = 0;
   self->size = 1;

   if (data_type <= lucVectorData)
      self->size = 3;
   if (data_type == lucTexCoordData)
      self->size = 2;

   lucGeometryData_Clear(self);

   return self;
}

void lucGeometryData_Clear(lucGeometryData* self)
{
   self->count = 0;

   self->width = 0;
   self->height = 0;

   self->minimum = HUGE_VAL;
   self->maximum = -HUGE_VAL;

   self->min[I_AXIS] = self->min[J_AXIS] = self->min[K_AXIS] = HUGE_VAL;
   self->max[I_AXIS] = self->max[J_AXIS] = self->max[K_AXIS] = -HUGE_VAL;
}

void lucGeometryData_Delete(lucGeometryData* self)
{
   if (self->data) Memory_Free(self->data);
   Memory_Free(self);
}

void lucGeometryData_Read(lucGeometryData* self, int items, float* data)
{
   int new = items * self->size;
   if (self->allocated < self->count + new)
   {
      /* Dynamic reallocation when needed */
      if (items > 3)
         self->allocated += new;        /* Enough for this block read */
      else
         self->allocated += 100 * new;  /* Enough for next 100 small reads */

      self->data = Memory_Realloc_Array(self->data, float, self->allocated);
   }

   memcpy(self->data + self->count, data, new * sizeof(float));
   self->count += new;
}

void lucGeometryData_Setup(lucGeometryData* self, float min, float max)
{
   self->minimum = min;
   self->maximum = max;
}

void lucDatabase_OpenDatabase(lucDatabase* self)
{
   /* Skip if db already open */
   if (!self->db)
   {
      /* Copy db from checkpointReadPath?? /
      if (restart && strlen(self->context->checkpointReadPath) > 0)*/
      int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
      if (self->viewonly) flags = SQLITE_OPEN_READWRITE;

      if (self->filename && strlen(self->filename))
      {
         sprintf(self->path, "%s.gldb", self->filename);
      }
      else
      {
         sprintf(self->path, "file:%s?mode=memory&cache=shared", self->name);
         flags = flags | SQLITE_OPEN_URI;
         Journal_Printf(lucDebug, "Defaulting to memory database: %s\n", self->path);
         /* Set the delete-after window to 1 step *
          * avoids accumulating old steps in memory */
         if (self->deleteAfter == 0) self->deleteAfter = 1;
      }

      if (sqlite3_open_v2(self->path, &self->db, flags, self->vfs))
      {
         Journal_Printf(lucError, "Can't open database: (%s) %s\n", self->path, sqlite3_errmsg(self->db));
         self->db = NULL;
         return;
      }

      /* No table modifications */
      if (self->viewonly) return;

      /* 10 sec timeout on busy(locked), as we are only accessing the db on root should not be necessary */
      sqlite3_busy_timeout(self->db, 10000);

      /* Create new tables when not present */
      lucDatabase_CreateDatabase(self);
   }
}

void lucDatabase_CreateDatabase(lucDatabase* self)
{
   /* Create database tables */
   lucDatabase_IssueSQL(self->db, "create table IF NOT EXISTS geometry (id INTEGER PRIMARY KEY ASC, object_id INTEGER, timestep INTEGER, rank INTEGER, idx INTEGER, type INTEGER, data_type INTEGER, size INTEGER, count INTEGER, width INTEGER, minimum REAL, maximum REAL, dim_factor REAL, units VARCHAR(32), minX REAL, minY REAL, minZ REAL, maxX REAL, maxY REAL, maxZ REAL, labels VARCHAR(2048), properties VARCHAR(2048), data BLOB, FOREIGN KEY (object_id) REFERENCES object (id) ON DELETE CASCADE ON UPDATE CASCADE, FOREIGN KEY (timestep) REFERENCES timestep (id) ON DELETE CASCADE ON UPDATE CASCADE)");

   /* Delete structure tables, always recreated */
   lucDatabase_IssueSQL(self->db, "drop table IF EXISTS object_colourmap");
   lucDatabase_IssueSQL(self->db, "drop table IF EXISTS colourmap");
   lucDatabase_IssueSQL(self->db, "drop table IF EXISTS object");
   lucDatabase_IssueSQL(self->db, "drop table IF EXISTS state");

   lucDatabase_IssueSQL(self->db, 
      "create table IF NOT EXISTS timestep (id INTEGER PRIMARY KEY ASC, time REAL, dim_factor REAL, units VARCHAR(32), properties VARCHAR(2048))");

   lucDatabase_IssueSQL(self->db, 
      "create table object_colourmap (id integer primary key asc, object_id integer, colourmap_id integer, data_type integer, foreign key (object_id) references object (id) on delete cascade on update cascade, foreign key (colourmap_id) references colourmap (id) on delete cascade on update cascade)");

   lucDatabase_IssueSQL(self->db, 
      "create table object (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), colourmap_id INTEGER, colour INTEGER, opacity REAL, properties VARCHAR(2048), FOREIGN KEY (colourmap_id) REFERENCES colourmap (id) ON DELETE CASCADE ON UPDATE CASCADE)"); 

   lucDatabase_IssueSQL(self->db, 
      "create table colourmap (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), minimum REAL, maximum REAL, logscale INTEGER, discrete INTEGER, properties VARCHAR(2048))"); 

   lucDatabase_IssueSQL(self->db, "create table state (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), data TEXT)");
}

Bool lucDatabase_IssueSQL(sqlite3* db, const char* SQL)
{
   /* Executes a basic SQLite command (ie: without pointer objects and ignoring result sets) and checks for errors */
   if (!db) return False;
   char* zErrMsg;
   if (sqlite3_exec(db, SQL, NULL, 0, &zErrMsg) != SQLITE_OK)
   {
      Journal_Printf(lucError, "SQLite Issue: %s\n%s\n", zErrMsg, SQL);
      sqlite3_free(zErrMsg);
      return False;
   }
   return True;
}

Bool lucDatabase_BeginTransaction(lucDatabase* self)
{
   return lucDatabase_IssueSQL(self->db, "BEGIN EXCLUSIVE TRANSACTION");
}

void lucDatabase_Commit(lucDatabase* self)
{
   lucDatabase_IssueSQL(self->db, "COMMIT");
}

void lucDatabase_AttachDatabase(lucDatabase* self)
{
   /* Detach previous */
   char path[256];
   if (self->db2)
      sqlite3_close(self->db2);

   /* Create path */
   sprintf(path, "%s%05d.gldb", self->filename, self->timeStep);

   /* Attach new database */
   if (sqlite3_open_v2(path, &self->db2, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, self->vfs))
   {
      Journal_Printf(lucError, "Can't open database: %s\n", sqlite3_errmsg(self->db2));
      self->db2 = NULL;
      return;
   }
   Journal_Printf(lucDebug, "Database file %s opened\n", path);
}

void lucDatabase_DeleteGeometry(lucDatabase* self, int start_timestep, int end_timestep)
{
   if (self->rank > 0) return;
   /* Remove data over timestep range */
   if (start_timestep < 0)
      snprintf(SQL, MAX_QUERY_LEN,  "delete from geometry where timestep <= %d; delete from timestep where id <= %d;", end_timestep, end_timestep);
   else if (end_timestep < 0)
      snprintf(SQL, MAX_QUERY_LEN,  "delete from geometry where timestep >= %d; delete from timestep where id >= %d;", start_timestep, start_timestep);
   else
      snprintf(SQL, MAX_QUERY_LEN,  "delete from geometry where timestep between %d and %d; delete from timestep where id between %d and %d;", start_timestep, end_timestep, start_timestep, end_timestep);

   lucDatabase_IssueSQL(self->db, SQL);
}

int lucDatabase_WriteGeometry(lucDatabase* self, int index, lucGeometryType type, lucGeometryDataType data_type, int object_id, lucGeometryData* block)
{
   sqlite3* db = self->db;
   if (self->db2) db = self->db2; //Use secondary per-timestep database
   sqlite3_stmt* statement;
   if (!block->count) return 0;
   
   /* Compress the data if enabled and > 1kb */
   unsigned char* buffer = (unsigned char*)block->data;
   unsigned long src_len = block->count * sizeof(float);
   unsigned long cmp_len = 0;
   unsigned char* cmp_buffer = NULL;
   if (self->compressed && src_len > 1000)
   {
      cmp_len = compressBound(src_len);
      cmp_buffer = (unsigned char*)malloc((size_t)cmp_len);
      Journal_Firewall(buffer != NULL, lucError, "Compress database: Out of memory! %s '%s'.\n", self->type, self->name );
      Journal_Firewall(compress2((mz_uint8*)cmp_buffer, &cmp_len, (const unsigned char *)block->data, src_len, 1) == Z_OK,
         lucError, "Compress database failed! %s '%s'.\n", self->type, self->name );
      if (cmp_len >= src_len)
      {
         cmp_len = 0;
      }
      else
      {
         src_len = cmp_len;
         buffer = cmp_buffer;
      }
   }

   if (fabs(block->minimum) == HUGE_VAL) block->minimum = 0;
   if (fabs(block->maximum) == HUGE_VAL) block->maximum = 0;

   if (block->min[0] > block->max[0]) block->min[0] = block->max[0] = 0;
   if (block->min[1] > block->max[1]) block->min[1] = block->max[1] = 0;
   if (block->min[2] > block->max[2]) block->min[2] = block->max[2] = 0;

   snprintf(SQL, MAX_QUERY_LEN, "insert into geometry (object_id, timestep, rank, idx, type, data_type, size, count, width, minimum, maximum, dim_factor, units, minX, minY, minZ, maxX, maxY, maxZ, labels, data) values (%d, %d, %d, %d, %d, %d, %d, %d, %d, %g, %g, %g, '%s', %g, %g, %g, %g, %g, %g, ?, ?)", object_id, self->timeStep, 0, index, type, data_type, block->size, block->count, block->width, block->minimum, block->maximum, 1.0, "", block->min[0], block->min[1], block->min[2], block->max[0], block->max[1], block->max[2]);

   /* Prepare statement... */
   if (sqlite3_prepare_v2(db, SQL, -1, &statement, NULL) != SQLITE_OK)
   {
      Journal_Printf(lucError, "SQL prepare error: (%s) %s\n", SQL, sqlite3_errmsg(db));
      src_len = 0;
   }

   /* Setup text data for insert */
   if (src_len > 0 && block->labels && sqlite3_bind_text(statement, 1, block->labels, strlen(block->labels), SQLITE_STATIC) != SQLITE_OK)
   {
      Journal_Printf(lucError, "SQL bind error: %s\n", sqlite3_errmsg(db));
      src_len = 0;
   }

   /* Setup blob data for insert */
   if (src_len > 0 && sqlite3_bind_blob(statement, 2, buffer, src_len, SQLITE_STATIC) != SQLITE_OK)
   {
      Journal_Printf(lucError, "SQL bind error: %s\n", sqlite3_errmsg(db));
      src_len = 0;
   }

   /* Execute statement */
   if (src_len > 0 && sqlite3_step(statement) != SQLITE_DONE )
   {
      Journal_Printf(lucError, "SQL step error: (%s) %s\n", SQL, sqlite3_errmsg(db));
      src_len = 0;
   }

   if (src_len > 0) sqlite3_finalize(statement);

   /* Free compression buffer */
   if (cmp_buffer)
      free(cmp_buffer);

   return src_len;
}

void lucDatabase_BackupDb(sqlite3 *fromDb, sqlite3* toDb)
{
   sqlite3_backup *pBackup;  /* Backup object used to copy data */
   pBackup = sqlite3_backup_init(toDb, "main", fromDb, "main");
   if (pBackup)
   {
      (void)sqlite3_backup_step(pBackup, -1);
      (void)sqlite3_backup_finish(pBackup);
   }
}

void lucDatabase_BackupDbFile(lucDatabase* self, char* filename)
{
   if (self->rank > 0 || !self->db) return;
   sqlite3* toDb;
   int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
   if (sqlite3_open_v2(filename, &toDb, flags, self->vfs))
   {
      Journal_Printf(lucError, "Can't open database: (%s) %s\n", filename, sqlite3_errmsg(toDb));
      return;
   }

   /* Remove existing data */
   lucDatabase_IssueSQL(toDb, "drop table IF EXISTS geometry");
   lucDatabase_IssueSQL(toDb, "drop table IF EXISTS timestep");
   lucDatabase_IssueSQL(toDb, "drop table IF EXISTS object_colourmap");
   lucDatabase_IssueSQL(toDb, "drop table IF EXISTS colourmap");
   lucDatabase_IssueSQL(toDb, "drop table IF EXISTS object");
   lucDatabase_IssueSQL(toDb, "drop table IF EXISTS state");

   sqlite3_backup *pBackup;  /* Backup object used to copy data */
   pBackup = sqlite3_backup_init(toDb, "main", self->db, "main");
   if (pBackup)
   {
      (void)sqlite3_backup_step(pBackup, -1);
      (void)sqlite3_backup_finish(pBackup);
   }
   sqlite3_close(toDb);
}

void lucDatabase_WriteState(lucDatabase* self, const char* name, const char* properties)
{
   if (self->rank > 0 || !self->db) return;
   const char* noname = "";
   if (!name) name = noname;

   lucDatabase_IssueSQL(self->db, "create table IF NOT EXISTS state (id INTEGER PRIMARY KEY ASC, name VARCHAR(256), data TEXT)");

   sqlite3* db = self->db;

   /* Delete any state entry with same name */
   snprintf(SQL, MAX_QUERY_LEN,  "delete from state where name == '%s'", name);
   lucDatabase_IssueSQL(db, SQL);
   
   /* Prepare statement... */
   sqlite3_stmt* statement;
   snprintf(SQL, MAX_QUERY_LEN,  "insert into state (name, data) values ('%s', ?)", name);
   if (sqlite3_prepare_v2(db, SQL, -1, &statement, NULL) != SQLITE_OK)
   {
      Journal_Printf(lucError, "SQL prepare error: (%s) %s\n", SQL, sqlite3_errmsg(db));
      return;
   }

   /* Setup text data for insert */
   if (sqlite3_bind_text(statement, 1, properties, strlen(properties), SQLITE_STATIC) != SQLITE_OK)
   {
      Journal_Printf(lucError, "SQL bind error: %s\n", sqlite3_errmsg(db));
      return;
   }

   /* Execute statement */
   if (sqlite3_step(statement) != SQLITE_DONE )
   {
      Journal_Printf(lucError, "SQL step error: (%s) %s\n", SQL, sqlite3_errmsg(db));
      return;
   }

   sqlite3_finalize(statement);
}

