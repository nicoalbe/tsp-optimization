/**
 * @file tsp.cpp
 * @brief 
 */

#include <cstdio>
#include <iostream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "cpxmacro.h"
#include <ctime>
#include <sys/time.h>

using namespace std;
/*
	notation
	N (or n): number of nodes
	A: the set of edges where (i,j) is the edge between node i and node j
	C: adjacency matrix of the weights of the edges in A. C_i_j is the weight of the edge from node i to node j
		this matrix is symmetric because the graph is undirected
		the values in the diagonal are zeros
*/

// error status and messagge buffer
int status;
char errmsg[BUF_SIZE];


const int NAME_SIZE = 512;
char name[NAME_SIZE];

void setupLP(CEnv env, Prob lp, const char* filename)
{
	//read n (number of nodes) and C (adjacency matrix) from file .dat

	int N;
	std::ifstream in(filename);
    in >> N;
	double C[N*N];
	for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			in >> C[i*N+j];
		}
	}

	//print weights
	/*for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			cout<<C[i*N+j]<<"\t";
		}
		cout<<endl;
	}*/

	//set name of nodes as 0, 1, 2 .. n
	int nameN[N];
	for(int n=0;n<N;n++)
    	nameN[n]=n;

	//mapping for reading the correct position of the variables x's and y's
    vector<vector<int>> map_x;
    map_x.resize(N);
    for ( int i = 0 ; i < N ; ++i ) {
        map_x[i].resize(N);
        for ( int j = 0 ; j < N ; ++j ) {
            map_x[i][j] = -1;
        }
    }
    vector<vector<int>> map_y;
    map_y.resize(N);
    for ( int i = 0 ; i < N ; ++i ) {
        map_y[i].resize(N);
        for ( int j = 0 ; j < N ; ++j ) {
            map_y[i][j] = -1;
        }
    }
	
    int current_var_position=0; //used for mapping

	// add x vars, creating variables one by one
	for (int i = 0; i < N; i++)
	{
		for (int j = 1; j < N; j++) // do not create variables x_i_0 or x_i_i for each i
		{
            if(i==j) continue;
			char xtype = 'C';	//continuous positive variables
			double lb = 0.0;
			double ub = CPX_INFBOUND;
			snprintf(name, NAME_SIZE, "x_%u_%u", nameN[i], nameN[j]);
			char* xname = (char*)(&name[0]);
			double obj=0; //not in the obj fun
			CHECKED_CPX_CALL( CPXnewcols, env, lp, 1   , &obj, &lb, &ub, &xtype, &xname );
			map_x[i][j] = current_var_position ++;
		}
	}
	// add y vars
	for (int i =0; i<N; i++){
		for (int j=0; j<N; j++){
            if(i==j) continue;
			char xtype = 'B'; //binary var
			double lb = 0;
			double ub = 1;
			snprintf(name, NAME_SIZE, "y_%u_%u", nameN[i], nameN[j]);
			char* xname = (char*)(&name[0]);
			CHECKED_CPX_CALL( CPXnewcols, env, lp, 1   , &C[i*N+j], &lb, &ub, &xtype, &xname );
            map_y[i][j] = current_var_position ++;
		}
	}
	
	// add flow in/out constraint: for each node: incoming flow - outgoing flow = 1
    //	for all k in N\{0} 
    //	sum (over i s.t.(i,k) in A) x_ik  -  sum (over j s.t.(k,j)in A and j!=start_node)  = 1 
	for (int k = 1; k < N; k++)
	{
		vector<int> idx;
		vector<double> coef;
		char sense = 'E';//equal
        //sum of incoming
		for (int i = 0; i < N; i++)
		{
			if(map_x[i][k]==-1) continue;
			idx.push_back(map_x[i][k]); // corresponds to the variable x_ik
            coef.push_back(1.0);
		}
        //sum of outgoing, subtracted
        for(int j=0;j<N;j++){ 
			if(map_x[k][j]==-1) continue; //not considering j=start_node
            idx.push_back(map_x[k][j]); //corresponds to the variable x_kj
            coef.push_back(-1.0); //subtracted
        }
        double rhs=1; //sum equals to 1
		int matbeg = 0;
		CHECKED_CPX_CALL( CPXaddrows, env, lp, 0     , 1     , idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL      , NULL      );
	}

	// add outgoing contstrain: for each node only one outgoing edge is allowed
    //	for all i in N
    //	sum (over j s.t.(i,j) in A) y_ij = 1
	for (int i = 0; i < N; i++)
	{
		vector<int> idx;
		vector<double> coef;
		char sense = 'E';//equals
		for (int j = 0; j < N; j++)
		{
			if(map_y[i][j]==-1) continue;
			idx.push_back(map_y[i][j]); // corresponds to the variable y_ij
            coef.push_back(1.0);
		}
        double rhs=1;//equals to 1
		int matbeg = 0;
		CHECKED_CPX_CALL( CPXaddrows, env, lp, 0     , 1     , idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL      , NULL      );
	}

    // add incoming contstrain: for each node only one incoming edge is allowed
    //	for all j in N
    //	sum (over i s.t.(i,j) in A) y_ij = 1
	for (int j = 0; j < N; j++)
	{
		vector<int> idx;
		vector<double> coef;
		char sense = 'E';//equals
		for (int i = 0; i < N; i++)
		{
			if(map_y[i][j]==-1) continue;
			idx.push_back(map_y[i][j]); // corresponds to the variable y_ij
            coef.push_back(1.0);
		}
        double rhs=1;//equals to 1
		int matbeg = 0;
		CHECKED_CPX_CALL( CPXaddrows, env, lp, 0     , 1     , idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL      , NULL      );
	}

    //add constraints for activation of y variables
    //	for all (i,j) in A, j!=0
    //	x_ij<=(|N|-1)y_ij  ==>   x_ij + (1-|N|) y_ij <= 0
	for (int i = 0; i < N; i++){
        for(int j=1; j<N; j++){
			if(map_x[i][j]==-1) continue;
            vector<int> idx = {map_x[i][j],map_y[i][j]}; //variables x_ij, y_ij
            vector<double> coef = {1,1.0-N};
            char sense = 'L';//less or equal
            double rhs=0;
            int matbeg = 0;//less or equal to 0
            CHECKED_CPX_CALL( CPXaddrows, env, lp, 0     , 1     , idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL      , NULL      );
        }
	}
	
	// print (debug)
	CHECKED_CPX_CALL( CPXwriteprob, env, lp, "tsp.lp", NULL );
}


int main (int argc, char const *argv[])
{
	try
	{
		cout << "Inizialize and set up... ";
		// initialize
		DECL_ENV( env );
		DECL_PROB( env, lp );
		// setup LP
		setupLP(env, lp, argv[1]);
		cout << "ok " << endl;

		cout << "Optimizing... ";
		//start time
		struct timeval  tv1, tv2;
    	gettimeofday(&tv1, NULL);
		// optimize
		CHECKED_CPX_CALL( CPXmipopt, env, lp );
		
		//stop time
    	gettimeofday(&tv2, NULL);
		
		cout << "ok " << endl;

		//get variable values
		int n = CPXgetnumcols(env, lp);
		std::vector<double> varVals;
		varVals.resize(n);
  		CHECKED_CPX_CALL( CPXgetx, env, lp, &varVals[0], 0, n - 1 );

		//get variable names
		int begin=0;
		char cur_colnamestore[n*9]; //n vars, at most 9 char each var: x_999_999
		int cur_storespace=n*9;
		char *cur_colname[n*9]; //pointers to the beginning of each name
		int surplus=0;
		CHECKED_CPX_CALL( CPXgetcolname, env, lp,  cur_colname, cur_colnamestore, cur_storespace, &surplus, begin, n-1);
		for ( int i = 0 ; i < n ; ++i ) {
			string name="";
			for(int k=0;cur_colname[i][k];k++){
				name+=cur_colname[i][k];
			}
			//print only variables that are used (value >0)
			if(varVals[i]>0) std::cout << "var " << name << " : " << varVals[i] << std::endl;
  	  	}

		// print objective value
		double objval;
		CHECKED_CPX_CALL( CPXgetobjval, env, lp, &objval );
		std::cout << "Optimal solution value: " << objval << std::endl;

		//print time
		std::cout << "time: " << (double)(tv2.tv_sec+tv2.tv_usec*1e-6 - (tv1.tv_sec+tv1.tv_usec*1e-6)) << " seconds (user time)\n";
		
		CHECKED_CPX_CALL( CPXsolwrite, env, lp, "tsp.sol" );
		// free
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
	}
	catch(std::exception& e)
	{
		std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
	}
	return 0;
}