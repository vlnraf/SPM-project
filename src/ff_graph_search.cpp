#include <iostream>
#include <thread>

#include "graph.hpp"
#include "Utimer.hpp"
#include <ff/ff.hpp>
#include <ff/farm.hpp>

#define MAX_NW 256

using namespace ff;

int Node::currID = 0; //Initialize the id for the graph
Graph g;
std::vector<bool> visited;

struct W_output{
  std::queue<int> local_queue;
  int local_sum;
};

struct Emitter: ff_monode_t<std::vector<int>, W_output> {
    Emitter(int nw, int src){
      this->nw = nw;
      //this->frontier = frontier;
      this->src = src;
      this->stage1 = true;
    }
    W_output* svc(std::vector<int> *task) {
      int turn = 0;
      std::vector<std::queue<int>> fs(nw);
      if(task == NULL){
        std::vector<int> tmp;
        tmp.push_back(src);

        for(int i=0; i<(int) tmp.size(); i++){
          fs[turn].push(tmp[i]);
          visited[tmp[i]] = true;
          //vis.insert(frontier[i]);
          turn = (turn + 1) % nw;
        }
        for(int i=0; i<nw; i++){
          W_output *out = new W_output;
          out->local_queue = fs[i];
          ff_send_out(out);
        }
        return GO_ON;
      }else{
        frontier.swap(*task);
        task->clear();
        for(int i=0; i<(int) frontier.size(); i++){
          fs[turn].push(frontier[i]);
          turn = (turn + 1) % nw;
        }
        for(int i=0; i<nw; i++){
          W_output *out = new W_output;
          out->local_queue = fs[i];
          ff_send_out(out);
        }
      }
      return GO_ON;
    }

    void svc_end(){
      for(int i=0; i<this->nw; i++){
        ff_send_out(EOS, i);
      }
    }

    int nw;
    int src;
    bool stage1;
    std::vector<int> frontier;
};

struct Worker: ff_node_t<W_output> {
    Worker(int val){
      this->value = val;
    }

    W_output* svc(W_output *task) { 
      int partialCounter = 0;
      std::queue<int> local_frontier = task->local_queue;
      std::queue<int> next_frontier;
      while(!local_frontier.empty()){
        Node n = g[local_frontier.front()];
        if(n.getValue() == this->value) partialCounter++;
        local_frontier.pop();

        for(auto neighbor: n.getDestination()){
          if(!visited[neighbor->getID()]){
            next_frontier.push(neighbor->getID());
            }
        }
      }
      //level++;
      //std::cout<<"level : " << level <<std::endl;
      W_output *out = new W_output;
      out->local_queue = next_frontier;
      out->local_sum = partialCounter;
      partialCounter = 0;
      delete task;
      return out;
    }

    int value;
    int level = 0;
};

struct Collector: ff_minode_t<W_output, std::vector<int>> {
    Collector(int nw){
      this->total_sum = 0;
      this->nw = nw;
      this->ntasks=0;
    }

    std::vector<int>* svc(W_output *task) { 
      W_output t = *task;
      this->total_sum += t.local_sum;
      std::queue<int> tmp = t.local_queue;
      while(!tmp.empty()){
        if(!visited[tmp.front()]){
          visited[tmp.front()] = true;
          frontier.push_back(tmp.front());
        }
        tmp.pop();
      }
      if(++ntasks == nw){
        if(frontier.empty()){
          return EOS;
        }

        ntasks=0;
        delete task;
        return &frontier;
      }else{
        delete task;
        return GO_ON;
      }
    }

    std::vector<int> frontier;
    int nw;
    int total_sum;
    int ntasks;
};


int main(int argc, char* argv[]){
  if(argc < 2){
    std::cerr << "To use the application this is the syntax : ./main [path file] [starting node] [value to count] [number of workers]" << std::endl;
    return 1;
  }

  std::string graph_file= argv[1];
  int src = std::atoi(argv[2]);
  int value = std::atoi(argv[3]);
  int nw = std::atoi(argv[4]);
  if(nw == MAX_NW){
    nw = nw -2;
  }

  std::vector<int> frontier;
  //long sequential, parallel;

  {
    utimer t("graph gen");
    //writeRandomDAG("graph_prova", N);
    createGraphFromFile(graph_file, g);
    //generateRandomDAG(g, N);
    //writeGraphImage(g, "../image_graph/image.gv");
  }
  int N = g.getNodes().size();

  for(int i=0; i<N; i++){
    visited.push_back(false);
  }

  frontier.push_back(g[src].getID());

  Emitter  Emitter(nw, src);
  Collector  Collector(nw);

  std::vector<std::unique_ptr<ff_node> > W;
  for(int i=0;i<nw;i++) 
    W.push_back(make_unique<Worker>(value));

  ff_Farm farm(std::move(W), Emitter, Collector);
  farm.wrap_around();

  ffTime(START_TIME);
  if (farm.run_and_wait_end()<0) {
      error("running farm");
      return -1;
  }
  ffTime(STOP_TIME);
  std::cout << "Total Sum: " << Collector.total_sum << std::endl;
  std::cout << "Time: " << ffTime(GET_TIME) << " ms\n";

  return 0;
}
