//
// Created by yulia on 21.11.18.
//

#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "utils.h"

void print_msg(int fd, char *msg) {
    if (write(fd, msg, strlen(msg)) < 0)
        perror("write");
}

void print_help() {
    print_msg(1, "\x1b[35;5m -Builtin commands:\n");
    print_msg(1, " > \x1b[4mcd\x1b[24m, \x1b[4mexport\x1b[24m, \x1b[4mexit\x1b[24m, ");
    print_msg(1, "\x1b[4mhelp\x1b[24m, \x1b[4mhistory\x1b[24m\n" );
    print_msg(1, "-Set variables:\n");
    print_msg(1, " \x1b[1mexample:\x1b[0m\n \x1b[35;4m> a=b\x1b[24m\n ");
    print_msg(1, "\x1b[35;4m> export a\x1b[24m\n \x1b[35;4m> echo $a\x1b[24m\n ");
    print_msg(1, "\x1b[4m> b\x1b[24m\n");
    print_msg(1, "-Pipe support\n");
    print_msg(1, "-IO redirection only with file names\n");
    print_msg(1, "-Print errors\n");
    print_msg(1, "-Save 5 recent commands, ");
    print_msg(1, "to see print '\x1b[4mhistory\x1b[24m' or \x1b[4m!n\x1b[24m, where n [1;5].\n");
    print_msg(1, "If want to see it again print '\x1b[4mhelp\x1b[24m'\x1b[0m \n");
}

int pipeline(command_t commands[]) {
    int fd[2];
    pid_t pid;
    int fdd = 0;
    int i = 0;

    if (commands[1].current_command == NULL) {
        exec_(commands[0], commands[0].args);
    } else {
        while (commands[i].current_command != NULL) {
            pipe(fd);
            if ((pid = fork()) == -1) {
                perror("fork");
                exit(1);
            } else if (pid == 0) {

                if (dup2(fdd, 0) < 0) {
                    perror(commands[i].current_command);
                    return -1;
                }

                if (commands[i + 1].current_command != NULL) {
                    if (dup2(fd[1], 1) < 0) {
                        perror(commands[i + 1].current_command);
                        return -1;
                    }
                } else {
                    if (commands[i].redirect.redirect) {
                        int return_val = do_redirect_stuff(commands[i]);
                        if (return_val < 0) {
                            perror("redirect");
                            return -1;
                        }
                    }
                }
                close(fd[0]);
                execvp(commands[i].current_command, commands[i].args);
                exit(1);
            } else {
                wait(NULL);
                close(fd[1]);
                fdd = fd[0];
                i++;
            }
        }
    }
    return 0;
}


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
    } else if (strcmp(command_name, "help") == 0) {
        print_help();
    } else if (strcmp(command_name, "history") == 0) {
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (history_stack.prev_cmd[i])
                print_msg(1, history_stack.prev_cmd[i]);
        }
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
            int return_value = do_redirect_stuff(command);

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
                exit(EXIT_FAILURE);
            }
            return 0;
        }
    }
    return 0;
}

int do_redirect_stuff(command_t command) {
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
    return return_value;
}


int redirect(char *from, char *to, int append) {
    int in;
    int out;

    //printf("from %s to %s\n", from, to);

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
        mode_t mode = 0666;

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
    if (!getcwd(cwd, sizeof(cwd)))
        perror("set pwd");
    else
        setenv("PWD", cwd, 1);
}

void set_(char *name) {
	if (!name){
		print_msg(2,"No arg passed.\n");
		return;
	}
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

void sig_handler() {
    //just catch signal
}


void push(char *cmd) {
    trim(cmd);
    if (strcmp(cmd, "\n") == 0)
        return;
    if (history_stack.pointer == HISTORY_SIZE - 1) {
        history_stack.pointer = 0;
        history_stack.prev_cmd[history_stack.pointer] = cmd;
        array_rotate_left(history_stack.prev_cmd, 5);
    } else {
        history_stack.pointer++;
        history_stack.prev_cmd[history_stack.pointer] = cmd;
    }
}

char *pop(int index) {
    return history_stack.prev_cmd[index];
}

char *check_history(char *string) {
    if (strcmp(string, "!\n") == 0 || strcmp(string, "!1\n") == 0)
        return pop(history_stack.pointer);
    if (strcmp(string, "!2\n") == 0)
        return pop(history_stack.pointer - 1 > -1 ? history_stack.pointer - 1 : 4);
    if (strcmp(string, "!3\n") == 0)
        return pop(history_stack.pointer - 2 > -1 ? history_stack.pointer - 2 : 4);
    if (strcmp(string, "!4\n") == 0)
        return pop(history_stack.pointer - 3 > -1 ? history_stack.pointer - 3 : 4);
    if (strcmp(string, "!5\n") == 0)
        return pop(history_stack.pointer - 4 > -1 ? history_stack.pointer - 4 : 4);
    else return string;
}


void array_rotate_left(char **array, int size) {
    register int i;
    char *temp = array[0];
    for (i = 0, size--; i < size; i++) array[i] = array[i + 1];
    array[size] = temp;
}


void trim(char *s) {
    // удаляем пробелы и табы с начала строки:
    size_t i = 0, j;
    while ((s[i] == ' ') || (s[i] == '\t')) {
        i++;
    }
    if (i > 0) {
        for (j = 0; j < strlen(s); j++) {
            s[j] = s[j + i];
        }
        s[j] = '\0';
    }

    // удаляем пробелы и табы с конца строки:
    i = strlen(s) - 1;
    while ((s[i] == ' ') || (s[i] == '\t')) {
        i--;
    }
    if (i < (strlen(s) - 1)) {
        s[i + 1] = '\0';
    }
}
