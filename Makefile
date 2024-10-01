CC = gcc
CFLAGS = -Wall -g

all: compiler

compiler: lex.yy.c parser.tab.c symbol_table.o AST.o semantic_analyzer.o optimizer.o code_generator.o
	$(CC) $(CFLAGS) -o $@ $^ -lfl

symbol_table.o: symbol_table.c symbol_table.h
	$(CC) $(CFLAGS) -c symbol_table.c

AST.o: AST.c AST.h
	$(CC) $(CFLAGS) -c AST.c

semantic_analyzer.o: semantic_analyzer.c semantic_analyzer.h
	$(CC) $(CFLAGS) -c semantic_analyzer.c

optimizer.o: optimizer.c optimizer.h
	$(CC) $(CFLAGS) -c optimizer.c

code_generator.o: code_generator.c code_generator.h
	$(CC) $(CFLAGS) -c code_generator.c

lex.yy.c: lexer.l
	flex $<

parser.tab.c parser.tab.h: parser.y
	bison -d $<

clean:
	rm -f compiler lex.yy.c parser.tab.c parser.tab.h symbol_table.o AST.o semantic_analyzer.o optimizer.o output.tac optimized.tac code_generator.o output.asm

.PHONY: all clean
