
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "include/constant.h"
#include "include/memory.h"
#include "include/hashmap.h"

void init_hash_map(hash_map_t* hm) {
    memset(hm, 0, sizeof(*hm));
}

void free_hash_map(hash_map_t* hm) {
    free(hm->entries);
    init_hash_map(hm);
}

/// djb2 hash function, taken from http://www.cse.yorku.ca/~oz/hash.html.
unsigned long hash_string(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/// Finds either existing entry or place where it can be inserted. Uses linear probing.
static entry_t* hash_map_find_entry(entry_t* entries, size_t capacity, obj_string_t* key) {
    uint32_t index = key->hash % capacity;
    entry_t* grave = NULL;

    for(;;) {
        entry_t* entry = &entries[index];

        // Save grave if encountered.
        if (entry->key == NULL) {
            if (entry->value == HASH_MAP_GRAVE) {
                grave = entry;
            }
            // Return grave if it was encountered.
            else {
                return grave != NULL ? grave : entry;
            }
        // Comparing by pointers, this is okay since
        // strings in constant pool are always the same
        } else if (entry->key == key) {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

static void hash_map_resize(hash_map_t *hm, size_t capacity) {
    entry_t* entries = calloc(capacity, sizeof(*entries));
    if (entries == NULL) {
        fprintf(stderr, "HashMap run out of memory.\n");
        exit(37);
    }

    hm->count = 0;
    // Reinsert all keys into new memory
    for (size_t i = 0; i < hm->capacity; ++ i) {
        entry_t* entry = &hm->entries[i];
        // This also works for graves, since they have NULL key.
        if (entry->key == NULL) {
            continue;
        }

        entry_t* dest = hash_map_find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        hm->count += 1;
    }

    free(hm->entries);
    hm->entries = entries;
    hm->capacity = capacity;
}

bool hash_map_insert(hash_map_t* hm, obj_string_t* key, void* value) {
    if (hm->count >= hm->capacity) {
        hash_map_resize(hm, NEW_CAPACITY(hm->capacity));
    }

    entry_t* entry = hash_map_find_entry(hm->entries, hm->capacity, key);
    bool is_new = entry->key == NULL;

    if (is_new && entry->value != HASH_MAP_GRAVE) {
        hm->count += 1;
    }

    entry->key = key;
    entry->value = value;
    return true;
}

bool hash_map_fetch(hash_map_t* hm, obj_string_t* key, void** value) {
    if (hm->count == 0) {
        return false;
    }

    entry_t* entry = hash_map_find_entry(hm->entries, hm->capacity, key);
    if (entry->key == NULL) {
        return false;
    }

    *value = entry->value;
    return true;
}

bool hash_map_delete(hash_map_t* hm, obj_string_t* key) {
    if (hm->count == 0) {
        return false;
    }

    entry_t* entry = hash_map_find_entry(hm->entries, hm->capacity, key);
    if (entry->key == NULL) {
        return false;
    }

    // Create grave
    entry->value = NULL;
    entry->key = HASH_MAP_GRAVE;
    return true;
}
