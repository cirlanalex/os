# We construct a separate target for the flex output file lex.yy.c, so that
# it can be compiled separately. Then in "all" you will combine all other
# code you might have into a single final executable.

all: structs usage parser lex.yy.c
	gcc structs.o usage.o parser.tab.c lex.yy.c -o shell -lfl

parser: parser.y
	bison -o parser.tab.c -d parser.y

lex.yy.c: shell.l
	flex -o lex.yy.c shell.l

structs: structs.c structs.h
	gcc -c structs.c

usage: usage.c usage.h
	gcc -c usage.c

clean:
	rm -f lex.yy.c
	rm -f parser.tab.c
	rm -f parser.tab.h
	rm -f structs.o
	rm -f usage.o
	rm -f shell
