#include "graph.hpp"

#define MAX_VALUE 50
#define PERCENT 30

Graph::Graph(){}
Graph::~Graph(){}

Node Graph::addNode(int value){
  Node temp(value);
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

void writeGraphImage(Graph &g, std::string filename){
  std::fstream f;
  f.open(filename, std::ios::out);
  std::vector<Node> nodes = g.getNodes();
  
  f << "digraph {" << std::endl;

  for (int i=0; i<(int) nodes.size(); i++){
    for(auto x : nodes[i].getDestination()){
      f << nodes[i].getID() << " -> " << x.getID() << std::endl;
    }
  }
  f << "}" << std::endl;
}
