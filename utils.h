//
// Created by yulia on 21.11.18.
//

#ifndef YULIASH_UTILS_H
#define YULIASH_UTILS_H
void print_msg(int fd, char * msg);
void open_dir(char * dir_name);
int redirect(char* from, char * to, int append);
int dup2_(int in, int out);
void exec_(char * command, char * args[]);
#endif //YULIASH_UTILS_H
