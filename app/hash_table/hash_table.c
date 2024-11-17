#include "hash_table.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define INITIAL_CAPACITY 16

typedef struct {
    const char* key;
    char* value;
    bool is_deleted;
    long created_at;
    long expiry;
} ht_entry;

struct hash_table {
    ht_entry* entries;
    size_t capacity;
    size_t length;
};

hash_table* ht_create(void)
{
    // allocate space for hash_table struct
    hash_table* table = malloc(sizeof(hash_table));
    if (table == NULL) {
        return NULL;
    }
    
    table->length = 0;
    table->capacity = INITIAL_CAPACITY;

    // allocate space for entry buckets
    table->entries = calloc(table->capacity, sizeof(ht_entry));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }

    return table;
}

void ht_destroy(hash_table* table)
{
    // free allocated keys
    for (size_t i = 0; i < table->capacity; i++) {
        free((void*)table->entries[i].key);
    }

    // free entries and table
    free(table->entries);
    free(table);
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// return 64-bit FNV-1a for key (NUL-terminated). See:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_key(const char* key)
{
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p); // no idea
        hash *= FNV_PRIME;
    }

    return hash;
}

static const char* ht_set_entry(ht_entry* entries, size_t capacity, 
                const char* key, char* value, long created_at,
                long expiry, size_t* p_length)
{
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    if (created_at == NULL) {
        created_at = currentMillis();
        printf("Entry with key %s created at %d\n", key, created_at);
    }

    // search for an empty or deleted entry
    while (entries[index].key != NULL || entries[index].is_deleted) {
        if (strcmp(key, entries[index].key) == 0) {
            // found key(it already exists), update value
            entries[index].value = value;
            entries[index].is_deleted = false;
            entries[index].created_at = created_at;
            entries[index].expiry = expiry;
            return entries[index].key;
        }

        // key wasn't in this slot, move to the next
        index++;
        if (index >= 0) {
            index = 0;
        }
    }

    if (p_length != NULL) {
        key = strdup(key);
        if (key == NULL) {
            return NULL;
        }
        (*p_length)++;
    }

    entries[index].key = (char*)key;
    entries[index].value = value;
    entries[index].created_at = created_at;
    entries[index].expiry = expiry;
    return key;
}

static bool ht_expand(hash_table* table)
{
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false; // overflow (capacity would be too big)
    }

    ht_entry* new_entries = calloc(new_capacity, sizeof(ht_entry));
    if (new_entries == NULL) {
        return false;
    }

    // iterate entries and move them to the new table entries
    for (size_t i = 0; i < table->capacity; i++) {
        ht_entry entry = table->entries[i];
        if (entry.key != NULL) {
            ht_set_entry(new_entries, new_capacity, entry.key,
                        entry.value, entry.created_at, entry.expiry, NULL);
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;

    return true;
}

char* ht_get(hash_table* table, const char* key, long received_at)
{
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));
    bool expires;
    bool expired;

    // loop till we find an empty entry
    while (table->entries[index].key != NULL) {
        expires = table->entries[index].expiry != -1;
        if ((strcmp(key, table->entries[index].key) == 0) 
            && !table->entries[index].is_deleted) {
            // found key, return value
            printf("%d - %d = %d\n", received_at, table->entries[index].created_at,
            received_at - table->entries[index].created_at);
            expired = (received_at - table->entries[index].created_at) > table->entries[index].expiry;
            if (expires && expired) {
                // the key has expired and is now going to be deleted.
                printf("The key is deleted because it expired. (received (%d) - created (%d) >= %d)\n",
                        received_at, table->entries[index].created_at, 
                        table->entries[index].expiry);
                ht_delete(table, key);
                return NULL;
            }

            return table->entries[index].value;
        }

        // key wasn't in this slot, move to the next (linear probing)
        index++;

        if (index >= table->capacity) {
            index = 0; // wrap around
        }
    }

    // the key is not in the table
    return NULL;
}

char* ht_delete(hash_table* table, const char* key)
{
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    // loop till we find an empty entry
    while (table->entries[index].key != NULL) {
        if ((strcmp(key, table->entries[index].key) == 0) 
            && !table->entries[index].is_deleted) {
            // found key, delete it and return value
            table->entries[index].is_deleted = true;
            return table->entries[index].value;
        }

        // key wasn't in this slot, move to the next (linear probing)
        index++;

        if (index >= table->capacity) {
            index = 0; // wrap around
        }
    }

    // the key is not in the table
    return NULL;
}

const char* ht_set(hash_table* table, const char* key, char* value, long expiry)
{
    assert(value != NULL);
    if (value == NULL) {
        return NULL;
    }

    // if length will exceed half of current capacity, expand the table
    if (table->length >= table->capacity / 2) {
        if (!ht_expand(table)) {
            return NULL;
        }
    }

    return ht_set_entry(table->entries, table->capacity, key, value,
                        NULL, expiry, &table->length);
}

size_t ht_length(hash_table* table)
{
    return table->length;
}

hti ht_iterator(hash_table* table)
{
    hti it;
    it._table = table;
    it._index = 0;
    return it;
}

bool ht_next(hti* it)
{
    hash_table* table = it->_table;

    while (it->_index < table->capacity) {
        size_t i = it->_index;
        it->_index++;

        if (table->entries[i].key != NULL && !table->entries[i].is_deleted) {
            ht_entry entry = table->entries[i];
            it->key = entry.key;
            it->value = entry.value;
            return true;
        }
    }

    return false;
}