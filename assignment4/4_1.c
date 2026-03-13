#include "library.c"
#include <stddef.h>

int max_allocator_depth() { return NUM_LEVELS - 1; }

int min_global_id_at_depth(size_t depth) { return (1 << depth) - 1; }

int max_global_id_at_depth(size_t depth) { return (1 << (depth + 1)) - 2; }

int max_global_id() { return max_global_id_at_depth(max_allocator_depth()); }

int parent_global_id(int global_id) {
    if (global_id <= 0) {
        return 0;
    }

    return (global_id - 1) / 2;
}

int left_child_global_id(int global_id) { return global_id * 2 + 1; }

int right_child_global_id(int global_id) { return global_id * 2 + 2; }

int level_index_to_global_id(int level_index, int depth) {
    return min_global_id_at_depth(depth) + level_index;
}

int global_id_to_depth(int global_id) {
    int depth = 0;

    while (max_global_id_at_depth(depth) < global_id) {
        depth++;
    }

    return depth;
}

int global_id_to_level_index(int global_id) {
    int depth = global_id_to_depth(global_id);
    return global_id - min_global_id_at_depth(depth);
}

uint8_t *global_id_to_ptr(int global_id) {
    int depth = global_id_to_depth(global_id);
    size_t block_size = MAX_ALLOC >> depth;
    int level_index = global_id_to_level_index(global_id);

    return base_ptr + ((size_t)level_index * block_size);
}

int ptr_to_global_id(void *ptr, size_t depth) {
    size_t block_size = MAX_ALLOC >> depth;
    int level_index = ((uint8_t *)ptr - base_ptr) / block_size;

    return level_index_to_global_id(level_index, depth);
}

int buddy_global_id(int global_id) {
    if (global_id == 0) {
        return 0;
    }

    if (global_id % 2 == 0) {
        return global_id - 1;
    }

    return global_id + 1;
}

int buddy_pair_unique_id(int global_id) { return (global_id + 1) / 2; }

int max_buddy_pair_unique_id() { return buddy_pair_unique_id(max_global_id()); }

size_t block_size_at_depth(size_t depth) { return MAX_ALLOC >> depth; }

int num_blocks_at_depth(size_t depth) { return 1 << depth; }

int depth_for_request(size_t requested_size) {
    if (requested_size > MAX_ALLOC) {
        return -1;
    }

    int depth = 0;
    while (depth < max_allocator_depth() &&
           block_size_at_depth(depth + 1) >= requested_size) {
        depth++;
    }

    return depth;
}
