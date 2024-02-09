#include <vector>
#include <climits>
#include <iostream>
#include "tsp.h"
using namespace std;

/*code from: https://github.com/sth144/christofides-algorithm-cpp/blob/master/main.cpp
	and adapted to my program with small changes
*/
vector<int> christofides(int n, vector<double>w){
    vector<int> solution(n);

	// Read file names from input
	//string input, output;
	//input = output = argv[1];
	//output.append(".tour");

	// Create new tsp object
	TSP tsp(n, w);
	//cout << "tsp created" << endl;
	int tsp_size = n;

	// Fill N x N matrix with distances between nodes
	/*cout << "Fillmatrix started" << endl;
	tsp.fillMatrix();
	cout << "Filled Matrix" << endl;*/

	// Find a MST T in graph G
	tsp.findMST();
	//cout << "MST created" << endl;

	// Find a minimum weighted matching M for odd vertices in T
	tsp.perfectMatching();
	//cout << "Matching completed" << endl;

	// Loop through each index and find shortest path
	int best = INT_MAX;
	int bestIndex;
	for (long t = 0; t < tsp_size; t++) {
		int result = tsp.findBestPath(t);

		tsp.path_vals[t][0] = t; // set start
		tsp.path_vals[t][1] = result; // set end

		if (tsp.path_vals[t][1] < best) {
			bestIndex = tsp.path_vals[t][0];
			best = tsp.path_vals[t][1];
		}
	}

	//Create path for best tour
	tsp.euler_tour(bestIndex,tsp.circuit);
	tsp.make_hamiltonian(tsp.circuit,tsp.pathLength);

	/*
	tsp.euler_tour(0, tsp.circuit);
	cout << "euler completed" << endl;
	tsp.make_hamiltonian(tsp.circuit, tsp.pathLength);
	cout << "ham completed" << endl;
	*/

	// Store best path
	//tsp.create_tour(bestIndex);

	//cout << "Final length: " << tsp.pathLength << endl;

	// Print to file
	/*tsp.printPath();
	tsp.printResult();*/
	int i=0;
	for (vector<int>::iterator it = tsp.circuit.begin(); it != tsp.circuit.end(); ++it) {
		solution[i++]=*it;
  	}

    return solution;
}

