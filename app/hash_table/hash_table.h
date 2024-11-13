#ifndef _HT_H
#define _HT_H

#include <stddef.h>
#include <stdbool.h>

typedef struct hash_table hash_table;

// create a hash table and return a pointer to it, or NULL if out of memory
hash_table* ht_create(void);

// free memory allocated for table, including keys
void ht_destroy(hash_table* table);

// get item with given key from hash table. Return value or NULL if key
// was not found
char* ht_get(hash_table* table, const char* key);

// set key-value pair in hash table. 
const char* ht_set(hash_table* table, const char* key, char* value);

// delete value with given key
char* ht_delete(hash_table* table, const char* key);

// return number of items in hash table
size_t ht_length(hash_table* table);

// hash table iterator, iterate with ht_next
typedef struct {
    const char* key;
    char* value;

    hash_table* _table;
    size_t _index;
} hti;

// return new ht iterator
hti ht_iterator(hash_table* table);

// move iterator to next item in table
bool ht_next(hti* it);

#endif