%option noyywrap

%{

#define DEBUG (1)

#include <stdio.h>
#define YY_DECL int yylex()
#include "metta.tab.h"

%}

%option yylineno

%%

[[:space:]] ;
[;].*\n {
    if (DEBUG) { fprintf(stdout, "COMMENT %s\n", yytext); }
}
[#].*\n {
    if (DEBUG) { fprintf(stdout, "COMMENT %s\n", yytext); }
}
[0-9]+\.[0-9]+ {
    yylval.fval = atof(yytext); 
    if (DEBUG) { fprintf(stdout, "T_FLOAT %g\n", yylval.fval); }
    return T_FLOAT;
}
[0-9]+ {
    yylval.ival = atoi(yytext); 
    if (DEBUG) { fprintf(stdout, "T_INT %ld\n", yylval.ival); }
    return T_INT;
}
"(" {
    if (DEBUG) { fprintf(stdout, "T_LEFTP\n"); }
    return T_LEFTP;
}
")" {
    if (DEBUG) { fprintf(stdout, "T_RIGHTP\n"); }
    return T_RIGHTP;
}
":" {
    if (DEBUG) { fprintf(stdout, "T_COLON\n"); }
    return T_COLON;
}
"<:" {
    if (DEBUG) { fprintf(stdout, "T_LESSTHANCOLON\n"); }
    return T_LESSTHANCOLON;
}
"->" {
    if (DEBUG) { fprintf(stdout, "T_ARROW\n"); }
    return T_ARROW;
}
"Type" {
    if (DEBUG) { fprintf(stdout, "T_TYPE\n"); }
    return T_TYPE;
}
([^[:space:]:()<>"]|\\(.|\n))+ {
    yylval.sval = strdup(yytext); 
    if (DEBUG) { fprintf(stdout, "T_SYMBOL <%s>\n", yytext); }
    return T_SYMBOL;
}
["]([^"\n\\]|\\(.|\n))*["] {
    yylval.sval = strdup(yytext); 
    if (DEBUG) { fprintf(stdout, "T_QUOTED_STR <%s>\n", yytext); }
    return T_QUOTED_STR;
}

%%

void yyerror(const char *s)
{
    fprintf(stderr,"Parse error near line %d\nInput: %s\nError message: %s\n", yylineno, yytext, s);
    exit(1);
}

