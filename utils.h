//
// Created by yulia on 21.11.18.
//

#ifndef YULIASH_UTILS_H
#define YULIASH_UTILS_H

#define ARGS_SIZE 32
#define TRUE 1
#define FALSE 0
#define PIPE_RD 0
#define PIPE_WR 1

typedef struct current_redirect{
    char * from;
    char * to;
    char * great;
    char * less;
    int redirect;
} redirect_t;

typedef struct current_command {
    char * current_command;
    redirect_t redirect;
    char * args[ARGS_SIZE];
} command_t;

typedef struct key_value{
    char * key;
    char * value;
} pairs_t;

pairs_t variables[ARGS_SIZE];

void print_msg(int fd, char * msg);
void open_dir(char * dir_name);
int redirect(char* from, char * to, int append);
void exec_(command_t command, char * args[]);
int pipe_(command_t command, command_t command2);
void cd(char * dir);
void set_(char *name);
int fork_exec(command_t command, char * args[]);
void slice_str(const char * str, char * buffer, size_t start, size_t end);
char * find_local_variable(char * name);
char * substitute_variable(char * arg );
void add_variable(char * key, char * value);
#endif //YULIASH_UTILS_H
