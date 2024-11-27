#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>
using namespace std;
using namespace std::chrono;

enum class Type {
    reader,
    writer
};

Type priority = Type::reader;

mutex mtx, mtx2;
condition_variable cv;

int readersCnt = 0;
bool writing = false;

int writersWait = 0;
int readersWait = 0;

int allThreads = 0;

int sharedData = 10;

size_t rnd (size_t a = 0, size_t b = INT32_MAX) {
    static auto now = system_clock::now().time_since_epoch().count();
    static default_random_engine generator(now);
    static uniform_int_distribution<size_t> distribution(0, UINT64_MAX);

    return a + distribution(generator) % (b - a);
}

void reader (int id) {
    {
        unique_lock<mutex> lock(mtx);

        if (writing) {
            ++readersWait;
            cv.wait (lock, [&] { return not writing; });
            --readersWait;
        }

        if (readersCnt == 0 and priority == Type::writer and writersWait != 0) {
            ++readersWait;
            cv.wait (lock, [&] { 
                return not writing and writersWait == 0;
            });
            --readersWait;
        }

        ++readersCnt;
    }

    this_thread::sleep_for(std::chrono::milliseconds(rnd (10, 1000)));

    {
        unique_lock<mutex> lock(mtx);
        cout << "Читатель " << id<< " прочитал:" << sharedData << endl;
        --readersCnt;

        if (readersCnt == 0) {
            cv.notify_all();
        }

        --allThreads;
    }
}

void writer (int id) {
    {
        unique_lock<mutex> lock(mtx);

        if (writing or readersCnt != 0 or priority == Type::reader and readersWait != 0) {
            ++writersWait;

            cv.wait (lock, [&] {
                if (priority == Type::reader and readersWait != 0) {
                    return false;
                }

                return not writing or readersCnt != 0;
            });

            --writersWait;
        }

        writing = true;
    }

    sharedData = rnd();
    cout << "Писатель " << id << " записал " << sharedData << endl;
    this_thread::sleep_for(std::chrono::milliseconds(rnd (10, 1000)));

    {
        unique_lock<mutex> lock(mtx);
        
        writing = false;
        cv.notify_all();

        --allThreads;
    }
}

int main()
{
    
    cin >> allThreads;
    vector<int> types (allThreads);
    for (auto& t : types) {
        cin >> t;
    }

    for (int i = 0; i < allThreads; ++i) {
        auto type = static_cast<Type> (types[i]);
        thread th ((type == Type::writer ? writer : reader), i);
        th.detach();
    }
 
    while (allThreads > 0)
        ;
}
