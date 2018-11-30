%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include "utils.h"

//DECLARE
//----------------------------------------------------------------------------------
int yyparse();
int yylex();

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

