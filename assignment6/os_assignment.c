/**
 * @file os_assignment.c
 *
 * This is the file you will be working in. You will have to complete the
 * (partial) implementation given below. Any functions you have to complete are
 * clearly marked as such.
 *
 * Good luck!
 */

#include "dfs-helpers.h"
#include "dfs-path.h"
#include "dfs.h"
#include "strdup.h"

// This is to silence an error locally, maybe it breaks shit on weblab, idk?
#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 31
#include <fuse.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * The root directory of the file system.
 */
DfsDir *root_dir;

/**
 * Initializes the default file system.
 *
 * This fills the DFS with a number of directories and files. Add, remove and
 * rename some files and directories, if you wish.
 *
 * Note how this creates a top-level directory root_dir. You will have to use
 * this directory.
 */
void create_dfs(void) {
    // Define some directories
    root_dir = dfs_create_dir();

    DfsDir *files_dir = dfs_create_dir();
    DfsDir *pictures_dir = dfs_create_dir();
    DfsDir *important_files_dir = dfs_create_dir();
    assert(files_dir && pictures_dir && important_files_dir);

    // Define some files
    DfsFile *welcome_file = make_file("Welcome to the DFS.\n");
    DfsFile *budget_file = make_file(budget_text);
    DfsFile *hello_1_file = make_file("Hello there!\n");
    DfsFile *hello_2_file = make_file("Hei siellä!\n");
    DfsFile *image_1_file = make_binary_file(image_1, sizeof(image_1));
    DfsFile *image_2_file = make_binary_file(image_2, sizeof(image_2));
    assert(welcome_file && budget_file && hello_1_file && hello_2_file &&
           image_1_file && image_2_file);

    // Add everything together
    assert_dfs_ok(dfs_add_dir(root_dir, "Files", files_dir));
    assert_dfs_ok(dfs_add_dir(root_dir, "Pictures", pictures_dir));

    assert_dfs_ok(
        dfs_add_dir(files_dir, "Important files", important_files_dir));

    assert_dfs_ok(dfs_add_file(root_dir, "README.txt", welcome_file));
    assert_dfs_ok(dfs_add_file(files_dir, "hello.txt", hello_1_file));
    assert_dfs_ok(dfs_add_file(files_dir, "hello2.txt", hello_2_file));
    assert_dfs_ok(dfs_add_file(important_files_dir, "budget.csv", budget_file));
    assert_dfs_ok(dfs_add_file(pictures_dir, "some image.png", image_1_file));
    assert_dfs_ok(
        dfs_add_file(pictures_dir, "some other image.png", image_2_file));
}

/**
 * @defgroup Assignment The assignment
 * These functions must be completed for the assignment.
 *
 * The functions here directly correspond to the syscall with the same name,
 * unless noted otherwise.
 * @{
 */

/**
 * Lists files/directories available in a specific directory.
 *
 * This function is specific to FUSE; there is no directly corresponding system
 * call.
 *
 * @param path directory which we are listing
 * @param buff buffer to fill with data
 * @param fill a function to help fill buffer
 * @param off not used
 * @param fi not used
 *
 * @return 0 or an appropriate error code
 */
static int os_readdir(const char *path, void *buff, fuse_fill_dir_t fill,
                      off_t off, struct fuse_file_info *fi) {
    DfsDir *dir;
    DfsPath *dfs_path = dfs_parse_path(path);
    if (dfs_path == NULL)
        return -1;

    assert_dfs_ok(dfs_find_dir(root_dir, dfs_path, &dir) != DFS_OK);

    // Example: this is how you could iterate over all entries in a dir
    size_t cursor = 0;
    for (;;) {
        const char *name;
        DfsStatus get_next_status = dfs_get_next(dir, &cursor, &name, NULL);
        if (get_next_status == DFS_E_NO_MORE_ENTRIES)
            break;
        else if (get_next_status != DFS_OK)
            return -1;
        fill(buff, name, NULL, 0);
    }

    return 0;
}

/**
 * Creates a new directory.
 *
 * @param path the path to the new directory
 * @param mode the mode (permissions) of the new directory. Can be ignored
 *
 * @return 0 or an appropriate error code
 *
 * @see [`man 2 mkdir`](https://linux.die.net/man/2/mkdir)
 */
static int os_mkdir(const char *path, mode_t mode) { return -ENOTSUP; }

/**
 * Removes a directory.
 *
 * This must return an error if the directory is not empty.
 *
 * @param path the path to the directory to remove
 *
 * @return 0 or an appropriate error code
 *
 * @see [`man 2 rmdir`](https://linux.die.net/man/2/rmdir)
 */
static int os_rmdir(const char *path) { return -ENOTSUP; }

/**
 * Reads (part of) a file.
 *
 * This may return fewer bytes than requested, but only if the end of the file
 * was reached. Note that the system call behaves differently!
 *
 * @param path the path to the file
 * @param buff where to write the read data
 * @param size the number of bytes to read
 * @param off where to start reading from
 * @param fi can be ignored
 *
 * @return the number of bytes read or an appropriate error code
 *
 * @see [`man 2 read`](https://linux.die.net/man/2/read)
 */
static int os_read(const char *path, char *buff, size_t size, off_t off,
                   struct fuse_file_info *fi) {
    return -ENOTSUP;
}

/**
 * Writes (part of) a file.
 *
 * This must write all given bytes. Note that the system call behaves
 * differently!
 *
 * @param path the path to the file
 * @param buff the data to write
 * @param size the number of bytes to write
 * @param off where to start writing from
 * @param fi can be ignored
 *
 * @return the number of bytes written or an appropriate error code
 *
 * @see [`man 2 write`](https://linux.die.net/man/2/write)
 */
static int os_write(const char *path, const char *buff, size_t size, off_t off,
                    struct fuse_file_info *fi) {
    return -ENOTSUP;
}

/**
 * Creates a file.
 *
 * @param path the path to the file
 * @param mode the mode (permissions) of the new file. Can be ignored
 * @param fi can be ignored
 *
 * @return 0 or an appropriate error code
 *
 * @see [`man 2 creat`](https://linux.die.net/man/2/creat) (that is not a typo)
 */
static int os_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    return -ENOTSUP;
}

/**
 * Unlinks (deletes) a file.
 *
 * @param path the path to the file
 *
 * @return 0 or appropriate error code
 *
 * @see [`man 2 unlink`](https://linux.die.net/man/2/unlink)
 */
static int os_unlink(const char *path) { return -ENOTSUP; }

/// @}
/**
 * @defgroup Predefined Predefined functions
 * You do not have to modify these for the assignment.
 * @{
 */

/**
 * Counts the number of (direct) subdirectories of a directory.
 *
 * @param dir the directory to scan
 *
 * @return the number of subdirectories
 */
static size_t os_count_subdirs(DfsDir *dir) {
    size_t cursor = 0;
    size_t num_subdirs = 0;
    DfsEntry *entry;
    for (;;) {
        DfsStatus get_next_status = dfs_get_next(dir, &cursor, NULL, &entry);
        if (get_next_status == DFS_E_NO_MORE_ENTRIES)
            break;
        if (entry->type == DFS_ENT_DIR)
            ++num_subdirs;
    }

    return num_subdirs;
}

/**
 * Retrieves file/directory attributes.
 *
 * @param path the path for which attributes are requested
 * @param st struct to be filled with attributes
 *
 * @return 0 or an appropriate error code
 *
 * @see [`man 2 stat`](https://linux.die.net/man/2/stat)
 */
static int os_getattr(const char *path, struct stat *st) {
    DfsEntry *entry;
    DfsStatus entry_status = dfs_find_entry_str(root_dir, path, &entry);
    if (entry_status == DFS_E_MALLOC_FAILURE)
        return -ENOMEM;
    if (entry_status == DFS_E_ENTRY_DOES_NOT_EXIST)
        return -ENOENT;

    st->st_uid = entry->user;
    st->st_gid = entry->group;
    st->st_ctime = entry->ctime.tv_sec;
    st->st_mtime = entry->mtime.tv_sec;
    st->st_atime = entry->atime.tv_sec;

    if (entry->type == DFS_ENT_FILE) {
        DfsFile *file = (DfsFile *)entry;
        st->st_mode =
            S_IFREG |
            entry->mode;  // Requested item is (probably) a regular file
        st->st_nlink = 1; // Number of hard links: a file has at least one
        st->st_size = file->length;
    } else if (entry->type == DFS_ENT_DIR) {
        DfsDir *dir = (DfsDir *)entry;
        st->st_mode = S_IFDIR | entry->mode;
        st->st_nlink = os_count_subdirs(dir) + 1;
        st->st_size = dfs_get_dir_size(dir) * 32; // Estimate :)
    }

    return 0;
}

/**
 * Resizes a file.
 *
 * @param path the path to the file
 * @param off the new size of the file in bytes
 *
 * @return 0 or an appropriate error code
 *
 * @see [`man 2 truncate`](https://linux.die.net/man/2/truncate)
 */
static int os_truncate(const char *path, off_t off) {
    DfsFile *file;
    DfsStatus file_status = dfs_find_file_str(root_dir, path, &file);
    if (file_status == DFS_E_MALLOC_FAILURE)
        return -ENOMEM;
    if (file_status == DFS_E_ENTRY_DOES_NOT_EXIST)
        return -ENOENT;
    if (file_status == DFS_E_NOT_A_FILE)
        return -EISDIR;

    if (off == 0) {
        free(file->contents);
        file->contents = NULL;
        file->length = 0;
        return 0;
    }

    char *new_contents = realloc(file->contents, off);
    if (!new_contents)
        return -ENOMEM;

    file->contents = new_contents;

    if ((size_t)off > file->length) {
        memset(file->contents + file->length, 0, off - file->length);
    }

    file->length = off;

    return 0;
}

/**
 * Sets extended attributes on an entry.
 *
 * This is currently not implemented (and you do not have to implement it
 * yourself).
 *
 * @param path the path to the file
 * @param name the name of the attribute
 * @param value the value of the attribute
 * @param length the length of the attribute
 * @param flags any flags
 *
 * @return 0 or an appropriate error code
 */
static int os_setxattr(const char *path, const char *name, const char *value,
                       size_t length, int flags) {
    return -ENOTSUP;
}

/**
 * Changes the mode (permissions) of an entry.
 *
 * @param path the path to the file
 * @param mode the new mode
 *
 * @return 0 or an appropriate error code
 *
 * @see [`man 2 chmod`](https://linux.die.net/man/2/chmod)
 */
static int os_chmod(const char *path, mode_t mode) {
    DfsEntry *entry;
    DfsStatus entry_status = dfs_find_entry_str(root_dir, path, &entry);
    if (entry_status == DFS_E_MALLOC_FAILURE)
        return -ENOMEM;
    if (entry_status == DFS_E_ENTRY_DOES_NOT_EXIST)
        return -ENOENT;

    entry->mode = mode;

    return 0;
}

/**
 * Changes the owning user and group of an entry.
 *
 * @param path the path to the file
 * @param uid the new owning user
 * @param gid the new owning group
 *
 * @return 0 or an appropriate error code
 *
 * @see [`man 2 chown`](https://linux.die.net/man/2/chown)
 */
static int os_chown(const char *path, uid_t uid, gid_t gid) {
    DfsEntry *entry;
    DfsStatus entry_status = dfs_find_entry_str(root_dir, path, &entry);
    if (entry_status == DFS_E_MALLOC_FAILURE)
        return -ENOMEM;
    if (entry_status == DFS_E_ENTRY_DOES_NOT_EXIST)
        return -ENOENT;

    entry->user = uid;
    entry->group = gid;

    return 0;
}

/**
 * Changes the atime and mtime of an entry.
 *
 * @param path the path to the entry
 * @param tv the new atime and mtime values, respectively
 *
 * @return 0 or an appropriate error code
 *
 * @see [`man 2 utimensat`](https://linux.die.net/man/2/utimensat)
 */
static int os_utimens(const char *path, const struct timespec tv[2]) {
    DfsEntry *entry;
    DfsStatus entry_status = dfs_find_entry_str(root_dir, path, &entry);
    if (entry_status == DFS_E_MALLOC_FAILURE)
        return -ENOMEM;
    if (entry_status == DFS_E_ENTRY_DOES_NOT_EXIST)
        return -ENOENT;

    entry->atime = tv[0];
    entry->mtime = tv[1];

    return 0;
}

/**
 * Here we define the operations FUSE has access to.
 *
 * Not all members have been populated and you should not call those!
 */
static struct fuse_operations operations = {.getattr = os_getattr,
                                            .readdir = os_readdir,
                                            .read = os_read,
                                            .mkdir = os_mkdir,
                                            .rmdir = os_rmdir,
                                            .write = os_write,
                                            .setxattr = os_setxattr,
                                            .truncate = os_truncate,
                                            .chmod = os_chmod,
                                            .chown = os_chown,
                                            .utimens = os_utimens,
                                            .create = os_create,
                                            .unlink = os_unlink};

/**
 * Main function.
 *
 * @param argc the number of arguments
 * @param argv the arguments
 */
int main(int argc, char **argv) {
    // Setup dumb file system
    create_dfs();

    // Pass arguments on to FUSE.
    return fuse_main(argc, argv, &operations, NULL);
}

/// @}
