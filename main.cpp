#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <chrono>
#include "util.h"
#include "graphColoringAlgorithms.h"

using namespace std;

/**
 * @parameters
 * 1) graph that will be used as input (number from 0 to graphPaths length )
 * 2) algorithm that will be run (
 *          0 = greedy
 *          1 = JP sequential
 *          2 = JP parallel
 *          3 = SDL sequential
 *          4 = SDL parallel
 *          5 = MIS sequential
 *          6 = MIS parallel
 * )
 */

//first parameter determines the
int main(int argc, char ** argv) {
    int maxColUsed = -1;
    string basePath = "./graphs/benchmark/";
    string graphPaths [] = {/*0)*/"manual/v10.gra"/*1KB*/, "manual/v100.gra"/*13KB*/, "manual/v1000.gra"/*1.6MB*/, 
            /*3)*/"small_sparse_real/agrocyc_dag_uniq.gra"/*1MB*/, "small_sparse_real/human_dag_uniq.gra"/*0.5MB*/, "small_dense_real/arXiv_sub_6000-1.gra"/*0.3MB*/, "scaleFree/ba10k5d.gra"/*0.2MB*/,
            // the next files are too large for git, need to import the "large" folder under "benchmark". It is already ignored in the .gitignore file
            /*7)*/"large/uniprotenc_150m.scc.gra"/*2MB*/, "large/citeseer.scc.gra"/*8MB*/, "large/uniprotenc_22m.scc.gra"/*19MB*/, "large/go_uniprot.gra"/*255MB*/ };
    string selectedGraph = graphPaths[atoi(argv[1])];
    string finalPath = basePath + selectedGraph;

    string algorithms [] = {"greedy", "JP sequential", "JP parallel","SDL sequential","SDL parallel", "MIS sequential", "MIS parallel"};
    string selectedAlg = algorithms[atoi(argv[2])];

    cout << endl << "-------------------------------------------------------------------------" << endl;
    cout << "running " << selectedAlg << " algorithm on graph " << selectedGraph << endl << endl ;
    map<int, list<int>> graph = readGraph(finalPath);
    vector<int> colors = initializeLabels(graph.size());
    chrono::time_point<chrono::system_clock> startTime = chrono::system_clock::now();

    
    switch(atoi(argv[2])){

        case 0:{
            vector<int> colorsGreedy = greedyAssignment(graph, colors, &maxColUsed);

            //some output just to be sure the application ran properly
            cout << "number of nodes: " << graph.size() << endl;
            cout << "number of greedy colors: " << maxColUsed + 1 << endl;
            cout << "for instance color " << colorsGreedy[maxColUsed] << " was assigned to node " << maxColUsed << endl; //should never be -1

            break;
        }
        case 1:{
            vector<int> colorsJPS = jonesPlassmannSequentialAssignment(graph, colors, &maxColUsed);

            //some output just to be sure the application ran properly
            cout << "number of nodes: " << graph.size() << endl;
            cout << "number of JP colors: " << maxColUsed + 1 << endl;
            cout << "for instance color " << colorsJPS[maxColUsed] << " was assigned to node " << maxColUsed << endl; //should never be -1

            break;
        }
        case 2:{
            vector<int> colorsJP = jonesPlassmannParallelAssignment(graph, colors, &maxColUsed);

            //some output just to be sure the application ran properly
            cout << "number of nodes: " << graph.size() << endl;
            cout << "number of JP colors: " << maxColUsed + 1 << endl;
            cout << "for instance color " << colorsJP[maxColUsed] << " was assigned to node " << maxColUsed << endl; //should never be -1

            break;
        }
        case 3:{
            vector<int> colorsSDLS = smallestDegreeLastSequentialAssignment(graph, colors, &maxColUsed);


            //some output just to be sure the application ran properly
            cout << "number of nodes: " << graph.size() << endl;
            cout << "number of SDLS colors: " << maxColUsed + 1 << endl;
            cout << "for instance color " << colorsSDLS[maxColUsed] << " was assigned to node " << maxColUsed << endl; //should never be -1

        }
        case 4:{
            vector<int> colorsSDLP = smallestDegreeLastParallelAssignment(graph, colors, &maxColUsed);

            //some output just to be sure the application ran properly
            cout << "number of nodes: " << graph.size() << endl;
            cout << "number of SDLP colors: " << maxColUsed + 1 << endl;
            cout << "for instance color " << colorsSDLP[maxColUsed] << " was assigned to node " << maxColUsed << endl; //should never be -1
        }

        case 5: {
            vector<int> colorsMIS = misSequentialAssignment(graph, colors, &maxColUsed);

            //some output just to be sure the application ran properly
            cout << "number of nodes: " << graph.size() << endl;
            cout << "number of MIS colors: " << maxColUsed + 1 << endl;
            cout << "for instance color " << colorsMIS[maxColUsed] << " was assigned to node " << maxColUsed << endl; //should never be -1
        }

        case 6: {
            vector<int> colorsMISP = misParallelAssignment(graph, colors, &maxColUsed);

            //some output just to be sure the application ran properly
            cout << "number of nodes: " << graph.size() << endl;
            cout << "number of MIS colors: " << maxColUsed + 1 << endl;
            cout << "for instance color " << colorsMISP[maxColUsed] << " was assigned to node " << maxColUsed << endl; //should never be -1
        }

        default:{
            cout << "Selected non-existing algorithm";
            break;
        }
    }

    chrono::time_point<chrono::system_clock> endTime = chrono::system_clock::now();
    cout << "Time taken: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << " milliseconds" << endl;

    maxColUsed = -1;

    maxColUsed = -1;

    return 0;
}
