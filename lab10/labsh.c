/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS    128    /* max args on a command line */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "labsh> ";  /* command line prompt (DO NOT CHANGE) */
char sbuf[MAXLINE];         /* for composing sprintf messages */

/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);

/* Here are helper routines that we've provided for you */
char *parseline(const char *cmdline, char **argv); 
void unix_error(char *msg);
void app_error(char *msg);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}

	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
        printf ("Shell closed.\n\n");
        fflush(stdout);
        exit(1);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 
    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * Fork a child process and run the job in the context of the child. 
 * Wait for the child process to terminate and then return. 
 * If the job is requested to be redirected, you should redirect
 * the stdout of the child process to the file.
 */
void eval(char *cmdline) 
{
    int check = 0;
    char *argv[MAXARGS];
    char buf[MAXLINE];
    pid_t pid;
    int fd;
    int fd2;
    parseline(cmdline, argv);
    
    // for(int i = 0;argv[i]!='\0';i++)
    // {
    //     if(strcmp(argv[i],'>')==0)
    //     {     
    //         check = 1;
    //     }         
    // }


    if(argv[0] == NULL)
        return;
    
    pid = fork();
    if(pid == 0){
        printf("0: %s\n", argv[0]);
        printf("1: %s\n", argv[1]);
        printf("2: %s\n", argv[2]);
        // if(argv[2] == NULL){
        //     app_error("NO redirection here");
        // }
        // else {
        //     int fd = open(argv[0], O_WRONLY | O_APPEND);
        //     if(fd<0)
        //         unix_error("exec error");
        //     dup2(fd, 1);
        //     close(fd);
        //     // int fd2 = open(argv[2], O_WRONLY | O_APPEND | O_CREAT, 0644);
        //     // if(fd < 0 || fd2 < 0){
        //     //     unix_error("exec error");
        //     // }
        //     execve(argv[0], argv, environ);
        //     exit(1);
        // }
        /* Child process: stdin redirection */
        fd = open(argv[0], O_RDONLY);
        close(0);
        dup(fd);
        close(fd);
        /* Child process: stdout redirection */
        fd2 = creat(argv[2], 0644);
        close(1);
        dup(fd2);
        close(fd2);
        /* Child process: exec other program */
        execve(argv[0], argv, environ); /* Does NOT return */
    } else {
        // while (!(wait(&status) == pid))
        wait(NULL);
    }
    // return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument. Return the name of the file if the user has 
 * requested a redirection to the file.
 */
char *parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    char *rd_file;                     /* redirection? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 0;

    /* should the job run in the background? */
    rd_file = NULL;
    if (argc > 2 && (*argv[argc-2] == '>')) {
        rd_file = argv[argc-1];
        argv[argc-2] = NULL;
    }
    return rd_file;
}

/***********************
 * Other helper routines
 ***********************/

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}




