#include "HashNode.h"
#include "KeyHash.h"
#include <pthread.h>

// Hash map class template
template <typename K, typename V, typename F = KeyHash<K> >
class HashMap {
public:
    HashMap() {
        // construct zero initialized hash table of size
        table = new HashNode<K, V> *[TABLE_SIZE]();
        pthread_rwlock_init(&lock, NULL);
        capacity = TABLE_SIZE;
    }

    ~HashMap() {
        // destroy all buckets one by one
        for (int i = 0; i < TABLE_SIZE; ++i) {
            HashNode<K, V> *entry = table[i];
            while (entry != NULL) {
                HashNode<K, V> *prev = entry;
                entry = entry->getNext();
                delete prev;
            }
            table[i] = NULL;
        }
        pthread_rwlock_destroy(&lock);
        // destroy the hash table
        delete [] table;
    }

    bool get(const K &key, V &value) {
        pthread_rwlock_rdlock(&lock);
        unsigned long hashValue = hashFunc(key) % capacity;
        HashNode<K, V> *entry = table[hashValue];

        while (entry != NULL) {
            if (entry->getKey() == key) {
                value = entry->getValue();
                pthread_rwlock_unlock(&lock);
                return true;
            }
            entry = entry->getNext();
        }
        pthread_rwlock_unlock(&lock);
        return false;
    }

    void put(const K &key, const V &value) {
        pthread_rwlock_wrlock(&lock);
        if((size/capacity) >= 21)
            resize();

        unsigned long hashValue = hashFunc(key) % capacity;
        HashNode<K, V> *prev = NULL;
        HashNode<K, V> *entry = table[hashValue];

        while (entry != NULL && entry->getKey() != key) {
            prev = entry;
            entry = entry->getNext();
        }

        if (entry == NULL) {
            entry = new HashNode<K, V>(key, value);
            if (prev == NULL) {
                // insert as first bucket
                table[hashValue] = entry;
            } else {
                prev->setNext(entry);
            }
            size++;
        } else {
            // just update the value
            entry->setValue(value);
        }
        pthread_rwlock_unlock(&lock);
    }

    void remove(const K &key) {
        pthread_rwlock_wrlock(&lock);
        unsigned long hashValue = hashFunc(key) % capacity;
        HashNode<K, V> *prev = NULL;
        HashNode<K, V> *entry = table[hashValue];

        while (entry != NULL && entry->getKey() != key) {
            prev = entry;
            entry = entry->getNext();
        }

        if (entry == NULL) {
            pthread_rwlock_unlock(&lock);
            // key not found
            return;
        }
        else {
            if (prev == NULL) {
                // remove first bucket of the list
                table[hashValue] = entry->getNext();
            } else {
                prev->setNext(entry->getNext());
            }
            delete entry;
            size--;
        }
        pthread_rwlock_unlock(&lock);
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
    pthread_rwlock_t lock;

    void putNoLock(const K &key, const V &value) {
        unsigned long hashValue = hashFunc(key) % capacity;
        HashNode<K, V> *prev = NULL;
        HashNode<K, V> *entry = table[hashValue];

        while (entry != NULL && entry->getKey() != key) {
            prev = entry;
            entry = entry->getNext();
        }

        if (entry == NULL) {
            entry = new HashNode<K, V>(key, value);
            if (prev == NULL) {
                // insert as first bucket
                table[hashValue] = entry;
            } else {
                prev->setNext(entry);
            }
            size++;
        } else {
            // just update the value
            entry->setValue(value);
        }
    }

    void resize() {
        HashNode<K, V> **oldTable = table;
        int oldCapacity = capacity;
        capacity *= 2;
        table = new HashNode<K, V> *[capacity]();
        size = 0;

        for(int i = 0; i < oldCapacity; i++) {
            table[i] = NULL;
            HashNode<K, V> *oldEntry = NULL;
            HashNode<K, V> *entry = oldTable[i];
            while(entry != NULL) {
                putNoLock(entry->getKey(), entry->getValue());
                oldEntry = entry;
                entry = entry->getNext();
                delete oldEntry;
            }
        }
        delete[] oldTable;
    }
};
