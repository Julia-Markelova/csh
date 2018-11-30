//
// Created by yulia on 21.11.18.
//

#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <wait.h>
#include <errno.h>
#include "utils.h"

//TODO: >& kill set_ alias new_process

void print_msg(int fd, char *msg) {
    if (write(fd, msg, strlen(msg)) < 0)
        perror("write");
}

void exec_(command_t command, char *args[]) {
    char *command_name = command.current_command;

    if (strcmp(command_name, "cd") == 0)
        cd(args[1]);

    else if (strcmp(command_name, "export") == 0){
        set_(args[1]);
    }

    else if (strcmp(command_name, "=") == 0){
        //do nothing
    }

    else if (strcmp(command_name, "echo") == 0 ) {
        char buf[1];
        slice_str(args[1], buf, 0, 0);
        if (strcmp(buf, "$") == 0) {
            const size_t len = strlen(args[1]);
            char buffer[len + 1];
            slice_str(args[1], buffer, 1, len);
            args[1] = getenv(buffer);
            if (!args[1])
                args[1] = find_local_variable(buffer);
            fork_exec(command, args);
        }
    }
    else {
        fork_exec(command, args);
    }
}

void fork_exec(command_t command, char *args[]) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork:");
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        // we are the child
        if (command.redirect.redirect != NULL) {
            int append = 0;
            char *redir = command.redirect.redirect;

            if (strcmp(redir, ">>") == 0)
                append = 1;

            if (strcmp(redir, "<") == 0)
                redirect(command.redirect.to, command.redirect.from, 0);
            else
                redirect(command.redirect.from, command.redirect.to, append);
        }
        execvp(command.current_command, args);
        perror(command.current_command);
        _exit(EXIT_FAILURE);   // exec never return
    }
}


int dup2_(int in, int out) {
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

int redirect(char *from, char *to, int append) {
    int in;
    int out;

    // checkcat  if there any file as stdout
    if (from)
        in = open(from, O_RDONLY);
    else
        in = 0;

    // check if there any file as stdin
    if (to) {
        mode_t mode = S_IRWXU | S_IRGRP | S_IROTH;
        // check for append flag
        if (append == 1)
            out = open(to, O_RDWR | O_APPEND | O_CREAT, mode);
        else
            out = open(to, O_RDWR | O_TRUNC | O_CREAT, mode);
    } else
        out = 1;

    if (in < 0 || out < 0) {
        perror("open");
        return -1;
    } else {
        int ret = dup2_(in, out);
        return ret;
    }
}

void cd(char *dir) {
    if (!dir) {
        dir = getenv("HOME");
    }
    if (chdir(dir) != 0)
        perror("cd");
}

void set_(char *name){
    for (size_t i = 0; i < ARGS_SIZE; i++){
        if (variables[i].key && strcmp(variables[i].key, name) == 0){
            if (setenv(variables[i].key, variables[i].value, 1) != 0) {
                perror("export var");
            }
        }
    }
}


void slice_str(const char * str, char * buffer, size_t start, size_t end) {
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

char * find_local_variable(char * name){
    for (size_t i = 0; i < ARGS_SIZE; i++) {
        if (variables[i].key && strcmp(variables[i].key, name) == 0)
            return variables[i].value;
    }
    return NULL;
}

