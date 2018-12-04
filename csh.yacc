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
command_t pipe_command;

size_t i = 1;
int is_pipe = FALSE;

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

%token NUMBER WORD NEWLINE SEMICOLON VARIABLE
%token EQUALS GREAT LESS GREAT_GREAT GREAT_AMP
%token REDIRECT PIPE

%union {
    char * str;
    char * sign;
    int number;
    command_t cmd;
}

%type <number> NUMBER
%type <sign> EQUALS PIPE
%type <str> WORD VARIABLE NEWLINE SEMICOLON
%type <str> GREAT LESS GREAT_GREAT GREAT_AMP REDIRECTS REDIRECTION
%type <str> ARG
%type <cmd> PIPE_LIST
%type <cmd> CMD_ARGS COMMAND COMMANDS EXPR COMMAND_LIST

%%

// RULES
//------------------------------------------------------------------------------------

COMMAND_LINE:
    EXPR COMMAND_LINE
    | /*nothing*/

EXPR:
     COMMAND_LIST PIPE_LIST NEWLINE
    | COMMAND_LIST
    | PIPE_LIST
    | NEWLINE


PIPE_LIST:
    PIPE_LIST PIPE CMD_ARGS
    | CMD_ARGS PIPE CMD_ARGS {
       printf("cmd1: %s; cmd2: %s \n", $1.current_command,
              $3.current_command);
       pipe_($1, $3);
    }

COMMAND_LIST:
    COMMANDS{
        exec_(command, command.args);
        i = 1;
    }

COMMANDS:
    CMD_ARGS REDIRECTION


REDIRECTION:
    REDIRECTS ARG{
        if (strcmp($1, "<") == 0) {
            command.redirect.less = $1;
            command.redirect.from = $2;
        }
        else{
            command.redirect.great = $1;
            command.redirect.to = $2;
        }
        command.redirect.redirect = TRUE;
    }
    |REDIRECTION REDIRECTS ARG{
        if (strcmp($2, "<") == 0) {
            command.redirect.less = $2;
            command.redirect.from = $3;
        }
        else{
            command.redirect.great = $2;
            command.redirect.to = $3;
        }
        command.redirect.redirect = TRUE;
    }
    |/*nothing*/
/*    |NUMBER GREAT_AMP ARG{
        command.redirect.from = (char *)$1;
        command.redirect.great = $2;
        command.redirect.to = $3;
    }
    |NUMBER GREAT_AMP NUMBER{
         command.redirect.from = (char *)$1;
         command.redirect.great = $2;
         command.redirect.to = (char *)$3;
    } */

REDIRECTS:
    GREAT
    |LESS
    |GREAT_GREAT
    |GREAT_AMP

CMD_ARGS:
    COMMAND ARGS{
        $$ = command;
    }


COMMAND:
    WORD {
        command.current_command = $1;
        command.args[0] = command.current_command;
        command.redirect.great = NULL;
        command.redirect.less = NULL;
        command.redirect.from = NULL;
        command.redirect.to = NULL;
        command.redirect.redirect = FALSE;
        for (int k = 1; k < ARGS_SIZE; k++)
             command.args[k] = NULL;
    }
    | WORD EQUALS WORD{
       add_variable($1, $3);
       command.current_command = $2;
       command.args[0] = command.current_command;
    }

ARGS:
    ARGS ARG {
        if (i < ARGS_SIZE ) {
            if ($2) {
                command.args[i] = $2;
                i++;
            }
        }
        else {
            print_msg(2, "Too many args.\n");
        }
    }
    | ARG {
        if ($1) {
            command.args[i] = $1;
            i++;
        }
    }
    | /*nothing*/


ARG:
    WORD
    | VARIABLE {
        $$ = substitute_variable($1);
    }

