#include <time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "ConcurrentHashMap.hpp"

#define MILLION 1000000L

int main(int argc, char const *argv[]) {
    struct timespec start, stop;
    clockid_t clk_id;
    clk_id = CLOCK_MONOTONIC;

    list<string> l = {"corpus-0", "corpus-1", "corpus-2", "corpus-3", "corpus-4"};
    std::vector<double> v[2];
    for (int i = 1; i <= 30; ++i) {
        printf("Tamaño: %d\n", i);
        vector<double> resultados[2];
        for (int k = 1; k <= 50; ++k) {
            clock_gettime(clk_id, &start);
            maximumOne(1, i, l);
            clock_gettime(clk_id, &stop);

            double diff1 = (stop.tv_sec - start.tv_sec) * MILLION +
                           (stop.tv_nsec - start.tv_nsec) / MILLION;

            if (diff1 < 10000) {
                resultados[0].push_back(diff1);
            }

            clock_gettime(clk_id, &start);
            maximumTwo(1, i, l);
            clock_gettime(clk_id, &stop);

            double diff2 = (stop.tv_sec - start.tv_sec) * MILLION +
                           (stop.tv_nsec - start.tv_nsec) / MILLION;

            if (diff2 < 10000){
                resultados[1].push_back(diff2);
            }

        }

        double asd = 0;
        for (std::vector<double>::iterator it = resultados[0].begin(); it != resultados[0].end(); ++it) {
            asd += *it;
        }
        v[0].push_back(asd/resultados[0].size());


        asd = 0;
        for (std::vector<double>::iterator it = resultados[1].begin(); it != resultados[1].end(); ++it) {
            asd += *it;
        }
        v[1].push_back(asd/resultados[1].size());

    }


    // Guardo los resultados en un archivo de texto
    ofstream outfile;
    outfile.open("tiempos.csv", ios::out);

    // Escribo todos los resultados en el archivo
    int asd = 1;
    for (uint i = 0; i < v[0].size(); ++i) {
        outfile << v[1][i] << ',' << endl;
        asd += 1;
    }

    outfile.close();


    return 0;
}
