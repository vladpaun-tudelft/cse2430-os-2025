#ifndef ASSIGNMENT_4_3
#define ASSIGNMENT_4_3
#endif

#include "library.c"

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

size_t block_size_at_depth(size_t depth) { return MAX_ALLOC >> depth; }

int num_blocks_at_depth(size_t depth) { return 1 << depth; }

uint8_t *global_id_to_ptr(int global_id) {
    int depth = global_id_to_depth(global_id);
    size_t block_size = block_size_at_depth(depth);
    int level_index = global_id_to_level_index(global_id);

    return base_ptr + ((size_t)level_index * block_size);
}

int ptr_to_global_id(void *ptr, size_t depth) {
    size_t block_size = block_size_at_depth(depth);
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

static uint8_t bit_get(uint8_t *index, int bit) {
    return (index[bit / 8] >> (bit % 8)) & 1U;
}

static void bit_flip(uint8_t *index, int bit) {
    index[bit / 8] ^= (uint8_t)(1U << (bit % 8));
}

uint8_t block_is_split(int global_id) {
    if (global_id < 0 ||
        global_id > max_global_id_at_depth(max_allocator_depth() - 1)) {
        return 0;
    }

    return bit_get(is_split_index, global_id);
}

int parent_is_split(int global_id) {
    if (global_id <= 0) {
        return 0;
    }

    return block_is_split(parent_global_id(global_id));
}

void flip_parent_is_split(int global_id) {
    if (global_id <= 0) {
        return;
    }

    bit_flip(is_split_index, parent_global_id(global_id));
}

int depth_allocated_ptr(void *ptr) {
    int depth = max_allocator_depth();

    while (depth > 0) {
        int parent_id = ptr_to_global_id(ptr, depth - 1);
        if (block_is_split(parent_id)) {
            return depth;
        }
        depth--;
    }

    return 0;
}

void flip_block_is_free(int global_id) {
    bit_flip(merge_index, buddy_pair_unique_id(global_id));
}

uint8_t block_is_free(int global_id) {
    return bit_get(merge_index, buddy_pair_unique_id(global_id));
}

uint8_t *allocator_malloc(size_t requested_size) {
    int target_depth = depth_for_request(requested_size);
    if (target_depth < 0) {
        return NULL;
    }

    int depth = target_depth;
    while (depth >= 0 && levels[depth].next == &levels[depth]) {
        depth--;
    }

    if (depth < 0) {
        return NULL;
    }

    block_list *block = list_pop(&levels[depth]);
    int block_id = ptr_to_global_id(block, depth);
    flip_block_is_free(block_id);

    while (depth < target_depth) {
        int left_id = left_child_global_id(block_id);
        int right_id = right_child_global_id(block_id);
        uint8_t *right_ptr = global_id_to_ptr(right_id);

        flip_parent_is_split(left_id);
        list_push(&levels[depth + 1], (block_list *)right_ptr);
        flip_block_is_free(right_id);

        block = (block_list *)global_id_to_ptr(left_id);
        block_id = left_id;
        depth++;
    }

    return (uint8_t *)block;
}

void allocator_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    uint8_t *block_ptr = ptr;
    if (block_ptr < base_ptr || block_ptr >= base_ptr + MAX_ALLOC) {
        return;
    }

    int depth = depth_allocated_ptr(ptr);
    int block_id = ptr_to_global_id(ptr, depth);

    flip_block_is_free(block_id);

    while (depth > 0 && block_is_free(block_id) == 0) {
        int old_block_id = block_id;
        int buddy_id = buddy_global_id(block_id);
        block_list *buddy = (block_list *)global_id_to_ptr(buddy_id);

        list_remove(buddy);
        flip_parent_is_split(old_block_id);

        block_id = parent_global_id(old_block_id);
        block_ptr = global_id_to_ptr(block_id);
        depth--;
        flip_block_is_free(block_id);
    }

    list_push(&levels[depth], (block_list *)block_ptr);
}