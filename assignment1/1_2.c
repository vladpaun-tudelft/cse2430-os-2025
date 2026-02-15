#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

int main(void) {
    system("mkdir -p tmpdir_system");
    system("ls");

    mkdir("tmpdir_lib", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    DIR *d = opendir(".");
    struct dirent *entry = readdir(d);
    while (entry != NULL) {
        printf("%s\n", entry->d_name);
        entry = readdir(d);
    }

    closedir(d);

    return 0;
}