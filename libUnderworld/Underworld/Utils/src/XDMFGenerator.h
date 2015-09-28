/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_XDMFGenerator_h__
#define __Underworld_Utils_XDMFGenerator_h__

   extern const Type XDMFGenerator_Type;

   void  XDMFGenerator_GenerateAll( void* _context );
   void  _XDMFGenerator_WriteHeader( UnderworldContext* context, Stream* stream );
   void  _XDMFGenerator_WriteFieldSchema( UnderworldContext* context, Stream* stream );
   void  _XDMFGenerator_WriteSwarmSchema( UnderworldContext* context, Stream* stream );
   void  _XDMFGenerator_SendInfo( UnderworldContext* context );
   void  _XDMFGenerator_WriteFooter( UnderworldContext* context, Stream* stream );
   void  _XDMFGenerator_UpdateTemporalFileLists( UnderworldContext* context );
   void _XDMFGenerator_FieldsOrSwarmsOnlyFiles( UnderworldContext* context );
   void XDMFGenerator_GenerateTemporalTopLevel( void* _context, void* ignored );
#endif
