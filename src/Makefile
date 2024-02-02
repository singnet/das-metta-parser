all: das_metta_parser

metta.tab.c metta.tab.h: metta.y
	bison -t -v -d metta.y

lex.yy.c: metta.l metta.tab.h
	flex metta.l

das_metta_parser: lex.yy.c metta.tab.c metta.tab.h
	gcc -o das_metta_parser metta.tab.c lex.yy.c
	mv das_metta_parser ../bin

clean:
	rm metta.tab.c lex.yy.c metta.tab.h metta.output
