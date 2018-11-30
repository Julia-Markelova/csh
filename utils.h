//
// Created by yulia on 21.11.18.
//

#ifndef YULIASH_UTILS_H
#define YULIASH_UTILS_H

typedef struct current_redirect{
    char * from;
    char * to;
    char * redirect;
} redirect_t;

typedef struct current_command {
    char * current_command;
    redirect_t redirect;
    int pid;
} command_t;

void print_msg(int fd, char * msg);
void open_dir(char * dir_name);
int redirect(char* from, char * to, int append);
int dup2_(int in, int out);
void exec_(command_t command, char * args[]);

#define ARGS_SIZE 32
#endif //YULIASH_UTILS_H
