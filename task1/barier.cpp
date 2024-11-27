#include <iostream>
#include <fstream>
#include <thread>
#include <random>
#include <chrono>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
using namespace std;
using namespace chrono;

class Barrier {
public:
    Barrier(int count)
        : count(count)
        , waiting(0)
        , barrier_broken(false)
    {}

    void wait() {
        unique_lock<mutex> lock(mtx);

        if (barrier_broken)
            return;

        ++waiting;

        if (waiting == count) {
            waiting = 0;
            cv.notify_all();
        } else {
            cv.wait(lock);
        }
    }

    void break_barrier(){
        unique_lock<mutex> lock(mtx);
        barrier_broken = true;
        cv.notify_all();
    }

    bool isBroken(){
        unique_lock<mutex> lock(mtx);
        return barrier_broken;
    }

private:
    int count;
    int waiting;
    mutex mtx;
    condition_variable cv;
    atomic<bool> barrier_broken;
};


size_t rnd (size_t a = 0, size_t b = INT32_MAX) {
    static auto now = system_clock::now().time_since_epoch().count();
    static default_random_engine generator(now);
    static uniform_int_distribution<size_t> distribution(0, UINT64_MAX);

    return a + distribution(generator) % (b - a);
}

string random_string (size_t symbolCnt) {
    string rndStr (symbolCnt, ' ');

    for (auto& symb : rndStr) {
        symb = 'a' + rnd ('A', 'Z') % ('z' - 'a');
    }

    return rndStr;
}


mutex output_mutex;
ofstream out ("out.txt");


void worker(int id, Barrier& barrier, int symbolCnt) {

    auto start = high_resolution_clock::now();

    barrier.wait();
    output_mutex.lock();
    out << "Thread " << id << ": " << random_string (symbolCnt) << endl;
    output_mutex.unlock();

    auto finish = high_resolution_clock::now();
    duration<double> duration = finish - start;

    {
        lock_guard<mutex> lock(output_mutex);
        cout << "Thread " << id << ", time: " << duration.count() << " seconds.\n";
    }
}

int main() {
    int symbolCnt, threadsCnt;
    cin >> symbolCnt >> threadsCnt;
    
    Barrier barrier (threadsCnt - 1);
    
    vector<thread> threads (threadsCnt);

    int i = 0;
    for (auto& th : threads) {
        th = thread(worker, i++, ref(barrier), symbolCnt);
    }

    for (auto& th : threads) {
        th.join();
    }
}
