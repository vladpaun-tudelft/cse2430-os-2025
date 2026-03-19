gcc -O0 -fno-stack-protector -std=gnu99 -c assignment.c -o assignment.o
gcc assignment.o -T linker.ld -o assignment
