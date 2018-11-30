//
// Created by yulia on 21.11.18.
//

#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <wait.h>


void print_msg(int fd, char * msg){
    if (write(fd, msg, strlen(msg)) < 0  )
        perror("write");
}

void exec_(char * command, char * args[]){
    pid_t parent = getpid();
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork:");
    }
    else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
    else {
        // we are the child
        execvp(command, args);
        perror(command);
        _exit(EXIT_FAILURE);   // exec never returns
    }
}


int dup2_(int in, int out){
    if (dup2(in, 0) < 0) {
        perror("redirect");
        return -1;
    }
    if (dup2(out, 1) < 0) {
        perror("redirect");
        return -1;
    }
    return 0;
}

int redirect(char* from, char * to, int append){
    int in;
    int out;

    // check if there any file as stdout
    if (from)
        in = open(from, O_RDONLY);
    else
        in = 1;

    // check if there any file as stdin
    if (to) {
        // check for append flag
        if (append == 0)
            out = open(to, O_WRONLY | O_TRUNC | O_CREAT);
        else
            out = open(to, O_WRONLY | O_APPEND | O_CREAT);
    }
    else
        out = 0;

    if (in < 0 || out < 0){
        perror("open");
        return -1;
    }

    else {
        int ret = dup2_(in, out);
        close(in);
        close(out);
        return ret;
    }
}


