#include <iostream>
#include <string>
#include <stdlib.h>
#include <assert.h>
#include "../src/HashMap.h"
#include <pthread.h>

#include "CycleTimer.h"

using namespace std;

#define NUM_THREADS 24

struct MyKeyHash {
    unsigned long operator()(const int& k) const
    {
        return k*48611;
    }
};

HashMap<int, string, MyKeyHash> hmap;

// PERFORMANCE TESTING
// Only performance testing 75% reads, rest writes
void *mostlyReads(void *arg) {
    int threadNum = (long) arg;
    for(int i = threadNum; i < 10000000; i+=NUM_THREADS) {
        hmap.put(i, to_string(i));
        string value;
        hmap.get(i, value);
        hmap.get(i+1, value);
        hmap.get(i+2, value);
    }
    return NULL;
}

// CORRECTNESS TESTING
void *threadRoutine(void *arg) {
    int threadNum = (long) arg;
    for(int i = threadNum; i < 100000; i+=NUM_THREADS) {
        hmap.put(i, to_string(i));
        string value;
        bool result = hmap.get(i, value);
        assert(value == to_string(i));
    }
    for(int i = threadNum; i < 10000; i+=NUM_THREADS) {
        hmap.remove(i);
        string value;
        bool result = hmap.get(i, value);
        assert(!result);
    }
    return NULL;
}

int main() 
{
    double startTime = CycleTimer::currentSeconds();
    pthread_t threads[NUM_THREADS];
    for(int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, mostlyReads, (void *)i);

    for(int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

	cout << "All tests passed!" << endl;
    double endTime = CycleTimer::currentSeconds();
    printf("Total Test Time = \t\t[%.3f] s\n", endTime - startTime);
    pthread_exit(NULL);
}
