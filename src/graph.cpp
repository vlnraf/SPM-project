#include <fstream>
#include <string.h>
#include "graph.hpp"

#define MAX_VALUE 50
#define PERCENT 5

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
  this->nodes.at(v_id).setDestination(&w);
}

void Graph::addEdge(int id1, int id2){
  this->nodes.at(id1).setDestination(&nodes.at(id2));
}

Node& Graph::operator [](int id){
  return nodes[id];
}

Node& Graph::at(int id){
  return nodes[id];
}

std::vector<Node> Graph::getNodes(){
  return this->nodes;
}

std::ostream& operator <<(std::ostream &out, Graph &g){
  for (int i=0; i < (int)g.nodes.size(); i++){
    out << "\nAdjacency list of vertex " << i << std::endl;
    if(g.nodes[i].getDestination().size() < 1){
      out << "Isolated Vertex! ";
    }
    for (auto x : g.nodes[i].getDestination()){
      out << i << "->" << x->getID() << ',';
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

void createGraphFromFile(std::string filename , Graph &g){
  std::ifstream graph_file(filename , std::ifstream::in);
  if(!graph_file.is_open()){
    std::cout << "Can't open the file try again!" << std::endl;
    exit(1);
  }else{
    std::string n;
    std::getline(graph_file, n);
    int num_nodes = std::stoi(n);
    for(int i=0; i<num_nodes; i++){
      g.addNode(rand() % MAX_VALUE);
    }
    int x;
    int y;
    while(graph_file >> x >> y){
      g.addEdge(x,y);
    }
  }
  graph_file.close();
}

void writeGraphImage(Graph &g, std::string filename){
  std::fstream f;
  f.open(filename, std::ios::out);
  std::vector<Node> nodes = g.getNodes();
  
  f << "digraph {" << std::endl;

  for (int i=0; i<(int) nodes.size(); i++){
    for(auto x : nodes[i].getDestination()){
      f << nodes[i].getID() << " -> " << x->getID() << ";" << std::endl;
    }
  }
  f << "}" << std::endl;
}
