#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(){
    pid_t pid;
    int x;
    x = 0;
    x++;
    pid = fork();
    printf("%d\n", pid);
    if(pid == 0){
        x++;
        printf("child PID : %ld, x : %d\n", getpid(), x);
        return 0;
    }
    printf("parent PID : %ld, x : %d\n", getpid(), x);
    return 0;
}