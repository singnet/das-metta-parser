%{

#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern void yyerror(const char* s);

%}

%locations

%union {
	int ival;
	float fval;
    char *sval;
}

%token<ival> T_INT
%token<fval> T_FLOAT
%token T_LEFTP T_RIGHTP
%token T_COLON T_LESSTHANCOLON T_ARROW
%token T_SYMBOL T_TYPE T_QUOTED_STR
%token T_NEWLINE

%start start
%define parse.error verbose

%%

start:
    | toplevel_list
;

toplevel_list: top_level
    | toplevel_list top_level
;

top_level: typedef
    | toplevel_expression
;

typedef: base_typedef
    | inherited_typedef
;

base_typedef: symbol_typedef
    | T_LEFTP T_COLON T_SYMBOL function_typedef T_RIGHTP;
;

inherited_typedef: T_LEFTP T_LESSTHANCOLON T_SYMBOL T_SYMBOL T_RIGHTP;

symbol_typedef: T_LEFTP T_COLON T_SYMBOL T_TYPE T_RIGHTP
    | T_LEFTP T_COLON T_SYMBOL T_SYMBOL T_RIGHTP
;

function_typedef: T_LEFTP T_ARROW type_desc_list T_RIGHTP;

type_desc_list: type_desc
    | type_desc_list type_desc
;

type_desc: T_TYPE
    | T_SYMBOL
    | function_typedef
;

toplevel_expression: T_LEFTP expression_list T_RIGHTP;

expression_list: expression
    | expression_list expression
;

expression: T_SYMBOL
    | literal
    | T_LEFTP expression_list T_RIGHTP
;

literal: T_QUOTED_STR
    | T_INT
    | T_FLOAT
;

%%

int main() {
	yyin = stdin;

	do {
		yyparse();
	} while(!feof(yyin));

	return 0;
}
