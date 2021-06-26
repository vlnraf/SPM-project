#include "graph.hpp"
#include <iostream>

int Node::currID = 0; //Initialize the id from where you want to start

int main(){
  Graph g;

  Node n = g.addNode(1);
  Node n2 = g.addNode(10);
  Node n3 = g.addNode(3);
  Node n4 = g.addNode(4);
  Node n5 = g.addNode(5);

  g.addEdge(n,n2);
  g.addEdge(n,n3);
  g.addEdge(n,n4);
  g.addEdge(n3,n4);

  std::cout << g << std::endl;
  return 0;
}
