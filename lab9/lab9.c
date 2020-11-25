#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int main(){
    pid_t pid;
    int x;
    x = 0;
    x++;
    pid = fork();
    if(pid == 0){
        x++;
        printf("child PID : %ld, x : %d\n", getpid(), x);
        return 0;
    }
    printf("parent PID : %ld, x : %d\n", getpid(), x);
    return 0;
}

int main(int argc, char *argv[]){
    char* execve_argv[] = {"ls", "-al"};
    char* execve_environ[] = {"0"};

    execve("/bin/ls", execve_argv, execve_environ);
    return 0;
}
int main(int argc, char *argv[]){
    if(argc>2){
        kill(atoi(argv[1]), atoi(argv[2]));
    }
    return 0;
}
