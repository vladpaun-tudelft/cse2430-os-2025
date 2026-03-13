#include <stdio.h>

void vulnerable() __attribute__ ((section (".secret")));

void vulnerable() {
    printf("I have been exploited!\n");
}

int main() {
    setbuf(stdout, 0);
    setbuf(stdin, 0);
    char buf[32];
    printf("Please tell me your student number: ");
    gets(buf);
    printf("Your student number is %s\n", buf);

}
