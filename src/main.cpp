#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <set>
#include <unordered_set>

#include "barrier.hpp"
#include "graph.hpp"
#include "Utimer.hpp"
#include "Queque.hpp"

#define DEBUG

#ifdef DEBUG
#define DEBUG_MSG(str) std::cout << str << std::endl;
#else
#define DEBUG_MSG(str) 
#endif

int Node::currID = 0; //Initialize the id from where you want to start

void bfs(Graph &g, int src, int val, std::atomic<int> &count){
  std::queue<int> fs;
  std::queue<int> ns;
  //std::map<int, bool> visited;
  std::vector<bool> visited(g.getNodes().size());
  fs.push(src);
  visited[src] = true;
  int level = 0;

  while(!fs.empty()){
    int fsSize = fs.size();
    for(int i=0; i<fsSize; i++){
      Node node = g[fs.front()];
      fs.pop();
      //std::cout<<node.getID()<<std::endl;
      if(node.getValue() == val) count ++;

      for(auto neighbor : node.getDestination()){
      //std::cout << "level(" << level << ")" << node.getID() << "->" << neighbor.getID() << std::endl;
      if(!visited[neighbor.getID()]){
        ns.push(neighbor.getID());
        visited[neighbor.getID()] = true;
        }
      }
    }
      fs = ns;
      while(!ns.empty()) ns.pop();
      level ++;
  }
}

void parallelBFS2(Graph &g, int src, int val, std::atomic<int> &count, int nw);

int main(int argc, char* argv[]){

  if(argc < 2){
    std::cerr << "To use the application this is the syntax : ./main [seed] [nodes in graph] [number of workers] [starting node] [value to count]" << std::endl;
    return 1;
  }else{
    std::srand(std::atoi(argv[1]));
  }

  Graph g;
  std::string filename = "../image_graph/image.gv";
  int N = atoi(argv[2]);
  int src = atoi(argv[3]);
  int value = atoi(argv[4]);
  int nw = atoi(argv[5]);
  std::atomic<int> count;
  long sequential, parallel;

  std::vector<std::thread> tids;

  {
    utimer t("graph gen");
    generateRandomDAG(g, N);
  }

  {
    utimer t("bfs", &sequential);
    count=0;
    bfs(g,src,value, std::ref(count));
    std::cout << "Sequential, starting from node " << "(" << src << ")" << " the occurences of" << " (" << value << ") are : " << count << std::endl;
  }

  {
    utimer r("par bfs", &parallel);
    count=0;
    parallelBFS2(g,src,value,std::ref(count),nw);
    std::cout << "Parallel, starting from node " << "(" << src << ")" << "the occurrences of" << " (" << value << ") are : " << count << std::endl;
    //std::cout << count <<std::endl;
  }
  std::cout << "Speedup (" << nw << ") = " << (float) sequential/parallel << std::endl;

  return 0;
}

std::mutex visitedNodes;
std::mutex messagesLock;
std::mutex nodeLock;


void parallelBFS2(Graph &g, int src, int val, std::atomic<int> &count, int nw){

  std::vector<std::queue<int>> fs(nw); //vector of queue to assign the queue at each thread
  std::vector<std::queue<int>> ns(nw);   //same but this is used for update the next frontiear
  //std::vector<std::atomic<bool>> visited(g.getNodes().size()); //Use it if you have problem with result
  std::vector<bool> visited(g.getNodes().size());
  std::unordered_set<int> vis;

  int level = 0;
  auto myjob = [&](std::vector<int> &frontier, int tid){
    /* function that each node of the farm compute
     */ 
    {
      //messagesLock.lock();
      //DEBUG_MSG("quque size of thread(" << tid << ") :" << fs[tid].size()<< "| Level : " << level);
      //messagesLock.unlock();
      while(!fs[tid].empty()){
        //nodeLock.lock();
        Node n = g[fs[tid].front()];
        //nodeLock.unlock();
        if(n.getValue() == val) count++;
        fs[tid].pop();

        for(auto neighbor: n.getDestination()){
          //visitedNodes.lock();
          if(vis.find(neighbor.getID()) == vis.end()){
            ns[tid].push(neighbor.getID());
          }
          //if(!visited[neighbor.getID()]){
          //  //visitedNodes.lock();
          //  visited[neighbor.getID()] = true;
          //  //visitedNodes.unlock();
          //  ns[tid].push(neighbor.getID());
          //  //frontier.push_back(neighbor.getID());
          //  }
          //visitedNodes.unlock();
        }
      }
    }
  };

  std::vector<int> frontier;
  std::vector<std::thread> tids(nw);

  //for(int i=0; i<(int) visited.size(); i++){
    //visited[i] = false;
  //}

  std::vector<Node> tmp;
  Node n = g[src];
  visited[n.getID()] = true;
  if(n.getValue() == val) count++;
  tmp = n.getDestination();

  for(int i=0; i<(int) tmp.size(); i++){
    frontier.push_back(tmp[i].getID());
    visited[tmp[i].getID()] = true;
    vis.insert(tmp[i].getID());
  }
  level++;
  tmp.clear();
  
  while(!frontier.empty()){
    int turn = 0;
    //std::cout << "frontier size : " << frontier.size() << std::endl;

    for(int i=0; i<(int) frontier.size(); i++){
      fs[turn].push(frontier[i]);
      visited[frontier[i]] = true;
      vis.insert(frontier[i]);
      turn = (turn + 1) % nw;
    }
    frontier.clear();

    {
      //utimer pr("parallel time");
      for(int i=0; i<nw; i++){
        //if(fs[i].size() != 0)
        tids[i] = std::thread(myjob, std::ref(frontier), i);
        //else
        //  continue;
      }
      for(std::thread& t: tids){
        t.join();
      }
      level++;
    }

    for(int i=0; i<nw; i++){
      while(!ns[i].empty()){
        if(vis.find(ns[i].front()) == vis.end()){
          vis.insert(ns[i].front());
          frontier.push_back(ns[i].front());
        }
        ns[i].pop();
      }
    }
  }
}
