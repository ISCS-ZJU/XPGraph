# Make lib files
cd **/LevelGraph/src/api
make

# Copy lib files
sudo cp libxpgraph.h /usr/include/
sudo cp lib/libxpgraph.so /usr/lib/

# Make test file
cd ../XPGraph_app
make

# Run test
./bin/main