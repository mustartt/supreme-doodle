lexer grammar LangLexer;

WS: [ \t\n\r\f]+ -> skip ;
LINE_COMMENT : '//' ~[\r\n]* -> skip;
BLOCK_COMMENT : '/*' .*? '*/' -> skip; 

PACKAGE: 'package';
IMPORT: 'import';
TYPE: 'type';
IMPL: 'impl';

TRAIT: 'trait';
PUBLIC: 'public';
PRIVATE: 'private';
MUT: 'mut';
DYN: 'dyn';
NULLABLE: 'nullable' ;
LET: 'let';
STRUCT: 'struct';
FUNC: 'func';
RETURN: 'return';
IF: 'if';
ELSE: 'else';
ENUM: 'enum';
FOR: 'for';
USE: 'use';
AS: 'as';
AND : 'and' ;
OR : 'or' ;
NOT : 'not' ;
EQ : '=' ;
PLUS: '+';
MINUS: '-';
COMMA : ',' ;
DOT : '.' ;
SEMI : ';' ;
COLON : ':' ;
STAR : '*' ;
MOD: '%';
DIV: '/';
GEQ: '>=';
LEQ: '<=';
CMP_EQ: '==';
NOT_EQ: '!=';
REF: '&';
LPAREN : '(' ;
RPAREN : ')' ;
LCURLY : '{' ;
RCURLY : '}' ;
LANGLE : '<' ;
RANGLE : '>' ;
LBRACKET : '[';
RBRACKET : ']';

// literals
BOOL_LITERAL        : 'true' | 'false';
CHAR_LITERAL        : '\'' ( ESC_SEQ | ~[\u0027\\] ) '\'';
NUM_LITERAL         : DIGITS ('.' DIGITS)? EXPONENT? | '.' DIGITS EXPONENT?;
STRING_LITERAL      : '"' ( ESC_SEQ | ~["\\] )* '"';

fragment DIGITS     : [0-9]+;
fragment EXPONENT   : [eE] [+\-]? DIGITS;
fragment ESC_SEQ    : '\\' ( [btnfr"'\\] | UNICODE_ESC );
fragment UNICODE_ESC: 'u' HEX HEX HEX HEX;
fragment HEX        : [0-9a-fA-F];

IDENTIFIER: [a-zA-Z_][a-zA-Z_0-9]*;

