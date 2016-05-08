
template <typename K, typename V> struct NodeType {
public:
    K key;
    V value;
    NodeType *next; // mark is stored in the low order bit
};

struct MarkPtrType {
    NodeType *next; // mark is stored in the low order bit
};

class HPRecType {
public:
    HPRecType *next;
    int active;
    void *hazard;
}


template <typename K, typename V, typename F = KeyHash<K> >
class HashMapThreaded {
public:
    HashMapThreaded(int thread_id) {
        thread_id = thread_id;
        
    }

    ~HashMapThreaded() {

    }

    
    static bool get(const K &key, V &value) {

        return true;
    }

    static void put(const K &key, const V &value) {

    }

    static void remove(const K &key) {


    }

    // Shared variables between all threads
    static MarkPtrType **table;
    static HPRecType **HP;
    static int capacity;
    static int size;
    static int num_threads;
    static F hashFunc;
   
private: 

    static HPRecType *head = NULL;
    static int hp_list_len =  0;

    // Per-thread variables
    int thread_id;
    MarkPtrType *prev; NodeType<K,V> *cur; NodeType<K,V> *next;
    HPRecType    *hp0; HPRecType     *hp1; HPRecType     *hp2;
    vector<Map<K, V> *> rlist;   
 
    //// Hazard pointer functions //// 
    
    // Acquires one hazard pointer
    static HPRecType *Acquire() {
        // Try to reuse a retired HP record
        HPRecType *p = head;
        for (; p != NULL; p = p->next) {
            if (p->active || !__sync_bool_compare_and_swap(&p->active, 0, 1)) {
                continue;
            }
            return p;
        }
        // Increment the list length
        int oldLen;
        do {
            oldLen = len;
        } while (!__sync_bool_compare_and_swap(&len, oldLen, oldLen + 1));
        // Allocate a new one
        p = new HPRecType;
        p->active = 1;
        p->hazard = NULL;
        // Push it to the front
        HPRecType *old;
        do {
            old = head;
            p->next = old;
        } while (!__sync_bool_compare_and_swap(&head, old, p));
        return p;
    }
    // Releases a hazard pointer
    static void Release(HPRecType* p) {
        p->hazard = NULL;
        p->active = 0;
    }

    void Scan(HPNode *head, int threadNum) {
        // Stage 1: Scan hazard pointers list
        // collecting all non-null ptrs
        vector<void*> hp;
        while (head) {
            void *p = head->ptr;
            if (p) hp.push_back(p);
            head = head->next;
        }
        // Stage 2: sort the hazard pointers
        sort(hp.begin(), hp.end(), less<void*>());
        // Stage 3: Search based on this thread's retired nodes 
        vector<HashNode<int,string> *>::iterator i = rlist[threadNum].begin();
        while (i != rlist[threadNum].end()) {
            if (!binary_search(hp.begin(), hp.end(), *i)) {
                delete *i;
                if (&*i != &rlist[threadNum].back()) {
                    *i = rlist[threadNum].back();
                }
                rlist[threadNum].pop_back();
            } else {
                ++i;
            }
        }
    }
    ////////////////////////////////// 

    // called in place of delete for a node in our hash table
    void DeleteNode(HashNode<K,V> *entry) {
        rlist.push_back(entry);
        if (rlist.size() >= RLIST_MAX_SIZE) {
            Scan();
        }
    }

}; 
