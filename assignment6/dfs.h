/**
 * @file
 * DFS API declarations.
 */

#pragma once

#include "hashtable.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef __GNUC__
#  define ATTRS(...) __attribute__((__VA_ARGS__))
#else
#  define ATTRS(...)
#endif

/**
 * Status return codes.
 */
typedef enum DfsStatus {
    DFS_E_INTERNAL = INT_MIN, ///< An impossible state was reached.
    DFS_E_MALLOC_FAILURE, ///< Could not allocate memory.
    DFS_E_ENTRY_DOES_NOT_EXIST, ///< There is no entry with the given name.
    DFS_E_ENTRY_ALREADY_EXISTS, ///< There is a name collision in a directory.
    DFS_E_NO_MORE_ENTRIES, ///< There are no more entries in the directory.
    DFS_E_NOT_A_DIRECTORY, ///< The retrieved entry is not a directory.
    DFS_E_NOT_A_FILE, ///< The retrieved entry is not a file.
    DFS_OK = 0 ///< No error.
} DfsStatus;

/**
 * Entry types.
 */
typedef enum DfsEntryType {
    DFS_ENT_NONE, ///< An uninitialized or invalid entry.
    DFS_ENT_FILE, ///< A file.
    DFS_ENT_DIR ///< A directory.
} DfsEntryType;

/**
 * The abstract, base entry type.
 * 
 * All extending types (only DfsDir and DfsFile for now) **must** start with a field of this type.
 * Because the types are polymorphic, DfsEntry and its subclasses **must not** be stored as a value.
 * Always use the associated creation and destruction functions and only handle pointers.
 * 
 * You can only cast an entry to its subtype if the type field contains the right value. The
 * functions dfs_dynamic_cast_to_file() and dfs_dynamic_cast_to_dir() can help you with doing this
 * (fairly) safely, as well as the specialized add, get and remove functions.
 */
typedef struct DfsEntry {
    DfsEntryType type; ///< The entry's type.

    /** 
     * @name Internal fields
     * You can ignore these values.
     * @{
     */
    mode_t mode; ///< The file mode and permissions.
    gid_t group; ///< The owning group.
    uid_t user; ///< The owning user.
    struct timespec ctime; ///< The datetime the entry was created.
    struct timespec mtime; ///< The datetime the entry was modified.
    struct timespec atime; ///< The datetime the entry was accessed.
    /// @}
} DfsEntry;

/**
 * A file.
 * 
 * A file holds arbitrary content and its length.
 * 
 * @extends DfsEntry
 */
typedef struct DfsFile {
    DfsEntry header; ///< The base type.
    char *contents; ///< The file's contents.
    size_t length; ///< The length of the file in bytes.
} DfsFile;

/**
 * A directory.
 * 
 * A directory holds a hash table mapping names to other entries.
 * 
 * @extends DfsEntry
 */
typedef struct DfsDir {
    DfsEntry header; ///< The base type.
    Hashtable entries; ///< The hash table mapping names to entities.

    /**
     * @name Internal fields
     * You can ignore these values.
     * @{
     */
    int being_destroyed; ///< If true, this entry is being destroyed and recursion should ignore 
                         ///< this one.
    /// @}
} DfsDir;

/**
 * Returns a description of the given status code.
 * 
 * The returned string must not be freed.
 * 
 * @param status the status code
 * 
 * @return the description
 */
const char *dfs_strerror(DfsStatus status) ATTRS(const);

/**
 * Destroys a file.
 * 
 * This frees the contents and the file itself.
 * 
 * @param file the file to destroy
 */
void dfs_destroy_file(DfsFile *file);

/**
 * Destroys a directory.
 * 
 * This recursively destroys all child directories and files. Hint: this may not always be what 
 * you want.
 * 
 * @param dir the directory to destroy
 */
void dfs_destroy_dir(DfsDir *dir);

/**
 * Destroys an entry.
 * 
 * The correct dfs_destroy_* function will be automatically selected based on the entry type.
 * 
 * @param ent the entry to destroy
 */
void dfs_destroy_entry(DfsEntry *ent);

/**
 * Creates a new file object.
 * 
 * The contents pointer is NULL and the length is 0.
 * 
 * @return the new file or NULL if no memory could be allocated
 */
DfsFile *dfs_create_file(void);

/**
 * Creates a new directory object.
 * 
 * The hash table is initialized and empty.
 *
 * @return the new directory or NULL if no memory could be allocated
 */
DfsDir *dfs_create_dir(void);

/**
 * Adds an entry to a directory.
 * 
 * Duplicate names (even with entries of different types) will result in an error.
 * 
 * @param dir the directory to add the entry to
 * @param name the name of the entry to add
 * @param entry the entry to add
 * 
 * @return any status code
 */
DfsStatus dfs_add_entry(DfsDir *dir, const char *name, DfsEntry *entry) 
        ATTRS(nonnull);

/**
 * Adds a file to a directory.
 * 
 * Duplicate names (even with entries of different types) will result in an error.
 * 
 * @param dir the directory to add the file to
 * @param name the name of the file to add
 * @param file the file to add
 *
 * @return any status code
 */
ATTRS(nonnull) static inline DfsStatus dfs_add_file(
    DfsDir *dir, const char *name, DfsFile *file
) {
    return dfs_add_entry(dir, name, (DfsEntry *) file);
}

/**
 * Adds a directory to another directory.
 * 
 * Duplicate names (even with entries of different types) will result in an error.
 * 
 * @param dir the directory to add the directory to
 * @param name the name of the directory to add
 * @param child_dir the directory to add
 *
 * @return any status code
 */
ATTRS(nonnull) static inline DfsStatus dfs_add_dir(
    DfsDir *dir, const char *name, DfsDir *child_dir
) {
    return dfs_add_entry(dir, name, (DfsEntry *) child_dir);
}

/**
 * Retrieves an entry in a directory.
 * 
 * entry_p may be NULL.
 * 
 * If the entry does not exist, `*entry_p` will not be touched.
 * 
 * @param dir the directory to look in
 * @param name the name to look for
 * @param entry_p where to store the address of the entry, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_get_entry(DfsDir *dir, const char *name, DfsEntry **entry_p) 
        ATTRS(nonnull(1, 2));

/**
 * Retrieves a file in a directory.
 * 
 * file_p may be NULL.
 * 
 * If the entry does not exist or is not a file, `*file_p` will not be touched.
 * 
 * @param dir the directory to look in
 * @param name the name to look for
 * @param file_p where to store the address of the file, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_get_file(DfsDir *dir, const char *name, DfsFile **file_p)
        ATTRS(nonnull(1, 2));

/**
 * Retrieves a child directory in a directory.
 * 
 * child_dir_p may be NULL.
 * 
 * If the entry does not exist or is not a directory, `*child_dir_p`  will not be touched.
 * 
 * @param dir the directory to look in
 * @param name the name to look for
 * @param child_dir_p where to store the address of the directory, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_get_dir(DfsDir *dir, const char *name, DfsDir **child_dir_p)
        ATTRS(nonnull(1, 2));

/**
 * Removes an entry from a directory.
 * 
 * If entry_p is NULL, the entry will be destroyed. The effect of this depends on the entry type.
 * 
 * If the entry does not exist, `*entry_p` will not be touched.
 * 
 * @param dir the directory to remove from
 * @param name the name of the entry to remove
 * @param entry_p where to store the address of the entry, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_remove_entry(DfsDir *dir, const char *name, DfsEntry **entry_p)
        ATTRS(nonnull(1, 2));

/**
 * Removes a file from a directory.
 * 
 * If file_p is NULL, the file will be destroyed.
 * 
 * If the entry does not exist or is not a file, `*file_p` will not be touched.
 * 
 * @param dir the directory to remove from
 * @param name the name of the file to remove
 * @param file_p where to store the address of the file, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_remove_file(DfsDir *dir, const char *name, DfsFile **file_p)
        ATTRS(nonnull(1, 2));

/**
 * Removes a child directory from a directory.
 * 
 * If child_dir_p is NULL, the child directory will be destroyed recursively.
 * 
 * If the entry does not exist or is not a directory, `*child_dir_p` will not be touched.
 * 
 * @param dir the directory to remove from
 * @param name the name of the directory to remove
 * @param child_dir_p where to store the address of the directory, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_remove_dir(DfsDir *dir, const char *name, DfsDir **child_dir_p)
        ATTRS(nonnull(1, 2));

/**
 * Returns the number of entries in a directory.
 * 
 * @param dir the directory to get the size of
 * 
 * @return the number of entries
 */
ATTRS(nonnull, pure) static inline size_t dfs_get_dir_size(DfsDir *dir) {
    assert(dir);

    return dir->entries.num_entries;
}

/**
 * Returns the next entry in the directory.
 *
 * This traverses the directory in an unspecified order. The cursor must be initialized to 0 and not 
 * be modified by anything other than this function. Its value can not be relied upon.
 * The directory itself must not be modified between calls to this function.
 * 
 * name_p and entry_p may be NULL.
 *
 * @param dir the directory
 * @param cursor where the iteration cursor is stored
 * @param name_p where to store the address of the name, or NULL
 * @param entry_p where to store the address of the data, or NULL
 *
 * @return any status code
 */
DfsStatus dfs_get_next(DfsDir *dir, size_t *cursor, const char **name_p, DfsEntry **entry_p)
    ATTRS(nonnull(1, 2));

/**
 * Casts a (nullable) entry to a file while checking its type.
 * 
 * If the entry is not NULL but its type is not a file, then the result will be NULL.
 * 
 * @param ent the entry to cast
 * 
 * @return the cast entry, or NULL
 */
ATTRS(warn_unused_result) static inline DfsFile *dfs_dynamic_cast_to_file(DfsEntry *ent) {
    return (ent && ent->type == DFS_ENT_FILE) ? (DfsFile *) ent : NULL;
}

/**
 * Casts a (nullable) entry to a directory while checking its type.
 * 
 * If the entry is not NULL but its type is not a directory, then the result will be NULL.
 * 
 * @param ent the entry to cast
 * 
 * @return the cast entry, or NULL
 */
ATTRS(warn_unused_result) static inline DfsDir *dfs_dynamic_cast_to_dir(DfsEntry *ent) {
    return (ent && ent->type == DFS_ENT_DIR) ? (DfsDir *) ent : NULL;
}

/**
 * Casts a (nullable) const entry to a file while checking its type.
 * 
 * If the entry is not NULL but its type is not a file, then the result will be NULL.
 * 
 * @param ent the entry to cast
 * 
 * @return the cast entry, or NULL
 */
ATTRS(warn_unused_result) static inline const DfsFile *dfs_dynamic_const_cast_to_file(
    const DfsEntry *ent
) {
    return (ent && ent->type == DFS_ENT_FILE) ? (const DfsFile *) ent : NULL;
}

/**
 * Casts a (nullable) const entry to a directory while checking its type.
 * 
 * If the entry is not NULL but its type is not a directory, then the result will be NULL.
 * 
 * @param ent the entry to cast
 * 
 * @return the cast entry, or NULL
 */
ATTRS(warn_unused_result) static inline const DfsDir *dfs_dynamic_const_cast_to_dir(
    const DfsEntry *ent
) {
    return (ent && ent->type == DFS_ENT_DIR) ? (const DfsDir *) ent : NULL;
}

#undef ATTRS
