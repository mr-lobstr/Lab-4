#include <iostream>
#include <fstream>
#include <thread>
#include <random>
#include <chrono>
#include <string>
#include <mutex>
#include <condition_variable>
using namespace std;
using namespace chrono;


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
condition_variable cv;
bool outIsFree = true;
ofstream out ("out.txt");


void worker(int id, mutex& mtx, int symbolCnt) {

    auto start = high_resolution_clock::now();

    {
        unique_lock<mutex> lock(mtx);
        
        if (not outIsFree) {
            cv.wait (lock, [&] { return outIsFree; });
        }

        outIsFree = false;
        out << "Thread " << id << ": " << random_string (symbolCnt) << endl;
        outIsFree = true;
        cv.notify_one();
    }

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
    
    mutex mtx;
    
    vector<thread> threads (threadsCnt);

    int i = 0;
    for (auto& th : threads) {
        th = thread(worker, i++, ref(mtx), symbolCnt);
    }

    for (auto& th : threads) {
        th.join();
    }
}
