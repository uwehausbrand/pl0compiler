compiler: parser

else
parser: parse.c sem.c list.c lexframe.c codegen.c
	gcc -g -Wall parse.c sem.c list.c lexframe.c codegen.c -o parse

lexer: lexframe.c
	gcc -D _LEXER_ -g -Wall lexframe.c -o lexer

clean:
	rm -f parse
	rm -f lexer
	rm -f *~
