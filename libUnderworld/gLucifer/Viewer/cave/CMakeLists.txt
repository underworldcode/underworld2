####################################################################################################################### 
# Omega Lib build for gLucifer
#---------------------------------------------------------------------------------------------------------------------
# Copyright 2013							Monash University
# Authors:										
#  Owen Kaluza							owen.kaluza@monash.edu
#######################################################################################################################
cmake_minimum_required(VERSION 2.8.1) 
project(LavaVR)

find_package(Omegalib)


# Source files
SET( srcs
        LavaVR.cpp
        ../src/LavaVu.cpp
        ../src/Server.cpp
        ../src/InteractiveViewer.cpp
        ../src/OpenGLViewer.cpp
        ../src/base64.cpp
        ../src/ColourMap.cpp
        ../src/DrawingObject.cpp
        ../src/Extensions.cpp
        ../src/FontSans.cpp
        ../src/Geometry.cpp
        ../src/GraphicsUtil.cpp
        ../src/Lines.cpp
        ../src/Model.cpp
        ../src/Points.cpp
        ../src/QuadSurfaces.cpp
        ../src/Shaders.cpp
        ../src/Shapes.cpp
        ../src/Tracers.cpp
        ../src/TriSurfaces.cpp
        ../src/Vectors.cpp
        ../src/Volumes.cpp
        ../src/VideoEncoder.cpp
        ../src/View.cpp
        ../src/Win.cpp
        ../src/tiny_obj_loader.cc
        ../src/jpeg/jpge.cpp
        ../src/jpeg/jpgd.cpp
        ../src/json.cpp
        ../src/mongoose/mongoose.c
        ../src/sqlite3/src/sqlite3.c
   )
    
#######################################################################################################################
# Headers
SET( headers 
        ../src/ViewerApp.h
        ../src/OpenGLViewer.h
   ) 
        
#######################################################################################################################
# Setup compile info

add_definitions(-DUSE_OMEGALIB -DUSE_ZLIB -DHAVE_LIBPNG -DUSE_FONTS -DSHADER_PATH="${CMAKE_BINARY_DIR}/../src/shaders/")
#add_definitions(-DDEBUG -g)
add_definitions(-O3)

#add_library(common OBJECT ${srcs})

include_directories(${OMEGA_INCLUDE_DIRS})

add_executable(LavaVR ${srcs} ${headers})

target_link_libraries(LavaVR dl z png python2.7 ${OMEGA_LIB} ${OMEGA_TOOLKIT_LIB})



