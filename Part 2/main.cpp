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
#include "methods/christofides.h"
#include "methods/one-tree.h"

//to compile: 
//  make main.exe (or just "make")
//  ./main tsp5.dat parameters.dat
using namespace std;

bool equal(vector<int>a, vector<int>b){
    //asssume same size
    int n=a.size();
    vector<int> ar=a;
    a.push_back(a[0]);
    b.push_back(b[0]);
    //check in normal order
    bool ok=true;
    int nexta;
    vector<int>::iterator nextb;
    for(int i=0;i<n&&ok;i++){
        //check for each node i, the next one nexta
        nexta=a[i+1];
        //cout<<"edgeA "<<a[i]<<" "<<a[i+1];
        //check if the node i in the path b is followd by nexta
        nextb=find (b.begin(), b.end(), a[i]);
        //cout<<" - edgeB "<<*nextb<<" ";
        nextb++;
        //cout<<*nextb<<endl;
        if (*nextb!=nexta) {
            ok=false;
            //cout<<"assign false";
        }
    }
    //check in reversed order TODO revers not working
    if(!ok){
        reverse(ar.begin(),ar.end());
        ar.push_back(ar[0]);
        ok=true;
        for(int i=0;i<n&&ok;i++){
            //check for each node i, the next one nexta
            nexta=ar[i+1];
            //check if the node i in the path b is followd by nexta
            nextb=find (b.begin(), b.end(), ar[i]);
            //cout<<" - edgeB "<<*nextb<<" ";
            nextb++;
            //cout<<*nextb<<endl;
            if (*nextb!=nexta) {
                ok=false;
                //cout<<"assign false2"<<endl;
            }
        }
    }
    return ok;
}

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

vector<int> best_neighbour_2opt(vector<int> instance, vector<double> weights, vector<vector<int>> tabu_list){
    int n_nodes=instance.size();
    vector<int> best_n (n_nodes);
    vector<int> curr_n (n_nodes);
    double best_val=numeric_limits<double>::max();
    double curr_val=0;
    
    for(int s=0;s<n_nodes-1;s++){
        for (int t=s+1;t<n_nodes;t++){
            //substring reversal from s to t
            curr_n=instance;
            reverse(curr_n.begin() + s, curr_n.begin() + t+1);
            if (find (tabu_list.begin(), tabu_list.end(), curr_n) == tabu_list.end() && !equal(instance,curr_n)){
                //the neighbour is not tabu
                curr_val=objective_function(curr_n,weights);
                if(curr_val<best_val){
                    best_val=curr_val;
                    best_n=curr_n;
                }
            }
        }
    }
    return best_n;
}

int main (int argc, char const *argv[])
{
try{
    //args: instance_file.dat, parameters.dat
    if (argc<3) throw runtime_error("missing args");

    //parameters: 1)multistart 2)time_limit(seconds) 3)tabu_lenght
    string param_file_name=argv[2];
    ifstream param_file(param_file_name);
    int n_multistart;
    int current_multistart=0;
    param_file >> n_multistart;
    double time_limit;
    param_file >> time_limit;
    int tabu_lenght;
    param_file >> tabu_lenght;
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
    cout<<"lower bound: "<<one_tree_lower_bound(weights,n_nodes)<<endl;

    //set up time
    struct timeval  tv_start, tv_current;

    //multistart cylce
    bool stop=false;
    double time_difference=0;
    gettimeofday(&tv_start, NULL);

    
    vector<int> curr_sol(n_nodes); //current solution
    int k=0; //iteration counter
    vector<vector<int>> tabu_list;
    vector<int> best_sol(n_nodes);
    vector<int> next_sol(n_nodes);
    curr_sol=christofides(n_nodes,weights); //first initial solution is christofides
    bool stopping_criteria=false;
    double curr_val,best_val;
    while(!stop){
        
        //create initial solution
        if(current_multistart!=0){
            //random initial solution
            for(int i=0;i<n_nodes;i++) curr_sol[i]=i;
            unsigned seed = chrono::system_clock::now().time_since_epoch().count();
            shuffle(curr_sol.begin(), curr_sol.end(), default_random_engine(seed));
        }
        cout<<"start from solution: ";
        printsol(curr_sol);
        cout<<"obj fun: "<<objective_function(curr_sol, weights)<<endl;
        //repeat until stopping criteria
        stopping_criteria=false;
        k=0;
        tabu_list.clear();
        best_sol=curr_sol;
        best_val=objective_function(best_sol,weights);
        /*vector<int> b {3,2,4,1,0};
        vector<int> c {0,3,2,4,1};
        vector<int> d {2,3,0,1,4};
        vector<int> e {4,2,3,0,2};
        cout<<"a e b "<<equal(curr_sol,b)<<endl;
        cout<<"a e c "<<equal(curr_sol,c)<<endl;
        cout<<"a e d "<<equal(curr_sol,d)<<endl;
        cout<<"a e e "<<equal(curr_sol,e)<<endl;*/
        
        while(!stopping_criteria){

            //list all neighbour of x
            //get best neighbour y
            next_sol=best_neighbour_2opt(curr_sol,weights,tabu_list);
            //cout<<"best neighbour: ";
            //printsol(next_sol);
            //cout<<"neighbour val: "<<objective_function(next_sol,weights)<<endl;
            //TODO: store also "good enough neighbour", to explore later?
            //TODO: alternate intensification (2 opt, small tabu lenght) with esploration (3 opt, long tabu lenght)
            //TODO: tabu list of moves instead of instances, because of the symmetry
            //add y to tabu list
            if(tabu_list.size()>=tabu_lenght){
                //remove the oldest element
                tabu_list.erase(tabu_list.begin());
            }
            tabu_list.push_back(next_sol);

            //if y is better than the best, store it
            curr_val=objective_function(next_sol,weights);
            if(curr_val<best_val){
                best_val=curr_val;
                best_sol=next_sol;
            }
            //x=y
            curr_sol=next_sol;
            k++;
            if(k>200) stopping_criteria=true;
            //stopping_criteria=true;
        }
        cout<<"best val: "<<best_val<<endl;
        //get the time
        gettimeofday(&tv_current, NULL);
        time_difference = (double)(tv_current.tv_sec+tv_current.tv_usec*1e-6 - (tv_start.tv_sec+tv_start.tv_usec*1e-6));
        //stop if time limit exceed
        if(time_difference>time_limit) stop=true;
        current_multistart++;
        //stop if you did all multistarts
        if(current_multistart>=n_multistart) stop=true;
    }

    

}
catch(exception& e)
{
    cout << ">>>EXCEPTION: " << e.what() << std::endl;
}

    return 0;
}