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

#define TABLE_SIZE 26

class ConcurrentHashMap {
    public:
        ConcurrentHashMap();

        ~ConcurrentHashMap();

        void addAndInc(string key);

        list<string> keys();

        unsigned int value(string key);

        pair<string, unsigned int> maximum(unsigned int n);

        friend Test;

    private:
        Lista<pair<string, unsigned int>> *tabla[TABLE_SIZE];

        pthread_mutex_t _ocupados[TABLE_SIZE];

        pair<string,unsigned int> getMaximumInCell(unsigned int position);

        void *threadFindMax(void *arg);


};

static ConcurrentHashMap countWordsInFile(string filePath);

static ConcurrentHashMap countWordsOneThreadPerFile(list <string> filePaths);

static ConcurrentHashMap countWordsArbitraryThreads(unsigned int n, list <string> filePaths);

static pair<string, unsigned int> maximumOne(unsigned int readingThreads, unsigned int maxingThreads, list <string> filePaths);

static pair<string, unsigned int> maximumTwo(unsigned int readingThreads, unsigned int maxingThreads, list <string> filePaths);


#endif
