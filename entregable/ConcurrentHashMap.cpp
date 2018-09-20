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

unsigned int getHashKey(string key){
    unsigned int code = (unsigned int)key[0];
    return code - 97;               // the characters a-z begin in 97
}


ConcurrentHashMap::ConcurrentHashMap() {
    for(int i=0; i<26; i++){
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
    Lista<pair<string, int>>::Iterador it = _hash_table[position].CrearIt();
    bool found = false;
    while(it.HaySiguiente()){
        if(it.Siguiente().first == key){
            found = true;
            it.Siguiente().second+=1;
        }
    }
    if(!found){
        _hash_table->push_front(make_pair(key, 1));
    }
    // La desbloqueo para que otro trabaje con ella.
    pthread_mutex_unlock(&_ocupados[position]);
}

list<string> ConcurrentHashMap::keys() {
    list<string> partial_keys;
    Lista<pair<string, int>>::Iterador it;
    for(int position=0; position<26; position++){
        it = _hash_table[position].CrearIt();
        while(it.HaySiguiente()){
            partial_keys.push_back(it.Siguiente().first);
        }
    }
    return partial_keys;
}

unsigned int ConcurrentHashMap::value(string key) {
    unsigned int value = 0;
    unsigned int position = getHashKey(key);
    Lista<pair<string, int>>::Iterador it = _hash_table[position].CrearIt();
    while(it.HaySiguiente()){
        if(it.Siguiente().first == key){
            value = (unsigned int) it.Siguiente().second;
        }
    }
    return value;
}

pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int n) {
    // Completar
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