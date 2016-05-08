// Code adapted from: 
// http://www.drdobbs.com/lock-free-data-structures-with-hazard-po/184401890
// Based on Maged Michal's paper: 
// https://www.research.ibm.com/people/m/michael/ieeetpds-2004.pdf

class HPNode {
public:
   int active;
   // The length of the list
   static int len;
   // Global header of the HP list
   static HPNode *head;

   // Can be used by the thread
   // that acquired it
   void *ptr; 

   HPNode *next;

   static HPNode *Head() {
      return HPNode::head;
   }
   // Acquires one hazard pointer
   static HPNode *Acquire() {
      // Try to reuse a retired HP record
      HPNode *p = head;
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
      p = new HPNode;
      p->active = 1;
      p->ptr = NULL;
      // Push it to the front
      HPNode *old;
      do {
         old = head;
         p->next = old;
      } while (!__sync_bool_compare_and_swap(&head, old, p));
      return p;
   }
   // Releases a hazard pointer
   static void Release(HPNode* p) {
      p->ptr = NULL;
      p->active = 0;
   }
};
