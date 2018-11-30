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

command_t command;

char * args[ARGS_SIZE];
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
%type <str> GREAT LESS GREAT_GREAT GREAT_AMP REDIRECTS
%type <str> COMMAND COMMANDS EXPR
%type <str> PIPE_LIST CMD_ARGS


%%

// RULES
//------------------------------------------------------------------------------------

COMMAND_LINE:
    EXPR COMMAND_LINE
    | /*nothing*/

EXPR:
    PIPE_LIST COMMANDS NEWLINE
    |COMMANDS
    | NEWLINE


PIPE_LIST:
    PIPE_LIST PIPE CMD_ARGS
    | CMD_ARGS


COMMANDS:
    CMD_ARGS REDIRECTION {
        args[0] = command.current_command;
        exec_(command, args);
        i = 1;
        for (int k = 0; k < ARGS_SIZE; k++)
            args[k] = NULL;
    }


REDIRECTION:
    REDIRECTS WORD{
        command.redirect.redirect = $1;
        command.redirect.to = $2;
    }
    |/*nothing*/

REDIRECTS:
    GREAT
    |LESS
    |GREAT_GREAT
    |GREAT_AMP

CMD_ARGS:
    COMMAND ARGS

COMMAND:
    WORD {
        command.current_command = $1;
        command.redirect.redirect = NULL;
    }


ARGS:
    ARGS WORD {
        if (i < ARGS_SIZE ) {
            args[i] = $2;
            i++;
        }
        else {
            print_msg(2, "Too many args.\n");
        }
    }
    | WORD {
        args[i] = $1;
        i++;
    }
    | /*nothing*/

