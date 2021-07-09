#ifndef BARRIER
#define BARRIER

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

class Barrier {
private:

    std::mutex d_mutex;
    std::condition_variable d_condition;
    int counter, nw, iteration;

public:
    Barrier(int num_w): counter(num_w), nw(num_w), iteration(0) {}

    void barrier_wait(int tid) {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        auto prev_iter = iteration;
        //std::cout<<"ciao";
        if (!--counter) {
            iteration++;
            counter = nw;
            std::cout<<tid << " ciao"<<std::endl;
            std::cout<<"--------"<<std::endl;
            d_condition.notify_all();
        } else {
            std::cout<<tid << " ciao"<<std::endl;
            d_condition.wait(lock, [this, prev_iter] { return prev_iter != iteration; });
        }
    }

};

#endif
