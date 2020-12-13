#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void main(){
    char c[3] = {'1', '2', '3'};
    int r, r2;
    r = open("three.txt", O_RDONLY);
    r2 = open("three.txt", O_RDWR);
    dup2(1,2);
    dup2(r2,1);
    read(r, &c[0], 1);
    printf("LION");
    fflush(stdout);
    read(r, &c[1], 1);
    read(r2, &c[2], 1);
    printf("%c%c%c", c[0], c[1], c[2]);
}