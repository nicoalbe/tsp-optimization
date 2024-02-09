#include <vector>
#include <iostream>

using namespace std;

//return the prim value
double prim_value(vector<double> w, int n){
    double tree_cost=0;

    bool tree[n]; //tree[i]==true iff ith node has been added to the tree
    int min_edge[2]; //from and to
    double min_edge_value;
    int next_edge[n];
    tree[0]=true;
    for(int make_false=1;make_false<n;make_false++){
        tree[make_false]=false;
    }
    //add all vertices, one by one. first v is zero
    for(int v=1;v<n;v++){
        min_edge_value=INT_MAX;

        //look for all edges
        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                //check only edges going from the tree to the outside
                if(tree[i] && !tree[j]){
                    //cout<<"add "<<i<<" "<<j<<"?"<<endl;
                    if(w[i*n+j]<min_edge_value){
                        min_edge_value=w[i*n+j];
                        min_edge[0]=i;
                        min_edge[1]=j;
                        //cout<<"yes"<<endl;
                    }
                }
            }
        }
        //cout<<"add min edge "<<min_edge_value<<endl;
        //add edge to the tree
        tree[min_edge[1]]=true;
        //add edge cost
        tree_cost+=min_edge_value;
    }
    //cout<<"end"<<endl;
    return tree_cost;
}

double one_tree_lower_bound(vector<double> w, int n){
    double highest_bound=0;
    double one_tree_i=0;
    vector<double> w_removed;
    double min_edge[2]; //the min edges value for the adding of the removed node

    for(int i=0;i<n;i++){
        w_removed.clear();
        //remove node i from G: copy everything but the ith column and the ith row
        for(int r=0;r<n;r++){
            for(int c=0;c<n;c++){
                if(r!=i && c!=i){
                    w_removed.push_back(w[r*n+c]);
                    //cout<<w[r*n+c]<<'\t';
                }
            }
        }
        //compute MST
        one_tree_i=prim_value(w_removed,n-1);
        //cout<<"prim: "<<one_tree_i<<endl;
        //add back i with the 2 min edges

        //search in all outgoing edges from i
        min_edge[0]=min_edge[1]=INT_MAX;
        for(int j=0;j<n;j++){
            if(i!=j){
                if(w[i*n+j]<min_edge[0]){
                    min_edge[1]=min_edge[0];
                    min_edge[0]=w[i*n+j];
                } else if(w[i*n+j]<min_edge[1]){
                    min_edge[1]=w[i*n+j];
                }
            }
        }
        one_tree_i+=min_edge[0]+min_edge[1];
        //check is the bound is better
        if(one_tree_i>highest_bound){
            highest_bound=one_tree_i;
        }
    }

    return highest_bound;
}