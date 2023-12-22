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

vector<int> some_neighbour_2opt(vector<int> instance,int k){
    int n_nodes=instance.size();

    //draw two number idicies for the substring reverse
    srand (time(NULL)*k);
    int s = rand() % (n_nodes-2);
    int t = rand() % (n_nodes-1-s) +s+1;
    reverse(instance.begin() + s, instance.begin() + t+1);

    return instance;
}

vector<int> some_neighbour_3opt(vector<int> instance, int k){
    srand (time(NULL)*k);
    int n_nodes=instance.size();
    int type=rand()%7;
    int x1,x2,y1,y2,z1,z2;
    z2=rand()%(n_nodes-2);
    x1=rand()%(n_nodes-1);
    y1=rand()%(n_nodes);
    x2=x1+1;
    y2=y1+1;
    z1=(z2==0)? n_nodes-1 : z2-1;
    //a=[z2 to x1] b=[x2 to y1] c=[y2 to z1]
    //7 (+1 original tour) cases for the swap
    switch(type){
        case 0://a'bc
            reverse(instance.begin()+z2,instance.begin()+x1+1);
        break;
        case 1://abc'
            reverse(instance.begin()+y2,instance.begin()+z1+1);
        break;
        case 2://ab'c
            reverse(instance.begin()+x2,instance.begin()+y1+1);
        break;
        case 3://ab'c'
            reverse(instance.begin()+x2,instance.begin()+y1+1);
            reverse(instance.begin()+y2,instance.begin()+z1+1);
        break;
        case 4://a'b'c
            reverse(instance.begin()+z2,instance.begin()+x1+1);
            reverse(instance.begin()+x2,instance.begin()+y1+1);
        break;
        case 5://a'bc'
            reverse(instance.begin()+z2,instance.begin()+x1+1);
            reverse(instance.begin()+y2,instance.begin()+z1+1);
        break;
        case 6://a'b'c'
            reverse(instance.begin()+z2,instance.begin()+x1+1);
            reverse(instance.begin()+y2,instance.begin()+z1+1);
            reverse(instance.begin()+x2,instance.begin()+y1+1);
        break;
    }
    return instance;
}

int get_prob(double loss,double k, double max_k){
    /*double probability;
    double temperature=(loss==0)? 1:1-((k+1)/max_k);
    probability=exp(-(loss/temperature))*1000;
    return probability;*/
    double temperature=(loss==0)? 0:(k/max_k)*10;
    double probability=exp(-temperature);
    return probability*1000;
}

//todo some neighbour3opt (considering also 2opt)

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
    int max_non_improving_iterations;
    param_file >> garbage;
    param_file >> max_non_improving_iterations;
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

    /* print weights
    for(int i=0;i<n_nodes;i++){
        for(int j=0;j<n_nodes;j++){
            cout<<weights[i*n_nodes+j]<<'\t';
        }
        cout<<endl;
    }*/
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
    vector<int> best_sol(n_nodes);
    vector<int> next_sol(n_nodes);
    curr_sol=christofides(n_nodes,weights); //first initial solution is christofides
    bool stopping_criteria;
    double curr_val,best_val,next_val;
    double best_multistart=objective_function(curr_sol,weights);
    int n_best_multistart=0;
    int k, k_non_improving; //non improving is consecutive with the acceptance
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
        
        //do simulated annealing here
        best_sol=curr_sol;
        k=0;
        k_non_improving=0;
        curr_val=objective_function(curr_sol,weights);
        cout<<"start obj val: "<<curr_val<<endl;
        best_val=curr_val;
        stopping_criteria=false;
        while(!stopping_criteria){
            k++;
            //generate a (random) neighbour y
            next_sol=some_neighbour_3opt(curr_sol,k);
            next_val=objective_function(next_sol,weights);
            //cout<<"next val "<<next_val<<endl;
            if(next_val<best_val){
                best_val=next_val;
                best_sol=next_sol;
            }
            loss=(next_val<curr_val)? 0:next_val-curr_val;
            //calculate probability of acceptance
            srand (time(NULL)*k);
            outcome = rand() % 1000;
            probability= get_prob(loss,k,max_iterations);
            //cout<<"prob "<<probability<<endl;
            accept= (outcome<probability);
            //cout<<"generated "<<next_val<<" accepting with prob "<<probability<<endl;
            //cout<<"outcome "<<outcome<<" prob "<<probability<<endl;
            if(accept){
                curr_sol=next_sol;
                curr_val=next_val;
                k_non_improving=0;
                //cout<<"accepted "<<curr_val<<endl;
            } else {
                k_non_improving++;
            }
            //calculate stopping criteria
            if(k>max_iterations){//if(k>max_iterations || k_non_improving>max_non_improving_iterations){
                stopping_criteria=true;
            }
            //TODO no futher neighbours, stop
        }

        cout<<"best val(m"<< current_multistart <<"): "<<best_val<<endl;
        if(best_val<best_multistart){
            best_multistart=best_val;
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

    return 0;

}
catch(exception& e)
{
    cout << ">>>EXCEPTION: " << e.what() << std::endl;
}
    return 0;
}