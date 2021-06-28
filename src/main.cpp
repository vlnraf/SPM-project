#include "graph.hpp"
#include <iostream>

int Node::currID = 0; //Initialize the id from where you want to start

int main(int argc, char* argv[]){

  if(argc < 2){
    std::cout << "Verra' utilizzato 10 come seed di default" << std::endl;
    std::srand(10);
  }else{
    std::srand(std::atoi(argv[1]));
  }

  Graph g;
  std::string filename = "image_graph0/image.gv";
  int N = atoi(argv[2]);

  generateRandomDAG(g, N);


  writeGraphImage(g, filename);

  //std::cout << g << std::endl;

  return 0;
}

//Node n = g.addNode(1);
//Node n2 = g.addNode(10);
//Node n3 = g.addNode(3);
//Node n4 = g.addNode(4);
//Node n5 = g.addNode(5);

//g.addEdge(n,n2);
//g.addEdge(n,n3);
//g.addEdge(n,n4);
//g.addEdge(n3,n4);
