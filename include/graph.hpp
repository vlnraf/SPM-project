#ifndef GRAPH_HPP
#define GRAPH_HPP
#include <iostream>
#include <fstream>
#include <vector>

class Node{
  public:
    const int ID;

    Node(int value) : ID(currID++){
      this-> value = value;
    }
    Node(int id, int value) : ID(id){
      this->value = value;
    }

    int getValue(){
      return value;
    }

    void setValue(int value){
      this->value = value;
    }

    int getID(){
      return ID;
    }

    void setDestination(Node *des){
      destination.push_back(des);
    }

    std::vector<Node*> getDestination(){
      return destination;
    }

  private:
    static int currID;
    int value;
    std::vector<Node*> destination;
};

class Graph{

public:
    Graph();
    ~Graph();
    Node addNode(int value);
    Node addNode(int id, int value);
    void addEdge(Node v, Node w); //Function to add an edge to the graph
    void addEdge(int id1, int id2);
    friend std::ostream& operator <<(std::ostream &out, Graph &g);
    Node& operator [](int id);
    Node& at(int id);
    std::vector<Node> getNodes();

private:
  std::vector<Node> nodes; //pointers to an array of adjacency for the arcs
};

void generateRandomDAG(Graph &g, int N);
void generateGraph(Graph &g);
void writeRandomDAG(std::string filename, int N);
void createGraphFromFile(std::string filename, Graph &g);
void writeGraphImage(Graph &g, std::string filename);

#endif
