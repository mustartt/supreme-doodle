parser grammar LangParser;
options { tokenVocab=LangLexer; }

program
    : package_decl? import_stmt* global_decl* EOF
    ;

global_decl
    : trait_decl
    | struct_decl
    | var_decl
    | func_decl
    ;

package_decl
    : PACKAGE IDENTIFIER
    ;

import_stmt
    : IMPORT import_path (AS IDENTIFIER)?
    ;
import_path
    : IDENTIFIER (DOT IDENTIFIER)*
    ;

qualified_identifier
    : IDENTIFIER (DOT IDENTIFIER)*
    ;

visibility
    : PUBLIC
    | PRIVATE
    ;

trait_decl
    : visibility? TRAIT IDENTIFIER 
        (COLON trait_list)?
        LCURLY RCURLY
    ;

trait_list
    : qualified_identifier (COMMA qualified_identifier)*
    ;

struct_decl
    : visibility? STRUCT IDENTIFIER 
        LCURLY struct_field* RCURLY
    ;

struct_field
    : visibility? IDENTIFIER (COLON type)? initializer? 
    ;

var_decl
    : visibility? LET IDENTIFIER (COLON type)? initializer?
    ;

func_decl
    : visibility? FUNC IDENTIFIER
        LPAREN func_param_list? RPAREN 
        func_return_type?
        func_body 
    ;

func_return_type
    : COLON type
    ;

func_param_list
    : func_param_decl (COMMA func_param_decl)*
    ;

func_param_decl
    : IDENTIFIER COLON type initializer?
    ;

func_body: LCURLY block RCURLY;

/* statements */

block: statement* ;
statement
    : return_stmt
    | decl_stmt
    | expr_stmt
    | if_stmt
    | for_stmt
    ;

decl_stmt: LET IDENTIFIER (COLON type)? initializer?;
initializer: EQ expr;

return_stmt: RETURN expr?;

expr_stmt: expr;

if_stmt: IF if_header if_body (ELSE if_body)? ;
if_header: expr;
if_body
    : statement
    | LCURLY block RCURLY
    ;

for_stmt: FOR for_header for_body;
for_header
    : expr
    | decl_stmt? SEMI expr? SEMI expr? 
    ;

for_body
    : statement
    | LCURLY block RCURLY
    ;

/* expression */
expr: expr RPAREN expr_list RPAREN
    | expr DOT IDENTIFIER 
    | expr LBRACKET expr RBRACKET 
    | (MINUS | NOT | REF) expr
    | expr (STAR | DIV) expr 
    | expr (PLUS | MINUS) expr 
    | expr (LANGLE | RANGLE | LEQ | GEQ | CMP_EQ | NOT_EQ) expr
    | expr EQ expr
    | IDENTIFIER
    | literal
    ;

expr_list: expr (COMMA expr)*;

/* production rule for testing types */
test_type: type+ EOF ;

type: MUT type
    | BOOL_TYPE
    | CHAR_TYPE
    | INTEGRAL_TYPE
    | FLOAT_TYPE
    | qualified_identifier
    | pointer_type
    | array_type
    | tuple_type
    | function_type
    ;

pointer_type
    : STAR DYN? type
    ;

array_type
    : LBRACKET type RBRACKET
    ;

tuple_type
    : LPAREN type (COMMA type)* RPAREN
    ;

function_type
    : LPAREN parameter_type_list? RPAREN COLON type
    ;

parameter_type_list
    : type (COMMA type)*
    ;

test_literal: literal* EOF;
literal
    : bool_literal
    | char_literal
    | num_literal
    | string_literal
    ;

bool_literal    : BOOL_LITERAL;
char_literal    : CHAR_LITERAL;
num_literal     : NUM_LITERAL;
string_literal  : STRING_LITERAL;





