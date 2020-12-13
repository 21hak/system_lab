#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void main(){
    char c0 = '4', c1='5', c2='6';
    char scrap[4];
    int pid, r, r2 = open("two.txt", O_RDONLY);
    r = dup(r2);
    if(!(pid=fork())){
        read(r, &c0, 1);
        close(r2);
        r2 = open("two.txt", O_RDONLY);
        read(r2, &scrap, 4);
    } else{
        waitpid(pid, NULL, 0);
        read(r, &c1, 1);
        read(r2, &c2, 1);
    }
    printf("%c%c%c", c0, c1, c2);
}