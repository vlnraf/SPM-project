#include <fstream>
#include "Utimer.hpp"

#define PERCENT 2

void writeRandomDAG(std::string filename, int N){
  std::ofstream graph_file(filename + std::to_string(N) + ".txt", std::ofstream::out | std::ofstream::trunc);
  graph_file << N << std::endl;
  for(int i=0; i<N; i++){
    for(int j=i+1; j<N; j++){
      if((rand() % 100) < PERCENT){
        graph_file << i << " " << j << " " << std::endl;
      }
    }
  }
  graph_file.close();
}


int main(int argc, char* argv[]){
  std::srand(std::atoi(argv[1]));
  int N = std::atoi(argv[2]);

  {
    utimer t("graph generation");
    writeRandomDAG("graph", N);
  }
  return 0;
}

