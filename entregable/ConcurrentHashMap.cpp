#ifndef CHM_CPP
#define CHM_CPP

#include <algorithm>
#include <atomic>
#include <ctype.h>
#include <utility>
#include <fstream>
#include <list>
#include <string>
#include <pthread.h>
#include "ConcurrentHashMap.hpp"

using namespace std;

struct share_data_t {
    bool cellsTaken[TABLE_SIZE] = {false};
    pair<string, unsigned int> maxPerCell[TABLE_SIZE];
};

unsigned int getHashKey(string key){
    unsigned int code = (unsigned int)key[0];
    return code - 97;               // the characters a-z begin in 97
}


ConcurrentHashMap::ConcurrentHashMap() {
    for(int i=0; i<TABLE_SIZE; i++){
        // Creo que no es necesario, estoy oxidado con c++ classes
        // Lista<pair<string,int>> lista_temporal;
        // _hash_table[i] = lista_temporal;
        pthread_mutex_init(&_ocupados[i], nullptr);
    }
}

void ConcurrentHashMap::addAndInc(string key) {
    unsigned int position = getHashKey(key);
    // Bloqueo la posicion de la tabla de hash para poder trabajar concurrentemente.
    pthread_mutex_lock(&_ocupados[position]);
    Lista<pair<string, unsigned int>>::Iterador it = _hash_table[position].CrearIt();
    bool found = false;
    while(it.HaySiguiente()){
        if(it.Siguiente().first == key){
            found = true;
            it.Siguiente().second+=1;
            break;
        }
    }
    if(!found){
        _hash_table[position].push_front(make_pair(key, 1));
    }
    // La desbloqueo para que otro trabaje con ella.
    pthread_mutex_unlock(&_ocupados[position]);
}

list<string> ConcurrentHashMap::keys() {
    list<string> partial_keys;
    Lista<pair<string, unsigned int>>::Iterador it;
    for(int position=0; position<TABLE_SIZE; position++){
        it = _hash_table[position].CrearIt();
        while(it.HaySiguiente()){
            partial_keys.push_back(it.Siguiente().first);
            it.Avanzar();
        }
    }
    return partial_keys;
}

unsigned int ConcurrentHashMap::value(string key) {
    unsigned int value = 0;
    unsigned int position = getHashKey(key);
    Lista<pair<string, unsigned int>>::Iterador it = _hash_table[position].CrearIt();
    while(it.HaySiguiente()){
        if(it.Siguiente().first == key){
            value = it.Siguiente().second;
        }
    }
    return value;
}

pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int n) {
    share_data_t share_data;
    pair<string, unsigned int> maxPair = make_pair("", 0);
    int tid;
    pthread_attr_t attr;
    pthread_t thread[n];
    int rc;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for(tid=0; tid<n; tid++){
        pthread_create(&thread[tid], &attr, threadFindMax, &share_data);
    }

    pthread_attr_destroy(&attr);

    for(tid=0; tid<n; tid++){
        rc = pthread_join(thread[tid], nullptr);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for(int i=0; i<TABLE_SIZE; i++){
        if(maxPair.second < share_data.maxPerCell[i].second){
            maxPair = share_data.maxPerCell[i];
        }
    }
    return maxPair;
}

pair<string, unsigned int> ConcurrentHashMap::getMaximumInCell(unsigned int position) {
    pair<string, unsigned int> max_pair = make_pair("",0);
    Lista<pair<string, unsigned int>>::Iterador it = _hash_table[position].CrearIt();
    while(it.HaySiguiente()){
        if(it.Siguiente().second > max_pair.second){
            max_pair = it.Siguiente();
        }
    }
    return max_pair;
}

void *ConcurrentHashMap::threadFindMax(void *arg) {
    bool allTaken;
    int taken;
    share_data_t *share_data = (share_data_t *)arg;
    while(true){
        allTaken = true;
        for(unsigned int i=0; i<TABLE_SIZE; i++){
            if(!share_data->cellsTaken[i]){
                taken = pthread_mutex_trylock(&_ocupados[i]);
                if(taken){
                    share_data->cellsTaken[i] = true;
                    share_data->maxPerCell[i] = getMaximumInCell(i);
                }else{
                    allTaken = false;
                }
            }
        }
        if(allTaken){
            break;
        }
    }
    return nullptr;
}

static ConcurrentHashMap countWordsInFile(string filePath) {
    // Completar
}

static ConcurrentHashMap countWordsOneThreadPerFile(list <string> filePaths) {
    // Completar
}

static ConcurrentHashMap countWordsArbitraryThreads(unsigned int n, list <string> filePaths) {
    // Completar
}

static pair<string, unsigned int>  maximumOne(unsigned int readingThreads, unsigned int maxingThreads, list <string> filePaths) {
    // Completar
}

static pair<string, unsigned int>  maximumTwo(unsigned int readingThreads, unsigned int maxingThreads, list <string> filePaths) {
    // Completar
}

#endif