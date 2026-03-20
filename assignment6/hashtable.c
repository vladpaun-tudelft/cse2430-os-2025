#include <stdlib.h>
#include <string.h>

#include <xxhash.h>

#include "strdup.h"
#include "hashtable.h"

#define EMBIGGEN_THRESHOLD 0.666667
#define ENSMALLEN_THRESHOLD 0.333333
#define GROWTH_FACTOR 1.414214

typedef uint_fast64_t ht_Hash;

typedef struct ht_Entry {
    const char *key; ///< The key for this entry.
    void *data; ///< The user-supplied data.
    ht_Hash hash; ///< The hash for the key. 
} ht_Entry;

static const char *TOMBSTONE_KEY = (const char *) -1;

/**
 * Hashes a string.
 *
 * @param key the string to hash
 *
 * @return the hash
 */
static ht_Hash hashs(const char *key) {
     return XXH64(key, strlen(key), 5206528764019819084UL);
}

static enum ht_Error resize_table(struct Hashtable *table) {
    size_t new_cap_entries = table->cap_entries;
    size_t num_nonnull_entries = table->num_entries + table->num_tombstones;
    if (table->num_entries > table->cap_entries * EMBIGGEN_THRESHOLD) {
        new_cap_entries = (size_t) (table->cap_entries * GROWTH_FACTOR);
    } else if (table->num_entries > table->min_cap_entries
            && table->num_entries < table->cap_entries * ENSMALLEN_THRESHOLD) {
        new_cap_entries = (size_t) (table->cap_entries / GROWTH_FACTOR);
    } else if (num_nonnull_entries <= table->cap_entries * EMBIGGEN_THRESHOLD 
            && num_nonnull_entries >= table->cap_entries * ENSMALLEN_THRESHOLD) {
        return HT_OK;
    }

    struct ht_Entry *new_entries = calloc(new_cap_entries, sizeof(struct ht_Entry));
    if (!new_entries) return HT_E_MALLOC_FAILURE;

    struct ht_Entry *old_entries = table->entries;

    for (size_t i = 0; i < table->cap_entries; ++i) {
        ht_Entry *entry = old_entries + i;
        if (!entry->key || entry->key == TOMBSTONE_KEY) continue;

        ht_Hash hash = entry->hash;
        size_t new_index = hash % new_cap_entries;
        while (new_entries[new_index].key) {
            new_index = (new_index + 1) % new_cap_entries;
        }

        new_entries[new_index] = old_entries[i];
    }

    table->cap_entries = new_cap_entries;
    table->entries = new_entries;

    free(old_entries);

    return HT_OK;
}

enum ht_Error ht_create(struct Hashtable *table, size_t capacity) {
    if (capacity < 8) capacity = 8;
    table->min_cap_entries = table->cap_entries = capacity;

    table->num_entries = 0;
    table->num_tombstones = 0;
    table->entries = calloc(table->cap_entries, sizeof(struct ht_Entry));

    return (table->entries != NULL) ? HT_OK : HT_E_MALLOC_FAILURE;
}

void ht_destroy(struct Hashtable *table) {
    free(table->entries);
    memset(table, 0, sizeof(struct Hashtable));
}

enum ht_Error ht_add(struct Hashtable *table, const char *key, void *data) {
    ht_Hash hash = hashs(key);

    // Find the next empty spot or exit on a duplicate key (if that's enabled)
    size_t index = hash % table->cap_entries;
    ht_Entry *entry = table->entries + index;
    while (entry->key) {
        if (entry->key != TOMBSTONE_KEY && entry->hash == hash && strcmp(key, entry->key) == 0) {
            return HT_E_KEY_ALREADY_PRESENT;
        }

        index = (index + 1) % table->cap_entries;
        entry = table->entries + index;
    }

    if (entry->key == TOMBSTONE_KEY) {
        --table->num_tombstones;
    }

    entry->key = strdup(key);
    entry->data = data;
    entry->hash = hash;

    ++table->num_entries;

    return resize_table(table);
}

static enum ht_Error locate(const struct Hashtable *table, const char *key, size_t *index) {
    ht_Hash hash = hashs(key);

    *index = hash % table->cap_entries;
    while (table->entries[*index].key) {
        ht_Entry *entry = table->entries + *index;
        if (entry->key != TOMBSTONE_KEY && entry->hash == hash && strcmp(key, entry->key) == 0) {
            return HT_E_KEY_ALREADY_PRESENT;
        }
        *index = (*index + 1) % table->cap_entries;
    }

    return HT_E_KEY_NOT_PRESENT;
}

enum ht_Error ht_get(const struct Hashtable *table, const char *key, void *datap) {
    void **data = (void **) datap;

    size_t index;
    if (locate(table, key, &index) == HT_E_KEY_ALREADY_PRESENT) {
        if (data) *data = table->entries[index].data;
        return HT_OK;
    }

    if (data) *data = NULL;
    return HT_E_KEY_NOT_PRESENT;
}

int ht_has_next(const struct Hashtable *table, size_t last_index) {
    if (!table) return 0;

    return ht_get_next(table, &last_index, NULL, NULL) == HT_OK;
}

enum ht_Error ht_get_next(const struct Hashtable *table, size_t *last_index, const char **key, void *datap) {
    void **data = (void **) datap;

    while (*last_index < table->cap_entries) {
        const ht_Entry *entry = table->entries + *last_index;
        const char *ekey = entry->key;
        if (ekey && ekey != TOMBSTONE_KEY) {
            if (key) *key = entry->key;
            if (data) *data = entry->data;
            ++*last_index;

            return HT_OK;
        }
        else ++*last_index;
    }

    if (data) *data = NULL;
    return HT_E_NO_ENTRIES_LEFT;
}

enum ht_Error ht_rem(struct Hashtable *table, const char *key, void *datap) {
    void **data = (void **) datap;

    size_t index;
    if (locate(table, key, &index) == HT_E_KEY_ALREADY_PRESENT) {
        ht_Entry *entry = table->entries + index;
        // Save the data and remove the key.
        if (data) *data = entry->data;
        free((char *) entry->key);
        entry->key = TOMBSTONE_KEY;
        --table->num_entries;
        ++table->num_tombstones;

        return resize_table(table);
    } else {
        if (data) *data = NULL;
        return HT_E_KEY_NOT_PRESENT;
    }
}

enum ht_Error ht_rem_next(struct Hashtable *table, char **key, void *datap) {
    void **data = (void **) datap;

    // Prevent the loop from going over the whole entries list
    if (table->num_entries == 0) {
        if (data) *data = NULL;
        return HT_E_NO_ENTRIES_LEFT;
    }

    size_t index = 0;

    // Keep looping over the entries until we either find an entry with a non-NULL key or we run
    // out of addresses to check
    while (index <= table->cap_entries) {
        ht_Entry *entry = table->entries + index;
        const char *ekey = entry->key;
        if (ekey && ekey != TOMBSTONE_KEY) {
            // Save the data and remove the key.
            if (data) *data = entry->data;

            if (key) {
                *key = (char *) ekey;
            } else {
                free((char *) ekey);
            }

            entry->key = TOMBSTONE_KEY;
            --table->num_entries;
            ++table->num_tombstones;

            return resize_table(table);
        }
        index++;
    }

    // Entry count got messed up somewhere...
    assert(table->num_entries == 0);
    table->num_entries = 0;
    return HT_E_NO_ENTRIES_LEFT;
}

const char * ht_strerror(enum ht_Error code) {
    switch(code) {
        case HT_OK:
            return "no error";
        case HT_E_MALLOC_FAILURE:
            return "allocation failure";
        case HT_E_NO_ENTRIES_LEFT:
            return "the hash table is empty";
        case HT_E_KEY_ALREADY_PRESENT:
            return "there's already an entry with this key";
        case HT_E_KEY_NOT_PRESENT:
            return "entry not found";
        default:
            return "(unknown error code)";
    }
}
