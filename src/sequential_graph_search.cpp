#include <queue>
#include "graph.hpp"
#include "Utimer.hpp"

int Node::currID = 0; //Initialize the id for the graph

void bfs(Graph &g, int src, int val, int &count, long &sequential){
  std::queue<int> fs;
  std::queue<int> ns;
  std::vector<bool> visited(g.getNodes().size());
  fs.push(src);
  visited[src] = true;
  int level = 0;

  {
    utimer sequential_time("Sequential time ", &sequential);
    while(!fs.empty()){
      int fsSize = fs.size();
      for(int i=0; i<fsSize; i++){
        Node node = g[fs.front()];
        fs.pop();
        if(node.getValue() == val) count ++;

        for(auto neighbor : node.getDestination()){
          if(!visited[neighbor->getID()]){
            ns.push(neighbor->getID());
            visited[neighbor->getID()] = true;
            }
        }
      }
        fs = ns;
        while(!ns.empty()) ns.pop();
        level ++;
    }
  }
}

int main(int argc, char* argv[]){

  if(argc < 2){
    std::cerr << "To use the application this is the syntax : ./main [path file] [starting node] [value to count]" << std::endl;
    return 1;
  }

  Graph g;
  std::string filename = "../image_graph/image.gv";
  std::string graph_file= argv[1];
  int src = std::atoi(argv[2]);
  int value = std::atoi(argv[3]);
  int count = 0;
  long sequential;

  {
    utimer t("graph gen");
    //writeRandomDAG("graph_prova", N);
    createGraphFromFile(graph_file, g);
    //generateRandomDAG(g, N);
    //std::cout << g << std::endl;
  }

  bfs(g,src,value,count,sequential);
  std::cout << "Sequential, starting from node " << "(" << src << ")" << " the occurences of" << " (" << value << ") are : " << count << std::endl;

  return 0;
}
