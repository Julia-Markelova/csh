//
// Created by yulia on 21.11.18.
//

#ifndef YULIASH_UTILS_H
#define YULIASH_UTILS_H

#define ARGS_SIZE 32

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

typedef struct key_value{
    char * key;
    char * value;
} pairs_t;

pairs_t variables[ARGS_SIZE];

void print_msg(int fd, char * msg);
void open_dir(char * dir_name);
int redirect(char* from, char * to, int append);
int dup2_(int in, int out);
void exec_(command_t command, char * args[]);
void cd(char * dir);
void set_(char *name);
void fork_exec(command_t command, char * args[]);
void slice_str(const char * str, char * buffer, size_t start, size_t end);
char * find_local_variable(char * name);
#endif //YULIASH_UTILS_H
