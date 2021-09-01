#include "algorithmJP.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <chrono>

using namespace std;

mutex jpmutex;
condition_variable cvJPColor;
int semJPColor;

//Starting with a sequential implementation of Jones-Plassman algorithm to check functionality and show a baseline
vector<int> jonesPlassmannSequentialAssignment(map<int, list<int>>& graph, map<int, int>& graphNumberMap, vector<int> colors, int* maxColUsed) {

    int coloredNodes = 0;

    while (coloredNodes < colors.size()) {

        for (auto const& node : graph) {
            if (colors[node.first] == -1) {

                //Find out if I'm the max weighted node amongst uncoloured neighbours
                bool maxNode = true;
                vector<int> neighbourColors = {};

                for (auto const& neighbour : node.second) {
                    if (colors[neighbour] == -1) {
                        if (graphNumberMap[node.first] < graphNumberMap[neighbour]) {
                            maxNode = false;
                            break;
                        }
                        else if (graphNumberMap[node.first] == graphNumberMap[neighbour]) {
                            if (node.first < neighbour) {
                                maxNode = false;
                                break;
                            }
                        }
                    }
                    else {
                        neighbourColors.push_back(colors[neighbour]);
                    }
                }

                if (maxNode) {
                    // Colour node
                    int colour = 0;
                    while (find(neighbourColors.begin(), neighbourColors.end(), colour) != neighbourColors.end()) {
                        /* neighborColors contains selectedCol */
                        colour++;
                    }
                    if (colour > *maxColUsed) {
                        *maxColUsed = colour;
                    }
                    colors[node.first] = colour;
                    coloredNodes++;
                }

            }

        }

    }

    return colors;
}

//Parallel implementation of Jones-Plassman algorithm
vector<int> jonesPlassmannParallelAssignment(map<int, list<int>>& graph, map<int, int>& graphNumberMap, vector<int> colors, int* maxColUsed, int nThreads) {

    int coloredNodes = 0;

    //Get maximum number of threads for the system
    int maxThreads = (nThreads <= 0 || nThreads > thread::hardware_concurrency()) ? thread::hardware_concurrency() : nThreads ;
    cout << "Max threads supported: " << maxThreads << endl;
    semJPColor = maxThreads;

    vector<thread> workers;

    for (int i = 0; i < maxThreads; i++) {

        //Create threads to find and colour independent sets
        workers.emplace_back([&graph, &graphNumberMap, &colors, i, maxThreads, &maxColUsed, &coloredNodes] {jpVertexColouring(graph, graphNumberMap, colors, i, maxThreads, maxColUsed, &coloredNodes); });

    }

    //Wait for all threads to finish
    for (auto& worker : workers) {
        worker.join();
    }

    return colors;

}

void jpVertexColouring(map<int, list<int>>& graph, map<int, int>& graphNumberMap, vector<int>& colors, int startOffset, int stepSize, int* maxColUsed, int* coloredNodes) {
    
    int numThreads = stepSize; 

    while (*coloredNodes < colors.size()) {

        // Step through the nodes with the step size of number of threads
        map<int, list<int>>::iterator iter = graph.begin();
        advance(iter, startOffset);

        int i = startOffset;

        do {
            if (colors[iter->first] == -1) {

                //Find out if I'm the max weighted node amongst uncoloured neighbours
                bool maxNode = true;
                vector<int> neighbourColors = {};

                for (auto const& neighbour : iter->second) {
                    if (colors[neighbour] == -1) {
                        if (graphNumberMap[iter->first] < graphNumberMap[neighbour]) {
                            maxNode = false;
                            break;
                        }
                        else if (graphNumberMap[iter->first] == graphNumberMap[neighbour]) {
                            if (iter->first < neighbour) {
                                maxNode = false;
                                break;
                            }
                        }
                    }
                    else {
                        neighbourColors.push_back(colors[neighbour]);
                    }
                }

                if (maxNode) {
                    // Colour node
                    int colour = 0;
                    while (find(neighbourColors.begin(), neighbourColors.end(), colour) != neighbourColors.end()) {
                        /* neighborColors contains selectedCol */
                        colour++;
                    }
                    if (colour > *maxColUsed) {
                        *maxColUsed = colour;
                    }
                    jpmutex.lock();
                    colors[iter->first] = colour;
                    *coloredNodes = *coloredNodes + 1;
                    jpmutex.unlock();
                }

            }

            i += stepSize;
            //Go to next node of thread
            if (i >= graph.size()) {
                break;
            }
            advance(iter, stepSize);

        } while (i < graph.size());

        // Signal completion of colouring set
        {
            unique_lock<mutex> lock(jpmutex);
            semJPColor--;
            if (semJPColor == 0) {
                semJPColor = numThreads;
                cvJPColor.notify_all();
            }
            else {
                // Wait here for other threads to finish colouring
                cvJPColor.wait(lock);
            }
        } 

    }

}

////Parallel implementation of Jones-Plassman algorithm
//vector<int> jonesPlassmannParallelAssignment(map<int, list<int>>& graph, vector<int> colors, int* maxColUsed) {
//
//    //In the main thread assign a random number to each vertex
//    chrono::time_point<chrono::system_clock> mappingStart = chrono::system_clock::now();
//
//    map<int, int> graphNumberMap = {};
//    for (auto const& node : graph) {
//        graphNumberMap[node.first] = rand();
//    }
//
//    chrono::time_point<chrono::system_clock> mappingEnd = chrono::system_clock::now();
//    cout << "Time taken to assign random number map: " << chrono::duration_cast<chrono::milliseconds>(mappingEnd - mappingStart).count() << " milliseconds" << endl;
//
//    map<int, list<int>> uncoloredNodes = graph;
//    map<int, list<int>> uncoloredNodesTemp = graph;
//
//    //Get maximum number of threads for the system
//    int maxThreads = thread::hardware_concurrency();
//    cout << "Max threads supported: " << maxThreads << endl;
//    semJPColor = maxThreads;
//    semJPNodes = maxThreads;
//
//    vector<thread> workers;
//
//    for (int i = 0; i < maxThreads; i++) {
//
//        //Create threads to find and colour independent sets
//        workers.emplace_back([&uncoloredNodes, &uncoloredNodesTemp, &graphNumberMap, &colors, i, maxThreads, &maxColUsed] {jpVertexColouring(uncoloredNodes, uncoloredNodesTemp, graphNumberMap, colors, i, maxThreads, maxColUsed); });
//
//    }
//
//    //Wait for all threads to finish
//    for (auto& worker : workers) {
//        worker.join();
//    }
//
//    //*maxColUsed = colour;
//
//    return colors;
//}
//
//void jpVertexColouring(map<int, list<int>>& uncoloredNodes, map<int, list<int>>& uncoloredNodesTemp, map<int, int>& graphNumberMap, vector<int>& colors, int startOffset, int stepSize, int* maxColUsed) {
//
//    int numThreads = stepSize;
//
//    while (uncoloredNodes.size() > startOffset) {
//
//        map<int, list<int>> independentSet{};
//
//        // Step through the nodes with the step size of number of threads
//        map<int, list<int>>::iterator iter = uncoloredNodes.begin();
//        advance(iter, startOffset);
//
//        int i = startOffset;
//
//        do {
//            //Find out if I'm the max weighted node amongst uncoloured neighbours
//            bool maxNode = true;
//            vector<int> neighbourColors = {};
//
//            for (auto const& neighbour : iter->second) {
//                if (colors[neighbour] == -1) {
//                    if (graphNumberMap[iter->first] < graphNumberMap[neighbour]) {
//                        maxNode = false;
//                        break;
//                    }
//                    else if (graphNumberMap[iter->first] == graphNumberMap[neighbour]) {
//                        if (iter->first < neighbour) {
//                            maxNode = false;
//                            break;
//                        }
//                    }
//                }
//                else {
//                    neighbourColors.push_back(colors[neighbour]);
//                }
//            }
//
//            if (maxNode) {
//                independentSet.emplace(*iter);
//                // Colour node
//                int colour = 0;
//                while (find(neighbourColors.begin(), neighbourColors.end(), colour) != neighbourColors.end()) {
//                    /* neighborColors contains selectedCol */
//                    colour++;
//                }
//                if (colour > *maxColUsed) {
//                    *maxColUsed = colour;
//                }
//                jpmutex.lock();
//                colors[iter->first] = colour;
//                uncoloredNodesTemp.erase(iter->first);
//                jpmutex.unlock();
//            }
//
//            i += stepSize;
//            //Go to next node of thread
//            if (i >= uncoloredNodes.size()) {
//                break;
//            }
//            advance(iter, stepSize);
//
//        } while (i < uncoloredNodes.size());
//
//        // Signal completion of colouring set
//        {
//            unique_lock<mutex> lock(jpmutex);
//            semJPColor--;
//            if (semJPColor == 0) {
//                uncoloredNodes = uncoloredNodesTemp;
//                if (uncoloredNodes.size() < numThreads) {
//                    semJPColor = uncoloredNodes.size();
//                }
//                else {
//                    semJPColor = numThreads;
//                }
//                cvJPColor.notify_all();
//            }
//            else {
//                // Wait here for other threads to finish colouring
//                cvJPColor.wait(lock);
//            }
//        }
//
//    }
//
//}

