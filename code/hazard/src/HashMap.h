#include "HashNode.h"
#include "KeyHash.h"
#include <pthread.h>
#include <mutex> 

// Hash map class template
template <typename K, typename V, typename F = KeyHash<K> >
class HashMap {
public:
    HashMap() {
        // construct zero initialized hash table of size
        table = new HashNode<K, V> *[TABLE_SIZE]();
        bucketLocks = new std::mutex [TABLE_SIZE]();
        pthread_rwlock_init(&tableLock, NULL);
        capacity = TABLE_SIZE;
    }

    ~HashMap() {
        // destroy all buckets one by one
//        for (int i = 0; i < TABLE_SIZE; ++i) {
        for (int i = 0; i < capacity; ++i) {
            HashNode<K, V> *entry = table[i];
            while (entry != NULL) {
                HashNode<K, V> *prev = entry;
                entry = entry->getNext();
                delete prev;
            }
            table[i] = NULL;
        }
        pthread_rwlock_destroy(&tableLock);
        delete [] bucketLocks;

        // destroy the hash table
        delete [] table;

    }

    bool get(const K &key, V &value, HPNode **HP, int threadNum) {
        int ind = 3 * threadNum;
        HPNode *hp0 = HP[ind]; HPNode *hp1 = HP[ind+1];HPNode *hp2 = HP[ind+2];
        HashNode<K, V> *head = table[hashFunc(key) % capacity];

        bool result = find(head, key, hp0, hp1, hp2);
        hp0->ptr = NULL; hp1->ptr = NULL; hp2->ptr = NULL;
        return result;        
    }

    void put(const K &key, const V &value, HPNode **HP, int threadNum) {
        int ind = 3 * threadNum;
        HPNode *hp0 = HP[ind]; HPNode *hp1 = HP[ind+1];HPNode *hp2 = HP[ind+2];
        HashNode<K, V> *head = table[hashFunc(key) % capacity];

    }

    void remove(const K &key, HPNode **HP, int threadNum) {
        int ind = 3 * threadNum;
        HPNode *hp0 = HP[ind]; HPNode *hp1 = HP[ind+1];HPNode *hp2 = HP[ind+2];
        HashNode<K, V> *head = table[hashFunc(key) % capacity];

    }

private:
    // hash table
    HashNode<K, V> **table;
    // hash function
    F hashFunc;
    // capacity
    int capacity;
    // size
    int size;

    bool find(HashNode<K,V> *head, const K &key, 
              HPNode *hp0, HPNode *hp1, HPNode *hp2) {
    try_again:
        HashNode<K,V> *prev = head;
        HashNode<K,V> *cur = prev->get_unmarked();
        int pmark   = prev->is_marked();
        hp1->ptr = cur;
        if (prev->get_marked() != cur) goto try_again;
        while (true) {
            if (!cur) return false;
            HPNode *next = cur->get_unmarked();
            bool cmark = cur->is_marked();
            hp0->ptr = next;
            HPNode *orig = (HPNode*)((long)next || cmark);
            if (orig != cur->get_marked) goto try_again;
            K ckey =  cur.getKey();
            if (prev->get_unmarked() != cur) goto try_again;
            if (!cmark) {
                if (ckey >= key) return ckey == key;
                // prev <- &cur^.{mark,next}
                prev = cur;
                hp2->ptr = cur;
            }
            else {
                if (__sync_bool_compare_and_swap(prev, cur, next))
                    // DeleteNode(cur)
                else goto try_again;
            } 
        }
    }



};
