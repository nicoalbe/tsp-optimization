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
//  make main.exe (or just "make")
//  ./main tsp5.dat parameters.dat
using namespace std;

void printsol(vector<int> sol){
    int n=sol.size();
    for(int i=0;i<n;i++){
        cout<<sol[i]<<" ";
    }
    cout<<endl;
}

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
//TODO: fix incremental evaluation (commented)
vector<int> first_improvement_2optneighbour(vector<int> instance, vector<double>weights){
    int n_nodes=instance.size();
    bool found_better_neighbour=false;
    vector<int> curr_n;
    double neighbour_val;
    double instance_val=objective_function(instance,weights);
    //int before_s,after_t;
    for(int s=0;s<n_nodes-1 &&!found_better_neighbour;s++){
        for (int t=s+1;t<n_nodes &&!found_better_neighbour;t++){
            curr_n=instance;
            reverse(curr_n.begin() + s, curr_n.begin() + t+1);
            neighbour_val=objective_function(curr_n,weights);
            //before_s=(s==0)? n_nodes-1:s-1;
            //after_t=(t==n_nodes-1)? 0:t+1;
            //cout<<"pre s "<<before_s<<" s "<<s<<" t "<<t<<" post t "<<after_t<<endl;
            //neighbour_val=instance_val -weights[before_s*n_nodes+s] -weights[t*n_nodes+after_t] +weights[before_s*n_nodes+t] +weights[s*n_nodes+after_t];
            if(neighbour_val<instance_val){
                found_better_neighbour=true;
                reverse(instance.begin() + s, instance.begin() + t+1);
                //cout<<"from "<<instance_val<<" i found a better neighbour: "<<neighbour_val<<endl;
                //cout<<"neigh "<<objective_function(instance,weights);
            }
        }
    }
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

    //parameters: 
    string garbage;//get the words from param file
    string param_file_name=argv[2];
    ifstream param_file(param_file_name);
    int n_multistart;
    int current_multistart=0;
    param_file>>garbage;
    param_file >> n_multistart;
    double time_limit;
    param_file>>garbage;
    param_file >> time_limit;
    int max_iterations; //iteration counter
    param_file >> garbage;
    param_file >> max_iterations;
    param_file.close();

    //get instance
    string instance_file_name=argv[1];
    ifstream instance_file(instance_file_name);
    int n_nodes;
    instance_file >> n_nodes;
    vector<double> weights (n_nodes*n_nodes);
    for(int i=0;i<n_nodes*n_nodes;i++){
        instance_file >> weights[i];
    }

    instance_file.close();

    //print lower bound
    double lower_bound=one_tree_lower_bound(weights,n_nodes);
    cout<<"lower bound: "<<lower_bound<<endl;

    //set up time
    struct timeval  tv_start, tv_current;

    //multistart cylce
    bool stop=false;
    double time_difference=0;
    gettimeofday(&tv_start, NULL);

    
    vector<int> curr_sol(n_nodes); //current solution
    vector<int> next_sol(n_nodes);
    curr_sol=christofides(n_nodes,weights); //first initial solution is christofides
    bool stopping_criteria;
    double curr_val,next_val;
    double best_multistart=objective_function(curr_sol,weights);
    int n_best_multistart=0;
    double loss;
    //bool intensification=true;
    bool accept;
    double outcome, probability;
    while(!stop){
        
        //create initial solution
        if(current_multistart!=0){
            //random initial solution
            for(int i=0;i<n_nodes;i++) curr_sol[i]=i;
            unsigned seed = chrono::system_clock::now().time_since_epoch().count();
            shuffle(curr_sol.begin(), curr_sol.end(), default_random_engine(seed));
        }
        
        curr_val=objective_function(curr_sol,weights);
        cout<<"start obj val: "<<curr_val<<endl;
        stopping_criteria=false;
        while(!stopping_criteria){
            //find first improvement
            next_sol=first_improvement_2optneighbour(curr_sol,weights);
            next_val=objective_function(next_sol,weights);
            if(next_sol[0]!=next_sol[1]){
                curr_sol=next_sol;
                curr_val=next_val;
                //cout<<"next val "<<next_val<<endl;
            } else {
                stopping_criteria=true;
                cout<<"exit for no better neighbour"<<endl;
            }
            //calculate stopping criteria
            //get the time
            gettimeofday(&tv_current, NULL);
            time_difference = (double)(tv_current.tv_sec+tv_current.tv_usec*1e-6 - (tv_start.tv_sec+tv_start.tv_usec*1e-6));
            //stop if time limit exceed
            if(time_difference>time_limit){
                stop=true;
                cout<<"exit for time limit"<<endl;
            }
        }

        cout<<"best val(m"<< current_multistart <<"): "<<curr_val<<endl;
        if(curr_val<best_multistart){
            best_multistart=curr_val;
            n_best_multistart=current_multistart;
        }
        current_multistart++;
        //stop if you did all multistarts
        if(current_multistart>=n_multistart) stop=true;
    }

    cout<<"\n--------------\nbest value: "<<best_multistart<<"(multistart "<<n_best_multistart<<")"<<endl;
    double ratio_to_lb=best_multistart/lower_bound;
    cout<<"ratio to lower bound: "<<ratio_to_lb<<endl;
    cout<<" - - -\n fin\n - - -";
    cout<<"\n--------------\n: "<<endl;

    return 0;

}
catch(exception& e)
{
    cout << ">>>EXCEPTION: " << e.what() << std::endl;
}
    return 0;
}