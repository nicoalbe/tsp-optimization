#include <ctime>
#include <sys/time.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <limits>
#include <math.h>
#include "./methods/christofides.h"
#include "methods/one-tree.h"

//to compile: 
//  make main.exe
//  ./main tsp50-2.dat parameters.dat
using namespace std;

//print the solution (vector of visited nodes in order)
void printsol(vector<int> sol){
    int n=sol.size();
    for(int i=0;i<n;i++){
        cout<<sol[i]<<" ";
    }
    cout<<endl;
}

//objective function
double objective_function(vector<int> instance, vector<double> weights){
    double obj_fun=0;
    int n_nodes=instance.size();
    instance.push_back(instance[0]); //add the first node at the end, to make a cycle
    for(int i=0;i<n_nodes;i++){
        obj_fun+= weights[instance[i]*n_nodes+instance[i+1]];
        //cout<<"from "<<instance[i]<<" to "<<instance[i+1]<<" : "<<weights[instance[i]*n_nodes+instance[i+1]]<<endl;
    }
    return obj_fun;
}

//return the first improvement neighour, if it doesn't exists it returns a vector where the fist two elements are 0
vector<int> first_improvement_2optneighbour(vector<int> instance, vector<double>weights){
    int n_nodes=instance.size();
    bool found_better_neighbour=false;
    vector<int> curr_n;
    double neighbour_val;
    double instance_val=objective_function(instance,weights);
    
    //look in the 2opt neighbourhood as subvector reverse from s to t.
    for(int s=0;s<n_nodes-1 &&!found_better_neighbour;s++){
        for (int t=s+1;t<n_nodes &&!found_better_neighbour;t++){
            curr_n=instance;
            reverse(curr_n.begin() + s, curr_n.begin() + t+1);
            neighbour_val=objective_function(curr_n,weights);
            //as soon as it find a better neighbour, stop
            if(neighbour_val<instance_val){
                found_better_neighbour=true;
                reverse(instance.begin() + s, instance.begin() + t+1);
            }
        }
    }
    //if no better neighbour is found, we are in the local optimum, return 0,0 in the first two positions
    if(!found_better_neighbour){
        instance[1]=instance[0]=0;
    }
    return instance;
}

//a simple local search with first improvement and no way to escape from local optimal
int main (int argc, char const *argv[])
{
try{
    //args: instance_file.dat, parameters.dat
    if (argc<3) throw runtime_error("missing args");

    //get parameters: 
    string garbage; //get out the words from param file
    string param_file_name=argv[2];
    ifstream param_file(param_file_name);
    double time_limit;
    param_file>>garbage;
    param_file >> time_limit;
    param_file.close();

    //get instance
    string instance_file_name=argv[1];
    ifstream instance_file(instance_file_name);
    int n_nodes;
    instance_file >> n_nodes;
    //adjacency matrix
    vector<double> weights (n_nodes*n_nodes);
    for(int i=0;i<n_nodes*n_nodes;i++){
        instance_file >> weights[i];
    }

    instance_file.close();

    //print lower bound
    double lower_bound=one_tree_lower_bound(weights,n_nodes);
    cout<<"lower bound: "<<lower_bound;

    //set up time
    struct timeval  tv_start, tv_current;

    //multistart cylce
    bool stop=false; //stop the overall program at time limit
    double time_difference=0;
    gettimeofday(&tv_start, NULL);
    
    vector<int> curr_sol(n_nodes); //current solution
    vector<int> next_sol(n_nodes); //first improvement neighbour
    vector<int> best_multistart_sol(n_nodes); //best solution over all multistart
    curr_sol=christofides(n_nodes,weights); //first initial solution is christofides
    bool stopping_criteria; //stop the current multistart iteration when local optimum is reached
    double curr_val,next_val;
    double best_multistart=objective_function(curr_sol,weights); //the best value found in all iterations of the multistarts
    cout<<". upper bound: "<<best_multistart<<endl;
    int n_best_multistart=0; //the multistart iteration in which the best value was found
    
    int current_multistart=0; //multistart iteration
    while(!stop){
        
        //create initial solution after the first (christofides)
        if(current_multistart!=0){
            //random initial solution
            unsigned seed = chrono::system_clock::now().time_since_epoch().count();
            shuffle(curr_sol.begin(), curr_sol.end(), default_random_engine(seed));
        }
        
        curr_val=objective_function(curr_sol,weights);
        cout<<"multistart "<<current_multistart<<": start from "<<curr_val;
        stopping_criteria=false;

        while(!stopping_criteria){
            //find first improvement
            next_sol=first_improvement_2optneighbour(curr_sol,weights);
            //next_val=objective_function(next_sol,weights);
            
            if(next_sol[0]==next_sol[1]){
                //if local optimum reached, stop this multistart
                stopping_criteria=true;
                curr_val=objective_function(curr_sol,weights);
            } else {
                //if there's a better neighbour, move there
                curr_sol=next_sol;
            }

            //get the time
            gettimeofday(&tv_current, NULL);
            time_difference = (double)(tv_current.tv_sec+tv_current.tv_usec*1e-6 - (tv_start.tv_sec+tv_start.tv_usec*1e-6));
            //stop if time limit exceed
            if(time_difference>time_limit){
                stop=true;
            }
        }

        cout<<" -> found "<<curr_val<<endl;
        //check if the local optimum is the best found so far
        if(curr_val<best_multistart){
            best_multistart=curr_val;
            n_best_multistart=current_multistart;
            best_multistart_sol=curr_sol;
        }
        current_multistart++;
    }
    
    cout<<"--------------\n\nThe best solution was found in #"<<n_best_multistart<<" multistart: "<<best_multistart<<endl;
    double ratio_to_lb=best_multistart/lower_bound;
    cout<<"ratio to lower bound: "<<ratio_to_lb<<endl;
    cout<<"best solution: "<<endl;
    printsol(best_multistart_sol);
    return 0;

}
catch(exception& e)
{
    cout << ">>>EXCEPTION: " << e.what() << std::endl;
}
    return 0;
}