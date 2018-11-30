%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

//DECLARE
//----------------------------------------------------------------------------------
int yyparse();
int yylex();
char * get_current_dir_name();
pid_t waitpid(pid_t pid, int *status, int options);

//----------------------------------------------------------------------------------

// VARIABLES
//----------------------------------------------------------------------------------
char * current_command;
char * args[32];
size_t i = 1;

//---------------------------------------------------------------------------------

void yyerror(const char *str) {
    fprintf(stderr,"ошибка: %s\n",str);
}

int yywrap() {
    return 1;
}

int main() {
        yyparse();
        return 0;
}

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

%}

// TOKENS
//----------------------------------------------------------------------------------

%token NUMBER WORD NEWLINE SEMICOLON
%token EQUALS GREAT LESS GREAT_GREAT GREAT_AMP
%token LS CD EXIT SET PWD ECHO

%token REDIRECT PIPE

%union {
    char * str;
}

%type <str> WORD EQUALS NEWLINE SEMICOLON
%type <str> GREAT LESS GREAT_GREAT GREAT_AMP
%type <str> COMMAND COMMANDS EXPR REDIRECTION REDIRECTS
%type <str> PIPE_LIST CMD_ARGS


%%

// RULES
//------------------------------------------------------------------------------------

COMMAND_LINE:
    EXPR COMMAND_LINE
    | /*nothing*/

EXPR:
    PIPE_LIST COMMANDS NEWLINE {
        printf("%s (%s %s)  \n", "commands pipe commands", $1,  $3);
    }
    |COMMANDS {
        printf("commands\n");
    }
    | NEWLINE


PIPE_LIST:
    PIPE_LIST PIPE CMD_ARGS
    | CMD_ARGS


COMMANDS:
    CMD_ARGS REDIRECTION {
        printf("command redirs\n");
    }
    |CMD_ARGS {
        args[0] = current_command;
        exec_(current_command, args);
        i = 0;
    }


REDIRECTION:
    REDIRECTS WORD {
        if ($2 == ">") {
            redirect(NULL, $2, 0);
        }
        else if ($2 == ">>") {
            redirect(NULL, $2, 1);
        }
        else if ($2 == "<") {
            redirect($2, NULL, 0);
        }
        else if (($2 == ">&") || ($2 == "&>")) {

        }
    }


REDIRECTS:
    GREAT
    | LESS
    | GREAT_GREAT
    | GREAT_AMP


CMD_ARGS:
    COMMAND ARGS {
        printf("cmd_args\n");
    }

COMMAND:
    WORD {
        printf("command\n");
        current_command = $1;
    }


ARGS:
    ARGS WORD {
        printf("args + word\n");
        if (i < 32 ) {
            args[i] = $2;
            i++;
        }
        else {
            print_msg(2, "Too many args.\n");
        }
    }
    | WORD {
        printf("word\n");
        args[i] = $1;
        i++;
    }
    | /*nothing*/

