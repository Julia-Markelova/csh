FLAGS=-Wall -Wextra --std=gnu99 -pedantic -Werror -c

all: csh

csh: lex.yy.o y.tab.o utils.o
	gcc -o my_csh lex.yy.o y.tab.o utils.o

lex.yy.o: lex.yy.c
	cc -c lex.yy.c

y.tab.o: y.tab.c
	cc -c y.tab.c

utils.o: utils.c
	cc $(FLAGS) utils.c

lex.yy.c: csh.lex
	lex csh.lex

y.tab.c: csh.yacc
	yacc -d csh.yacc