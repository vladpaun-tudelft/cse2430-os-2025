#include "dfs.h"
#include "strdup.h"

#include <sys/types.h>
#include <sys/stat.h>

const char *dfs_strerror(DfsStatus status) {
    switch (status) {
        case DFS_E_INTERNAL: return "internal error";
        case DFS_E_MALLOC_FAILURE: return "malloc failure";
        case DFS_E_ENTRY_DOES_NOT_EXIST: return "entry does not exist";
        case DFS_E_ENTRY_ALREADY_EXISTS: return "entry already exists";
        case DFS_E_NOT_A_DIRECTORY: return "not a directory";
        case DFS_E_NOT_A_FILE: return "not a file";
        case DFS_E_NO_MORE_ENTRIES: return "no more entries";
        case DFS_OK: return "no error";
    }

    return "unknown error";
}

void dfs_destroy_file(DfsFile *file) {
    if (!file) return;

    free(file->contents);
    free(file);
}

void dfs_destroy_dir(DfsDir *dir) {
    if (!dir || dir->being_destroyed) return;

    dir->being_destroyed = 1;

    for (;;) {
        DfsEntry *ent;

        ht_Error status = ht_rem_next(&dir->entries, NULL, &ent);
        if (status == HT_E_NO_ENTRIES_LEFT) break;
        assert(status >= HT_OK);

        dfs_destroy_entry(ent);
    }

    ht_destroy(&dir->entries);
    free(dir);
}

void dfs_destroy_entry(DfsEntry *ent) {
    switch (ent->type) {
        case DFS_ENT_FILE: 
            dfs_destroy_file((DfsFile *) ent);
            break;
        case DFS_ENT_DIR: 
            dfs_destroy_dir((DfsDir *) ent);
            break;
        default:
            assert(!"Bad entity type");
    }
}

DfsStatus dfs_map_ht_status(ht_Error e) {
    switch (e) {
        case HT_E_MALLOC_FAILURE: return DFS_E_MALLOC_FAILURE;
        case HT_E_KEY_NOT_PRESENT: return DFS_E_ENTRY_DOES_NOT_EXIST;
        case HT_E_KEY_ALREADY_PRESENT: return DFS_E_ENTRY_ALREADY_EXISTS;
        case HT_E_NO_ENTRIES_LEFT: return DFS_E_NO_MORE_ENTRIES;
        case HT_OK: return DFS_OK;
        default: return DFS_E_INTERNAL;
    }
}

static void populate_header(DfsEntry *entry) {
    mode_t um = umask(0);
    umask(um);
    timespec_get(&entry->ctime, TIME_UTC);
    entry->mtime = entry->atime = entry->ctime;
    entry->mode = ~um & 0777;
    entry->group = getgid();
    entry->user = getuid();
}

DfsFile *dfs_create_file(void) {
    DfsFile *file = calloc(1, sizeof(DfsFile));
    if (!file) return NULL;

    file->header.type = DFS_ENT_FILE;
    populate_header(&file->header);

    return file;
}

DfsDir *dfs_create_dir(void) {
    DfsDir *dir = calloc(1, sizeof(DfsDir));
    if (!dir) return NULL;

    dir->header.type = DFS_ENT_DIR;
    populate_header(&dir->header);
    ht_Error status = ht_create(&dir->entries, 0);

    if (status < HT_OK) {
        free(dir);
        return NULL;
    }

    return dir;
}

DfsStatus dfs_add_entry(DfsDir *dir, const char *name, DfsEntry *ent) {
    assert(dir);
    assert(name);
    assert(ent);

    timespec_get(&dir->header.atime, TIME_UTC);

    DfsStatus status = dfs_map_ht_status(ht_add(&dir->entries, name, ent));
    if (status != DFS_OK) return status;

    timespec_get(&dir->header.mtime, TIME_UTC);

    return DFS_OK;
}

DfsStatus dfs_get_entry(DfsDir *dir, const char *name, DfsEntry **entry_p) {
    assert(dir);
    assert(name);

    timespec_get(&dir->header.atime, TIME_UTC);

    DfsEntry *entry;
    DfsStatus status = dfs_map_ht_status(ht_get(&dir->entries, name, &entry));
    if (status != DFS_OK) return status;

    timespec_get(&entry->atime, TIME_UTC);

    if (entry_p) *entry_p = entry;

    return DFS_OK;
}

DfsStatus dfs_get_file(DfsDir *dir, const char *name, DfsFile **file_p) {
    assert(dir);
    assert(name);

    DfsEntry *ent;
    DfsStatus status = dfs_get_entry(dir, name, &ent);
    if (status) return status;

    if (ent->type != DFS_ENT_FILE) return DFS_E_NOT_A_FILE;
    if (file_p) *file_p = (DfsFile *) ent;

    return DFS_OK;
}

DfsStatus dfs_get_dir(DfsDir *dir, const char *name, DfsDir **child_dir_p) {
    assert(dir);
    assert(name);

    DfsEntry *ent;
    DfsStatus status = dfs_get_entry(dir, name, &ent);
    if (status) return status;

    if (ent->type != DFS_ENT_DIR) return DFS_E_NOT_A_DIRECTORY;
    if (child_dir_p) *child_dir_p = (DfsDir *) ent;

    return DFS_OK;
}

DfsStatus dfs_remove_entry(DfsDir *dir, const char *name, DfsEntry **entry_p) {
    assert(dir);
    assert(name);

    timespec_get(&dir->header.atime, TIME_UTC);

    DfsEntry *ent;
    ht_Error status = ht_rem(&dir->entries, name, &ent);
    if (status < HT_OK) return dfs_map_ht_status(status);

    timespec_get(&dir->header.mtime, TIME_UTC);

    if (entry_p) {
        *entry_p = ent;
    } else {
        dfs_destroy_entry(ent);
    }

    return DFS_OK;
}

DfsStatus dfs_remove_file(DfsDir *dir, const char *name, DfsFile **file_p) {
    assert(dir);
    assert(name);

    DfsEntry *ent;
    DfsStatus status = dfs_remove_entry(dir, name, &ent);
    if (status) return status;

    if (ent->type != DFS_ENT_FILE) return DFS_E_NOT_A_FILE;
    DfsFile *file = (DfsFile *) ent;

    if (file_p) {
        *file_p = file;
    } else {
        dfs_destroy_file(file);
    }

    return DFS_OK;
}

DfsStatus dfs_remove_dir(DfsDir *dir, const char *name, DfsDir **child_dir_p) {
    assert(dir);
    assert(name);

    DfsEntry *ent;
    DfsStatus status = dfs_remove_entry(dir, name, &ent);
    if (status) return status;

    if (ent->type != DFS_ENT_DIR) return DFS_E_NOT_A_DIRECTORY;
    DfsDir *child_dir = (DfsDir *) ent;

    if (child_dir_p) {
        *child_dir_p = child_dir;
    } else {
        dfs_destroy_dir(child_dir);
    }

    return DFS_OK;
}

DfsStatus dfs_get_next(DfsDir *dir, size_t *cursor, const char **key_p, DfsEntry **entry_p) {
    assert(dir);
    assert(cursor);

    timespec_get(&dir->header.atime, TIME_UTC);

    return dfs_map_ht_status(ht_get_next(&dir->entries, cursor, key_p, entry_p));
}
