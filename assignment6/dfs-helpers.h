/**
 * @file
 * DFS helpers for os_assignment and example data.
 */

#pragma once

#include "dfs.h"
#include <stdio.h>

const char image_1[] =
    "\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x08\x00\x00\x00\x08\x08"
    "\x00\x00\x00\x00"
    "\xe1"
    "d\xe1W\x00\x00\x00'IDAT\x08\xd7U\x8b"
    "A\n\x00\x00\x08\xc2"
    "f\xff\xff\xf3:DA^\x1c"
    "2#\x93\xe2 "
    "\x10\xb2\x8b\xe0\xb4\x05\xfa\xe4\xec\xbd\x01\x02\xd9\x08\x08\xbf\xe5/<\x00"
    "\x00\x00\x00IEND\xae"
    "B`\x82";
const char image_2[] =
    "\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x08\x00\x00\x00\x08\x08"
    "\x04\x00\x00\x00n"
    "\x06v\x00\x00\x00\x00"
    "4IDAT\x08\xd7U\x8d"
    "1\x12\x00"
    "0\x08\xc2\x82\xe7\xff\xbfL\x07m"
    "\xb5N@Td\xf3M\x8c\x14\x02\xf2\x1a\x00W\xa0\xde\xa8\xdb\xd0\x93\x85"
    "b[\xcf\xd3\xa9J/\np"
    "\x00~9\n\x14\xf4"
    "f\x0c\x9e\x00\x00\x00\x00IEND\xae"
    "B`\x82";
// https://twitter.com/dril/status/384408932061417472
const char *budget_text =
    "Item;Amount\nRent;€963\nFood;€200\nData;€95\nRPis;€4,300\nUtilities;€120\n"
    ";;someone who is good at the economy please help me budget this. my "
    "family is dying\n";

/**
 * Creates a new text file.
 *
 * The file does not include the NUL terminator.
 *
 * @param contents the contents for the new file
 *
 * @return the file or NULL if not enough memory could be allocated
 */
static DfsFile *make_file(const char *contents) {
    DfsFile *file = dfs_create_file();
    if (!file)
        return NULL;

    file->length = strlen(contents);
    file->contents = (char *)malloc(file->length);
    if (!file->contents) {
        dfs_destroy_file(file);
        return NULL;
    }
    memcpy(file->contents, contents, file->length);
    return file;
}

/**
 * Creates a new binary file.
 *
 * @param contents the contents for the new file
 * @param length the length of the contents
 *
 * @return the file or NULL if not enough memory could be allocated
 */
static DfsFile *make_binary_file(const char *contents, size_t length) {
    DfsFile *file = dfs_create_file();
    if (!file)
        return NULL;

    file->contents = (char *)malloc(length);
    if (!file->contents) {
        dfs_destroy_file(file);
        return NULL;
    }
    memcpy(file->contents, contents, length);
    file->length = length;
    return file;
}

/**
 * Asserts that the call to DFS succeeded with a DFS_OK status.
 *
 * Use this if there should be absolutely no way the call can fail. If the call
 * fails, a message will be logged and the program will be aborted.
 *
 * The output error message will be the clearest if the DFS function call is
 * passed directly and not via a separate variable.
 *
 * @param e_ the DFS function call
 */
#define assert_dfs_ok(e_)                                                      \
    assert_dfs_ok_impl(e_, #e_, __func__, __FILE__, __LINE__)

static void assert_dfs_ok_impl(DfsStatus status, const char *exp,
                               const char *func, const char *file, int line) {
    if (status == DFS_OK)
        return;

    fprintf(stderr, "DFS error on %s:%d in %s: %s returned %s\n", file, line,
            func, exp, dfs_strerror(status));
    abort();
}
