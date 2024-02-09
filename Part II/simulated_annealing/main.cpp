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
//  make
//  ./main data/tsp50-2.dat parameters.dat
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

//get a random neighbour from the 2opt neighbourhood
vector<int> some_neighbour_2opt(vector<int> instance,int k){
    int n_nodes=instance.size();

    //draw two random indicies for the substring reverse
    srand (time(NULL)*k);
    int s = rand() % (n_nodes-2);
    int t = rand() % (n_nodes-1-s) +s+1;
    reverse(instance.begin() + s, instance.begin() + t+1);

    return instance;
}
//get a random neighbour from the 3opt neighbourhood (2opt included)
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
    //reference schemas/drawings: https://tsp-basics.blogspot.com/2017/03/3-opt-move.html
    //a=[z2 to x1] b=[x2 to y1] c=[y2 to z1]
    //7 (+1 original tour) cases for the swap
    //a,b,c swap points are randomly chosen as well as the swap type
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

int get_prob(double loss,double k, double iterations_range, double temp_desc){
    double temperature=(loss==0)? 0:(k/iterations_range)*temp_desc;
    //if loss is 0 (improving neighbour), probability is 1.
    double probability=exp(-temperature);
    //probability is scaled from [0,1] to [0,1000]
    return probability*1000;
}

int main (int argc, char const *argv[])
{
try{
    //args: instance_file.dat, parameters.dat
    if (argc<3) throw runtime_error("missing args");

    //parameters: 
    string garbage;//get the words from param file
    string param_file_name=argv[2];
    ifstream param_file(param_file_name);
    int n_multistart; //max number of multistart iterations
    int current_multistart=0; //multistart iterator
    param_file>>garbage;
    param_file >> n_multistart;
    double time_limit; //overall time limit
    param_file>>garbage;
    param_file >> time_limit;
    int iterations_range; //temperature parameter
    param_file >> garbage;
    param_file >> iterations_range;
    int max_non_accepted; //after some non accepted (no better neighbours and low temperature) stop
    param_file >> garbage;
    param_file >> max_non_accepted;
    double temp_desc; //temperature parameter, the higher the temp_par, the faster the temperature descends. it's the end point of e^-k
    param_file >> garbage;
    param_file >> temp_desc;
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

    /* print weights
    for(int i=0;i<n_nodes;i++){
        for(int j=0;j<n_nodes;j++){
            cout<<weights[i*n_nodes+j]<<'\t';
        }
        cout<<endl;
    }*/

    //print lower bound
    double lower_bound=one_tree_lower_bound(weights,n_nodes);
    cout<<"lower bound: "<<lower_bound;

    //set up time
    struct timeval  tv_start, tv_current;

    //multistart cylce
    bool stop=false;
    double time_difference=0;
    gettimeofday(&tv_start, NULL);

    
    vector<int> curr_sol(n_nodes); //current solution
    vector<int> best_sol(n_nodes);//best solution found so far inside the single multistart iteration
    vector<int> best_multistart_sol(n_nodes);//best solution over all multistart iterations
    vector<int> next_sol(n_nodes); //next solution (random neighbour), to be accepted
    curr_sol=christofides(n_nodes,weights); //first initial solution is christofides
    bool stopping_criteria; //stop the single multistart iteration if you reach the max of non accepted neighbours or you reach the time limit (=overall time limit/number of iteration)
    double curr_val,best_val,next_val;
    double best_multistart=objective_function(curr_sol,weights); //the value of the best solution found in all multistarts
    cout<<". upper bound: "<<best_multistart<<endl;
    int n_best_multistart=0; //where the best solution was found
    int k,k_non_accepted;
    double loss;
    bool accept;
    double outcome, probability;
    //multistart iterations:
    while(!stop){
        
        //create initial solution after the first one(christofifes)
        if(current_multistart!=0){
            //random initial solution
            unsigned seed = chrono::system_clock::now().time_since_epoch().count();
            shuffle(curr_sol.begin(), curr_sol.end(), default_random_engine(seed));
        }
        
        best_sol=curr_sol;
        k=0;
        k_non_accepted=0;
        curr_val=objective_function(curr_sol,weights);
        cout<<"m"<<current_multistart<<") start from: "<<curr_val;
        best_val=curr_val;
        stopping_criteria=false;
        //start the simulated annealing:
        while(!stopping_criteria){
            k++;
            //generate a (random) neighbour y from the 3opt neighbourhood of x
            next_sol=some_neighbour_3opt(curr_sol,k);
            next_val=objective_function(next_sol,weights);
            //check if y it's better than the best solution found so far
            if(next_val<best_val){
                best_val=next_val;
                best_sol=next_sol;
            }
            //the loss is zero if y is improving from x
            loss=(next_val<curr_val)? 0:next_val-curr_val;
            //calculate probability of acceptance
            srand (time(NULL)*k);
            outcome = rand() % 1000; //the probability is in a scale of [0,1000]
            probability= get_prob(loss,k,iterations_range, temp_desc);
            accept= (outcome<probability);

            if(accept){
                //if accepted, move to y
                curr_sol=next_sol;
                curr_val=next_val;
                //if(loss!=0){ cout<<"accepted a bad neighbour, k:"<<k<<endl;}
                k_non_accepted=0;
            } else {
                //if not accepted increase the counter of consecutive non accepted moves
                k_non_accepted++;
            }
            //go to the next multistart if the limit of non improving iterations is reached
            if(k_non_accepted>max_non_accepted){
                stopping_criteria=true;
            }

            //get the time
            gettimeofday(&tv_current, NULL);
            time_difference = (double)(tv_current.tv_sec+tv_current.tv_usec*1e-6 - (tv_start.tv_sec+tv_start.tv_usec*1e-6));
            //go to next multistart if time limit is reached
            if(time_difference>(time_limit/n_multistart)*(current_multistart+1)){
                stopping_criteria=true;
            }

            //stop everything if overall time limit exceed
            if(time_difference>time_limit){
                stop=true;
            }
        }

        cout<<" -> found: "<<best_val<<endl;
        //check if the best solution is better than the other multistarts
        if(best_val<best_multistart){
            best_multistart=best_val;
            n_best_multistart=current_multistart;
            best_multistart_sol=curr_sol;
        }
        current_multistart++;

        //stop if you did all multistarts
        if(current_multistart>=n_multistart) stop=true;
    }

    cout<<"--------------\n\n The best solution was found in #"<<n_best_multistart<<" multistart, value= "<<best_multistart<<endl;
    double ratio_to_lb=best_multistart/lower_bound;
    cout<<"ratio to lower bound: "<<ratio_to_lb<<endl;
    cout<<"solution: "<<endl;
    printsol(best_multistart_sol);

    return 0;

}
catch(exception& e)
{
    cout << ">>>EXCEPTION: " << e.what() << std::endl;
}
    return 0;
}