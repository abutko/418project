#include <iostream>
#include <string>
#include <stdlib.h>
#include <cmath>
#include <assert.h>
#include "../src/HashMap.h"
#include <pthread.h>
#include <random>
#include "CycleTimer.h"

using namespace std;

#define NUM_THREADS 24
// Uncomment the following for correctness testing
//#define DEBUG
// Uncomment the following for better (but slower) correctness testing for lock-free
//#define CONTENTION

int   OPS    = 1000000;
int   RANGE  = 1000;
float INSERT = 0.33;
float DELETE = 0.33;
float SEARCH = 0.34;

struct MyKeyHash {
    unsigned long operator()(const int& k) const
    {
#ifdef CONTENTION
        return 0;
#endif
        return k*48611;
    }
};

// Simple linked list queue used to keep track of insertions 
// To perform corresponding deletions
struct node {
    struct node *next;
    int key;
};

struct list {
    struct node *front;
    struct node *back;
};

void list_insert(struct list *L, int x) {
    L->back->key = x;
    struct node *add = new node;
    L->back->next = add;
    L->back = add;
}

struct list *list_new() {
    struct list *L = new list;
    struct node *dummy = new node;
    L->front = dummy; L->back = dummy;
    return L;
}

int list_remove(struct list *L) {
    int x = L->front->key;
    L->front = L->front->next;
    return x;
}

// The hashtable declaration
HashMap<int, int, MyKeyHash> hmap;

// PER-THREAD PERFORMANCE TESTING
/*******************************************************************************/
void *performance(void *arg) {
    int threadNum = (long) arg;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, RANGE - 1);

    struct list *L = list_new();

    for(int i = 0; i < OPS; i++) {
        // choose element uniformly over specified range
        int key = dis(gen);
        int val = 0;
        // use to choose operation based on given probabilities 
        float p = (float)rand()/(float)(RAND_MAX);
        if (p < INSERT) { 
            // choose insert
            hmap.put(key, val);  
            list_insert(L, key); 
        }
        else if (p < INSERT + DELETE) { 
            // choose delete, only on elements this thread inserted
            if (L->front != L->back) {
                // if there are still elements to be deleted, actually do it
                int x = list_remove(L); 
                hmap.remove(x);
            }
        }
        else { 
            // choose search 
            int value;
            hmap.get(key, value);   
        }
    }
    return NULL;
}
/*******************************************************************************/

// PER-THREAD CORRECTNESS TESTING
/*******************************************************************************/
void *correctness(void *arg) {
    // If you want correctness testing on higher contention then
    // just uncomment the CONTENTION flag on the top of the file, this
    // just makes the hashtable essentially become a linked list
    // (only suitable for lock-free ofc)
    int threadNum = (long) arg;

    for(int i = threadNum; i < 100000; i+=NUM_THREADS) {
        int key = i;
        int val = 0;
        hmap.put(key, val);
        int value;
        bool result = hmap.get(key, value);
        assert(result);
        assert(value == 0);
    }
    
    for(int i = threadNum; i < 100000; i+=NUM_THREADS) {
        int key = i;
        int value;
        hmap.remove(key);
        bool result = hmap.get(key, value);
        assert(!result);
    }

    return NULL;
}
/*******************************************************************************/

int main(int argc, char **argv) 
{

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-elems") == 0) {
            // size of the universe
            int top = atoi(argv[i+1]);
            if (top == 0) {
                printf("Invalid range specified\n");
                return 1;
            }
            RANGE = top;
        }
        else if (strcmp(argv[i], "-ops") == 0) {
            // number of operations per thread
            int top = atoi(argv[i+1]);
            if (top == 0) {
                printf("Invalid range specified\n");
                return 1;
            }
            RANGE = top;
        }
        else if (strcmp(argv[i], "-put") == 0) {
            // relative proportion of inserts
            float put = atof(argv[i+1]);
            if (put == 0) {
                printf("Invalid insert argument\n");
                return 1;
            }
            INSERT = put;
        }
        else if (strcmp(argv[i], "-get") == 0) {
            // relative proportion of searches
            float get = atof(argv[i+1]);
            if (get == 0) {
                printf("Invalid search argument\n");
                return 1;
            }
            SEARCH = get;
        }
        else if (strcmp(argv[i], "-rem") == 0) {
            // relative proportion of removes
            float rem = atof(argv[i+1]);
            if (rem == 0) {
                printf("Invalid remove argument\n");
                return 1;
            }
            DELETE = rem;
        }
    
    }
    printf("put: %f get: %f rem %f\n", INSERT, SEARCH, DELETE); 
    // need (put+get+rem) ~= 1, these are the relative ratios (probabilities) 
    // of the corresponding operations appearing
    if (abs(INSERT + DELETE + SEARCH - 1.0) > 0.0001 || DELETE > INSERT) {
        printf("Invalid put, get, remove ratios\n");
        return 1;
    }

    double startTime = CycleTimer::currentSeconds();
    pthread_t threads[NUM_THREADS];
    for(long i = 0; i < NUM_THREADS; i++) {
#ifdef DEBUG
        pthread_create(&threads[i], NULL, correctness, (void *)i);
#endif
#ifndef DEBUG
        pthread_create(&threads[i], NULL, performance, (void *)i);
#endif
    }

    for(int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

	cout << "All tests passed!" << endl;
    double endTime = CycleTimer::currentSeconds();
    printf("Total Test Time = \t\t[%.3f] s\n", endTime - startTime);
    pthread_exit(NULL);
}
