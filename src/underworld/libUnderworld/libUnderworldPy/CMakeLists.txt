
# SWIG
# load swig's cmake module
include(${SWIG_USE_FILE})

SET(UW_LIBRARIES
    Underworld
    StGermain
    StgDomain
    StgFEM
    PICellerator
    gLucifer
    Solvers
    pcu)

if(NOT APPLE)
    message(STATUS ">>> Linux System")
    set(CMAKE_INSTALL_RPATH "\$ORIGIN")
else()
    message(STATUS ">>> Apple")
    set(CMAKE_MACOSX_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@loader_path")
endif()

add_compile_options(-DU_SHOW_CPLUSPLUS_API=0)
add_compile_options(-DSWIG_PYTHON_SILENT_MEMLEAK)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

SET(CMAKE_SWIG_FLAGS "-v" "-Wextra" "-O" "-c++" "-DSWIG_DO_NOT_WRAP")

message("Swig section diagnostics:")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "INCLUDE_DIRECTORIES")
foreach(dir ${dirs})
  message(STATUS "  '${dir}'")
endforeach()
message(STATUS "UW_LIBRARIES=${UW_LIBRARIES}")
message(STATUS "PETSc_LIBRARIES=${PETSc_LIBRARIES}")
message("End Swig diagnostics:")

set_source_files_properties(Function.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME Function) 
# This create swig module target, SWIG_MODULE_Function_REAL_NAME with CMP0078 != NEW
swig_add_library(Function TYPE SHARED LANGUAGE python SOURCES Function.i)
set_target_properties(${SWIG_MODULE_Function_REAL_NAME} PROPERTIES SUFFIX ".so" )
target_link_libraries(${SWIG_MODULE_Function_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(c_arrays.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME c_arrays)
swig_add_library(c_arrays TYPE SHARED LANGUAGE python SOURCES c_arrays.i)
set_target_properties(${SWIG_MODULE_c_arrays_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_c_arrays_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(c_pointers.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME c_pointers)
swig_add_library(c_pointers TYPE SHARED LANGUAGE python SOURCES c_pointers.i)
set_target_properties(${SWIG_MODULE_c_pointers_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_c_pointers_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(gLucifer.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME gLucifer)
#set_property(SOURCE gLucifer.i PROPERTY SWIG_MODULE_NAME gLucifer)
swig_add_library(gLucifer TYPE SHARED LANGUAGE python SOURCES gLucifer.i)
set_target_properties(${SWIG_MODULE_gLucifer_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_gLucifer_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)


set_source_files_properties(petsc.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME petsc)
#set_property(SOURCE petsc.i PROPERTY SWIG_MODULE_NAME petsc)
swig_add_library(petsc TYPE SHARED LANGUAGE python SOURCES petsc.i)
set_target_properties(${SWIG_MODULE_petsc_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_petsc_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(PICellerator.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME PICellerator)
#set_property(SOURCE PICellerator.i PROPERTY SWIG_MODULE_NAME PICellerator)
swig_add_library(PICellerator TYPE SHARED LANGUAGE python SOURCES PICellerator.i)
set_target_properties(${SWIG_MODULE_PICellerator_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_PICellerator_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(Solvers.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME Solvers)
#set_property(SOURCE Solvers.i PROPERTY SWIG_MODULE_NAME Solvers)
swig_add_library(Solvers TYPE SHARED LANGUAGE python SOURCES Solvers.i)
set_target_properties(${SWIG_MODULE_Solvers_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_Solvers_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(StgDomain.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME StgDomain)
#set_property(SOURCE StgDomain.i PROPERTY SWIG_MODULE_NAME StgDomain)
swig_add_library(StgDomain TYPE SHARED LANGUAGE python SOURCES StgDomain.i)
set_target_properties(${SWIG_MODULE_StgDomain_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_StgDomain_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(StGermain.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME StGermain)
#set_property(SOURCE StGermain.i PROPERTY SWIG_MODULE_NAME StGermain)
swig_add_library(StGermain TYPE SHARED LANGUAGE python SOURCES StGermain.i)
set_target_properties(${SWIG_MODULE_StGermain_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_StGermain_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(StGermain_Tools.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME StGermain_Tools)
#set_property(SOURCE StGermain_Tools.i PROPERTY SWIG_MODULE_NAME StGermain_Tools)
swig_add_library(StGermain_Tools TYPE SHARED LANGUAGE python SOURCES StGermain_Tools.i StGermain_Tools.c)
set_target_properties(${SWIG_MODULE_StGermain_Tools_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_StGermain_Tools_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(StgFEM.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME StgFEM)
#set_property(SOURCE StgFEM.i PROPERTY SWIG_MODULE_NAME StgFEM)
swig_add_library(StgFEM TYPE SHARED LANGUAGE python SOURCES StgFEM.i)
set_target_properties(${SWIG_MODULE_StgFEM_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_StgFEM_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(Underworld.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME Underworld)
#set_property(SOURCE Underworld.i PROPERTY SWIG_MODULE_NAME Underworld)
swig_add_library(Underworld TYPE SHARED LANGUAGE python SOURCES Underworld.i)
set_target_properties(${SWIG_MODULE_Underworld_REAL_NAME} PROPERTIES SUFFIX ".so")
target_link_libraries(${SWIG_MODULE_Underworld_REAL_NAME} PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

# Handle where to install the resulting Python package
if(CALL_FROM_SETUP_PY)
    # The CMakeExtension will set CMAKE_INSTALL_PREFIX to the root
    # of the resulting sdist / wheel archive
    set(UW_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
else()
    # The Python package is installed directly in the folder of the
    # detected interpreter (system, user, or virtualenv)
    set(UW_INSTALL_PREFIX ${Python3_SITELIB})
endif()

# Find the SWIG_SUPPORT_FILES from each wrapping
get_property(FUNCTION_PY_FILE TARGET ${SWIG_MODULE_Function_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(CARRAYS_PY_FILE TARGET ${SWIG_MODULE_c_arrays_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(CPOINTERS_PY_FILE TARGET ${SWIG_MODULE_c_pointers_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(GLUCIFER_PY_FILE TARGET ${SWIG_MODULE_gLucifer_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(PETSC_PY_FILE TARGET ${SWIG_MODULE_petsc_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(PICELLERATOR_PY_FILE TARGET ${SWIG_MODULE_PICellerator_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(SOLVERS_PY_FILE TARGET ${SWIG_MODULE_Solvers_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(STGDOMAIN_PY_FILE TARGET ${SWIG_MODULE_StgDomain_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(STGERMAIN_PY_FILE TARGET ${SWIG_MODULE_StGermain_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(STGERMAIN_TOOLS_PY_FILE TARGET ${SWIG_MODULE_StGermain_Tools_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(STGFEM_PY_FILE TARGET ${SWIG_MODULE_StgFEM_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)
get_property(UNDERWORLD_PY_FILE TARGET ${SWIG_MODULE_Underworld_REAL_NAME} PROPERTY SWIG_SUPPORT_FILES)

set(WRAPPER_FILES
    ${FUNCTION_PY_FILE}
    ${CARRAYS_PY_FILE}
    ${CPOINTERS_PY_FILE} 
    ${GLUCIFER_PY_FILE}
    ${PETSC_PY_FILE}
    ${PICELLERATOR_PY_FILE}
    ${SOLVERS_PY_FILE}
    ${STGDOMAIN_PY_FILE} 
    ${STGERMAIN_PY_FILE} 
    ${STGERMAIN_TOOLS_PY_FILE}
    ${STGFEM_PY_FILE}
    ${UNDERWORLD_PY_FILE}
)

# install the wrapper files
install(FILES ${WRAPPER_FILES} DESTINATION "${UW_INSTALL_PREFIX}/libUnderworld/libUnderworldPy")


set(SWIG_LIBRARIES
    ${SWIG_MODULE_Function_REAL_NAME}
    ${SWIG_MODULE_c_arrays_REAL_NAME}
    ${SWIG_MODULE_c_pointers_REAL_NAME}
    ${SWIG_MODULE_gLucifer_REAL_NAME}
    ${SWIG_MODULE_petsc_REAL_NAME}
    ${SWIG_MODULE_PICellerator_REAL_NAME}
    ${SWIG_MODULE_Solvers_REAL_NAME}
    ${SWIG_MODULE_StgDomain_REAL_NAME}
    ${SWIG_MODULE_StGermain_REAL_NAME}
    ${SWIG_MODULE_StGermain_Tools_REAL_NAME}
    ${SWIG_MODULE_StgFEM_REAL_NAME}
    ${SWIG_MODULE_Underworld_REAL_NAME}
)

install(TARGETS ${SWIG_LIBRARIES}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
