/**
 * @file
 * Interface to the hash table.
 */

#pragma once
#ifndef HANSTABLE_H_
#define HANSTABLE_H_

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Error codes that the table could return.
 */
enum ht_Error {
    HT_E_MALLOC_FAILURE = INT_MIN, ///< A memory allocation failed.
    HT_E_KEY_NOT_PRESENT, ///< The searched-for key is not present in the table.
    HT_E_KEY_ALREADY_PRESENT, ///< The to-be-inserted key is already present in the table.
    HT_E_NO_ENTRIES_LEFT, ///< The next item could not be accessed because the table is empty.

    HT_OK = 0 ///< No error.
};

/**
 * The main structure of the hash table.
 *
 * The full layout is known to allow it to be directly included in other structures or unions,
 * potentially saving space. The members may be inspected (not modified!) externally, but this
 * should be done with care as it can restrict the user to a specific version of Hanstable.
 */
struct Hashtable {
    struct ht_Entry *entries; ///< The entries in the table.

    size_t num_entries; ///< The total number of valid entries in the table.
    size_t num_tombstones; ///< The total number of invalidated entries in the table.
    size_t cap_entries; ///< The number of entries in the table.
    size_t min_cap_entries; ///< The minimum number of entries in the table.
};

// For those wanting a cleaner namespace
#ifndef HT_DONT_TYPEDEF
    typedef enum ht_Error ht_Error;
    typedef struct Hashtable Hashtable;
#endif


/**
 * Initializes a hash table.
 *
 * @param ht the table to initialize
 * @param init_cap the initial (and minimum) capacity of the hash table
 *
 * @return an error code, if any
 */
enum ht_Error ht_create(struct Hashtable *ht, size_t init_cap);

/**
 * Destroys a hash table.
 *
 * Any stored data will be lost.
 * 
 * @param ht the hash table to destroy
 */
void ht_destroy(struct Hashtable *ht);

/**
 * Inserts an entry into the hash table.
 *
 * @param ht the hash table
 * @param key the key of the entry
 * @param data the data of the entry
 *
 * @return an error code, if any
 */
enum ht_Error ht_add(struct Hashtable *ht, const char *key, void *data);

/**
 * Returns an entry in the table.
 * 
 * @param ht the hash table
 * @param key the key to look for
 * @param datap where to write the pointer to the data
 *
 * @return an error code, if any
 */
enum ht_Error ht_get(const struct Hashtable *ht, const char *key, void *datap);

/**
 * Returns the next entry in the table.
 *
 * This traverses the hash map in an unspecified order. The cursor must be initialized to 0 and not 
 * be modified by anything other than this function. Its value can not be relied upon.
 * The hash table itself must not be modified between calls to this function.
 *
 * @param ht the hash table
 * @param cursor where the iteration cursor is stored
 * @param key where to store the key, or NULL
 * @param datap where to store the data, or NULL
 *
 * @return an error code, if any
 */
enum ht_Error ht_get_next(const struct Hashtable *ht, size_t *cursor, const char **key, void *datap);

/**
 * Checks if the next call to ht_get_next with the given cursor will yield a value.
 *
 * The same restrictions around the hash table and cursor apply.
 *
 * @param ht the hash table
 * @param cursor the cursor value to check
 *
 * @return truthy if ht_get_next will return a value, falsy otherwise
 */
int ht_has_next(const struct Hashtable *ht, size_t cursor);

/**
 * Removes an entry from the table.
 *
 * @param ht the hash table
 * @param key the key to look for
 * @param datap where to write the pointer to the data, or NULL
 *
 * @return an error code, if any
 */
enum ht_Error ht_rem(struct Hashtable *ht, const char *key, void *datap);

/**
 * Removes the next entry from the table.
 *
 * This traverses the hash map in an unspecified order. 
 *
 * If key is non-NULL, then ownership of the memory pointed to by *key is transferred to the caller. 
 * It must be freed using free.
 *
 * @param ht the hash table
 * @param key where to store the pointer to the key, or NULL
 * @param datap where to store the pointer to the data, or NULL
 *
 * @return an error code, if any
 */
enum ht_Error ht_rem_next(struct Hashtable *ht, char **key, void *datap);

/**
 * Translates an error code to a human-readable string.
 *
 * @param e the error code
 *
 * @return the string
 */
const char *ht_strerror(enum ht_Error e);

#endif
