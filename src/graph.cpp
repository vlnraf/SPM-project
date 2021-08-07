#include <algorithm>
#include "graph.hpp"

#define MAX_VALUE 1
#define PERCENT 20
#define MIN_PER_RANK 50 // How fat should be the graph
#define MAX_PER_RANK 50
#define MIN_RANKS 500 // How tall should be the graph
#define MAX_RANKS 500

Graph::Graph(){}
Graph::~Graph(){}

Node Graph::addNode(int value){
  Node temp(value);
  this->nodes.push_back(temp);

  return temp;
}

Node Graph::addNode(int id, int value){
  Node temp(id, value);
  this->nodes.push_back(temp);

  return temp;
}

void Graph::addEdge(Node v, Node w){
  int v_id = v.getID();
  this->nodes.at(v_id).setDestination(w);
}

void Graph::addEdge(int id1, int id2){
  this->nodes.at(id1).setDestination(nodes.at(id2));
}

Node& Graph::operator [](int id){
  return nodes[id];
}

std::vector<Node> Graph::getNodes(){
  return this->nodes;
}

//std::ostream& Graph::operator <<(std::ostream &out, Graph &g){
std::ostream& operator <<(std::ostream &out, Graph &g){
  for (int i=0; i < (int)g.nodes.size(); i++){
    out << "\nAdjacency list of vertex " << i << std::endl;
    if(g.nodes[i].getDestination().size() < 1){
      out << "Isolated Vertex! ";
    }
    for (auto x : g.nodes[i].getDestination()){
      out << i << "->" << x.getID() << ',';
    }
    out << std::endl;
  }
  return out;
}

void generateRandomDAG(Graph &g, int N){
  for(int i=0; i<N; i++){
    g.addNode(rand() % MAX_VALUE);
  }
  
  for(int i=0; i<N; i++){
    for(int j=i+1; j<N; j++){
      if((rand() % 100) < PERCENT){
        g.addEdge(i,j);
      }
    }
  }
}

//void generateGraph(Graph &g){
//  int i, j, k,nodes = 0;
//
//  int ranks = MIN_RANKS + (rand() % (MAX_RANKS - MIN_RANKS + 1));
//
//  for(int i=0; i<30010; i++){
//    g.addNode(rand() % MAX_VALUE);
//  }
//
//  for (i=0; i<ranks; i++)
//    {
//      int new_nodes = MIN_PER_RANK + (rand() % (MAX_PER_RANK - MIN_PER_RANK + 1));
//
//      for (j=0; j<nodes; j++)
//        for (k=0; k<new_nodes; k++)
//          if ((rand() % 100) < PERCENT){
//              //std::cout<<j<<std::endl;
//              //printf ("  %d -> %d;\n", j, k + nodes); /* An Edge.  */
//              //g.addNode(j, rand() % MAX_VALUE);
//              //g.addNode(k + nodes, rand() % MAX_VALUE);
//              g.addEdge(j, k + nodes);
//          }
//
//      nodes += new_nodes; /* Accumulate into old node set.  */
//    }
//  std::cout << nodes << std::endl;
//}

void writeGraphImage(Graph &g, std::string filename){
  std::fstream f;
  f.open(filename, std::ios::out);
  std::vector<Node> nodes = g.getNodes();
  
  f << "digraph {" << std::endl;

  for (int i=0; i<(int) nodes.size(); i++){
    // if wanna display not connected nodes too add this for
    // if(nodes[i].getDestination().size() < 1){
    // f << nodes[i].getID() << ";" << std::endl;
    // }
    for(auto x : nodes[i].getDestination()){
      f << nodes[i].getID() << " -> " << x.getID() << ";" << std::endl;
    }
  }
  f << "}" << std::endl;
}
