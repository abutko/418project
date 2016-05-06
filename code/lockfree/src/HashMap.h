#include "KeyHash.h"
#include <pthread.h>
#include <mutex>

// Hash node per bucket
template <typename K, typename V>
class Node {
public:
    K key;
    V value;
    Node *next;

    Node (const K &key, const V &value) : 
    key(key), value(value), next(NULL) {
    }

    Node() {
        next = NULL;
    }
};

// Hash bucket
template <typename K, typename V>
class List {
private:
    bool is_marked(Node<K,V> *node) {
        return ((unsigned long)(char *)node) & 1;
    }

    Node<K,V> *get_unmarked(Node<K,V> *node) {
        return (Node<K, V> *)((((unsigned long)(char *)node)>>1)<<1);
    }

    Node<K,V> *get_marked(Node<K,V> *node) {
        return (Node<K, V> *)(((unsigned long)(char *)node) | 1);
    }

public:
    Node<K, V> *head;
    Node<K, V> *tail;

    List() {
        head = new Node<K, V>();
        tail = new Node<K, V>();
        head->next = tail;
    }

    void search(const K &key, Node<K,V> *&left_node, Node<K,V> *&right_node) {
search_again:
        while(true) {
            left_node = head;
            Node<K,V> *left_node_next = head->next;

            Node<K,V> *t = head;
            Node<K,V> *t_next = head->next;

            // 1. Find left_node and right_node
            do {
                if(!is_marked(t_next)) {
                    left_node = t;
                    left_node_next = t_next;
                }
                t = get_unmarked(t_next);
                if(t == tail) break;
                t_next = t->next;
            } while(is_marked(t_next) || (t->key < key));
            right_node = t;

            // 2. Check nodes are adjacent
            if(left_node_next == right_node) {
                if ((right_node != tail) && is_marked(right_node->next))
                    goto search_again;
                else
                    return;
            }

            // 3. Remove one or more marked node
            if(__sync_bool_compare_and_swap(&(left_node->next), left_node_next, right_node)) {
                if ((right_node != tail) && is_marked(right_node->next))
                    goto search_again;
                else
                    return;
            }
        }
    }

    bool find(const K &key, V &value) {
        Node<K,V> *right_node, *left_node;

        search(key, left_node, right_node);
        if((right_node == tail) || (right_node->key != key))
            return false;
        else {
            value = right_node->value;
            return true;
        }
    }

    void insert(const K &key, const V &value) {
        Node<K, V> *new_node = new Node<K, V>(key, value);
        Node<K, V> *left_node = NULL;
        Node<K, V> *right_node = NULL;
        while(true) {
            search(key, left_node, right_node);

            if((right_node != tail) && (right_node->key == key)) {
                right_node->value = value;
                delete new_node;
                return;
            }
            new_node->next = right_node;
            if(__sync_bool_compare_and_swap(&(left_node->next),
                                            right_node, new_node))
                return;
        }
    }

    void erase(const K &key) {
        Node<K,V> *right_node, *right_node_next, *left_node;

        while(true) {
            search(key, left_node, right_node);
            if((right_node == tail) || (right_node->key != key))
                return;
            right_node_next = right_node->next;

            if(!is_marked(right_node_next)) {
                if(__sync_bool_compare_and_swap(&(right_node->next),
                                                right_node_next, get_marked(right_node_next)))
                    break;
            }
        }
        if(!__sync_bool_compare_and_swap(&(left_node->next), right_node, right_node_next))
            search(right_node->key, left_node, right_node);
    }
};

// Hash map class template
template <typename K, typename V, typename F = KeyHash<K> >
class HashMap {
public:
    HashMap() {
        // construct zero initialized hash table of size TABLE_SIZE
        table = new List<K, V>[TABLE_SIZE]();
        capacity = TABLE_SIZE;
    }

    ~HashMap() {
        // destroy all buckets one by one
        for (int i = 0; i < capacity; i++) {
            Node<K, V> *entry = table[i].head;
            while (entry != NULL) {
                Node<K, V> *prev = entry;
                entry = entry->next;
                delete prev;
            }
        }

        // destroy the hash table
        delete [] table;
    }

    bool get(const K &key, V &value) {
        unsigned long hashValue = hashFunc(key) % capacity;

        List<K, V> list = table[hashValue];
        return list.find(key, value);
    }

    void put(const K &key, const V &value) {
        unsigned long hashValue = hashFunc(key) % capacity;

        List<K, V> list = table[hashValue];
        list.insert(key, value);
    }

    void remove(const K &key) {
        unsigned long hashValue = hashFunc(key) % capacity;

        List<K, V> list = table[hashValue];
        list.erase(key);
    }

private:
    // hash table
    List<K, V> *table;
    // hash function
    F hashFunc;
    // capacity
    int capacity;
};
