#ifndef CHM_CPP
#define CHM_CPP

#include <algorithm>
#include <atomic>
#include <ctype.h>
#include <utility>
#include <fstream>
#include <list>
#include <string>
#include <sstream>
#include <pthread.h>
#include <vector>
#include "ConcurrentHashMap.hpp"

using namespace std;

struct share_data_t {
    ConcurrentHashMap *map;
    atomic<int> &row_index;
    pair<string, unsigned int> *maxPerRow;
};


unsigned int getHashKey(string key){
    unsigned int code = (unsigned int)key[0];
    return code - 97;               // the characters a-z begin in 97
}


ConcurrentHashMap::ConcurrentHashMap() {
    for(int i=0; i<TABLE_SIZE; i++){
        auto lista_temporal = new Lista<pair<string,unsigned int>>;
        tabla[i] = lista_temporal;
        pthread_mutex_init(&_ocupados[i], nullptr);
    }
}

// No estaria borrando completamente, ni funcionando.
/*ConcurrentHashMap::~ConcurrentHashMap() {
    for(int i=0; i<TABLE_SIZE; i++){
        delete[] tabla[i];
        pthread_mutex_destroy(&_ocupados[i]);
    }
}
*/
void ConcurrentHashMap::addAndInc(string key) {
    unsigned int position = getHashKey(key);
    // Bloqueo la posicion de la tabla de hash para poder trabajar concurrentemente.
    pthread_mutex_lock(&_ocupados[position]);
    Lista<pair<string, unsigned int>>::Iterador it = tabla[position]->CrearIt();
    bool found = false;
    while(it.HaySiguiente()){
        if(it.Siguiente().first == key){
            found = true;
            it.Siguiente().second+=1;
            // La desbloqueo para que otro trabaje con ella.
            pthread_mutex_unlock(&_ocupados[position]);
            break;
        }else{
            it.Avanzar();
        }
    }
    if(!found){
        tabla[position]->push_front(make_pair(key, 1));
        // La desbloqueo para que otro trabaje con ella.
        pthread_mutex_unlock(&_ocupados[position]);
    }
}

list<string> ConcurrentHashMap::keys() {
    list<string> partial_keys;
    Lista<pair<string, unsigned int>>::Iterador it;
    for(int position=0; position<TABLE_SIZE; position++){
        it = tabla[position]->CrearIt();
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
    Lista<pair<string, unsigned int>>::Iterador it = tabla[position]->CrearIt();
    while(it.HaySiguiente()){
        if(it.Siguiente().first == key){
            value = it.Siguiente().second;
        }
        it.Avanzar();
    }
    return value;
}

pair<string, unsigned int> ConcurrentHashMap::getMaximumInRow(int position) {
    pair<string, unsigned int> max_pair = make_pair("",0);
    Lista<pair<string, unsigned int>>::Iterador it = tabla[position]->CrearIt();
    while(it.HaySiguiente()){
        if(it.Siguiente().second > max_pair.second){
            max_pair = it.Siguiente();
        }
        it.Avanzar();
    }
    return max_pair;
}

void *threadFindMax(void *arg) {
    auto *share_data = (share_data_t *)arg;
    while(true){
        int index = share_data->row_index.fetch_add(1);
        if (index >= TABLE_SIZE)
            break;
        *(share_data->maxPerRow + index) = share_data->map->getMaximumInRow(index);
    }
    return nullptr;
}


pair<string, unsigned int> ConcurrentHashMap::maximum(unsigned int n) {
    pair<string, unsigned int> maxPair = make_pair("", 0);
    int tid;
    pthread_attr_t attr;
    pthread_t thread[n];
    int rc;


    for (auto &_ocupado : _ocupados) {
        pthread_mutex_lock(&_ocupado);
    }

    pthread_mutex_t _procesando[TABLE_SIZE];

    for (auto &i : _procesando) {
        pthread_mutex_init(&i, nullptr);
    }

    pair<string, unsigned int> *pShared_maxPerCell;
    pair<string, unsigned int> shared_maxPerCell[TABLE_SIZE];
    pShared_maxPerCell = shared_maxPerCell;
    atomic<int> file_queue_index(0);
    share_data_t share_data = {this, file_queue_index, pShared_maxPerCell};

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for(tid=0; tid<n; tid++){
        pthread_create(&thread[tid], &attr, &threadFindMax, &share_data);
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
        if(maxPair.second < (*(share_data.maxPerRow + i)).second){
            maxPair = share_data.maxPerRow[i];
        }
    }


    for (auto &i : _procesando) {
        pthread_mutex_destroy(&i);
    }

    for (auto &_ocupado : _ocupados) {
        pthread_mutex_unlock(&_ocupado);
    }

    return maxPair;
}

vector<string> split_line(string &line, char delim) {
    stringstream line_stream(line);
    vector<string> words;
    string word;
    while(getline(line_stream, word, delim)) {
        words.push_back(word);
    }
    return words;
}


ConcurrentHashMap countWordsInFile(string filePath) {
    ConcurrentHashMap map;
    string line;
    ifstream file(filePath);
    if(file.is_open()) {
        while (getline(file, line)) {
            vector<string> words = split_line(line, ' ');
            for (auto &word: words) {
                map.addAndInc(word);
            }
        }
    }else{
        cout << "Couldn't open file " << filePath << endl;
    }
    file.close();
    return map;
}

struct map_and_file_t {
    ConcurrentHashMap *map{};
    string filePath;
};

void *threadCountWordsInFile(void *arg) {
    auto *map_and_file = (map_and_file_t*)arg;

    string line, word;
    unsigned long lastSpace, nextSpace;
    ifstream file(map_and_file->filePath);
    if(file.is_open()) {
        while (getline(file, line)) {
            vector<string> words = split_line(line, ' ');
            for (auto &word: words) {
                map_and_file->map->addAndInc(word);
            }
        }
    }else{
        cout << "Couldn't open file " << map_and_file->filePath << endl;
    }
    free(map_and_file);
    return nullptr;
}

ConcurrentHashMap countWordsOneThreadPerFile(list <string> filePaths) {
    unsigned long cantFiles = filePaths.size();
    int tid = 0;
    pthread_attr_t attr;
    pthread_t thread[cantFiles];
    int rc;

    map_and_file_t *map_and_file;
    auto *pSharedMap = new ConcurrentHashMap;


    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    string filePath;
    for(auto const& file : filePaths){
        map_and_file = new map_and_file_t;
        map_and_file->map = pSharedMap;
        map_and_file->filePath = file;
        pthread_create(&thread[tid], &attr, &threadCountWordsInFile, map_and_file);
        tid++;
    }

    pthread_attr_destroy(&attr);

    for(tid=0; tid<cantFiles; tid++){
        rc = pthread_join(thread[tid], nullptr);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    return *pSharedMap;

}

struct thread_argument_c {
    atomic<int> &file_queue_index;
    vector<string> &file_queue;
    ConcurrentHashMap &hm;
};

//        #include <pthread.h>
//
//       int pthread_create(pthread_t *restrict thread,
//              const pthread_attr_t *restrict attr,
//              void *(*start_routine)(void*), void *restrict arg);


void* count_file(void* args) {
    thread_argument_c *targs = (thread_argument_c *) args;

    while (true) {
        int index = targs->file_queue_index.fetch_add(1);
        if (index >= targs->file_queue.size())
            break;

        ifstream file(targs->file_queue[index]);
        string line;
        if (file.is_open()) {
            while (getline(file, line)) {
                vector<string> words = split_line(line, ' ');
                for (auto &word: words) {
                    targs->hm.addAndInc(word);
                }
            }
        } else {
            cerr << "Problema abriendo archivo en count_file de countWordsArbitraryThreads" << endl;
            continue;
        }
        file.close();
    }
    return nullptr;
}

ConcurrentHashMap countWordsArbitraryThreads(unsigned int n, list <string> filePaths) {
    atomic<int> file_queue_index(0);
    vector<string> file_queue{filePaths.begin(), filePaths.end()};
    ConcurrentHashMap hm;
    unsigned int tid;

    thread_argument_c targs = {file_queue_index, file_queue, hm};
    pthread_t threads[n];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (tid = 0; tid < n; tid++) {
        pthread_create(&threads[tid], &attr, &count_file, &targs);
    }

    for (tid = 0; tid < n; tid++) {
        pthread_join(threads[tid], nullptr);
    }

    pthread_attr_destroy(&attr);

    return hm;    
}

struct thread_argument_many_maps {
    atomic<int> &file_queue_index;
    vector<string> &file_queue;
    vector<ConcurrentHashMap> &hm;
};

void* count_file_many_maps(void* args) {
    thread_argument_many_maps *targs = (thread_argument_many_maps *) args;

    while (true) {
        int index = targs->file_queue_index.fetch_add(1);
        if (index >= targs->file_queue.size())
            break;

        ifstream file(targs->file_queue[index]);
        string line;
        if (file.is_open()) {
            while (getline(file, line)) {
                vector<string> words = split_line(line, ' ');
                for (auto &word: words) {
                    targs->hm[index].addAndInc(word);
                }
            }
        } else {
            cerr << "Problema abriendo archivo en count_file_many_maps de maximumOne" << endl;
            continue;
        }
        file.close();
    }
    return nullptr;
}

void* join_many_maps(void* args) {
    thread_argument_many_maps *targs = (thread_argument_many_maps *) args;

    while (true) {
        int index = targs->file_queue_index.fetch_add(1);
        if (index >= targs->hm.size())
            break;

        for(auto &key : targs->hm[index].keys()) {
            for(int time=0; time<targs->hm[index].value(key); time++){
                targs->hm[0].addAndInc(key);
            }
        }
    }
    return nullptr;
}

pair<string, unsigned int>  maximumOne(unsigned int readingThreads, unsigned int maxingThreads, list <string> filePaths) {
    int n = filePaths.size();
    int rc;
    std::vector<ConcurrentHashMap> maps;

    for(int i=0; i<n; i++){
        auto mapa = *new ConcurrentHashMap;
        maps.push_back(mapa);

    }

    pthread_t readingThread[readingThreads];
    pthread_t maxingThread[maxingThreads];

    atomic<int> file_queue_index(0);
    vector<string> file_queue{filePaths.begin(), filePaths.end()};
    unsigned int tid;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    thread_argument_many_maps targs = {file_queue_index, file_queue, maps};

    for (tid = 0; tid < readingThreads; tid++) {
        pthread_create(&readingThread[tid], &attr, &count_file_many_maps, &targs);
    }

    for (tid = 0; tid < readingThreads; tid++) {
        rc = pthread_join(readingThread[tid], nullptr);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    file_queue_index = 1;
    for (tid = 0; tid < maxingThreads; tid++) {
        pthread_create(&maxingThread[tid], &attr, &join_many_maps, &targs);
    }

    for (tid = 0; tid < maxingThreads; tid++) {
        rc = pthread_join(maxingThread[tid], nullptr);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    pthread_attr_destroy(&attr);

    pair<string, unsigned int> maxPair = maps[0].maximum(maxingThreads);

    return maxPair;
}

pair<string, unsigned int>  maximumTwo(unsigned int readingThreads, unsigned int maxingThreads, list <string> filePaths) {
    ConcurrentHashMap map;
    map = countWordsArbitraryThreads(readingThreads, filePaths);

    pair<string, unsigned int> maxPair = make_pair("", 0);
    maxPair = map.maximum(maxingThreads);

    return maxPair;
}

#endif
