#ifndef LIBRARY_C
#define LIBRARY_C

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ALLOC_LOG2 20
#define MAX_ALLOC ((size_t) 1 << MAX_ALLOC_LOG2)

#define MIN_ALLOC_LOG2 16
#define MIN_ALLOC ((size_t) 1 << MIN_ALLOC_LOG2)

#define NUM_LEVELS (MAX_ALLOC_LOG2 - MIN_ALLOC_LOG2 + 1)

/*
 * Starting address of the address range for this allocator. 
 * Every returned allocation will be an offset of this pointer 
 * from 0 (incl.) to MAX_ALLOC (excl.)
 */
uint8_t *base_ptr;

/*
 * A doubly chained linked list
 */
typedef struct block_list {
  struct block_list *prev;
  struct block_list *next;
} block_list;

/*
 * A doubly-chained list of free blocks per depth 
 * Depth = 0, block size = MAX_ALLOC
 * From one depth to the next the block size is divided by 2
 * Depth = NUM_LEVELS-1, block_size = MIN_ALLOC
 */
block_list levels[NUM_LEVELS];

uint8_t *is_split_index; // indexed by global_id (0 to last index in last level-1), used to decide on the size of an allocated block based on its ptr

uint8_t *merge_index; // indexed by buddy_pair_unique_id, used to decide whether two buddies should be merged

int max_allocator_depth();
int min_global_id_at_depth(size_t depth);
int max_global_id_at_depth(size_t depth);
int parent_global_id(int global_id);
int left_child_global_id(int global_id);
int global_id_to_depth(int global_id);
uint8_t *global_id_to_ptr(int global_id);
int buddy_pair_unique_id(int global_id);
int num_blocks_at_depth(size_t depth);
uint8_t block_is_split(int global_id); // returns 0 or 1
int parent_is_split(int global_id); // returns 0 or 1
uint8_t block_is_free(int global_id);



// Check if assignment 4_3, otherwise skip since helper functions are not implemented
#ifdef ASSIGNMENT_4_3

/*
 * Append the provided entry to the end of the list. This assumes the entry
 * isn't in a list already because it overwrites the linked list pointers.
 */
void list_push(block_list *list, block_list *entry) {
  block_list *prev = list->prev;
  entry->prev = prev;
  entry->next = list;
  prev->next = entry;
  list->prev = entry;
}

/*
 * Remove the provided entry from whichever list it's currently in. This
 * assumes that the entry is in a list. You don't need to provide the list
 * because the lists are circular, so the list's pointers will automatically
 * be updated if the first or last entries are removed.
 */
void list_remove(block_list *entry) {
  block_list *prev = entry->prev;
  block_list *next = entry->next;
  prev->next = next;
  next->prev = prev;
}

/*
 * Remove and return the first entry in the list or NULL if the list is empty.
 */
block_list *list_pop(block_list *list) {
  block_list *back = list->prev;
  if (back == list) return NULL;
  list_remove(back);
  return back;
}

// Free the datastructures used
void free_allocator() {
//     printf("free allocator\n");
    free(base_ptr);
//     printf("free merge_index\n");
    free(merge_index);
//     printf("free is_split_index\n");
    free(is_split_index);
}

/*
 * Initialise all the datastructures used
 * Including, the base pointer, the merge_index and is_split_index and the lists
 */
void init_allocator() {
    // Allocate memory region
    base_ptr = malloc(MAX_ALLOC);

    // Initialize buckets with empty lists
    for (int i = 0; i < NUM_LEVELS; i++) {
        levels[i].prev = &(levels[i]);
        levels[i].next = &(levels[i]);
    }
    // Initialize first (largest) level
    list_push(&levels[0], (block_list *)base_ptr);
    
    // Init free index
    // Compute number of characters needed to store one bit per buddy pair
    int size_merge_index = buddy_pair_unique_id(max_global_id_at_depth(max_allocator_depth()));
    if (size_merge_index % 8 != 0) {
        size_merge_index = (size_merge_index / 8) * 8 + 8;
    }
    size_merge_index /= 8;
    // TODO: size_merge_index is also given by formula (1 << (max_allocator_depth()-1)) / 8
        
    merge_index = calloc(size_merge_index, sizeof(uint8_t));
    
    // Compute number of characters needed to store one bit per internal buddy 
    // Note that the leaves do not need an entry
    int size_block_is_split = max_global_id_at_depth(max_allocator_depth()-1);
    if (size_block_is_split % 8 != 0) {
        size_block_is_split = (size_block_is_split / 8) * 8 + 8;
    }
    size_block_is_split /= 8;
    
    is_split_index = calloc(size_block_is_split, sizeof(uint8_t));
}

int block_is_in_free_list(int global_id) {
    int block_depth = global_id_to_depth(global_id);
    void *block_ptr = global_id_to_ptr(global_id);

    block_list *list = &levels[block_depth];
    block_list *cur = list->prev;
    if (cur == list) return 0;

    while (cur != list) {
        if ((void *) cur == block_ptr) {
            return 1;
        }
        cur = cur->prev;
    }

    return 0;
}

void print_block_states() {
    int print_len = num_blocks_at_depth(max_allocator_depth());
    int num_separators = print_len-1;
    print_len += num_separators + 2;
    
    for (int k=0; k<print_len; k++) {printf("-");}
    printf("\n");
    
    for (int d = 0; d <= max_allocator_depth(); d++) {
        int whites;
        if (d == max_allocator_depth()) {
            whites = 0; 
        } else {
            whites = (1 << (max_allocator_depth()-d)) - 1;
        }
        printf("|");
        for (int i = min_global_id_at_depth(d); i <= max_global_id_at_depth(d); i++) {
            for (int k=0; k<whites; k++) {printf(" ");}
            if (d != max_allocator_depth()) { // internal block
                if (block_is_split(i)) {
                    printf("S");
                } else if (i!=0 && block_is_split(parent_global_id(i)) && !block_is_in_free_list(i)) {
                    printf("A");
                } else {
                    printf("F");
                }

            } else { // leaves: cannot be split
                if (block_is_split(parent_global_id(i)) && !block_is_in_free_list(i)) {
                    printf("A");
                } else {
                    printf("F");
                }
            }
            for (int k=0; k<whites; k++) {printf(" ");}
            if (i != max_global_id_at_depth(d)) {
                printf("|");
            }
        }
        printf("|\n");
        for (int k=0; k<print_len; k++) {printf("-");}
        printf("\n");
    }
}
#endif


#endif
