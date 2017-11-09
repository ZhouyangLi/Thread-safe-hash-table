# Thread-safe-hash-table
1. A thread-safe fixed-size hash table 

Multiple threads can insert and lookup items in the hash table concurrently using a fine-grained locking strategy.

2. A reader-writer lock

A reader-writer lock can be acquired in either the "read" (also referred to as "shared") or "write" (also referred to as "excluse") mode. The reader-writer lock allows multiple readers but only one writer at a time. If the lock has been acquired in the "read" mode, then other threads can also grab the lock in the "read" mode without blocking. If the lock has been acquired in the "write" mode, then no other thread should be able to acquire the lock in either "read" or "write" mode.

3. A thread-safe resizable hash table 

If a hash table resize is ongoing, no other threads can perform any other other hash table operations, such as lookups, inserts, and resize. On the other hands, hash table lookups and inserts can be viewed as "shared" activities: multiple hash table lookups and inserts can occur at the same time in the absence of resize.
