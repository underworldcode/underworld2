/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* -*- C -*-  (not really, but good for syntax highlighting) */


#if (SWIG_VERSION <= 0x040000)
%module (package="underworld.libUnderworld.libUnderworldPy") StGermain
#else
%module (package="underworld.libUnderworld.libUnderworldPy", moduleimport="import $module") StGermain
#endif

%{
/* Includes the header in the wrapper code */
#define SWIG_FILE_WITH_INIT
#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
}

%}


%include "StGermain_Typemaps.i"

int    StgVariable_GetValueAtInt(   StgVariable*, unsigned int, unsigned int);
long   StgVariable_GetValueAtLong(  StgVariable*, unsigned int, unsigned int);
float  StgVariable_GetValueAtFloat( StgVariable*, unsigned int, unsigned int);
double StgVariable_GetValueAtDouble(StgVariable*, unsigned int, unsigned int);

%include "numpy.i"

%init %{
import_array();
%}

%include "exception.i"
%exception {
    try {
        $action
    } catch (const std::range_error& e) {
        SWIG_exception(SWIG_ValueError, e.what());
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}


%rename(CFalse) False;
%rename(CTrue) True;

%include "Base/Foundation/types.h"       
%include "Base/Container/types.h"       
%include "Base/Automation/types.h"
%include "Base/Context/types.h"
%include "Base/Extensibility/types.h"
%include "Base/Foundation/types.h"
%include "Base/IO/types.h"
%include "Utils/types.h"
%include "Base/Foundation/Class.h"
%include "Base/Foundation/Object.h"
%include "Base/Foundation/NamedObject_Register.h"
%include "Base/Automation/Stg_Component.h"
%include "Base/Automation/LiveComponentRegister.h"       
%include "Base/Automation/Stg_ComponentFactory.h"       
%include "Base/Automation/Stg_ComponentRegister.h"
%include "Base/Context/Variable.h"
%include "Base/Context/types.h"
%include "Base/Context/VariableCondition.h"
%include "Base/Context/PythonVC.h"
%include "Base/Extensibility/EntryPoint.h"
%include "Base/Extensibility/Init.h"
%include "Base/Extensibility/ModulesManager.h"
%include "Base/Container/IndexSet.h"
%include "Base/IO/IO_Handler.h"
%include "Base/IO/XML_IO_Handler.h"
%include "Base/IO/Dictionary.h"
%include "Base/IO/DictionaryUtils.h"
%include "Base/IO/Dictionary_Entry.h"
%include "Base/IO/Dictionary_Entry_Value.h"
%include "Base/Foundation/ObjectList.h"       


/* # The following allows us to add values to IndexSets from numpy arrays */
%extend IndexSet
{

    void AddOrRemoveWithNumpyInt( int* IN_ARRAY1, int DIM1, Bool adding ){
        int ii;

        if (adding) {
            for (ii=0; ii<DIM1; ii++)
                IndexSet_Add($self, IN_ARRAY1[ii]);
        } else {
            for (ii=0; ii<DIM1; ii++)
                IndexSet_Remove($self, IN_ARRAY1[ii]);
        }
        
    }

}
%extend IndexSet
{

    void AddOrRemoveWithNumpyLong( long* IN_ARRAY1, int DIM1, Bool adding ){
        int ii;

        if (adding) {
            for (ii=0; ii<DIM1; ii++)
                IndexSet_Add($self, IN_ARRAY1[ii]);
        } else {
            for (ii=0; ii<DIM1; ii++)
                IndexSet_Remove($self, IN_ARRAY1[ii]);
        }
    }

}
%extend IndexSet
{

    void AddOrRemoveWithNumpyUInt( unsigned int* IN_ARRAY1, int DIM1, Bool adding ){
        int ii;

        if (adding) {
            for (ii=0; ii<DIM1; ii++)
                IndexSet_Add($self, IN_ARRAY1[ii]);
        } else {
            for (ii=0; ii<DIM1; ii++)
                IndexSet_Remove($self, IN_ARRAY1[ii]);
        }
    }

}
%extend IndexSet
{

    void AddOrRemoveWithNumpyULong( unsigned long* IN_ARRAY1, int DIM1, Bool adding ){
        int ii;

        if (adding) {
            for (ii=0; ii<DIM1; ii++)
                IndexSet_Add($self, IN_ARRAY1[ii]);
        } else {
            for (ii=0; ii<DIM1; ii++)
                IndexSet_Remove($self, IN_ARRAY1[ii]);
        }
    }

}
/* # The following allows us to retrieve values from IndexSets from numpy arrays */
%extend IndexSet
{
    void GetAsNumpyArray( unsigned int* INPLACE_ARRAY1, int DIM1 ){

        Journal_Firewall( DIM1 == IndexSet_UpdateMembersCount($self), $self->error, "Error- %s: Numpy array size (%u) is different to member count (%u). Aborting.",
            __func__, DIM1, IndexSet_UpdateMembersCount($self));

        IndexSet_GetMembers2( $self, INPLACE_ARRAY1 );

    }
}

