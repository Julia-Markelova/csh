%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include "utils.h"

//DECLARE
//----------------------------------------------------------------------------------
//typedef struct yy_buffer_state * YY_BUFFER_STATE;
//extern YY_BUFFER_STATE yy_scan_string(char * str);
//extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
int yyparse();
int yylex();
void parse(char * string);
//----------------------------------------------------------------------------------

// VARIABLES
//----------------------------------------------------------------------------------

command_t command;
command_t pipe_command;
command_t commands[ARGS_SIZE];
size_t c = 0;

size_t i = 1;
int is_pipe = FALSE;

//---------------------------------------------------------------------------------

void yyerror(const char *str) {
    fprintf(stderr,"ошибка: %s\n",str);
}

int yywrap() {
    return 1;
}

int main(int argc, char **argv) {
    if (argv[1] != NULL) {
        if (strcmp((char *)argv[1], "-") == 0){

            print_help();
            history_stack.pointer = -1;

            while (TRUE){
                char * prompt = concat("\x1b[32;1m", getenv("USER"));
                prompt = concat(prompt, "\033[0m");
                prompt = concat(prompt, ":");
                prompt = concat(prompt, "\x1b[34;1m");
                prompt = concat(prompt, getenv("PWD"));
                prompt = concat(prompt, "\033[0m");
                prompt = concat(prompt, "> ");
                print_msg(1, prompt);
                char * string = malloc(sizeof(char) * 128);

                if (string) {
                    signal(SIGINT, sig_handler);

                    if(read(0, string, 128) < 0)
                         perror("read syscall");

                    char * hist = check_history(string);
                    if (hist != NULL) {
                        if (strcmp(hist, string) != 0) {
                            print_msg(1, hist);
                            string = hist;
                        }
                        else
                            push(string);
                    }
                    else {
                        print_msg(2, "No prev command.\n");
                        string = "\n";
                    }

                    //YY_BUFFER_STATE buffer = yy_scan_string(string);
                    //yyparse();
                    //yy_delete_buffer(buffer);
					parse(string);
                }
                else {
                    perror("memory");
                }
            }
        }

        else {
            char * string;
            string = concat(argv[1], " ");
            for (int i = 2; i < argc; i++) {
                string = concat(string, argv[i]);
                string = concat(string, " ");
            }
            //YY_BUFFER_STATE buffer = yy_scan_string(string);
           // yyparse();
           // yy_delete_buffer(buffer);
			parse(string);
            return 0;

        }
    } else{
        print_msg(1, "No arguments passed.\n");
        exit(0);
    }
}

%}

/* TOKENS
----------------------------------------------------------------------------------*/

%token NUMBER WORD NEWLINE VARIABLE
%token EQUALS GREAT LESS GREAT_GREAT
%token REDIRECT PIPE HISTORY

%union {
    char * str;
    char * sign;
    int number;
    command_t cmd;
}

%type <number> NUMBER
%type <sign> EQUALS PIPE HISTORY
%type <str> WORD VARIABLE NEWLINE
%type <str> GREAT LESS GREAT_GREAT REDIRECTS 
%type <str> ARG 
%type <cmd> PIPE_LIST PIPES
%type <cmd> CMD_ARGS COMMAND COMMANDS COMMAND_LIST



%%

/* RULES
------------------------------------------------------------------------------------*/

COMMAND_LINE:
    EXPR COMMAND_LINE
    | /*nothing*/

EXPR:
     COMMAND_LIST PIPES
    | COMMAND_LIST
    | PIPES {
        pipeline(commands);
        for (int k = 0; k < ARGS_SIZE; k++)
            commands[k].current_command = NULL;
        c = 0; //important!
    }
    | COMMAND_LIST PIPES COMMAND_LIST NEWLINE
    | PIPES COMMAND_LIST
    | NEWLINE 

PIPES:
    PIPE_LIST REDIRECTION 

PIPE_LIST:
     PIPE_LIST PIPE CMD_ARGS {
        commands[c] = $3;
        c++;
        is_pipe = TRUE;
    }
    | CMD_ARGS  {
        if (c < ARGS_SIZE) {
            commands[c] = $1;
            c++;

        }
        else
            print_msg(2, "Too many pipes.\n");
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
        if (is_pipe)
            commands[c] = command;
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


REDIRECTS:
    GREAT
    |LESS
    |GREAT_GREAT

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
        i = 1;
    }
    | WORD EQUALS WORD{
       add_variable($1, $3);
       command.current_command = $2;
       command.args[0] = command.current_command;
    }
    | NUMBER{
        char str[12];
        sprintf(str, "%d", $1);
        command.current_command = str;
        command.args[0] = command.current_command;
        command.redirect.great = NULL;
        command.redirect.less = NULL;
        command.redirect.from = NULL;
        command.redirect.to = NULL;
        command.redirect.redirect = FALSE;
        for (int k = 1; k < ARGS_SIZE; k++)
            command.args[k] = NULL;
        i = 1;
    }
	| VARIABLE {
		command.current_command = substitute_variable($1);;
        command.args[0] = command.current_command;
        command.redirect.great = NULL;
        command.redirect.less = NULL;
        command.redirect.from = NULL;
        command.redirect.to = NULL;
        command.redirect.redirect = FALSE;
        for (int k = 1; k < ARGS_SIZE; k++)
            command.args[k] = NULL;
        i = 1;

	}
    | HISTORY {
        command.current_command = $1;
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
    |/* nothing*/


ARG:
    WORD
    | VARIABLE {
        $$ = substitute_variable($1);
    }
    | NUMBER {
        char str[12];
        sprintf(str, "%d", $1);
        $$ = str;
    }
