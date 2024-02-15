%option noyywrap

%{

#define DEBUG (0)

#include <stdio.h>
#define YY_DECL int yylex()
#include "metta.tab.h"

%}

%option yylineno

%%

[[:space:]] ;
[0-9]+\.[0-9]+ {
    yylval.fval = atof(yytext); 
    if (DEBUG) { fprintf(stderr, "T_FLOAT %g\n", yylval.fval); }
    return T_FLOAT;
}
[0-9]+ {
    yylval.ival = atoi(yytext); 
    if (DEBUG) { fprintf(stderr, "T_INT %ld\n", yylval.ival); }
    return T_INT;
}
"(" {
    if (DEBUG) { fprintf(stderr, "T_LEFTP\n"); }
    return T_LEFTP;
}
")" {
    if (DEBUG) { fprintf(stderr, "T_RIGHTP\n"); }
    return T_RIGHTP;
}
":" {
    if (DEBUG) { fprintf(stderr, "T_COLON\n"); }
    return T_COLON;
}
"<:" {
    if (DEBUG) { fprintf(stderr, "T_LESSTHANCOLON\n"); }
    return T_LESSTHANCOLON;
}
"->" {
    if (DEBUG) { fprintf(stderr, "T_ARROW\n"); }
    return T_ARROW;
}
"Type" {
    if (DEBUG) { fprintf(stderr, "T_TYPE\n"); }
    return T_TYPE;
}
[^[:space:]:()<>\-"]+ {
    yylval.sval = yytext; 
    if (DEBUG) { fprintf(stderr, "T_SYMBOL <%s>\n", yytext); }
    return T_SYMBOL;
}
["]([^"])*["] {
    yylval.sval = yytext; 
    if (DEBUG) { fprintf(stderr, "T_QUOTED_STR <%s>\n", yytext); }
    return T_QUOTED_STR;
}

%%

void yyerror(const char *s)
{
    fprintf(stderr,"Parse error near line %d\nInput: %s\nError message: %s\n", yylineno, yytext, s);
    exit(1);
}

