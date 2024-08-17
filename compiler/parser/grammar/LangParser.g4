parser grammar LangParser;
options { tokenVocab=LangLexer; }

program: package_decl? import_stmt* (global_decl SEMI?)* EOF
    ;

qualified_identifier: IDENTIFIER (DOT IDENTIFIER)*
    ;

visibility: PUBLIC | PRIVATE
    ;

package_decl: PACKAGE IDENTIFIER
    ;

import_stmt: IMPORT import_path (AS IDENTIFIER)?
    ;
import_path: IDENTIFIER (DOT IDENTIFIER)*
    ;

global_decl
    : type_decl
    | use_decl
    | impl_decl
    | var_decl 
    | func_decl
    ;

impl_decl: 
    IMPL qualified_identifier 
        (FOR qualified_identifier)? 
    LCURLY func_decl* RCURLY
    ;

type_decl: visibility? TYPE IDENTIFIER EQ type 
    ;

use_decl: visibility? USE IDENTIFIER EQ type
    ;

var_decl
    : visibility? LET IDENTIFIER (COLON type)? initializer?
    ;

func_decl
    : visibility? FUNC IDENTIFIER
        LPAREN func_param_list? RPAREN 
        func_return_type?
        func_body? 
    ;
func_return_type: type ;
func_param_list: func_param_decl (COMMA func_param_decl)* ;
func_param_decl: IDENTIFIER COLON type initializer? ;
func_body: block_stmt ;

/* statements */

block_stmt: LCURLY (statement SEMI?)* RCURLY;
statement
    : return_stmt
    | decl_stmt
    | expr_stmt
    | for_stmt
    | block_stmt
    ;

decl_stmt: var_decl;
initializer: EQ expr;

return_stmt: RETURN expr?;

expr_stmt: expr;


for_stmt: FOR for_header for_body;
for_header
    : expr
    | decl_stmt? SEMI expr? SEMI expr? 
    ;

for_body
    : statement
    | block_stmt
    ;

/* expression */
expr: expr DOT IDENTIFIER           #accessExpr 
    | expr LBRACKET expr RBRACKET   #indexExpr
    | expr LPAREN arguments? RPAREN #callExpr
    | op=(MINUS | NOT | REF) expr   #unaryExpr
    | expr op=(STAR | DIV) expr     #binaryExpr 
    | expr op=(PLUS | MINUS) expr   #binaryExpr    
    | expr op=(
            LANGLE | 
            RANGLE | 
            LEQ | 
            GEQ | 
            CMP_EQ | 
            NOT_EQ
        ) expr                      #binaryExpr
    | expr EQ expr                  #assignExpr
    | if_expr                       #ifExpr
    | object_expr                   #objectExpr
    | identifier                    #identifierExpr
    | literal                       #literalExpr
    ;

object_expr: LCURLY (object_field (COMMA object_field)*)? RCURLY ;
object_field: IDENTIFIER COLON expr ;

identifier: IDENTIFIER ;
if_expr: IF if_header if_body (ELSE if_body)? ;
if_header: expr ;
if_body: statement ;

arguments: expr (COMMA expr)*;

/* production rule for testing types */
test_type: type+ EOF ;

type: MUT type
    | qualified_identifier
    | pointer_type
    | array_type
    | function_type
    | object_type
    | enum_type
    ;

pointer_type : STAR NULLABLE? type ;
array_type : LBRACKET type RBRACKET ;
function_type : FUNC LPAREN parameter_type_list? RPAREN type ;
parameter_type_list : type (COMMA type)* ;
object_type: LCURLY object_field_type (COMMA object_field_type)* RCURLY ;
object_field_type: visibility? IDENTIFIER COLON type ;
enum_type: ENUM LCURLY enum_member (COMMA enum_member)* RCURLY;
enum_member: IDENTIFIER (COLON type)?;  

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





