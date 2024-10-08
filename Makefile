# We construct a separate target for the flex output file lex.yy.c, so that
# it can be compiled separately. Then in "all" you will combine all other
# code you might have into a single final executable.

all: stack list structs usage parser lex.yy.c
	gcc stack.o list.o structs.o usage.o parser.tab.c lex.yy.c -o shell -lfl

parser: parser.y
	bison -o parser.tab.c -d parser.y

lex.yy.c: shell.l
	flex -o lex.yy.c shell.l

stack: stack.c stack.h
	gcc -c stack.c

list: list.c list.h
	gcc -c list.c

structs: structs.c structs.h
	gcc -c structs.c

usage: usage.c usage.h
	gcc -c usage.c

clean:
	rm -f lex.yy.c
	rm -f parser.tab.c
	rm -f parser.tab.h
	rm -f stack.o
	rm -f list.o
	rm -f structs.o
	rm -f usage.o
	rm -f shell
