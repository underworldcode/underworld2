
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
swig_add_library(FunctionSwig TYPE SHARED LANGUAGE python SOURCES Function.i)
set_target_properties(FunctionSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "Function")
target_link_libraries(FunctionSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(c_arrays.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME c_arrays)
swig_add_library(c_arraysSwig TYPE SHARED LANGUAGE python SOURCES c_arrays.i)
set_target_properties(c_arraysSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "c_arrays")
target_link_libraries(c_arraysSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(c_pointers.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME c_pointers)
swig_add_library(c_pointersSwig TYPE SHARED LANGUAGE python SOURCES c_pointers.i)
set_target_properties(c_pointersSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "c_pointers")
target_link_libraries(c_pointersSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(gLucifer.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME gLucifer)
swig_add_library(gLuciferSwig TYPE SHARED LANGUAGE python SOURCES gLucifer.i)
set_target_properties(gLuciferSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "gLucifer")
target_link_libraries(gLuciferSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(petsc.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME petsc)
swig_add_library(petscSwig TYPE SHARED LANGUAGE python SOURCES petsc.i)
set_target_properties(petscSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "petsc")
target_link_libraries(petscSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(PICellerator.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME PICellerator)
swig_add_library(PICelleratorSwig TYPE SHARED LANGUAGE python SOURCES PICellerator.i)
set_target_properties(PICelleratorSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "PICellerator")
target_link_libraries(PICelleratorSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(Solvers.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME Solvers)
swig_add_library(SolversSwig TYPE SHARED LANGUAGE python SOURCES Solvers.i)
set_target_properties(SolversSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "Solvers")
target_link_libraries(SolversSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(StgDomain.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME StgDomain)
swig_add_library(StgDomainSwig TYPE SHARED LANGUAGE python SOURCES StgDomain.i)
set_target_properties(StgDomainSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "StgDomain")
target_link_libraries(StgDomainSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(StGermain.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME StGermain)
swig_add_library(StGermainSwig TYPE SHARED LANGUAGE python SOURCES StGermain.i)
set_target_properties(StGermainSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "StGermain")
target_link_libraries(StGermainSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(StGermain_Tools.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME StGermain_Tools)
swig_add_library(StGermain_ToolsSwig TYPE SHARED LANGUAGE python SOURCES StGermain_Tools.i StGermain_Tools.c)
set_target_properties(StGermain_ToolsSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "StGermain_Tools")
target_link_libraries(StGermain_ToolsSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(StgFEM.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME StgFEM)
swig_add_library(StgFEMSwig TYPE SHARED LANGUAGE python SOURCES StgFEM.i)
set_target_properties(StgFEMSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "StgFEM")
target_link_libraries(StgFEMSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

set_source_files_properties(Underworld.i PROPERTIES CPLUSPLUS ON SWIG_MODULE_NAME Underworld)
swig_add_library(UnderworldSwig TYPE SHARED LANGUAGE python SOURCES Underworld.i)
set_target_properties(UnderworldSwig PROPERTIES SUFFIX ".so" OUTPUT_NAME "Underworld")
target_link_libraries(UnderworldSwig PRIVATE ${UW_LIBRARIES} PRIVATE ${PETSc_LIBRARIES} PRIVATE Python3::Module)

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
get_property(FUNCTION_PY_FILE TARGET FunctionSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(CARRAYS_PY_FILE TARGET c_arraysSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(CPOINTERS_PY_FILE TARGET c_pointersSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(GLUCIFER_PY_FILE TARGET gLuciferSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(PETSC_PY_FILE TARGET petscSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(PICELLERATOR_PY_FILE TARGET PICelleratorSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(SOLVERS_PY_FILE TARGET SolversSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(STGDOMAIN_PY_FILE TARGET StgDomainSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(STGERMAIN_PY_FILE TARGET StGermainSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(STGERMAIN_TOOLS_PY_FILE TARGET StGermain_ToolsSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(STGFEM_PY_FILE TARGET StgFEMSwig PROPERTY SWIG_SUPPORT_FILES)
get_property(UNDERWORLD_PY_FILE TARGET UnderworldSwig PROPERTY SWIG_SUPPORT_FILES)

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
    FunctionSwig
    c_arraysSwig
    c_pointersSwig
    gLuciferSwig
    petscSwig
    PICelleratorSwig
    SolversSwig
    StgDomainSwig
    StGermainSwig
    StGermain_ToolsSwig
    StgFEMSwig
    UnderworldSwig
)

install(TARGETS ${SWIG_LIBRARIES}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
