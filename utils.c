//
// Created by yulia on 21.11.18.
//

#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "utils.h"

//TODO: >& unset? pipe ctrl+c


void print_msg(int fd, char *msg) {
    if (write(fd, msg, strlen(msg)) < 0)
        perror("write");
}

int pipeline(command_t commands[])
{
    int fd[2];
    pid_t pid;
    int fdd = 0;
    int i = 0;

    while (commands[i].current_command != NULL) {
        pipe(fd);
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(1);
        }
        else if (pid == 0) {

            if (dup2(fdd, 0) < 0){
                perror(commands[i].current_command);
                return -1;
            }

            if (commands[i+1].current_command != NULL) {
                if (dup2(fd[1], 1) < 0) {
                    perror(commands[i+1].current_command);
                    return -1;
                }
            } else {
                perror(commands[i].redirect.to);
                if (commands[i].redirect.redirect) {
                    printf("%s %d", commands[i].current_command,
                           commands[i].redirect.redirect);
                    int append = 0;
                    int return_value = 0;
                    // out redirection
                    if (commands[i].redirect.great) {
                        char *redir = commands[i].redirect.great;
                        if (strcmp(redir, ">>") == 0)
                            append = 1;
                        return_value = redirect(commands[i].redirect.from,
                                                commands[i].redirect.to, append);
                        if (return_value < 0)
                            return -1;
                    }

                    //in redirection
                    if (commands[i].redirect.less) {
                        return_value = redirect(commands[i].redirect.from,
                                                commands[i].redirect.to, 0);
                        if (return_value < 0)
                            return -1;
                    }
                }
            }
            close(fd[0]);
            execvp(commands[i].current_command, commands[i].args);
            exit(1);
        }
        else {
            wait(NULL);
            close(fd[1]);
            fdd = fd[0];
            i++;
        }
    }
}

/*
int pipe_(command_t command1, command_t command2) {
    int pipes[2];
    pid_t p1, p2;

    if (pipe(pipes) < 0) {
        perror("fork");
        return -1;
    }

    p1 = fork();
    if (p1 < 0) {
        perror("fork");
        return -1;
    }

    if (p1 == 0) {
        close(pipes[RD_END]);
        if (dup2(pipes[WR_END], STDOUT_FILENO) < 0) {
            perror(command1.current_command);
            return -1;
        }
        close(pipes[WR_END]);
        execvp(command1.current_command, command1.args);
        perror(command1.current_command);

    } else {
        p2 = fork();
        if (p2 == 0) {
            close(pipes[WR_END]);
            if (dup2(pipes[RD_END], STDIN_FILENO) < 0) {
                perror(command2.current_command);
                return -1;
            }
            close(pipes[RD_END]);

            execvp(command2.current_command, command2.args);
            perror(command2.current_command);

        }

        close(pipes[RD_END]);
        close(pipes[WR_END]);
    }

    close(pipes[RD_END]);
    close(pipes[WR_END]);
    return 0;

}
*/

void exec_(command_t command, char *args[]) {
    char *command_name = command.current_command;

    if (strcmp(command_name, "cd") == 0) {
        cd(args[1]);
    } else if (strcmp(command_name, "export") == 0) {
        set_(args[1]);
    } else if (strcmp(command_name, "=") == 0) {
        //do nothing
    } else if (strcmp(command_name, "exit") == 0) {
        exit(0);
    } else {
        fork_exec(command, args);
    }
}

int fork_exec(command_t command, char *args[]) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork:");
        return -1;
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        // we are the child
        // check io redirection
        if (command.redirect.redirect) {

            int append = 0;
            int return_value = 0;
            // out redirection
            if (command.redirect.great) {
                char *redir = command.redirect.great;
                if (strcmp(redir, ">>") == 0)
                    append = 1;
                return_value = redirect(command.redirect.from,
                                        command.redirect.to, append);
                if (return_value < 0)
                    return -1;
            }

            //in redirection
            if (command.redirect.less) {
                return_value = redirect(command.redirect.from,
                                        command.redirect.to, 0);
            }
            if (return_value == 0) {
                execvp(command.current_command, args);
                perror(command.current_command);
                _exit(EXIT_FAILURE);   // exec never return
            } else {
                return return_value;
            }
        } else {

            if (execvp(command.current_command, args)) {
                perror(command.current_command);
                return -1;
            }
            return 0;
        }
    }
}


int redirect(char *from, char *to, int append) {
    int in;
    int out;

    printf("from %s to %s\n", from, to);

    // check if there any file as stdin
    if (from) {
        in = open(from, O_RDONLY);
        if (in < 0) {
            perror(from);
            return errno;
        } else if (dup2(in, 0) < 0) {
            perror("redirect");
            return errno;
        }
    }

    // check if there any file as stdout
    if (to) {
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

        // check for append flag
        if (append == 1)
            out = open(to, O_RDWR | O_APPEND | O_CREAT, mode);
        else
            out = open(to, O_RDWR | O_TRUNC | O_CREAT, mode);

        if (out < 0) {
            perror(to);
            return errno;
        } else if (dup2(out, 1) < 0) {
            perror("redirect");
            return errno;
        }
    }

    if (!to && !from) {
        errno = EIO;
        perror("csh");
        return errno;
    }
    return 0;
}

void cd(char *dir) {
    if (!dir) {
        dir = getenv("HOME");
    }
    if (chdir(dir) != 0)
        perror(dir);

    char cwd[128];
    if (getcwd(cwd, sizeof(cwd)) < 0)
        perror("set pwd");
    else
        setenv("PWD", cwd, 1);
}

void set_(char *name) {
    for (size_t i = 0; i < ARGS_SIZE; i++) {
        if (variables[i].key && strcmp(variables[i].key, name) == 0) {
            if (setenv(variables[i].key, variables[i].value, 1) != 0) {
                perror("export var");
            }
        }
    }
}

char *substitute_variable(char *arg) {
    const size_t len = strlen(arg);
    char buffer[len + 1];
    slice_str(arg, buffer, 1, len);
    arg = getenv(buffer);

    if (!arg)
        arg = find_local_variable(buffer);

    return arg;
}

void slice_str(const char *str, char *buffer, size_t start, size_t end) {
    size_t j = 0;
    for (size_t i = start; i <= end; ++i) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

char *find_local_variable(char *name) {
    for (size_t i = 0; i < ARGS_SIZE; i++) {
        if (variables[i].key && strcmp(variables[i].key, name) == 0)
            return variables[i].value;
    }
    return NULL;
}

void add_variable(char *key, char *value) {
    int i = 0;
    while (i < ARGS_SIZE && variables[i].key && strcmp(variables[i].key, key) != 0) {
        i++;
    }
    variables[i].key = key;
    variables[i].value = value;
}


char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1);

    if (result) {
        strcpy(result, s1);
        strcat(result, s2);
        return result;
    } else {
        perror("malloc");
        return NULL;
    }
}
