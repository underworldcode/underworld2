include ../../config.mk

SDIR=../..
QUANTA_CFLAGS=-I${SDIR}/QUANTA/include
QUANTA_LDFLAGS=-L${SDIR}/lib -lquanta

CFLAGS+= $(SAGE_CFLAGS) -O3 -I../../include $(QUANTA_CFLAGS) $(PORTAUDIO_CFLAGS)
CC=$(COMPILER)

ifeq ($(MACHINE), Darwin)
   CFLAGS  += -FGLUT -FOpenGL
   LIBS=-ldl -lpthread -L../../lib -lsail -framework GLUT -framework OpenGL -lobjc -lm 
else
ifeq ($(MACHINE), SunOS)
   CFLAGS=$(SAGE_CFLAGS) -O3 -I../../include $(QUANTA_CFLAGS) $(GLUT_CFLAGS) 
   LIBS=-ldl -lpthread -lm -lGL -lGLU $(QUANTA_LDFLAGS) -L../../lib -lsail $(GLUT_LDFLAGS) 
else
   LIBS=-ldl -lpthread -lm -lGL -lGLU $(GLUT_LDFLAGS) $(QUANTA_LDFLAGS) $(GLEW_LIB) -L../../lib -lsail
endif
endif

SRC := $(wildcard *.cpp)
INC := $(wildcard *.h)
FILES = $(SRC) jpeg/jpge.cpp Main/main.cpp Main/GlutViewer.cpp
OBJ := $(FILES:%.cpp=%.o)

PROGRAM = gLucifer

LIBS+=$(PAUDIO_LIB)

default: $(PROGRAM)

install: $(PROGRAM)
	cp $(PROGRAM) ../../bin

#Rebuild *.cpp
$(OBJ): %.o : %.cpp
	$(CC) $(CFLAGS) -DHAVE_GLUT -DHAVE_SAGE -c $< -o $@

$(PROGRAM): $(OBJ) sqlite3.o mongoose.o
	$(CC) -o $(PROGRAM) $(OBJ) mongoose.o sqlite3.o $(SAGE_LDFLAGS) $(LIBS)

sqlite3.o : sqlite3/src/sqlite3.c
	gcc -o $@ -c $^ 

mongoose.o : mongoose/mongoose.c
	gcc -o $@ -c $^ 

clean:
	/bin/rm -f *~ *.o $(PROGRAM)

