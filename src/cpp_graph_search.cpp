#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

#include "graph.hpp"
#include "Utimer.hpp"

//#define DEBUG

std::mutex messagesLock;
std::chrono::steady_clock::time_point start;
std::chrono::steady_clock::time_point stop;


#ifdef DEBUG
#define DEBUG_MSG(str) messagesLock.lock(); std::cout << str << std::endl; messagesLock.unlock();
#else
#define DEBUG_MSG(str) 
#endif

int Node::currID = 0; //Initialize the id for the graph

void bfs(Graph &g, int src, int val, std::atomic<int> &count, long &sequential){
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

void parallelBFS(Graph &g, int src, int val, std::atomic<int> &count, int nw, long &parallel){

    std::vector<std::queue<int>> fs(nw); //vector of queue to assign the queue at each thread
    std::vector<std::queue<int>> ns(nw);   //same but this is used for update the next frontiear
    std::vector<bool> visited(g.getNodes().size());
    std::vector<int> frontier;
    std::vector<std::thread> tids(nw);

    int level = 0;
    auto myjob = [&](std::vector<int> &frontier, int tid){
      {
        DEBUG_MSG("quque size of thread(" << tid << ") :" << fs[tid].size()<< "| Level : " << level);
        { 
          start = std::chrono::steady_clock::now();
          //messagesLock.lock();
          //utimer worker_time("time of worker (" + std::to_string(tid) + ")");
          //messagesLock.unlock();
          while(!fs[tid].empty()){
            Node n = g[fs[tid].front()];
            if(n.getValue() == val) count++;
            fs[tid].pop();

            for(auto neighbor: n.getDestination()){
              if(!visited[neighbor->getID()]){
                ns[tid].push(neighbor->getID());
                }
            }
          }
        }
        stop = std::chrono::steady_clock::now();
        auto t = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();
        messagesLock.lock();
        std::cout << "Worker(" + std::to_string(tid) + ") time: " + std::to_string(t) + " ms" << std::endl;
        messagesLock.unlock();
      }
    };

  {
    utimer parallel_time("Parallel time", &parallel);
  
    std::vector<Node*> tmp;
    Node n = g[src];
    visited[n.getID()] = true;
    if(n.getValue() == val) count++;
    tmp = n.getDestination();

    for(int i=0; i<(int) tmp.size(); i++){
      frontier.push_back(tmp[i]->getID());
      visited[tmp[i]->getID()] = true;
    }
    level++;
    tmp.clear();
    
    while(!frontier.empty()){
      int turn = 0;

      {
        //utimer te("emitter time: ");
#if DEBUG
        DEBUG_MSG("Frontier size: " << frontier.size());
        //std::cout << "Frontier size: " << frontier.size() << std::endl;
#endif
        for(int i=0; i<(int) frontier.size(); i++){
          fs[turn].push(frontier[i]);
          visited[frontier[i]] = true;
          turn = (turn + 1) % nw;
        }
        frontier.clear();
      }

      {
#if DEBUG
        messagesLock.lock();
        utimer worker("worker creation + worker time + worker join: ");
        messagesLock.unlock();
#endif
        for(int i=0; i<nw; i++){
          {
            //utimer wt("worker time: ");
            //if(fs[i].size() !=0)
            tids[i] = std::thread(myjob, std::ref(frontier), i);
            //else
            //continue;
          }
        }
        for(std::thread& t: tids){
          t.join();
        }
        std::cout << "------------------------------------------" << std::endl;
        level++;
      }

      {
#if DEBUG
        //utimer tc("collector time: ");
#endif
        for(int i=0; i<nw; i++){
          while(!ns[i].empty()){
            if(!visited[ns[i].front()]){
              visited[ns[i].front()] = true;
              frontier.push_back(ns[i].front());
            }
            ns[i].pop();
          }
        }
      }
    }
  }
}

int main(int argc, char* argv[]){

  if(argc < 2){
    std::cerr << "To use the application this is the syntax : ./main [path file] [starting node] [value to count] [number of workers]" << std::endl;
    return 1;
  }

  Graph g;
  std::string filename = "../image_graph/image.gv";
  std::string graph_file= argv[1];
  int src = std::atoi(argv[2]);
  int value = std::atoi(argv[3]);
  int nw = std::atoi(argv[4]);
  std::atomic<int> count;
  //long sequential;
  long parallel;

  {
    utimer t("graph gen");
    //writeRandomDAG("graph_prova", N);
    createGraphFromFile(graph_file, g);
    //generateRandomDAG(g, N);
    //std::cout << g << std::endl;
  }

//  count=0;
//  bfs(g,src,value, std::ref(count), std::ref(sequential));
//
//  std::cout << "Sequential, starting from node " << "(" << src << ")" << " the occurences of" << " (" << value << ") are : " << count << std::endl;

  {
    //utimer r("par bfs", &parallel);
    count=0;
    parallelBFS(g,src,value,std::ref(count),nw,std::ref(parallel));
    //std::cout << count <<std::endl;
  }
  std::cout << "Parallel, starting from node " << "(" << src << ")" << " the occurrences of" << " (" << value << ") are : " << count << std::endl;
  //std::cout << "Speedup (" << nw << ") = " << (float) sequential/parallel << std::endl;

  return 0;
}
