lex scanner.l
yacc -d parser.y
gcc y.tab.c -o csimple -Ly
