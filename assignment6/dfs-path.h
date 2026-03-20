/**
 * @file
 * DFS path utilities.
 */

#pragma once

#include "dfs.h"

#ifdef __GNUC__
#  define ATTRS(...) __attribute__((__VA_ARGS__))
#else
#  define ATTRS(...)
#endif

/**
 * A path.
 */
typedef struct DfsPath {
    char **components; ///< The components of the path.
    size_t num_components; ///< The number of components of the path.
} DfsPath;

/**
 * Destroys a path.
 * 
 * @p path may be NULL.
 * 
 * @param path the path to destroy
 */
void dfs_destroy_path(DfsPath *path);

/**
 * Creates a new, empty path.
 * 
 * This returns NULL if no memory could be allocated.
 * 
 * @return the path, or NULL
 */
DfsPath *dfs_create_path(void);

/**
 * Resets a path.
 * 
 * The path will be a valid, but empty path.
 * 
 * @param path the path
 */
void dfs_reset_path(DfsPath *path) ATTRS(nonnull);

/**
 * Deep-clones a path.
 * 
 * The path may be NULL, in which case NULL will be returned as well.
 * 
 * @param orig the path to clone
 * 
 * @return a deep clone of the path, or NULL if no memory could be allocated
 */
DfsPath *dfs_clone_path(const DfsPath *orig);

/**
 * Concatenates two paths.
 * 
 * The first path will be modified.
 * 
 * @param first the path to concatenate to
 * @param second the path to concatenate
 * 
 * @return any status code
 */
DfsStatus dfs_path_concatenate(DfsPath *first, const DfsPath *second) ATTRS(nonnull);

/**
 * Appends a new component to a path.
 * 
 * The path will be modified.
 * 
 * @param path the path
 * @param elem the new component
 * 
 * @return any status code
 */
DfsStatus dfs_path_append(DfsPath *path, const char *elem) ATTRS(nonnull);

/**
 * Splits a path into a parent directory and a name.
 * 
 * If name_p is NULL, the name will be freed instead. If it is non-NULL, the address of the removed
 * name will be written there. This address must eventually be freed.
 * 
 * If the original path is empty, the returned path will also be empty and *name_p will be NULL.
 * 
 * @param path the path
 * @param name_p where to write the address of the removed name, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_path_pop(DfsPath *path, char **name_p) ATTRS(nonnull(1));

/**
 * Returns the last component of the path.
 * 
 * If the original path is empty, this will return NULL.
 * 
 * The returned string is part of the given path instance. It will remain valid as long as the
 * path remains valid.
 * 
 * @param path the path
 * 
 * @return the filename, or NULL
 */
ATTRS(const) static inline const char *dfs_path_filename(const DfsPath *path) {
    if (!path->num_components) return NULL;
    return path->components[path->num_components - 1];
}

/**
 * Parses a string to a path.
 * 
 * The string must conform to the usual Linux rules. That is, one or more slashes (`/`) separate
 * path components. A path component can be any character sequence of at least size 1 and not 
 * containing a slash or a NUL (`\0`).
 * 
 * An empty string yields a path of 0 components.
 * 
 * @param str the string to parse
 * 
 * @return the path, or NULL
 */
DfsPath *dfs_parse_path(const char *str) ATTRS(nonnull);

/**
 * Represents a path as a string.
 * 
 * The string does not have a leading or trailing slash. It must be freed using free.
 * 
 * str_p and len_p may be NULL.
 * 
 * If an error occurs, then the values `*str_p` and `*len_p` (if any) are unspecified.
 * 
 * @param path the path to represent
 * @param str_p where to store the address of the string, or NULL
 * @param len_p where to store the length of the string, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_unparse_path(const DfsPath *path, char **str_p, size_t *len_p) 
        ATTRS(nonnull(1));

/**
 * Looks up an entry by following the given path.
 * 
 * If the path is empty, the starting directory is returned.
 * 
 * @param dir the directory to start looking at
 * @param path the path to follow
 * @param entry_p where to write the address of the found entry, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_find_entry(DfsDir *dir, DfsPath *path, DfsEntry **entry_p)
        ATTRS(nonnull(1, 2));

/**
 * Looks up a file by following the given path.
 * 
 * If the path is empty, this returns DFS_E_NOT_A_FILE (because it attempts to return the root
 * directory).
 * 
 * @param dir the directory to start looking at
 * @param path the path to follow
 * @param file_p where to write the address of the found file, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_find_file(DfsDir *dir, DfsPath *path, DfsFile **file_p) 
        ATTRS(nonnull(1, 2));

/**
 * Looks up a directory by following the given path.
 * 
 * If the path is empty, the starting directory is returned.
 * 
 * @param dir the directory to start looking at
 * @param path the path to follow
 * @param dir_p where to write the address of the found directory, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_find_dir(DfsDir *dir, DfsPath *path, DfsDir **dir_p) 
        ATTRS(nonnull(1, 2));

/**
 * Looks up an entry by following the given path.
 * 
 * If the path is empty, the starting directory is returned.
 * 
 * @param dir the directory to start looking at
 * @param path the path to follow
 * @param entry_p where to write the address of the found entry, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_find_entry_str(DfsDir *dir, const char *path, DfsEntry **entry_p);

/**
 * Looks up a file by following the given path.
 * 
 * If the path is empty, this returns DFS_E_NOT_A_FILE (because it attempts to return the root
 * directory).
 * 
 * @param dir the directory to start looking at
 * @param path the path to follow
 * @param file_p where to write the address of the found file, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_find_file_str(DfsDir *dir, const char *path, DfsFile **file_p);

/**
 * Looks up a directory by following the given path.
 * 
 * If the path is empty, the starting directory is returned.
 * 
 * @param dir the directory to start looking at
 * @param path the path to follow
 * @param dir_p where to write the address of the found directory, or NULL
 * 
 * @return any status code
 */
DfsStatus dfs_find_dir_str(DfsDir *dir, const char *path, DfsDir **dir_p);

#undef ATTRS
