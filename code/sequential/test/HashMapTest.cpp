#include <iostream>
#include <string>
#include <stdlib.h>
#include <assert.h>
#include "../src/HashMap.h"

#include "CycleTimer.h"

using namespace std;

struct MyKeyHash {
    unsigned long operator()(const int& k) const
    {
        return k % 10;
    }
};

HashMap<int, string, MyKeyHash> hmap;

/* PERFORMANCE TESTING
 * ----------------------------------------------------------- */
// Only performance testing 75% reads, rest writes
void mostlyReads() {
    for(int i = 0; i < 100000; i++) {
        hmap.put(i, to_string(i));
        string value;
        hmap.get(i, value);
        hmap.get(i+1, value);
        hmap.get(i+2, value);
    }
}

int main() 
{
    double startTime = CycleTimer::currentSeconds();
	//HashMap<int, string, MyKeyHash> hmap;
    /*
    for(int i = 0; i < 100000; i++) {
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
	assert(!result);
    */
    mostlyReads();

	cout << "All tests passed!" << endl;
    double endTime = CycleTimer::currentSeconds();
    printf("Total Test Time = \t\t[%.3f] ms\n", endTime - startTime);
}
