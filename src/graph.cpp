#include "graph.hpp"

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

Node& Graph::operator [](int id){
  return nodes[id];
}

//std::ostream& Graph::operator <<(std::ostream &out, Graph &g){
std::ostream& operator <<(std::ostream &out, Graph g){
  for (int i=0; i < g.nodes.size(); ++i){
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
