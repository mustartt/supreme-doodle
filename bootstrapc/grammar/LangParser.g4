parser grammar LangParser;
options { tokenVocab=LangLexer; }

program
    : stat EOF
    | def EOF
    ;

stat: ID EQ expr SEMI
    | expr SEMI
    ;

def : ID LPAREN ID (COMMA ID)* RPAREN LCURLY stat* RCURLY ;

expr: ID
    | INT
    | func
    | NOT expr
    | expr AND expr
    | expr OR expr
    ;

func : ID LPAREN expr (COMMA expr)* RPAREN ;
