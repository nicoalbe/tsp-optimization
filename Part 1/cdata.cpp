#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
using namespace std;

int rowof(int i, int c){
  return i/c;
}

int colof(int i, int c){
  return i%c;
}

void print_grid(int node[], int r, int c){
  for(int i=0;i<r;i++){
    for(int j=0;j<c;j++){
      if(node[i*c+j]==-1) cout<<'.'<<'\t';
      else cout<<node[i*c+j]<<'\t';
    }
    cout<<endl;
  }
}

int main () {
  ofstream myfile;
  srand (time(NULL));
  
  //parameters to set:
  myfile.open ("tsp20rg.dat");
  int n=20;
  //random weights params
  int min_random_weight=1;
  int max_random_weight=100;
  //grid params
  int unit=1;
  int r=200;
  int c=100;
  
  //style: 0=random weights 1=full grid 2=random grid
  int style=2;

  double cost[n*n];

  if(style==2){
    //randomly filled grid graph

    int node[r*c]; //which node is in position i,j in the grid
    for(int i=0;i<r;i++){
      for(int j=0;j<c;j++){
        node[i*c+j]=-1;
      }
    }
    int pos[n][2]; //node k is in position i,j if pos[k][0]=i and pos[k][1]=j
    
    //randomly put the nodes in the grid
    int new_pos[2];
    for(int k=0;k<n;k++){
      //find a position for k
      do{
        new_pos[0]=rand() % r;
        new_pos[1]=rand() % c;
      }while(node[new_pos[0]*c + new_pos[1]]!=-1);
      //put k
      node[new_pos[0]*c + new_pos[1]] = k;
      pos[k][0]=new_pos[0];
      pos[k][1]=new_pos[1];
    }

    //print_grid(node, r, c);

    //compute weights
    for(int i=0;i<n;i++){
      cost[i*n+i]=0;
      int ri=pos[i][0];
      int ci=pos[i][1];
      for(int j=i+1;j<n;j++){
        int rj=pos[j][0];
        int cj=pos[j][1];
        //i and j in the same column
        if(ci==cj) cost[i*n+j]=abs(rj-ri)*unit;
        //i and j in the same row
        else if(ri==rj) cost[i*n+j]=abs(cj-ci)*unit;
        //i and j are diagonal
        else cost[i*n+j]=sqrt(pow(cj-ci,2)+pow(rj-ri,2))*unit;
        cost[j*n+i]=cost[i*n+j];
      }
    }
  }
  else if(style==1){
    //full grid graph

    //number of rows and columns of the grid r*c=n

    for(int i=0;i<n;i++){
      cost[i*n+i]=0;
      int ci=colof(i,c);
      int ri=rowof(i,c);
      for(int j=i+1;j<n;j++){
        int rj=rowof(j,c);
        int cj=colof(j,c);
        //i and j in the same column
        if(ci==cj) cost[i*n+j]=(rj-ri)*unit;
        //i and j in the same row
        else if(ri==rj) cost[i*n+j]=(cj-ci)*unit;
        //i and j are diagonal
        else cost[i*n+j]=sqrt(pow(cj-ci,2)+pow(rj-ri,2))*unit;
        cost[j*n+i]=cost[i*n+j];
      }
    }

  }
  else{
    //random graph
    int range=max_random_weight-min_random_weight+1;
    for(int i=0;i<n;i++){
      cost[i*n+i]=0;
      for(int j=i+1;j<n;j++){
        double val=rand() % range + min_random_weight;
        cost[i*n+j]=val;
        cost[j*n+i]=val;
      }
    }
  }

  myfile<<n<<endl;
  for(int i=0;i<n;i++){
    for(int j=0;j<n;j++){
      myfile << cost[i*n+j]<<"\t";
    }
    myfile<<endl;
  }

  myfile.close();
  return 0;
}