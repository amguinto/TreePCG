/*
 * Converts .txt files in graph sparse format to matrix market format
 * 
 * takes parameters:
 *     input file name
 *     output file name
 *
 * Copied from graphSp2MM written by Haoran,
 *     editted by Richard 
 */

#include <iostream>
#include <cstdio>
#include <string>
#include "io.h"
#include "graph.h"
#include "matrix.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "Please specify graph file and output file" << endl;
        return -1;
    }

    fprintf(stderr, "CONVERTING BINARY FILE:\n");
    fprintf(stderr, "%s\n", argv[1]);
    fprintf(stderr, "TO MTX FILE:\n");
    fprintf(stderr, "%s\n", argv[2]);

    string graphFile = argv[1];
    string outFile = argv[2];

    GraphSP g = IO::readGraphBin(graphFile);
    fprintf(stderr, "FINISHED READING FILE\n");
    Mat A = IO::constructMatrixFromGraph(g);
    fprintf(stderr, "CONSTRUCTED MATRIX\n");
    IO::saveMM(A, outFile);
    fprintf(stderr, "DONE SAVING TO FILE\n");
    return 0;
}