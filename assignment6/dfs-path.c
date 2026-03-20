#include "dfs-path.h"

void dfs_destroy_path(DfsPath *path) {
    if (!path) return;

    for (size_t i = 0; i < path->num_components; ++i) {
        free(path->components[i]);
    }

    free(path->components);
    free(path);
}

DfsPath *dfs_create_path(void) {
    return calloc(1, sizeof(DfsPath));
}

void dfs_reset_path(DfsPath *path) {
    assert(path);

    for (size_t i = 0; i < path->num_components; ++i) {
        free(path->components[i]);
    }

    free(path->components);
    
    path->components = NULL;
    path->num_components = 0;
}

DfsPath *dfs_clone_path(const DfsPath *orig) {
    if (!orig) return NULL;

    DfsPath *clone = dfs_create_path();
    if (!clone) return NULL;

    DfsStatus concat_status = dfs_path_concatenate(clone, orig);
    if (concat_status == DFS_E_MALLOC_FAILURE) return NULL;
    if (concat_status != DFS_OK) {
        dfs_destroy_path(clone);
        return NULL;
    }

    return clone;
}

DfsStatus dfs_path_concatenate(DfsPath *first, const DfsPath *second) {
    assert(first);
    assert(second);

    size_t new_len = first->num_components + second->num_components;
    char **new_components = realloc(first->components, new_len * sizeof(*new_components));
    if (!new_components) {
        return DFS_E_MALLOC_FAILURE;
    }

    first->components = new_components;

    for (size_t i = 0; i < second->num_components; ++i) {
        size_t d_idx = first->num_components + i;

        char *com_clone = strdup(second->components[i]);
        if (!com_clone) {
            first->num_components = d_idx;
            return DFS_E_MALLOC_FAILURE;
        }

        first->components[d_idx] = com_clone;
    }

    first->num_components = new_len;

    return DFS_OK;
}

DfsStatus dfs_path_append(DfsPath *path, const char *elem) {
    assert(path);
    assert(elem);

    size_t new_len = path->num_components + 1;
    char **new_components = realloc(path->components, new_len * sizeof(*new_components));
    if (!new_components) {
        return DFS_E_MALLOC_FAILURE;
    }

    path->components = new_components;

    char **last = path->components + path->num_components - 1;
    *last = strdup(elem);
    if (!*last) {
        return DFS_E_MALLOC_FAILURE;
    }

    path->num_components = new_len;

    return DFS_OK;
}

DfsStatus dfs_path_pop(DfsPath *path, char **filename) {
    assert(path);

    if (!path->num_components) {
        if (filename) *filename = NULL;
        return DFS_OK;
    }

    char *last = path->components[--path->num_components];
    if (filename) {
        *filename = last;
    } else {
        free(last);
    }

    if (path->num_components == 0) {
        free(path->components);
        path->components = NULL;
        return DFS_OK;
    }

    char **new_components = realloc(
        path->components, path->num_components * sizeof(*path->components)
    );
    if (!new_components) {
        return DFS_E_MALLOC_FAILURE;
    }

    path->components = new_components;

    return DFS_OK;
}

DfsPath *dfs_parse_path(const char *istr) {
    assert(istr);

    DfsPath *path = dfs_create_path();
    if (!path) return NULL;

    // Trim leading and trailing slashes
    while (*istr == '/') ++istr;
    size_t str_len = strlen(istr);
    while (str_len > 0 && istr[str_len - 1] == '/') --str_len;

    if (str_len == 0) return path;

    char *str = strdup(istr);
    if (!str) {
        dfs_destroy_path(path);
        return NULL;
    }

    // Count components
    char *tok = strtok(str, "/");
    while (tok) {
        ++path->num_components;
        tok = strtok(NULL, "/");
    }
    
    path->components = calloc(path->num_components, sizeof(char *));
    if (!path->components) {
        path->num_components = 0;
        dfs_destroy_path(path);
        return NULL;
    }

    // Copy components
    memcpy(str, istr, str_len);
    tok = strtok(str, "/");
    for (size_t i = 0; tok; ++i) {
        path->components[i] = strdup(tok);

        if (!path->components[i]) {
            path->num_components = i;
            dfs_destroy_path(path);
            return NULL;
        }

        tok = strtok(NULL, "/");
    }

    free(str);

    return path;
}

DfsStatus dfs_unparse_path(const DfsPath *path, char **str_p, size_t *len_p) {
    assert(path);

    if (!str_p && !len_p) return DFS_OK;

    if (!path->num_components) {
        if (str_p) {
            char *estr = calloc(1, 1);
            if (!estr) return DFS_E_MALLOC_FAILURE;
            *str_p = estr;
        }
        if (len_p) *len_p = 0;

        return DFS_OK;
    }

    size_t len = 0;
    for (unsigned int i = 0; i < path->num_components; ++i) {
        len += strlen(path->components[i]) + 1;
    }

    if (len_p) *len_p = len - 1;
    if (!str_p) return DFS_OK;

    *str_p = calloc(len, 1);
    if (!*str_p) return DFS_E_MALLOC_FAILURE;

    size_t pos = 0;
    for (unsigned int i = 0; i < path->num_components; ++i) {
        size_t component_len = strlen(path->components[i]);
        memcpy(*str_p + pos, path->components[i], component_len);

        pos += component_len;
        (*str_p)[pos++] = '/';
    }

    (*str_p)[pos - 1] = 0;

    return DFS_OK;
}

DfsStatus dfs_find_entry(DfsDir *dir, DfsPath *path, DfsEntry **entry_p) {
    assert(dir);
    assert(path);

    DfsEntry *entry = (DfsEntry *) dir;
    for (size_t i = 0; i < path->num_components; ++i) {
        if (entry->type != DFS_ENT_DIR) return DFS_E_NOT_A_DIRECTORY;

        DfsDir *dir_next = (DfsDir *) entry;
        DfsStatus status = dfs_get_entry(dir_next, path->components[i], &entry);
        if (status) return status;
    }

    if (entry_p) *entry_p = entry;
    return DFS_OK;
}

DfsStatus dfs_find_file(DfsDir *dir, DfsPath *path, DfsFile **file_p) {
    DfsEntry *entry;
    DfsStatus status = dfs_find_entry(dir, path, &entry);
    if (status) return status;

    if (entry->type != DFS_ENT_FILE) return DFS_E_NOT_A_FILE;
    if (file_p) *file_p = (DfsFile *) entry;

    return DFS_OK;
}

DfsStatus dfs_find_dir(DfsDir *dir, DfsPath *path, DfsDir **dir_p) {
    DfsEntry *entry;
    DfsStatus status = dfs_find_entry(dir, path, &entry);
    if (status) return status;

    if (entry->type != DFS_ENT_DIR) return DFS_E_NOT_A_DIRECTORY;
    if (dir_p) *dir_p = (DfsDir *) entry;

    return DFS_OK;
}

DfsStatus dfs_find_entry_str(DfsDir *dir, const char *path, DfsEntry **entry_p) {
    DfsPath *dpath = dfs_parse_path(path);
    if (!dpath) {
        return DFS_E_MALLOC_FAILURE;
    }

    DfsStatus find_status = dfs_find_entry(dir, dpath, entry_p);
    dfs_destroy_path(dpath);
    return find_status;
}

DfsStatus dfs_find_file_str(DfsDir *dir, const char *path, DfsFile **file_p) {
    DfsPath *dpath = dfs_parse_path(path);
    if (!dpath) {
        return DFS_E_MALLOC_FAILURE;
    }

    DfsStatus find_status = dfs_find_file(dir, dpath, file_p);
    dfs_destroy_path(dpath);
    return find_status;
}


DfsStatus dfs_find_dir_str(DfsDir *dir, const char *path, DfsDir **dir_p) {
    DfsPath *dpath = dfs_parse_path(path);
    if (!dpath) {
        return DFS_E_MALLOC_FAILURE;
    }

    DfsStatus find_status = dfs_find_dir(dir, dpath, dir_p);
    dfs_destroy_path(dpath);
    return find_status;
}
