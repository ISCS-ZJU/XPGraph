INCFLAGS = -I/usr/local/include/ -I./src/api/ -I./apps/ 

CPP = g++
CPPFLAGS = -g -O3 $(INCFLAGS)  -fopenmp -Wall -Wno-strict-aliasing  -Wno-unknown-pragmas
LDFLAGS = -fPIC -L./src/api/lib/
LINKERFLAGS = -lz -lpmem -lpmemobj -lnuma -lxpgraph
DEBUGFLAGS = -g -ggdb $(INCFLAGS)
HEADERS=$(shell find . -name '*.hpp')

NEW_LIB_PATH:=$(addsuffix ./src/api/lib/:, $(LD_LIBRARY_PATH))

all : lib clean main

lib:
	cd ./src/api/ && $(MAKE)

clean:
	@rm -rf bin/

main: main.cpp $(HEADERS)
	@mkdir -p bin/
	$(CPP) $(CPPFLAGS) $(LDFLAGS) main.cpp -o bin/main $(LINKERFLAGS)

# test ingest
testm:export LD_LIBRARY_PATH=$(NEW_LIB_PATH)
testm: lib clean main
	@echo LD_LIBRARY_PATH=$(LD_LIBRARY_PATH)
	./bin/main

# test recovery
testr:export LD_LIBRARY_PATH=$(NEW_LIB_PATH)
testr: lib main
	@echo LD_LIBRARY_PATH=$(LD_LIBRARY_PATH)
	./bin/main -j 3

testgdb:export LD_LIBRARY_PATH=$(NEW_LIB_PATH)
testgdb: lib clean main
	@echo LD_LIBRARY_PATH=$(LD_LIBRARY_PATH)
	gdb ./bin/main