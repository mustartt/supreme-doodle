lexer grammar LangLexer;

WS: [ \t\n\r\f]+ -> skip ;
LINE_COMMENT : '//' ~[\r\n]* -> skip;
BLOCK_COMMENT : '/*' .*? '*/' -> skip; 

PACKAGE: 'package';
IMPORT: 'import';
TRAIT: 'trait';
PUBLIC: 'public';
PRIVATE: 'private';
MUT: 'mut';
DYN: 'dyn';
LET: 'let';
STRUCT: 'struct';
FUNC: 'func';
RETURN: 'return';

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
DIV: '/';
LPAREN : '(' ;
RPAREN : ')' ;
LCURLY : '{' ;
RCURLY : '}' ;
LANGLE : '<' ;
RANGLE : '>' ;
LBRACKET : '[';
RBRACKET : ']';

BOOL_TYPE: 'bool';
CHAR_TYPE: 'char';
INTEGRAL_TYPE
    : I8_TYPE
    | I16_TYPE
    | I32_TYPE
    | I64_TYPE
    | U8_TYPE
    | U16_TYPE
    | U32_TYPE
    | U64_TYPE
    ;
fragment I8_TYPE: 'i8';
fragment I16_TYPE: 'i16';
fragment I32_TYPE: 'i32';
fragment I64_TYPE: 'i64';
fragment U8_TYPE: 'u8';
fragment U16_TYPE: 'u16';
fragment U32_TYPE: 'u32';
fragment U64_TYPE: 'u64';

FLOAT_TYPE
    : F32_TYPE
    | F64_TYPE
    ;
fragment F32_TYPE: 'f32';
fragment F64_TYPE: 'f64';

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

