#ifndef CHM_HPP
#define CHM_HPP

#include <atomic>
#include <ctype.h>
#include <iostream>
#include <list>
#include <string>
#include <pthread.h>
#include "ListaAtomica.hpp"
#include "test.hpp"

using namespace std;

class ConcurrentHashMap {
    public:
        ConcurrentHashMap();

        void addAndInc(string key);

        list<string> keys();

        unsigned int value(string key);

        pair<string, unsigned int> maximum(unsigned int n);

        friend Test;

    private:
        Lista<pair<string, int>> _hash_table[26];
        pthread_mutex_t _ocupados[26];


};

static ConcurrentHashMap countWordsInFile(string filePath);

static ConcurrentHashMap countWordsOneThreadPerFile(list <string> filePaths);

static ConcurrentHashMap countWordsArbitraryThreads(unsigned int n, list <string> filePaths);

static pair<string, unsigned int> maximumOne(unsigned int readingThreads, unsigned int maxingThreads, list <string> filePaths);

static pair<string, unsigned int> maximumTwo(unsigned int readingThreads, unsigned int maxingThreads, list <string> filePaths);


#endif
