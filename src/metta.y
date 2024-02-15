%{

#include <stdio.h>
#include <stdlib.h>
#include "actions.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern void yyerror(const char* s);

%}

%locations

%code requires {
#include "handle_list.h"
}

%union {
	long ival;
	double fval;
    char *sval;
    char **cval;
    struct HandleList eval;
}

%token<ival> T_INT
%token<fval> T_FLOAT
%token<sval> T_SYMBOL T_TYPE T_QUOTED_STR
%token T_LEFTP T_RIGHTP
%token T_COLON T_LESSTHANCOLON T_ARROW
%token T_NEWLINE

%type<sval> toplevel_expression
%type<eval> expression_list
%type<eval> expression
%type<sval> literal

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
            | T_LEFTP T_COLON T_SYMBOL function_typedef T_RIGHTP
;

inherited_typedef: T_LEFTP T_LESSTHANCOLON T_SYMBOL T_SYMBOL T_RIGHTP
;

symbol_typedef: T_LEFTP T_COLON T_SYMBOL T_TYPE T_RIGHTP
              | T_LEFTP T_COLON T_SYMBOL T_SYMBOL T_RIGHTP
              | T_LEFTP T_COLON literal T_TYPE T_RIGHTP
              | T_LEFTP T_COLON literal T_SYMBOL T_RIGHTP
;

function_typedef: T_LEFTP T_ARROW type_desc_list T_RIGHTP
;

type_desc_list: type_desc
              | type_desc_list type_desc
;

type_desc: T_TYPE
         | T_SYMBOL
         | function_typedef
;

toplevel_expression: T_LEFTP expression_list T_RIGHTP { $$ = toplevel_expression($2); }
;

expression_list: expression                 { $$ = expression_list_base($1); }
               | expression_list expression { $$ = expression_list_recursion($1, $2); }
;

expression: T_SYMBOL                         { $$ = expression_symbol($1);    }
          | literal                          { $$ = expression_literal($1);   }
          | T_LEFTP expression_list T_RIGHTP { $$ = expression_composite($2); }
;

literal: T_QUOTED_STR { $$ = literal_string($1); }
       | T_INT        { $$ = literal_int($1);    }
       | T_FLOAT      { $$ = literal_float($1);  }
;

%%

struct HandleList EMPTY_HANDLE_LIST;

int main() {

    EMPTY_HANDLE_LIST.size = 0;
    EMPTY_HANDLE_LIST.elements = NULL;
    EMPTY_HANDLE_LIST.elements_type = NULL;

	yyin = stdin;
    initialize_actions();
	do {
		yyparse();
	} while(!feof(yyin));
    finalize_actions();

	return 0;
}
