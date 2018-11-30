//
// Created by yulia on 21.11.18.
//

#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <wait.h>
#include "utils.h"


void print_msg(int fd, char * msg){
    if (write(fd, msg, strlen(msg)) < 0  )
        perror("write");
}

void exec_(command_t command, char * args[]){
    printf("%s \n", command.redirect.redirect);
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
        if(command.redirect.redirect != NULL){
            int append = 0;

            if (strcmp(command.redirect.redirect, ">>") == 0)
                append = 1;

            redirect(command.redirect.from, command.redirect.to, append);
        }
        execvp(command.current_command, args);
        perror(command.current_command);
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
        if (append == 1)
            out = open(to, O_WRONLY | O_APPEND | O_CREAT);
        else
            out = open(to, O_WRONLY | O_CREAT | O_CREAT);
    }
    else
        out = 0;

    if (in < 0 || out < 0){
        perror("open");
        return -1;
    }
    else {
        int ret = dup2_(in, out);
        return ret;
    }
}


