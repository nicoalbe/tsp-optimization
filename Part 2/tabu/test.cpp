#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

int main(){
    vector<int> ciao={0,1,2,3,4,5,6,7,8,9};
    reverse(ciao.begin()+1,ciao.begin()+5);
    for(int i=0;i<10;i++){
        cout<<ciao[i]<<" ";
    }
    return 0;
}

//3opt with incremental
vector<int> best_neighbour_3opt(vector<int> instance, vector<double> weights, vector<vector<int>> tabu_list){
    int n_nodes=instance.size();
    //vector<int> best_n (n_nodes);
    double best_val=numeric_limits<double>::max();
    int best_move[7];//{case,x1,x2,y1,y2,z1,z2}
    double instance_val=objective_function(instance,weights);
    //int pre_s,post_t;
    int x1,x2,y1,y2,z1,z2;
    //vector<vector<int>> 3optcases;
    vector<double> moves(7);
    double x_edge,y_edge,z_edge;
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
                moves[0]=instance_val -x_edge -z_edge +weights[x1*n_nodes+z1] +weights[x2*n_nodes+z2]; //a'bc
                moves[1]=instance_val -y_edge -z_edge +weights[y1*n_nodes+z1] +weights[y2*n_nodes+z2]; //abc'
                moves[2]=instance_val -x_edge -y_edge +weights[x1*n_nodes+y1] +weights[x2*n_nodes+y2]; //a'bc

                //actual 3opt:
                moves[3]=instance_val -x_edge -y_edge -z_edge +weights[x2*n_nodes+z1] +weights[x1*n_nodes+y1] +weights[y2*n_nodes+z2]; //ab'c'
                moves[4]=instance_val -x_edge -y_edge -z_edge +weights[x1*n_nodes+z1] +weights[x2*n_nodes+y2] +weights[y1*n_nodes+z2]; //a'b'c
                moves[5]=instance_val -x_edge -y_edge -z_edge +weights[x1*n_nodes+y2] +weights[x2*n_nodes+z2] +weights[y1*n_nodes+z1]; //a'bc'
                moves[6]=instance_val -x_edge -y_edge -z_edge +weights[x1*n_nodes+y2] +weights[x2*n_nodes+z1] +weights[y1*n_nodes+z2]; //a'b'c'

                for(int checkneigh=0;checkneigh<7;checkneigh++){
                    if(moves[checkneigh]<best_val){
                        best_val=moves[checkneigh];
                        best_move[0]=checkneigh;
                        best_move[1]=x1;
                        best_move[2]=x2;
                        best_move[3]=y1;
                        best_move[4]=y2;
                        best_move[5]=z1;
                        best_move[6]=z2;
                    }
                }

            }
        }
    }
    //apply the move
    //a=[z2 to x1] b=[x2 to y1] c=[y2 to z1]
    x1=best_move[1],x2=best_move[2],y1=best_move[3],y2=best_move[4],z1=best_move[5],z2=best_move[6];
    switch(best_move[0]){
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