# We construct a separate target for the flex output file lex.yy.c, so that
# it can be compiled separately. Then in "all" you will combine all other
# code you might have into a single final executable.

all: stack structs usage parser lex.yy.c
	gcc -DEXT_PROMPT stack.o structs.o usage.o parser.tab.c lex.yy.c -o shell -lfl

parser: parser.y
	bison -o parser.tab.c -d parser.y

lex.yy.c: shell.l
	flex -o lex.yy.c shell.l

stack: stack.c stack.h
	gcc -c stack.c

structs: structs.c structs.h
	gcc -c structs.c -DEXT_PROMPT

usage: usage.c usage.h
	gcc -c usage.c -DEXT_PROMPT

clean:
	rm -f lex.yy.c
	rm -f parser.tab.c
	rm -f parser.tab.h
	rm -f stack.o
	rm -f structs.o
	rm -f usage.o
	rm -f shell
