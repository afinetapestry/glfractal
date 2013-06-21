CC=g++
CXX=g++
LD=g++
CFLAGS=-g -c $(shell sdl2-config --cflags) -I/opt/local/include -I/usr/include/OpenEXR -I../SDL/glengine
LDFLAGS=$(shell sdl2-config --libs) -lSDL2_image -L/opt/local/lib -lGLEW -framework OpenGL -framework OpenCL
SRC_C=../SDL/glengine/glerror.c clerr.c
SRC_CXX=main.cpp
OBJECTS=$(SRC_C:.c=.o) $(SRC_CXX:.cpp=.o)
EXECUTABLE=glfractal

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

.PHONY: clean

