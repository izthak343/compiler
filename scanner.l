/*Lex-File*/

%{
    #include "y.tab.h"
%}


%%    
	/*Types*/
boolean  		return BOOL;
char     		return CHAR;
void     		return VOID;
int      		return INT;
string   		return STRING;
intp     		return INTP;
charp    		return CHARP;
NULL     		return NULLL;
main			return MAIN;

    	/*Questions*/
if       		return IF;
else     		return ELSE;

    	/*Loops*/
while    		return WHILE;
do       		return DO;
for    	 		return FOR;

    	/*Operators*/
[&][&]   		return AND;
[/]    	 		return DIV;
[*]    	 		return MUL;
[=]    	 		return PLACEMENT;
[=][=]   		return EQUAL;
[>]    	 		return BIGGER_THEN;
[>][=]   		return BIGGER_EQUAL_THEN;
[<]      		return SMALLER_THEN;
[<][=]   		return SMALLER_EQUAL_THEN;
"^"      		return POINTER; 
[-]      		return MINUS;
[+]      		return PLUS;
[!]      		return NOT;
[!][=]   		return NOT_EQUAL;
[|][|]   		return OR;
[&]      		return ADDRESS;

    	/*Values*/
return                  return RETURN;
true                  	return TRUEE;
false                 	return FALSEE;
['][ -!#-~][']             {yylval.string= strdup(yytext);return CHARR;}   
0|[1-9][0-9]*           {yylval.string= strdup(yytext);
			 return INTEGER_NUMBER;}
0(x|X)[A-F0-9][A-F0-9]* {yylval.string= strdup(yytext);
			 return HEX_NUMBER;}
0[0-7]*                 {yylval.string= strdup(yytext);
			 return OCT_NUMBER;}
(0|1)(0|1)*b            {yylval.string= strdup(yytext);
			 return BIN_NUMBER;}
[a-zA-Z][a-zA-Z0-9_]*   {yylval.string= strdup(yytext);
			 return IDENTIFIER;} //Note that 'int _;' is valid althought its not.

["][ -!#-~]*["] {yylval.string= strdup(yytext);
			    return T_STRING;}
	/*Detecting Comments*/
"//".*                                    { /* DO NOTHING */ ;}
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { /* DO NOTHING */ ;}


	/*Others*/
[;] 			return SEMICOLON;
[:] 			return COLON;
[,] 			return COMMA;
[{] 			return OPEN_BLOCK;
[}] 			return CLOSE_BLOCK;
[(] 			return OPEN_PAREN;
[)] 			return CLOSE_PAREN;
[|] 			return LENGTH;
"[" 			return OPEN_ARRAY;
"]" 			return CLOSE_ARRAY;

	/*Skip Whitespace, Enter, Tab*/
[\n]			{yylineno++;} 
[ \t]			;//Do nothing if: space/tab/enter
.			return yytext[0]; //Ignore others
%%

