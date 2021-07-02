#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
//#include <opm.h>

#include "graph.hpp"
#include "Utimer.hpp"
#include "Queque.hpp"

int Node::currID = 0; //Initialize the id from where you want to start

void bfs(Graph &g, int src, int val, int &count){
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
      //std::cout<<node.getID()<<std::endl;
      if(node.getValue() == val) count ++;
      fs.pop();

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

std::mutex queue;
std::mutex c;

void parallelBFS(Graph &g, int src, int val, int &count, int nw){
  std::vector<std::thread> tids;
  int turn = 0;

  std::vector<std::queue<int>> firstQueue(nw); //vector of queue to assign the queue at each thread
  std::vector<std::queue<int>> secondQueue(nw);   //same but this is used for update the next frontiear
  std::vector<bool> visited(g.getNodes().size());
  Node n = g[src];
  if(n.getValue() == val) count++;
  std::vector<Node> tmp = n.getDestination();
  int tmpSize = tmp.size();

  for(int i=0; i<tmpSize; i++){
    firstQueue[turn].push(tmp[i].getID());
    visited[tmp[i].getID()] = true;
    turn = (turn + 1) % nw;
  }

  auto myjob = [&](int tid){
    /* function that each node of the farm compute
     */ 
    //std::cout<<"ququq size : " << firstQueue[tid].size()<<std::endl;
    while(!firstQueue[tid].empty()){ //while the queue assigned to the thread is not empty
      while(!firstQueue[tid].empty()){
        c.lock();
        Node n = g[firstQueue[tid].front()];
        //std::cout<<n.getID()<<std::endl;
        if(n.getValue() == val){
          count++;
        }
        //std::cout << "I am thread : " << tid << "->" << n.getID() << std::endl;
        firstQueue[tid].pop();
        c.unlock();

        for(auto neighbor: n.getDestination()){
          if(!visited[neighbor.getID()]){
            //std::cout << neighbor.getID()<<std::endl;
            secondQueue[tid].push(neighbor.getID());
            visited[neighbor.getID()] = true;
            }
        }
      }
      firstQueue[tid] = secondQueue[tid];
      std::cout<<"ququq size of thread(" << tid << ") :" << firstQueue[tid].size()<<std::endl;
      while(!secondQueue[tid].empty()) secondQueue[tid].pop();
    }
  };

  for(int i=0; i<nw; i++){
    tids.push_back(std::thread(myjob, i));
  }
  for(std::thread& t: tids){
    t.join();
  }
}



int main(int argc, char* argv[]){

  if(argc < 2){
    std::cout << "Verra' utilizzato 10 come seed di default" << std::endl;
    std::srand(10);
  }else{
    std::srand(std::atoi(argv[1]));
  }

  Graph g;
  std::string filename = "../image_graph/image.gv";
  int N = atoi(argv[2]);
  int nw = atoi(argv[3]);
  int count = 0;

  std::vector<std::thread> tids;

  generateRandomDAG(g, N);

  {
    utimer t("bfs");
    count=0;
    bfs(g,0,0, std::ref(count));
    std::cout << count << std::endl;
  }

  {
    utimer r("par bfs");
    count=0;
    parallelBFS(g,0,0,std::ref(count),nw);
    std::cout << count <<std::endl;
  }

  //{
  //  utimer to("parallel bfs");
  //  count=0;
  //  for(int i=0; i<nw; i++){
  //    tids.push_back(std::thread(bfs, std::ref(g), 0, 0, std::ref(count)));
  //  }
  //  for(std::thread& t: tids){
  //    t.join();
  //  }
  //  std::cout << count << std::endl;
  //}

  //writeGraphImage(g, filename);

  // std::cout << g << std::endl;

  return 0;
}
