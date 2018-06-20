/*Yacc-File*/

%{

#include "func.h"

extern int yylex();
extern int yylineno;
extern char *yytext;

node *globalTree;

%}

%union 
{
	char *string; //terminals
	struct node *node; //leafs
}

/*Keyword Lexem*/
%token <string> BOOL, CHAR, VOID, INT, STRING, INTP, CHARP, IF, ELSE, WHILE, DO, FOR, RETURN ,NULLL, MAIN ,CHARR

/*Operator Lexem*/
%token <string> AND, OR, PLACEMENT, EQUAL, BIGGER_THEN, BIGGER_EQUAL_THEN, SMALLER_THEN, SMALLER_EQUAL_THEN, MINUS, PLUS, NOT, NOT_EQUAL, MUL, DIV, COMMA

/*Other Tokens*/
%token <string> TRUEE, CHAR, FALSEE, INTEGER_NUMBER, HEX_NUMBER, OCT_NUMBER, BIN_NUMBER, IDENTIFIER, SEMICOLON
%token <string> ADDRESS, POINTER, T_STRING, COLON, OPEN_ARRAY, CLOSE_ARRAY, OPEN_BLOCK, CLOSE_BLOCK, OPEN_PAREN, CLOSE_PAREN, LENGTH

/*All Rules*/
%type <node> ok, state, initialization, identifier, multivars, type, array, num, expr, condition, boolean, block, simpelAct, loop, for, place13, place2 ,while, do, func, parameters, multiparm ,return ,string ,sendedParameters ,numericExpr ,oneState ,oneORmore ,Gstate ,funcWmain
%type <string> relation, opr,minusPlus

%nonassoc IFX
%nonassoc ELSE

%%

ok:		Gstate		{globalTree = $1;};
Gstate: funcWmain Gstate			{$$ = makeNode("func statement", $1, $2, NULL);}		|
        /*empty*/					{$$ = NULL;}											;

funcWmain:	VOID MAIN OPEN_PAREN CLOSE_PAREN block	
{$$ = makeNode("MAIN",makeNode("MAININFO", makeNode("void", NULL, NULL, NULL), makeNode("main", NULL, NULL, NULL), NULL), $5, NULL);}|			

		type identifier OPEN_PAREN parameters CLOSE_PAREN block			
{$$ = makeNode("func", makeNode("FUNCINFO", $1, $2, $4), $6, NULL);}	;	

state:	initialization state		{$$ = makeNode("init statement", $1, $2, NULL);}		|
		condition state				{$$ = makeNode("if/else statement", $1, $2, NULL);}		|
		simpelAct SEMICOLON state	{$$ = makeNode("simple statement", $1, $3, NULL);}		|
		loop state					{$$ = makeNode("loops statement", $1, $2, NULL);}		|
		func state					{$$ = makeNode("func statement", $1, $2, NULL);}		|
		return state				{$$ = makeNode("return statement", $1, $2, NULL);}		|
		block state					{$$ = makeNode("block statement", $1, $2, NULL);}		|
		/*empty*/					{$$ = NULL;}											;
 
oneState:	simpelAct SEMICOLON		{$$ = makeNode("simple statement", $1, NULL, NULL);}	|
			loop					{$$ = makeNode("loops statement", $1, NULL, NULL);}		|
			condition				{$$ = makeNode("if/else statement", $1, NULL, NULL);}	|
			return					{$$ = makeNode("return statement", $1, NULL, NULL);}	|
			func					{$$ = makeNode("func statement", $1, NULL, NULL);}		;


initialization:	type identifier SEMICOLON					{$$ = makeNode("init", $1, $2, NULL);}	| /*Without Initialization*/
                type identifier COMMA multivars SEMICOLON	{$$ = makeNode("init", $1, $2, $4);}    ; /*Multiple Variables*/

identifier:	IDENTIFIER			{$$ = makeNode($1, NULL, NULL, NULL);}								|
			IDENTIFIER array	{$$ = makeNode("array",makeNode($1, NULL, NULL, NULL), $2, NULL);}	;

array:		OPEN_ARRAY numericExpr CLOSE_ARRAY	{$$ = makeNode("arrayLength", $2, NULL, NULL);}		;

numericExpr:	num							{$$ = makeNode("NUM", $1 , NULL, NULL);}								| 
				num opr numericExpr			{$$ = makeNode($2,makeNode("NUM", $1 , NULL, NULL), $3, NULL);}										|
				IDENTIFIER					{$$ =makeNode("identifier",makeNode($1, NULL, NULL, NULL), NULL, NULL);}|
				IDENTIFIER opr numericExpr	{$$ = makeNode($2, makeNode("identifier",makeNode($1, NULL, NULL, NULL), NULL, NULL), $3, NULL);}			;

		
multivars:	identifier					{$$ = makeNode("multivars", $1, NULL, NULL);}	|	
    		identifier COMMA multivars	{$$ = makeNode("multivars", $1, $3, NULL);}		;

type:	BOOL				{$$ = makeNode("BOOLEAN", NULL, NULL, NULL);}				| /*Boolean Type*/
		INT					{$$ = makeNode("INT", NULL, NULL, NULL);}					| /*Integer Type*/ 
		CHAR				{$$ = makeNode("CHAR", NULL, NULL, NULL);}					| /*Char Type*/
		STRING				{$$ = makeNode("STRING", NULL, NULL, NULL);}				| /*String Type*/
		INTP				{$$ = makeNode("INTP", NULL, NULL, NULL);}					| /*Int Pointer Type*/
		CHARP				{$$ = makeNode("CHARP", NULL, NULL, NULL);}					| /*Char Pointer Type*/
		VOID				{$$ = makeNode("VOID", NULL, NULL, NULL);}					; /*Void Type*/

num:	INTEGER_NUMBER		{$$ = makeNode($1, NULL, NULL, NULL);}						|
		HEX_NUMBER			{$$ = makeNode($1, NULL, NULL, NULL);}						|
		OCT_NUMBER			{$$ = makeNode($1, NULL, NULL, NULL);}						|
		BIN_NUMBER			{$$ = makeNode($1, NULL, NULL, NULL);}						;	

condition:	IF OPEN_PAREN expr CLOSE_PAREN oneORmore %prec IFX
				{$$ = makeNode("IF", $3, $5, NULL);}									| /*IF*/
	        IF OPEN_PAREN expr CLOSE_PAREN oneORmore ELSE oneORmore
				{$$ = makeNode("IF", $3, $5, makeNode("ELSE", $7, NULL, NULL));}		; /*IF-ELSE*/

oneORmore:	oneState	{$$ = $1;}	|
			block		{$$ = $1;}	;

expr:	NOT identifier														{$$ = makeNode("!", makeNode("identifier", $2, NULL, NULL), NULL, NULL);}										|
		NOT identifier relation expr										{$$ = makeNode($3,makeNode("!", makeNode("identifier", $2, NULL, NULL), NULL, NULL),$4,NULL);}										|
		identifier															{$$ = makeNode("identifier", $1, NULL, NULL);}								|
		minusPlus identifier												{$$ = makeNode($1, makeNode("identifier", $2, NULL, NULL), NULL, NULL);}	|
		minusPlus identifier opr expr										{$$ = makeNode($3,makeNode($1, makeNode("identifier", $2, NULL, NULL), NULL, NULL),$4, NULL);}	|
		minusPlus identifier relation expr									{$$ = makeNode($3,makeNode($1, makeNode("identifier", $2, NULL, NULL), NULL, NULL),$4, NULL);}	|
		identifier OPEN_PAREN sendedParameters CLOSE_PAREN					{$$ = makeNode("FuncExpr", $1, $3, NULL);}									|
		identifier OPEN_PAREN sendedParameters CLOSE_PAREN relation expr	{$$ = makeNode($5, makeNode("FuncExpr", $1, $3, NULL), $6, NULL);}			|
		identifier OPEN_PAREN sendedParameters CLOSE_PAREN opr expr			{$$ = makeNode($5, makeNode("FuncExpr", $1, $3, NULL), $6, NULL);}			|
		LENGTH identifier LENGTH											{$$ = makeNode("length/absolute",  makeNode("identifier", $2, NULL, NULL), NULL, NULL);}							|
		LENGTH identifier LENGTH relation expr								{$$ = makeNode($4,makeNode("length/absolute",  makeNode("identifier", $2, NULL, NULL), NULL, NULL),$5, NULL);}	|
		LENGTH identifier LENGTH opr expr									{$$ = makeNode($4,makeNode("length/absolute",  makeNode("identifier", $2, NULL, NULL), NULL, NULL),$5, NULL);}	|
		num                          										{$$ = makeNode("NUM", $1, NULL, NULL);}										|
		minusPlus num														{$$ = makeNode($1, makeNode("NUM",$2, NULL, NULL),NULL, NULL);}		    	|
		minusPlus num opr expr												{$$ = makeNode($3,makeNode($1, makeNode("NUM",$2, NULL, NULL),NULL, NULL),$4,NULL);}		    	|
		boolean																{$$ = makeNode("boolean", $1, NULL, NULL);}									| /*All Expressions - Need To Think More*/
		CHARR																{$$ = makeNode("CHAR", makeNode( $1,NULL, NULL, NULL), NULL, NULL);}		|	
		string																{$$ = makeNode("string", $1, NULL, NULL);}									|
		NULLL																{$$ = makeNode("NULL", NULL, NULL, NULL);}									|
		ADDRESS identifier													{$$ = makeNode("addres",makeNode("identifier", $2, NULL, NULL), NULL, NULL);}									|
		ADDRESS identifier relation expr									{$$ = makeNode($3,makeNode("addres", makeNode("identifier", $2, NULL, NULL), NULL, NULL), $4, NULL);}			|
		ADDRESS identifier opr expr											{$$ = makeNode($3,makeNode("addres", makeNode("identifier", $2, NULL, NULL), NULL, NULL), $4, NULL);}			|
		POINTER identifier													{$$ = makeNode("pointer", makeNode("identifier", $2, NULL, NULL), NULL, NULL);}									|
		POINTER identifier relation expr									{$$ = makeNode($3,makeNode("pointer", makeNode("identifier", $2, NULL, NULL), NULL, NULL), $4, NULL);}			|
		POINTER identifier opr expr											{$$ = makeNode($3,makeNode("pointer", makeNode("identifier", $2, NULL, NULL), NULL, NULL), $4, NULL);}			|
		identifier relation expr											{$$ = makeNode($2, makeNode("identifier", $1, NULL, NULL), $3, NULL);}		|
		identifier opr expr													{$$ = makeNode($2, makeNode("identifier", $1, NULL, NULL), $3, NULL);}		|
		num relation expr													{$$ = makeNode($2, makeNode("NUM", $1, NULL, NULL), $3, NULL);}				|
		num opr expr														{$$ = makeNode($2, makeNode("NUM", $1, NULL, NULL), $3, NULL);}				|
		boolean relation expr												{$$ = makeNode($2, makeNode("boolean", $1, NULL, NULL), $3, NULL);}			|
		boolean opr expr													{$$ = makeNode($2, makeNode("boolean", $1, NULL, NULL), $3, NULL);}			|
		OPEN_PAREN expr CLOSE_PAREN											{$$ = $2;}																	|
		OPEN_PAREN expr CLOSE_PAREN relation expr							{$$ = makeNode($4, $2, $5, NULL);}											|
		OPEN_PAREN expr CLOSE_PAREN opr expr								{$$ = makeNode($4, $2, $5, NULL);}											;

minusPlus:	MINUS	{$$ ="-";}	|
			PLUS	{$$ ="+";}	;

sendedParameters:	expr							{$$ = makeNode("parmater", $1, NULL, NULL);}					| 
					expr COMMA sendedParameters		{$$ = makeNode("multiparm", $1, $3, NULL);}						|
					/*empty*/						{$$ = NULL;}													;

string:	T_STRING				{$$ = makeNode($1, NULL, NULL, NULL);}			;

boolean:	TRUEE				{$$ = makeNode("true", NULL, NULL, NULL);}		|
			FALSEE				{$$ = makeNode("false", NULL, NULL, NULL);}		;

relation:	EQUAL					{$$ = "=="; }				|
	    	NOT_EQUAL				{$$ = "!=";}				|
	    	BIGGER_THEN				{$$ = ">"; }				|
	    	BIGGER_EQUAL_THEN		{$$ = ">=";}				|
	    	SMALLER_THEN			{$$ = "<";}					|
	    	SMALLER_EQUAL_THEN		{$$ ="<=";}					|
			OR						{$$ = "OR";}				|
	    	AND						{$$ = "AND";}				;


opr:	PLUS			{$$ = "+";}						|
		MINUS			{$$ = "-";}						|
		MUL				{$$ = "*";}						| 
		DIV				{$$ = "/";}						;

return:	RETURN expr SEMICOLON		{$$ = makeNode("return", $2, NULL, NULL);}		|	
		RETURN SEMICOLON			{$$ = makeNode("return", NULL, NULL, NULL);}	;

block:	OPEN_BLOCK state CLOSE_BLOCK				{$$ = makeNode("BLOCK", $2, NULL, NULL);}	;

simpelAct:	identifier PLACEMENT expr					{$$ = makeNode("=", $1, $3, NULL);}										|
			POINTER identifier PLACEMENT expr					{$$ = makeNode("=", makeNode("pointer", $2, NULL, NULL), $4, NULL);}										|
            identifier PLACEMENT ADDRESS OPEN_PAREN identifier minusPlus numericExpr CLOSE_PAREN	
				{$$ = makeNode("=", $1, makeNode("addres", makeNode($6, $5, $7, NULL), NULL, NULL), NULL);}						|
		    identifier OPEN_PAREN sendedParameters CLOSE_PAREN 	{$$ = makeNode("callFunc",$1, $3, NULL);}						;

loop:	for				{$$ = makeNode("LOOP", $1, NULL, NULL);}		|
		while			{$$ = makeNode("LOOP", $1, NULL, NULL);}		|
		do				{$$ = makeNode("LOOP", $1, NULL, NULL);}		;

for:	FOR OPEN_PAREN place13 SEMICOLON place2 SEMICOLON place13 CLOSE_PAREN oneORmore
			{$$ = makeNode("FOR", makeNode("FORPART", $3, $5, $7), $9, NULL);}			;

place13:	simpelAct	{$$ = $1;}		|
			/*empty*/	{$$ = NULL;}	;

place2:		expr		{$$ = $1;}		|
			/*empty*/	{$$ = NULL;}	;

while:		WHILE OPEN_PAREN place2 CLOSE_PAREN oneORmore	{$$ = makeNode("WHILE", $3, $5, NULL);}												;

do:			DO block WHILE OPEN_PAREN place2 CLOSE_PAREN	{$$ = makeNode("DO", $2, makeNode("WHILE", $5, NULL, NULL), NULL);}					;



						
func: type identifier OPEN_PAREN parameters CLOSE_PAREN block			{$$ = makeNode("func", makeNode("FUNCINFO", $1, $2, $4), $6, NULL);}	;

parameters:	type identifier				{$$ = makeNode("parm", $1, $2, NULL);}			|
			type identifier multiparm 	{$$ = makeNode("parm", $1, $2, $3);}			|
			/*empty*/					{$$ = NULL;}	                				; /*Parameters For The Function*/

multiparm:	COMMA type identifier multiparm		{$$ = makeNode("multiparm", $2, $3, $4);}	|
			COMMA type identifier				{$$ = makeNode("multiparm", $2, $3, NULL);}	;

%%

#include "lex.yy.c"

void yyerror(const char *msg)
{
	fflush(stdout);
	fprintf(stderr, "Error: %s at line %d\n", msg, yylineno);
	fprintf(stderr, "Parser does not expect '%s'\n", yytext);
}

int yywrap(void)
{
	return 1;
}

int main()
{
	int check=1;
	if (!yyparse())
	{
		//printTree(globalTree, 0);
		check = checkProgram(globalTree, push(0));
		pop();
		if (cMain == 0 && check==1)
			{
				check=0;
				printf("ERROR: The code must have a main function!\n");
			}
		printglobalScope(stack);
		if(check==1)
			printf("%s",GenProgram(globalTree));
		return 0; //ok
	}
	else
		return 1; //error
}
