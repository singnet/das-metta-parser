%{

#include <stdio.h>
#include <stdlib.h>
#include "actions.h"
#include "action_util.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern void yyerror(const char* s);
unsigned long INPUT_LINE_COUNT;

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

%type<eval> type_desc
%type<eval> type_desc_list
%type<eval> expression
%type<eval> expression_list

%type<sval> typedef
%type<sval> atom_typedef
%type<sval> function_typedef
%type<sval> toplevel
%type<sval> toplevel_expression
%type<sval> literal

%start start

%define parse.error verbose

%%

start:               { printf("Empty file\n"); }
     | toplevel_list { start();                }
;

toplevel_list: toplevel               { toplevel_list_base($1);      }
             | toplevel_list toplevel { toplevel_list_recursion($2); }
;

toplevel: typedef             { $$ = $1; }
        | toplevel_expression { $$ = $1; }
;

typedef: atom_typedef                                         { $$ = $1; }
       | T_LEFTP T_COLON expression function_typedef T_RIGHTP { $$ = typedef_function($3, $4); }
;

atom_typedef: T_LEFTP T_COLON expression T_TYPE T_RIGHTP     { $$ = atom_typedef_atom_type($3);     }
            | T_LEFTP T_COLON expression expression T_RIGHTP { $$ = atom_typedef_atom_atom($3, $4); }
;

function_typedef: T_LEFTP T_ARROW type_desc_list T_RIGHTP { $$ = function_typedef($3); }
;

type_desc_list: type_desc                { $$ = type_desc_list_base($1);          }
              | type_desc_list type_desc { $$ = type_desc_list_recursion($1, $2); }
;

type_desc: T_TYPE           { $$ = type_desc_type();     }
         | T_SYMBOL         { $$ = type_desc_symbol($1);   }
         | function_typedef { $$ = type_desc_function($1); }
;

toplevel_expression: T_LEFTP expression_list T_RIGHTP { $$ = toplevel_expression($2); }
;

expression_list: expression                 { $$ = expression_list_base($1);          }
               | expression_list expression { $$ = expression_list_recursion($1, $2); }
;

expression: T_SYMBOL                         { $$ = expression_symbol($1);    }
          | literal                          { $$ = expression_literal($1);   }
          | T_LEFTP expression_list T_RIGHTP { $$ = expression_composite($2); }
          | T_LEFTP T_RIGHTP { $$ = expression_empty(); }
;

literal: T_QUOTED_STR { $$ = literal_string($1); }
       | T_INT        { $$ = literal_int($1);    }
       | T_FLOAT      { $$ = literal_float($1);  }
;

%%

struct HandleList EMPTY_HANDLE_LIST;

int main(int argc, char *argv[]) {

    EMPTY_HANDLE_LIST.size = 0;
    EMPTY_HANDLE_LIST.elements = NULL;
    EMPTY_HANDLE_LIST.elements_type = NULL;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file path>\n", argv[0]);
        exit(1);
    }

    INPUT_LINE_COUNT = get_line_count(argv[1]);
    
	yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        yyerror("Invalid input file path\n");
        exit(1);
    }

    initialize_actions();
	do {
		yyparse();
	} while(!feof(yyin));
    finalize_actions();

	return 0;
}
