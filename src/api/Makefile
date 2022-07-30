INCFLAGS = -I/usr/local/include/ -I../../src/

CPP = g++
# CPPFLAGS = -g -O0 $(INCFLAGS)  -fopenmp -Wall -Wno-strict-aliasing 
CPPFLAGS = -g -O3 $(INCFLAGS)  -fopenmp -Wall -Wno-strict-aliasing 
LDFLAGS = -shared -fPIC
LINKERFLAGS = -lz -lpmem -lpmemobj -lnuma
DEBUGFLAGS = -g -ggdb $(INCFLAGS)
HEADERS=$(shell find . -name '*.hpp')

LIB = lib/libxpgraph.so
OBJ = lib/libxpgraph.o

all: clean $(LIB)

$(LIB) : $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LINKERFLAGS) $(LDFLAGS)

$(OBJ) : libxpgraph.cpp
	@mkdir -p lib/
	$(CPP) $(CPPFLAGS) -c $^ -o $@ $(LINKERFLAGS) $(LDFLAGS)
 
clean:
	@rm -rf lib/