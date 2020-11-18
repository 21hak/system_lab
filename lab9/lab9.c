#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

// int main(){
//     pid_t pid;
//     int x;
//     x = 0;
//     x++;
//     pid = fork();
//     if(pid == 0){
//         x++;
//         printf("child PID : %ld, x : %d\n", getpid(), x);
//         return 0;
//     }
//     printf("parent PID : %ld, x : %d\n", getpid(), x);
//     return 0;
// }

int main(int argc, char *argv[]){
    // char* execve_argv[] = {"bin/ls", '-al'};
    char* execve_environ[] = {"0"};

    execve("bin/ls", "-al", execve_environ);
    return 0;
}
