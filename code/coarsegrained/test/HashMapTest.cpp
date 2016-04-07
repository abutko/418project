#include <iostream>
#include <string>
#include <stdlib.h>
#include <assert.h>
#include "../src/HashMap.h"
#include <pthread.h>

using namespace std;

#define NUM_THREADS 24

struct MyKeyHash {
    unsigned long operator()(const int& k) const
    {
        return k % 10;
    }
};

HashMap<int, string, MyKeyHash> hmap;

void *threadRoutine(void *arg) {
    int threadNum = (long) arg;
    for(int i = threadNum; i < 10000; i+=NUM_THREADS) {
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
}

int main() 
{
	//HashMap<int, string, MyKeyHash> hmap;
    /*
    for(int i = 0; i < 10000; i++) {
        hmap.put(i, "1");//std::to_string(i));
        string value;
        hmap.get(i, value);
        assert(value == "1");//std::to_string(i));
    }

	hmap.put(1, "1");
	hmap.put(2, "2");
	hmap.put(3, "3");

	string value;
	bool result = hmap.get(2, value);
	assert(result);
	assert(value == "2");

	result = hmap.get(3, value);
	assert(result);
	assert(value == "3");

	hmap.remove(3);
	result = hmap.get(3, value);
	assert(!result);*/
    pthread_t threads[NUM_THREADS];
    for(int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, threadRoutine, (void *)i);

    for(int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

	cout << "All tests passed!" << endl;
    pthread_exit(NULL);
}
