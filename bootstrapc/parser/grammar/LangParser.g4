parser grammar LangParser;
options { tokenVocab=LangLexer; }

program
    : package import_stmt* 
        (trait_decl)* EOF
    ;

package
    : PACKAGE IDENTIFIER SEMI
    ;

import_stmt
    : IMPORT import_path (AS IDENTIFIER)? SEMI
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

/* production rule for testing types */
test_type: type+ EOF ;

type: BOOL_TYPE
    | CHAR_TYPE
    | INTEGRAL_TYPE
    | FLOAT_TYPE
    | pointer_type
    | array_type
    | tuple_type
    | function_type
    ;

pointer_type
    : STAR pointer_attr type
    ;

pointer_attr
    : MUT? DYN?
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







