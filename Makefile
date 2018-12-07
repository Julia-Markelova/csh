FLAGS=-Wall -Wextra --std=gnu99 -pedantic -Werror -c
CC=gcc

all: csh

csh: lex.yy.o y.tab.o utils.o
	$(CC) -o my_csh lex.yy.o y.tab.o utils.o

lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c: y.tab.o

y.tab.o: y.tab.c
	$(CC) -c y.tab.c

utils.o: utils.c
	$(CC) $(FLAGS) utils.c

lex.yy.c: csh.lex
	lex csh.lex

y.tab.c: csh.yacc
	yacc -d csh.yacc

clean:
	rm lex*
	rm y*
	rm *.o