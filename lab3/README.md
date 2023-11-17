# Hash Hash Hash

## Roland Yang 506053914

This lab compares time and accuracy metrics between three different hash table implementations: single-threaded, single mutex multi-threaded, and mutiple mutex multi-threaded.

## Building
Run the following command within the directory of all the files.
```shell
make
```

## Running
Once the program has been built, execute the following command in the same folder as the executable with the appropriate arguments.
```shell
./hashtable-tester -t [number_of_threads] -s [number_of_hash_table_entries_added_per_thread]
```

Here is an example output of the program.

```shell
./hash-table-tester -t 8 -s 50000
Generation: 38,744 usec
Hash table base: 682,520 usec
  - 0 missing
Hash table v1: 1,059,375 usec
  - 0 missing
Hash table v2: 84,892 usec
  - 0 missing
```

## First Implementation
In the `hash_table_v1_add_entry` function, I locked the mutex inside the hash table struct at the top of the function then unlocked it after updating the value of the list entry after it exists or after it inserts the entry, which are both the ends of the function. As mentioned previously, I added a single mutex into the `hash_table_v1` struct, which I initialized within `hash_table_v1_create` and destroyed in `hash_table_v1_destroy` to prevent memory leaks. 

The race condition exists within the `hash_table_v1_add_entry` function when two threads attempt to enter the function at the same time, thus we block off the entire function as a critical section guaranteeing correctness because all operations involved in adding an entry into the hash table are protected and reserved for a single thread to use at a time whenever an add into the hash table is requested.
### Performance
```shell
./hash-table-tester -t 8 -s 50000
Generation: 38,744 usec
Hash table base: 682,520 usec
  - 0 missing
Hash table v1: 1,059,375 usec
  - 0 missing
```
Version 1 is a slower than the base version (~1.55x slower). 

This is because only one thread can execute at a time, while all the other threads have to wait for the working thread to release before executing, thus creating significant overhead. Furthermore, there is additional overhead created by the computational work of creating, initializing, locking, unlocking, and destorying the mutex.

## Second Implementation
In the `hash_table_v2_add_entry` function, I locked the mutex inside of the entry right after obtaining it from `get_hash_table_entry` and unlocked it in the same spots as with the first implementation (the "end" of the function). Instead of adding a mutex into the hash table struct, I decided to add it into every entry which is the `hash_table_entry` struct. Similarly as within the first implementation, I initialized the mutex in every entry within `hash_table_v2_create` and destroyed them in `hash_table_v2_destroy`. In other words, I used multiple mutexes within this implementation.

The same race condition exists in this scenario, however we use a more superior locking strategy in this implementation - modifying the amount of mutexes used and where. With this implementation, the mutexes block off the same critical section as the first implementation (which is essentially the entire function except for `get_hash_table_entry`) protecting the operations involved with adding entires and thus guaranteeing correctness as well.

### Performance
```shell
./hash-table-tester -t 8 -s 50000
Generation: 38,744 usec
Hash table base: 682,520 usec
  - 0 missing
Hash table v1: 1,059,375 usec
  - 0 missing
Hash table v2: 84,892 usec
  - 0 missing
```

This time speed up is ~12.48x faster than the first implementation and ~8.04x faster than the base implementation when using 8 cores.

This is because with our new mutex strategy, we are only locking the entries when we add them as opposed to the whole table. This eliminates the extra overhead and/or time wasted when adding entries that aren't in the same bucket. Thus the mutex comes only into play when two threads try to add to the same bucket, and creates traffic there. In other words, we are saving time by not locking resources when adding entires to different buckets. Although there is some more overhead in this implementation compared to the first one created by the creation, destruction, and management of more mutexes, it is practically negligible compared to the amount of overhead eliminated through our strategy. Thus, this implementation increases parallelization since threads that don't interfere with each other are able to continue operating in parallel- only threads involving the same entry will be slowed.

## Cleaning up
Run the following command within the directory of all the files.
```shell
make clean
```