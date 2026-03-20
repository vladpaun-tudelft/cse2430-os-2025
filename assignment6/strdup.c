#include "strdup.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if !(_XOPEN_SOURCE >= 500 || _POSIX_C_SOURCE >= 200809L || _BSD_SOURCE || _SVID_SOURCE)
char *strdup(const char *str) {
    assert(str);
    const size_t len = strlen(str) + 1;
    char *nstr = malloc(len);
    if (!nstr) return NULL;
    memcpy(nstr, str, len);
    return nstr;
}
#endif
