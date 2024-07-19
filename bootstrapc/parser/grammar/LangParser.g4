parser grammar LangParser;
options { tokenVocab=LangLexer; }

program
    : package? import_stmt* 
        (   
            trait_decl  | 
            struct_decl | 
            var_decl    | 
            func_decl
        )* EOF
    ;

package
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
    : IDENTIFIER (COLON type)? struct_field_initializer? 
    ;

struct_field_initializer
    : EQ expr
    ;

var_decl
    : visibility? LET IDENTIFIER (COLON type)? var_initializer?
    ;

var_initializer
    : EQ expr
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
    : IDENTIFIER COLON type func_param_initializer?
    ;

func_param_initializer
    : EQ expr
    ;

func_body: LCURLY block RCURLY;

/* statements */

block: (return_stmt | decl_stmt | expr_stmt)* ;

decl_stmt: LET IDENTIFIER (COLON type)? initializer?;
initializer: EQ expr;

return_stmt: RETURN expr?;

expr_stmt: expr;

/* expression */
expr: MINUS expr
    | expr (STAR | DIV) expr 
    | expr (PLUS | MINUS) expr 
    | expr EQ expr
    | qualified_identifier
    | literal
    ;

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





