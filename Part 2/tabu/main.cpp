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
    //double instance_val=objective_function(instance,weights);
    //int best_move[2];//{s,2} for swap
    double curr_val=0;
    //int pre_s,post_t;
    bool is_tabu;
    int tabu_lenght=tabu_list.size();
    for(int s=0;s<n_nodes-1;s++){
        for (int t=s+1;t<n_nodes;t++){
            //substring reversal from s to t
            curr_n=instance;
            reverse(curr_n.begin() + s, curr_n.begin() + t+1);
            is_tabu=false;
            for(int checktabu=0;checktabu<tabu_lenght;checktabu++){
                if(equal(tabu_list[checktabu],curr_n)){
                    is_tabu=true;
                }
            }
            if (!is_tabu && !equal(instance,curr_n)){
                //the neighbour is not tabu
                //not doing incremental evaluation bc i need to check if it's tabu
                //pre_s= (s==0) ? n_nodes-1 : s-1;
                //post_t= (t==n_nodes-1) ? 0 : t+1;
                //curr_val = instance_val - weights[pre_s*n_nodes+s] - weights[t*n_nodes+post_t] + weights[pre_s*n_nodes+t] + weights[s*n_nodes+post_t];
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

//https://tsp-basics.blogspot.com/2017/03/3-opt-move.html
vector<int> best_neighbour_3opt(vector<int> instance, vector<double> weights, vector<vector<int>> tabu_list){
    int n_nodes=instance.size();
    vector<int> best_neighbour(n_nodes);
    vector<int> current_neighbour(n_nodes);
    //vector<int> best_n (n_nodes);
    double best_val=numeric_limits<double>::max();
    int best_move[7];//{case,x1,x2,y1,y2,z1,z2}
    double instance_val=objective_function(instance,weights);
    //int pre_s,post_t;
    int x1,x2,y1,y2,z1,z2;
    //vector<vector<int>> 3optcases;
    vector<double> moves(4);
    double x_edge,y_edge,z_edge;
    bool is_tabu;
    int tabu_lenght=tabu_list.size();
    for(z2=0;z2<n_nodes-2;z2++){
        for(x1=z2+1;x1<n_nodes-1;x1++){
            for(y1=x1+2;y1<n_nodes;y1++){
                x2=x1+1;
                y2=y1+1;
                z1=(z2==0)? n_nodes-1 : z2-1;
                //for(int addtovec=0;addtovec<8;addtovec++){
                    //3optcases[addtovec]=instance;
                //}
                //a=[z2 to x1] b=[x2 to y1] c=[y2 to z1]
                //7 (+1 original tour) cases for the swap
                //equivalent to 2opt:
                //reverse(3optcases[0].begin()+z2,3optcases[0].begin()+x1); //a'bc
                //reverse(3optcases[1].begin()+y2,3optcases[1].begin()+z1); //abc'
                //reverse(3optcases[2].begin()+x2,3optcases[2].begin()+y1); //ab'c
                //actual 3 opt (2 subsequent 2opt)
                x_edge=weights[x1*n_nodes+x2];
                y_edge=weights[y1*n_nodes+y2];
                z_edge=weights[z1*n_nodes+z2];
                //equivalent to 2opt:
                //moves[0]=instance_val -x_edge -z_edge +weights[x1*n_nodes+z1] +weights[x2*n_nodes+z2]; //a'bc
                //moves[1]=instance_val -y_edge -z_edge +weights[y1*n_nodes+z1] +weights[y2*n_nodes+z2]; //abc'
                //moves[2]=instance_val -x_edge -y_edge +weights[x1*n_nodes+y1] +weights[x2*n_nodes+y2]; //a'bc

                //actual 3opt:
                moves[0]=instance_val -x_edge -y_edge -z_edge +weights[x2*n_nodes+z1] +weights[x1*n_nodes+y1] +weights[y2*n_nodes+z2]; //ab'c'
                moves[1]=instance_val -x_edge -y_edge -z_edge +weights[x1*n_nodes+z1] +weights[x2*n_nodes+y2] +weights[y1*n_nodes+z2]; //a'b'c
                moves[2]=instance_val -x_edge -y_edge -z_edge +weights[x1*n_nodes+y2] +weights[x2*n_nodes+z2] +weights[y1*n_nodes+z1]; //a'bc'
                moves[3]=instance_val -x_edge -y_edge -z_edge +weights[x1*n_nodes+y2] +weights[x2*n_nodes+z1] +weights[y1*n_nodes+z2]; //a'b'c'

                for(int checkneigh=3;checkneigh<7;checkneigh++){
                    if(moves[checkneigh]<best_val){
                        current_neighbour=instance;
                        //apply the move
                        switch(checkneigh){
                            case 0://a'bc
                                reverse(current_neighbour.begin()+z2,current_neighbour.begin()+x1+1);
                            break;
                            case 1://abc'
                                reverse(current_neighbour.begin()+y2,current_neighbour.begin()+z1+1);
                            break;
                            case 2://ab'c
                                reverse(current_neighbour.begin()+x2,current_neighbour.begin()+y1+1);
                            break;
                            case 3://ab'c'
                                reverse(current_neighbour.begin()+x2,current_neighbour.begin()+y1+1);
                                reverse(current_neighbour.begin()+y2,current_neighbour.begin()+z1+1);
                            break;
                            case 4://a'b'c
                                reverse(current_neighbour.begin()+z2,current_neighbour.begin()+x1+1);
                                reverse(current_neighbour.begin()+x2,current_neighbour.begin()+y1+1);
                            break;
                            case 5://a'bc'
                                reverse(current_neighbour.begin()+z2,current_neighbour.begin()+x1+1);
                                reverse(current_neighbour.begin()+y2,current_neighbour.begin()+z1+1);
                            break;
                            case 6://a'b'c'
                                reverse(current_neighbour.begin()+z2,current_neighbour.begin()+x1+1);
                                reverse(current_neighbour.begin()+y2,current_neighbour.begin()+z1+1);
                                reverse(current_neighbour.begin()+x2,current_neighbour.begin()+y1+1);
                            break;
                        }
                        //check if not tabu
                        is_tabu=false;
                        for(int checktabu=0;checktabu<tabu_lenght;checktabu++){
                            if(equal(tabu_list[checktabu],current_neighbour)){
                                is_tabu=true;
                            }
                        }
                        if (!is_tabu && !equal(instance,current_neighbour)){
                            best_val=moves[checkneigh];
                            best_neighbour=current_neighbour;
                        }
                    }
                }

            }
        }
    }
    //apply the move
    //a=[z2 to x1] b=[x2 to y1] c=[y2 to z1]
    //x1=best_move[1],x2=best_move[2],y1=best_move[3],y2=best_move[4],z1=best_move[5],z2=best_move[6];
    
    return best_neighbour;
}

int main (int argc, char const *argv[])
{
try{
    //args: instance_file.dat, parameters.dat
    if (argc<3) throw runtime_error("missing args");

    //parameters: 1)multistart 2)time_limit(seconds) 3)tabu_lenght
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
    int tabu_lenght;
    param_file>>garbage;
    param_file >> tabu_lenght;
    int tabu_lenght_diversification;
    param_file>>garbage;
    param_file >> tabu_lenght_diversification;
    int max_iterations; //iteration counter
    param_file >> garbage;
    param_file >> max_iterations;
    int max_non_improving_iterations;
    param_file >> garbage;
    param_file >> max_non_improving_iterations;
    int max_diversification_iterations;
    param_file >> garbage;
    param_file >> max_diversification_iterations;
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
    vector<vector<int>> tabu_list;
    vector<int> best_sol(n_nodes);
    vector<int> next_sol(n_nodes);
    curr_sol=christofides(n_nodes,weights); //first initial solution is christofides
    bool stopping_criteria=false;
    double curr_val,best_val;
    double best_multistart=objective_function(curr_sol,weights);
    int n_best_multistart=0;
    int k, k_non_improving, k_diversification, best_k;
    bool intensification=true;
    while(!stop){
        
        //create initial solution
        if(current_multistart!=0){
            //random initial solution
            for(int i=0;i<n_nodes;i++) curr_sol[i]=i;
            unsigned seed = chrono::system_clock::now().time_since_epoch().count();
            shuffle(curr_sol.begin(), curr_sol.end(), default_random_engine(seed));
        }
        //cout<<"start from solution: ";
        //printsol(curr_sol);
        cout<<"\n _-_-_\nstart obj fun: "<<objective_function(curr_sol, weights)<<endl;
        //repeat until stopping criteria
        stopping_criteria=false;
        k=0;
        tabu_list.clear();
        best_sol=curr_sol;
        best_val=objective_function(best_sol,weights);
        intensification=true;
        while(!stopping_criteria){

            if(intensification){
                //list all neighbour of x
                //get best neighbour y
                next_sol=best_neighbour_2opt(curr_sol,weights,tabu_list);
                //cout<<"best neighbour: ";
                //printsol(next_sol);
                //cout<<"neighbour val: "<<objective_function(next_sol,weights)<<endl;
                //TODO: store also "good enough neighbour", to explore later?
                
                //add y to tabu list
                if(tabu_list.size()>=tabu_lenght){
                    //remove the oldest element
                    tabu_list.erase(tabu_list.begin());
                }
                tabu_list.push_back(next_sol);
            } else {
                //diversification: use 3opt and a larger tabu list
                //next_sol=best_neighbour_2opt(curr_sol,weights,tabu_list);
                next_sol=best_neighbour_3opt(curr_sol,weights,tabu_list);

                //add y to tabu list
                if(tabu_list.size()>=tabu_lenght_diversification){
                    //remove the oldest element
                    tabu_list.erase(tabu_list.begin());
                }
                tabu_list.push_back(next_sol);
                k_diversification++;
            }

            //if y is better than the best, store it
            curr_val=objective_function(next_sol,weights);
            //cout<<k<<":"<<"current solution: "<<curr_val;
            if(curr_val<best_val){
                best_val=curr_val;
                best_sol=next_sol;
                k_non_improving=0;
                best_k=k;
                //cout<<" *better solution found: "<<best_val<<" iteration "<<k<<endl;
                //cout<<"***********";
            } else {
                k_non_improving++;
            }
            //x=y
            //cout<<endl;
            curr_sol=next_sol;
            k++;
            if(k>max_iterations) {
                stopping_criteria=true;
                cout<<"exit for max iterations "<<endl;
            }
            //start with diversification phase after 'some' non imporving iterations
            if(intensification && k_non_improving>max_non_improving_iterations){
                //cout<<"start diversification"<<endl;
                intensification=false;
                k_diversification=0;
            }

            if(!intensification && (k_diversification>=max_diversification_iterations || k_non_improving<1)){
                //start again with intensification, if you diversify enough or you found an improving solution
                intensification=true;
                //remove the extra oldest tabu elements
                for(int i=0;i<tabu_lenght_diversification-tabu_lenght;i++){
                    tabu_list.erase(tabu_list.begin());
                }
                k_non_improving=0;
                //cout<<"start back with intensification"<<endl;
            }
            //get the time
            gettimeofday(&tv_current, NULL);
            time_difference = (double)(tv_current.tv_sec+tv_current.tv_usec*1e-6 - (tv_start.tv_sec+tv_start.tv_usec*1e-6));
            //go to next multistart if time 
            if(time_difference>(time_limit/n_multistart)*(current_multistart+1)){
                stopping_criteria=true;
                cout<<"exit multistart time limit"<<endl;
            }
            //stop if time limit exceed
            if(time_difference>time_limit){
                stop=true;
                cout<<"exit for time limit, k:"<<k<<endl;
            }
                //stopping_criteria=true;
        }
        cout<<"best val(m"<< current_multistart <<"): "<<best_val<<" at iteration "<<best_k<<endl;
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