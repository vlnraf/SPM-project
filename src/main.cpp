#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
//#include <opm.h>

#include "barrier.hpp"
#include "graph.hpp"
#include "Utimer.hpp"
#include "Queque.hpp"

//#define DEBUG

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

std::mutex visitedNodes;
std::mutex jobStealingLocker;
std::mutex messagesLock;
std::mutex nodeLock;

void parallelBFS(Graph &g, int src, int val, std::atomic<int> &count, int nw){
  std::vector<std::thread> tids;
  int turn = 0;

  Barrier barrier(nw);

  std::vector<std::queue<int>> fs(nw); //vector of queue to assign the queue at each thread
  std::vector<std::queue<int>> ns(nw);   //same but this is used for update the next frontiear
  std::vector<std::atomic<bool>> visited(g.getNodes().size());
  Node n = g[src];
  visited[n.getID()] = true;
  if(n.getValue() == val) count++;
  std::vector<Node> tmp = n.getDestination();
  int tmpSize = tmp.size();

  for(int i=0; i<tmpSize; i++){
    fs[turn].push(tmp[i].getID());
    visited[tmp[i].getID()] = true;
    turn = (turn + 1) % nw;
  }

  //auto jobStealing = [&](std::queue<int> q){
  ////  done = true;
  //  jobStealingLocker.lock();
  //  for (int i=0; i<nw; i++){
  // //   done |= !fs[i].empty();
  //    if(fs[i].size() > 4){
  //      std::cout<< " stealing" << std::endl;
  //      for (int i=0; i<(int)fs[i].size()/2; i++){
  //        q.push(fs[i].front());
  //        fs[i].pop();
  //      }
  //      jobStealingLocker.unlock();
  //      return;
  //    }
  //  }
  //  jobStealingLocker.unlock();
  //};

  std::atomic<int> counter;
  counter = nw;

  auto myjob = [&](int tid){
    /* function that each node of the farm compute
     */ 
    bool done = false;
    int level = 0;
    

    while(!done){ //while the queue assigned to the thread is not empty
      //std::this_thread::sleep_for(std::chrono::seconds(3));
      while(!fs[tid].empty()){
        //nodeLock.lock();
        Node n = g[fs[tid].front()];
        if(n.getValue() == val) count++;
        fs[tid].pop();
        //nodeLock.unlock();

        //Uncomment and don't use atomic vector of bool!!!
        //visitedNodes.lock();
        for(auto neighbor: n.getDestination()){
          if(!visited[neighbor.getID()]){
            visited[neighbor.getID()] = true;
            ns[tid].push(neighbor.getID());
            }
        }
        //visitedNodes.unlock();
      }
      //barrier.barrier_wait(tid);
      fs[tid] = ns[tid];
      //messages.lock();
      //std::cout<<"quque size of thread(" << tid << ") :" << fs[tid].size()<< " | done = " << done << "| Level : " << level <<std::endl;
      //messages.unlock();
      while(!ns[tid].empty()) ns[tid].pop();
      level ++;
      if(!fs[tid].empty()) done=false;
      else{ 
        //counter--;
        done=true;
        //barrier.barrier_wait(tid);
        //counter++;
        //do{
        //  barrier.barrier_wait(tid);
        //}while(counter!=nw);
        //while(1){

        //  //std::this_thread::sleep_for(std::chrono::seconds(1));
        //  barrier.barrier_wait(tid);
        //  //messages.lock();
        //  //std::cout<<"( " << tid << ") I am here:" << counter<<std::endl;
        //  //messages.unlock();
        //  if(counter == 0){
        //    //barrier.barrier_wait(tid);
        //    done = true;
        //    break;
        //  }
        //}
      }
    }
    //std::cout<<tid<< " BYE BYE" << std::endl;
  };

  for(int i=0; i<nw; i++){
    tids.push_back(std::thread(myjob, i));
  }
  for(std::thread& t: tids){
    t.join();
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
  std::atomic<int> count ;

  std::vector<std::thread> tids;

  {
    utimer t("graph gen");
    generateRandomDAG(g, N);
  }

  //{
  //  utimer t("bfs");
  //  count=0;
  //  bfs(g,src,value, std::ref(count));
  //  std::cout << count << std::endl;
  //}

  {
    utimer r("par bfs");
    count=0;
    parallelBFS2(g,src,value,std::ref(count),nw);
    std::cout << count <<std::endl;
  }
  //{
  //  utimer r("par bfs2");
  //  count=0;
  //  parallelBFS2(g,0,0,std::ref(count),nw);
  //  std::cout << count <<std::endl;
  //}

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

void parallelBFS2(Graph &g, int src, int val, std::atomic<int> &count, int nw){

  Barrier barrier(nw);

  std::vector<std::queue<int>> fs(nw); //vector of queue to assign the queue at each thread
  std::vector<std::queue<int>> ns(nw);   //same but this is used for update the next frontiear
  std::vector<std::atomic<bool>> visited(g.getNodes().size());
  //std::atomic<int> totalLevel;
  //totalLevel=0;

  int level = 0;
  auto myjob = [&](std::vector<int> &frontier, int tid){
    /* function that each node of the farm compute
     */ 
      //std::this_thread::sleep_for(std::chrono::seconds(3));
    {
      utimer t("thread time (" + std::to_string(tid) + "): ");
      //messagesLock.lock();
      DEBUG_MSG("quque size of thread(" << tid << ") :" << fs[tid].size()<< "| Level : " << level);
      //std::cout<<"quque size of thread(" << tid << ") :" << fs[tid].size()<< "| Level : " << level <<std::endl;
      //messagesLock.unlock();
      while(!fs[tid].empty()){
        nodeLock.lock();
        Node n = g[fs[tid].front()];
        //std::cout<<"(" << tid << ")" << n.getID() <<std::endl;;
        if(n.getValue() == val) count++;
        fs[tid].pop();
        nodeLock.unlock();

        //visitedNodes.lock();
        for(auto neighbor: n.getDestination()){
          if(!visited[neighbor.getID()]){
            visited[neighbor.getID()] = true;
            ns[tid].push(neighbor.getID());
            }
        }
        //visitedNodes.unlock();
      }
      //fs[tid] = ns[tid];
      while(!ns[tid].empty()){
        nodeLock.lock();
        frontier.push_back(ns[tid].front());
        nodeLock.unlock();
        ns[tid].pop();
      }
      //level ++;
      //totalLevel = level;
    }
  };

  bool done = false;
  std::vector<int> frontier;
  while(!done){
    std::vector<std::thread> tids;
    if(level==0){
      std::vector<Node> tmp;
      int turn = 0;
      Node n = g[src];
      visited[n.getID()] = true;
      if(n.getValue() == val) count++;
      tmp = n.getDestination();
      int tmpSize = tmp.size();

      for(int i=0; i<tmpSize; i++){
        fs[turn].push(tmp[i].getID());
        visited[tmp[i].getID()] = true;
        turn = (turn + 1) % nw;
      }
      tmp.clear();
      level++;
    }else{
      if(frontier.size() == 0) done = true;

      int turn = 0;
      for(int i=0; i<(int) frontier.size(); i++){
        fs[turn].push(frontier[i]);
        //visited[tmp[i].getID()] = true;
        turn = (turn + 1) % nw;
      }
      frontier.clear();
      level++;
    }
    if(!done){
      for(int i=0; i<nw; i++){
        if(fs[i].size() != 0)
        tids.push_back(std::thread(myjob, std::ref(frontier), i));
        else
          continue;
      }
      for(std::thread& t: tids){
        t.join();
      }
    }
  }

}
