
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>


#include "include/constant.h"
#include "include/memory.h"
#include "include/hashmap.h"
#include "include/dissasembler.h"

#define IS_GRAVE(node) ((node)->key == NULL && IS_BOOL((node)->value) && AS_BOOL((node)->value))

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
            // Return grave if it was encountered.
            if (IS_BOOL(entry->value) && AS_BOOL(entry->value)) {
                // Save only the first occurence of the grave
                if (grave == NULL) {
                    grave = entry;
                }
            } else {
                return grave != NULL ? grave : entry;
            }
        // Comparing by pointers, this is okay since
        // strings in constant pool are always the same
        } else if (strcmp(entry->key->data, key->data) == 0) {
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

bool hash_map_insert(hash_map_t* hm, obj_string_t* key, value_t value) {
#ifdef __DEBUG__
    if (key->obj.type != OBJ_STRING) {
        fprintf(stderr, "Hashmap key is '");
        dissasemble_object(stderr, &key->obj);
        fprintf(stderr, "' with value '");
        dissasemble_value(stderr, value);
        fprintf(stderr, "', expected string.\n");
        exit(33);
    }
#endif
    if (hm->count >= hm->capacity) {
        hash_map_resize(hm, NEW_CAPACITY(hm->capacity));
    }
    entry_t* entry = hash_map_find_entry(hm->entries, hm->capacity, key);
    bool is_new = entry->key == NULL;

    if (is_new && !IS_GRAVE(entry)) {
        hm->count += 1;
    }

    entry->key = key;
    entry->value = value;
    return true;
}

bool hash_map_fetch(hash_map_t* hm, obj_string_t* key, value_t* value) {
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
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

bool hash_map_update(hash_map_t* hm, obj_string_t* key, value_t new_val) {
    if (hm->count == 0) {
        return false;
    }

    entry_t* entry = hash_map_find_entry(hm->entries, hm->capacity, key);
    if (entry->key == NULL) {
        return false;
    }

    entry->value = new_val;
    return true;
}
