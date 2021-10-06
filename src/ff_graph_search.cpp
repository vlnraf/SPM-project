#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

#include "graph.hpp"
#include "Utimer.hpp"
#include <ff/ff.hpp>
#include <ff/farm.hpp>

#define MAX_NW 256

std::chrono::steady_clock::time_point begin;
std::chrono::steady_clock::time_point end;

using namespace ff;

int Node::currID = 0; //Initialize the id for the graph
Graph g;
std::vector<bool> visited;

std::mutex messageLock;

struct W_output{
  std::queue<int> local_queue;
  int local_sum;
  unsigned long time;
};

struct Emitter: ff_monode_t<std::vector<int>, W_output> {
    Emitter(int nw, int src){
      this->nw = nw;
      //this->frontier = frontier;
      this->src = src;
      this->stage1 = true;
    }

    W_output* svc(std::vector<int> *task) {
#if DEBUG
      auto start = std::chrono::steady_clock::now();
#endif
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
        delete task;
#if DEBUG
        auto stop = std::chrono::steady_clock::now();
        std::cout << "Time of the emitter:" << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count()  << " ms\n";
#endif
        return GO_ON;
      }else{
        //::ffTime(START_TIME);
#if DEBUG
        auto start = std::chrono::steady_clock::now();
#endif
        frontier.swap(*task);
        //std::random_shuffle(frontier.begin(), frontier.end());
        task->clear();
        for(int i=0; i<(int) frontier.size(); i++){
          fs[turn].push(frontier[i]);
          turn = (turn + 1) % nw;
        }
#if DEBUG
        std::cout<< "size of queue in bytes: " << sizeof(int) * fs[0].size() << "bytes" << std::endl;
#endif
        for(int i=0; i<nw; i++){
          W_output *out = new W_output;
          out->local_queue = fs[i];
          ff_send_out(out);
        }
#if DEBUG
        auto stop = std::chrono::steady_clock::now();
        //::ffTime(STOP_TIME);
        std::cout << "Time of the emitter:" << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count()  << " ms\n";
#endif
        return GO_ON;
      }
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
#if DEBUG
      auto start = std::chrono::steady_clock::now();
#endif
      int partialCounter = 0;
      //::ffTime(START_TIME);
      {
        //utimer work_time("",&t_w); 
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
        //auto stop = std::chrono::steady_clock::now();
        //messageLock.lock();
        //std::cout << "thread id (" << get_my_id() << ") computed in : " << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << "ms" << std::endl;
        //messageLock.unlock();
        //level++;
        //std::cout<<"level : " << level <<std::endl;
        //start = std::chrono::steady_clock::now();
        W_output *out = new W_output;
        out->local_queue = next_frontier;
        out->local_sum = partialCounter;
        //messageLock.lock();
        //std::cout << "thread id (" << get_my_id() << ") communication time : " << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << "ms" << std::endl;
        //messageLock.unlock();
        //partialCounter = 0;
        delete task;
#if DEBUG
        auto stop = std::chrono::steady_clock::now();
        unsigned long time = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();
        out->time = time;
#endif
        begin = std::chrono::steady_clock::now();
        return out;
      }
    }


    int value;
    int level = 0;
};

struct Collector: ff_minode_t<W_output, std::vector<int>> {
    Collector(int nw){
      this->total_sum = 0;
      this->nw = nw;
      this->ntasks=0;
      this->timer = 0;
      //::ffTime(START_TIME);
    }

    //int svc_init(){
    //  if(ntasks==0){
    //    ::ffTime(START_TIME);
    //  }
    //  return 0;
    //}

    std::vector<int>* svc(W_output *task) { 
      //::ffTime(START_TIME);
      if(ntasks == 0){
        start = std::chrono::steady_clock::now();
      }
      end = std::chrono::steady_clock::now();
#if DEBUG
      auto s = std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count();
      if(s > 50){
      std::cout<<"Communication between worker and collector: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count() << "ms" << std::endl;
      }
#endif
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
      auto stop = std::chrono::steady_clock::now();

      timer = t.time;
      if(t.time > timer){
        timer = t.time;
      }
      //timer+= t.time;
      
      //::ffTime(STOP_TIME);
      if(++ntasks == nw){
#if DEBUG
        std::cout<<"max worker time: " << timer << " ms" << std::endl;
#endif
        timer = 0;
        if(frontier.empty()){
          return EOS;
        }

        ntasks=0;
        //::ffTime(STOP_TIME); //timer for see how much time is needed for the computation
        //std::cout << "Time to compute the frontier of size (" << frontier.size() << "):" << ::ffTime(GET_TIME) << " ms\n";
        //::ffTime(START_TIME);
#if DEBUG
        std::cout << "Time of the collector : " << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << "ms" << std::endl;
        std::cout << "------------------------------------" << std::endl;
#endif
        delete task;
        return &frontier;
      }else{
        //::ffTime(STOP_TIME); //timer for see how much time is needed for the computation
        delete task;
        return GO_ON;
      }
    }

    std::vector<int> frontier;
    int nw;
    int total_sum;
    int ntasks;
    unsigned long timer;
    std::chrono::steady_clock::time_point start;
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
    //utimer t("graph gen");
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
  //ffTime(START_TIME);
  for(int i=0;i<nw;i++) 
    W.push_back(make_unique<Worker>(value));
  //ffTime(STOP_TIME);
  //std::cout << "time to craete farm " << ffTime(GET_TIME) << std::endl;

  ff_Farm farm(std::move(W), Emitter, Collector);
  farm.wrap_around();

  ffTime(START_TIME);
  if (farm.run_and_wait_end()<0) {
      error("running farm");
      return -1;
  }
  ffTime(STOP_TIME);
  //std::cout << "Total Sum: " << Collector.total_sum << std::endl;
  std::cout << "Time: " << ffTime(GET_TIME) << " ms\n";

  return 0;
}
