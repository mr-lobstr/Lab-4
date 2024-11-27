#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include <algorithm>
#include <list>
#include "directors.h"
using namespace std;
using namespace std::chrono;

struct Film {
    string name;
    size_t year;
    vector<string> directors;
};

size_t rnd (size_t a = 0, size_t b = INT32_MAX) {
    static auto now = system_clock::now().time_since_epoch().count();
    static default_random_engine generator(now);
    static uniform_int_distribution<size_t> distribution(0, UINT64_MAX);

    return a + distribution(generator) % (b - a);
}

string random_string (size_t symbolCnt) {
    string rndStr (' ', symbolCnt);

    for (auto& symb : rndStr) {
        symb = 'a' + rnd ('A', 'Z') % ('z' - 'a');
    }

    return rndStr;
}

Film random_film() {
    Film film;
    
    film.name = random_string (rnd() % 20);
    film.year = (2565 * rnd() % 60) + 1970;

    film.directors.resize (1 + rnd() % 4);

    for (auto& director : film.directors) {
        auto ind = rnd(0, directorsList.size() - 1);
        director = directorsList[ind];
    }

    return film;
}

auto films_gen (size_t cnt) {
    vector<Film> films (cnt);

    for (auto& film : films) {
        film = random_film();
    }

    return films;
}

using filmIt = vector<Film>::const_iterator;


auto just_func (filmIt beg, filmIt end, string const& director) {
    list<Film> res;
    
    for (; beg != end; ++beg) {
        auto& ds = beg->directors;

        if (find (ds.begin(), ds.end(), director) != ds.end()) {
            res.push_back (*beg);
        }
    }

    return res;
}

mutex mtx;
list<Film> listT;

void thread_func (filmIt beg, filmIt end, string const& director) {
    auto res = just_func (beg, end, director);
    
    lock_guard<mutex> lock(mtx);
    listT.splice (listT.end(), res);
}

list<Film> just_test (vector<Film> const& films, string const& director) {
    return just_func (films.begin(), films.end(), director);
}

list<Film> thread_test (vector<Film> const& films, string const& director, size_t threadsCnt) {
    vector<thread> threads (threadsCnt);

    auto rangeSize = films.size() / threadsCnt + 1;
    auto beg = films.begin();

    for (auto& th : threads) {
        auto end = beg + rangeSize;
        end = (end > films.end()) ? films.end() : end;

        th = thread (thread_func, beg, end, ref(director));

        beg = end;
    }

    for (auto& th : threads) {
        th.join();
    }

    return listT;
}

int main()
{
    size_t filmsCnt, threadsCnt;
    string director;
    
    cin >> filmsCnt >> threadsCnt;
    getline (cin, director);
    getline (cin, director);

    auto films = films_gen (filmsCnt);
    auto films2 = films;

    auto start = high_resolution_clock::now();
    auto l1 = just_test (films, director);
    auto finish = high_resolution_clock::now();
    duration<double> duration1 = finish - start;

    cout << "just test time: " << duration1.count() << " seconds" << endl;

    auto start2 = high_resolution_clock::now();
    auto l2 = thread_test (films2, director, threadsCnt);
    auto finish2 = high_resolution_clock::now();
    duration<double> duration2 = finish2 - start2;

    cout << "threads test time: " << duration2.count() << " seconds" << endl;

    cout << "results: " << l1.size() << " " << l2.size() << endl;
}