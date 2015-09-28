//Global includes file for gLucifer Viewer
//
#ifndef Include__
#define Include__

//Handles compatibility on Linux, Windows, Mac OS X

#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#if defined USE_OMEGALIB
#include <omegaGl.h>
#endif

#define ISFINITE(val) (!isnan(val) && !isinf(val))

//C++ STL
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <map>
#include <deque>
#include <iomanip>

//C headers
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <ctype.h>
#include <pthread.h>

//Include the decompression routines
#if defined HAVE_LIBPNG and not defined USE_ZLIB
#define USE_ZLIB
#endif

#ifdef USE_ZLIB
#include <zlib.h>
#else
#define MINIZ_HEADER_FILE_ONLY
#include "miniz/miniz.c"
#endif

//Utils
#include "jpeg/jpge.h"
#include "jpeg/jpgd.h"
#include "json.h"

#ifndef _WIN32
#include <sys/poll.h>
#include <unistd.h>

#define PAUSE(msecs) usleep(msecs * 1000);

#if defined __APPLE__ //Don't use function pointers!
#define GL_GLEXT_PROTOTYPES
#elif not defined EXTENSION_POINTERS
//Extension pointer retrieval on Linux now only if explicity enabled
#define GL_GLEXT_PROTOTYPES
#define GL_GLXEXT_PROTOTYPES
#endif

//This could be problematic with different configurations...
#if defined __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#else

/* WINDOWS */
#define snprintf sprintf_s
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#include <conio.h>
#include "windows/inc/pthread.h"
#include "windows/inc/zlib.h"
#define PAUSE(msecs) Sleep(msecs);
//Include the libraries -- */
#pragma comment(lib, "sqlite3.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "OPENGL32.LIB")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "pthreadVCE2.lib")

//#include "windows/inc/SDL.h"
#include <SDL/SDL_opengl.h>

#endif

//Define pointers to required gl 2.0 functions
#if defined _WIN32 
#define EXTENSION_POINTERS
#endif

typedef void* (*getProcAddressFN)(const char* procName);
#if not defined _WIN32
extern getProcAddressFN GetProcAddress;
#endif 

#include "Extensions.h"

#endif //Include__
