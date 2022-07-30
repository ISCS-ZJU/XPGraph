
// #include <omp.h>
// #include <getopt.h>
// #include <stdlib.h>

#include <iostream>
#include <string>
#include <fstream>

#include <libxpgraph.h>
#include "apps/graph_benchmarks.hpp"

int main(int argc, const char ** argv)
{
    XPGraph *xpgraph = new XPGraph(argc, argv);
    xpgraph->import_graph_by_config();

    #pragma region test_neighbors
    double start, end; 
	start = mywtime();
    test_out_neighbors(xpgraph);
    test_in_neighbors(xpgraph);
	end = mywtime();
    std::cout << "test_out/in_neighbors cost: " << (end - start) << std::endl;
    #pragma endregion test_neighbors

    // test_graph_benchmarks(xpgraph);
    test_graph_benchmarks_numa(xpgraph);
    xpgraph->save_gragh();
    delete xpgraph;
    return 0;
}
