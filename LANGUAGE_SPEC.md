# RX Language Specification

Version 0.1.0

## 1. Introduction

RX is a statically-typed systems programming language with a focus on safety and expressiveness. It features a rich type system, trait-based polymorphism, and modern control flow constructs. The language is designed with explicit memory management through pointers and references, while maintaining type safety.

## 2. Lexical Structure

### 2.1 Keywords

The following identifiers are reserved keywords in RX:

```
package    import     type       impl       trait
public     private    mut        dyn        nullable
let        struct     func       return     if
else       enum       for        use        as
and        or         not
```

### 2.2 Identifiers

Identifiers must start with a letter or underscore, followed by any sequence of letters, digits, or underscores:

```
IDENTIFIER: [a-zA-Z_][a-zA-Z_0-9]*
```

Examples:
- `count`
- `_temp`
- `value_123`
- `MyType`

### 2.3 Literals

#### 2.3.1 Boolean Literals

Boolean literals are either `true` or `false`.

```
BOOL_LITERAL: 'true' | 'false'
```

#### 2.3.2 Character Literals

Character literals are single characters enclosed in single quotes. They support escape sequences.

```
CHAR_LITERAL: '\'' ( ESC_SEQ | ~[\u0027\\] ) '\''
```

Examples:
- `'a'`
- `'Z'`
- `'\n'`
- `'\t'`

#### 2.3.3 Numeric Literals

Numeric literals support both integer and floating-point values with optional scientific notation.

```
NUM_LITERAL: DIGITS ('.' DIGITS)? EXPONENT? | '.' DIGITS EXPONENT?
EXPONENT: [eE] [+\-]? DIGITS
```

Examples:
- `42`
- `3.14`
- `1e-9`
- `.001`
- `1.27e-09`
- `5e+3`

#### 2.3.4 String Literals

String literals are sequences of characters enclosed in double quotes. They support escape sequences.

```
STRING_LITERAL: '"' ( ESC_SEQ | ~["\\] )* '"'
```

Supported escape sequences:
- `\b` - backspace
- `\t` - tab
- `\n` - newline
- `\f` - form feed
- `\r` - carriage return
- `\"` - double quote
- `\'` - single quote
- `\\` - backslash
- `\uXXXX` - Unicode escape (4 hex digits)

Examples:
- `"Hello"`
- `"Hello World!"`
- `"Hello \t World!\n"`

### 2.4 Operators and Punctuation

#### 2.4.1 Arithmetic Operators
- `+` - addition
- `-` - subtraction
- `*` - multiplication
- `/` - division
- `%` - modulo

#### 2.4.2 Comparison Operators
- `==` - equal to
- `!=` - not equal to
- `<` - less than
- `>` - greater than
- `<=` - less than or equal to
- `>=` - greater than or equal to

#### 2.4.3 Logical Operators
- `and` - logical AND
- `or` - logical OR
- `not` - logical NOT

#### 2.4.4 Other Operators
- `=` - assignment
- `&` - reference
- `.` - member access
- `:` - type annotation

#### 2.4.5 Punctuation
- `,` - comma (separator)
- `;` - semicolon (statement terminator)
- `()` - parentheses
- `{}` - curly braces
- `[]` - square brackets
- `<>` - angle brackets

### 2.5 Comments

RX supports two types of comments:

#### Line Comments
Line comments begin with `//` and continue to the end of the line:
```
// This is a line comment
```

#### Block Comments
Block comments are enclosed between `/*` and `*/`:
```
/* This is a block comment
   that spans multiple lines */
```

### 2.6 Whitespace

Whitespace characters (space, tab, newline, carriage return, form feed) are used to separate tokens and are otherwise ignored.

## 3. Type System

### 3.1 Primitive Types

#### 3.1.1 Boolean Type
- `bool` - Boolean value (`true` or `false`)

#### 3.1.2 Character Type
- `char` - Single Unicode character

#### 3.1.3 Integer Types
- `i32` - 32-bit signed integer
- `i64` - 64-bit signed integer
- `int` - Platform-dependent integer

#### 3.1.4 Floating-Point Types
- `f32` - 32-bit IEEE 754 floating-point
- `f64` - 64-bit IEEE 754 floating-point

### 3.2 Composite Types

#### 3.2.1 Pointer Types

Pointer types are denoted with `*` prefix and can be marked as nullable:

```
*T           // Non-nullable pointer to T
*nullable T  // Nullable pointer to T
```

Examples:
```rx
let p: *i32           // Pointer to i32
let q: *nullable char // Nullable pointer to char
```

#### 3.2.2 Array Types

Array types are denoted with square brackets:

```
[T]  // Array of type T
```

Examples:
```rx
let arr: [i32]        // Array of i32
let matrix: [[f64]]   // Array of arrays of f64
```

#### 3.2.3 Function Types

Function types specify parameter types and return type:

```
func(T1, T2, ..., Tn) R
```

Where `T1, T2, ..., Tn` are parameter types and `R` is the return type.

Examples:
```rx
let f: func(i32) f64              // Function taking i32, returning f64
let g: func(i32, i32) bool        // Function taking two i32, returning bool
let h: func() void                // Function with no parameters, no return
```

#### 3.2.4 Object Types

Object types (also called struct types) are defined using curly braces with field declarations:

```
{ field1: T1, field2: T2, ..., fieldN: TN }
```

Fields can have visibility modifiers (`public` or `private`).

Examples:
```rx
type Point = {
    x: i32,
    y: i32
}

type Person = {
    public name: string,
    private age: i32
}
```

#### 3.2.5 Enum Types

Enum types define tagged unions with optional associated data:

```
enum { Variant1: T1, Variant2: T2, ..., VariantN }
```

Variants can have no associated data or specify a type.

Examples:
```rx
type Option = enum {
    Some: i32,
    None
}

type Tagged = enum {
    Tag1: {x: int, y: int},
    Tag2: int
}
```

### 3.3 Type Modifiers

#### 3.3.1 Mutability

The `mut` modifier indicates that a type is mutable:

```
mut T  // Mutable type T
```

Examples:
```rx
let x: mut i32 = 5
let ptr: mut *Point
```

#### 3.3.2 Qualified Types

Types can be qualified with module or namespace prefixes using dot notation:

```
module.Type
```

Examples:
```rx
let file: io.file
let conn: net.Connection
```

## 4. Grammar Specification

### 4.1 Program Structure

A program consists of an optional package declaration, zero or more import statements, and global declarations:

```
program:
    package_decl?
    import_stmt*
    (global_export SEMI?)*
    EOF
```

### 4.2 Package Declaration

A package declaration specifies the package name:

```
package_decl: PACKAGE IDENTIFIER
```

Example:
```rx
package io
```

### 4.3 Import Statements

Import statements bring external modules into scope:

```
import_stmt: IMPORT import_path (AS IDENTIFIER)?
import_path: string_literal | IDENTIFIER (DOT IDENTIFIER)*
```

Examples:
```rx
import "UseDecl.rx"
import io
import std.collections as col
```

### 4.4 Global Declarations

Global declarations can be preceded by visibility modifiers:

```
global_export: visibility? global_decl
global_decl: type_decl | use_decl | impl_decl | var_decl | func_decl
visibility: PUBLIC | PRIVATE
```

## 5. Declarations

### 5.1 Variable Declarations

Variable declarations introduce a new variable with optional type annotation and initializer:

```
var_decl: LET IDENTIFIER (COLON type)? initializer?
initializer: EQ expr
```

Examples:
```rx
let x: i32                    // Declaration without initializer
let y: i32 = 42              // Declaration with initializer
let z = 3.14                 // Type inference from initializer
let ptr: mut *Point          // Mutable pointer
```

### 5.2 Type Declarations

Type declarations create type aliases:

```
type_decl: TYPE IDENTIFIER EQ type
```

Examples:
```rx
type Integer = i32
type Point = { x: f32, y: f32 }
type Result = enum { Ok: i32, Error: string }
```

### 5.3 Use Declarations

Use declarations create type aliases (similar to type declarations):

```
use_decl: USE IDENTIFIER EQ type
```

Examples:
```rx
public use alias = i32
use MyInt = i64
```

### 5.4 Function Declarations

Function declarations define named functions:

```
func_decl:
    FUNC IDENTIFIER
    LPAREN func_param_list? RPAREN
    type?
    func_body?

func_param_list: func_param_decl (COMMA func_param_decl)*
func_param_decl: IDENTIFIER COLON type initializer?
func_body: block_stmt
```

The return type is optional; if omitted, the function returns `void`.

Examples:
```rx
func add(a: i32, b: i32) i32 {
    return a + b
}

func println() {
    // No return type specified (void)
}

func greet(name: string = "World") {
    // Parameter with default value
}

// Function declaration without body
func external_func(x: i32) bool
```

### 5.5 Implementation Blocks

Implementation blocks attach methods to types:

```
impl_decl: IMPL type LCURLY func_decl* RCURLY
```

Examples:
```rx
type Point = { x: i32, y: i32 }

impl Point {
    func distance(self: Point, other: Point) f64 {
        // Method implementation
    }
    
    func scale(self: mut *Point, factor: f64) {
        self.x = self.x * factor
        self.y = self.y * factor
    }
}
```

## 6. Statements

### 6.1 Block Statements

Block statements group multiple statements together:

```
block_stmt: LCURLY (statement SEMI?)* RCURLY
```

Semicolons after statements are optional.

Example:
```rx
{
    let x = 5
    let y = 10
    x + y
}
```

### 6.2 Expression Statements

Any expression can be used as a statement:

```
expr_stmt: expr
```

Examples:
```rx
x = 42
add(1, 2)
obj.method()
```

### 6.3 Declaration Statements

Declarations can appear as statements within blocks:

```
decl_stmt: var_decl | type_decl | use_decl
```

### 6.4 Return Statements

Return statements exit from a function with an optional value:

```
return_stmt: RETURN expr?
```

Examples:
```rx
return
return 42
return x + y
```

### 6.5 For Loops

For loops support two forms: condition-based and C-style three-part:

```
for_stmt: FOR for_header for_body
for_header:
    expr                              // Condition-only
    | decl_stmt? SEMI expr? SEMI expr?  // Three-part (init; condition; update)
for_body: statement | block_stmt
```

Examples:
```rx
// Condition-only loop
for true {
    // infinite loop
}

for x < 10
    x = x + 1

// C-style loop
for let i: mut i32 = 0; i < 10; i = i + 1 {
    // loop body
}

// With optional parts
for ; x < 10; x = x + 1 {
    // loop body
}
```

## 7. Expressions

### 7.1 Operator Precedence

Operators are listed from highest to lowest precedence:

1. Member access: `.`
2. Array indexing: `[]`, Function call: `()`
3. Unary operators: `-`, `not`, `&`
4. Multiplicative: `*`, `/`, `%`
5. Additive: `+`, `-`
6. Relational: `<`, `>`, `<=`, `>=`, `==`, `!=`
7. Assignment: `=`

### 7.2 Primary Expressions

#### 7.2.1 Identifier Expressions

Identifiers refer to variables, functions, or types in scope:

```rx
x
count
myFunction
```

#### 7.2.2 Literal Expressions

Literal expressions are constant values:

```rx
true
42
3.14
'a'
"Hello"
```

### 7.3 Member Access

Member access uses the dot operator:

```
expr DOT IDENTIFIER
```

Examples:
```rx
point.x
obj.field
module.function
```

### 7.4 Array Indexing

Array indexing uses square brackets:

```
expr LBRACKET expr RBRACKET
```

Example:
```rx
arr[0]
matrix[i][j]
```

### 7.5 Function Calls

Function calls pass arguments to a callable expression:

```
expr LPAREN arguments? RPAREN
arguments: expr (COMMA expr)*
```

Examples:
```rx
add(1, 2)
obj.method(x, y)
println("Hello")
```

### 7.6 Unary Expressions

Unary operators apply to a single operand:

```
unary_expr: op=(MINUS | NOT | REF) expr
```

- `-expr` - Numeric negation
- `not expr` - Logical negation
- `&expr` - Reference/address-of

Examples:
```rx
-5
not flag
&variable
```

### 7.7 Binary Expressions

Binary operators apply to two operands:

#### Arithmetic Operators
```rx
a + b    // Addition
a - b    // Subtraction
a * b    // Multiplication
a / b    // Division
a % b    // Modulo
```

#### Comparison Operators
```rx
a < b     // Less than
a > b     // Greater than
a <= b    // Less than or equal
a >= b    // Greater than or equal
a == b    // Equal to
a != b    // Not equal to
```

### 7.8 Assignment Expressions

Assignment expressions store values in variables:

```
expr EQ expr
```

Example:
```rx
x = 42
arr[i] = value
obj.field = 10
```

### 7.9 If Expressions

If expressions provide conditional evaluation:

```
if_expr: IF if_header if_body (ELSE if_body)?
if_header: expr
if_body: statement
```

If expressions can be used as expressions, returning values.

Examples:
```rx
if x > 0
    return x

if condition {
    value1
} else {
    value2
}

if x < 0
    return -1
else if x > 0
    return 1
else
    return 0
```

### 7.10 Object Expressions

Object expressions create object instances:

```
object_expr: LCURLY (object_field (COMMA object_field)*)? RCURLY
object_field: IDENTIFIER COLON expr
```

Example:
```rx
let point = { x: 10, y: 20 }
let rect = { width: 10.0, height: 5.0 }
```

## 8. Visibility and Access Control

### 8.1 Visibility Modifiers

RX supports two visibility levels:

- `public` - Accessible from any module
- `private` - Accessible only within the current module (default)

### 8.2 Applying Visibility

Visibility modifiers can be applied to:

- Global declarations (functions, types, variables)
- Object fields
- Type aliases (use declarations)

Examples:
```rx
public func add(a: i32, b: i32) i32 { }
private type Internal = i32

type Person = {
    public name: string,
    private age: i32
}
```

## 9. Scoping Rules

### 9.1 Lexical Scoping

RX uses lexical scoping. Variables are accessible within the block they are declared and any nested blocks.

### 9.2 Shadowing

Inner declarations can shadow outer declarations with the same name.

Example:
```rx
let x = 10
{
    let x = 20  // Shadows outer x
    // x is 20 here
}
// x is 10 here
```

## 10. Memory Model

### 10.1 Value Semantics

Most types in RX have value semantics by default.

### 10.2 Reference Semantics

Pointers provide reference semantics:
- `*T` - Non-nullable pointer to T
- `*nullable T` - Nullable pointer to T

### 10.3 Mutability

The `mut` keyword indicates mutability:
- Variables can be mutable or immutable
- Pointers can point to mutable or immutable data
- `mut *T` - Mutable pointer to T
- `*mut T` - Pointer to mutable T

## 11. Module System

### 11.1 Packages

Packages are declared at the top of a file:

```rx
package mypackage
```

### 11.2 Imports

Modules can be imported in two ways:

```rx
import "filepath.rx"           // Import by file path
import module.submodule        // Import by qualified name
import std.collections as col  // Import with alias
```

### 11.3 Qualified Access

Imported modules can be accessed using dot notation:

```rx
io.print("Hello")
col.List
```

## 12. Type Inference

RX supports limited type inference:

- Variable declarations can omit type annotations if an initializer is present
- The type is inferred from the initializer expression
- Function return types must be explicitly specified (or omitted for `void`)

Examples:
```rx
let x = 42        // Inferred as i32
let y = 3.14      // Inferred as f64
let s = "Hello"   // Inferred as string
```

## 13. Reserved for Future Use

The following features are mentioned in the grammar but may not be fully implemented:

### 13.1 Traits

Traits define interfaces that types can implement:

```rx
trait Printable {
    func print()
}

impl Printable for Integer {
    func print(self: Integer) { }
}
```

### 13.2 Dynamic Types

The `dyn` keyword is reserved for dynamic/trait object types.

### 13.3 Advanced Control Flow

Additional control flow constructs may be added in future versions.

## 14. Examples

### 14.1 Hello World

```rx
package main

import io

func main() {
    io.println("Hello, World!")
}
```

### 14.2 Fibonacci

```rx
func fibonacci(n: i32) i32 {
    if n <= 1
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)
}
```

### 14.3 Data Structures

```rx
type Rectangle = {
    width: f32,
    height: f32
}

impl Rectangle {
    func area(self: *Rectangle) f32 {
        return self.width * self.height
    }

    func scale(self: mut *Rectangle, factor: f32) {
        self.width = self.width * factor
        self.height = self.height * factor
    }
}

func main() {
    let rect: mut Rectangle = { width: 10.0, height: 5.0 }
    io.print("Area: ", rect.area())
    rect.scale(2.0)
    io.print("Scaled area: ", rect.area())
}
```

### 14.4 Enums

```rx
type Option = enum {
    Some: i32,
    None
}

func unwrap_or(opt: Option, default: i32) i32 {
    // Pattern matching would be used here
    // (not yet fully specified)
    return default
}
```

## 15. Formal Grammar Summary

```
program          ::= package_decl? import_stmt* global_export*
package_decl     ::= 'package' IDENTIFIER
import_stmt      ::= 'import' import_path ('as' IDENTIFIER)?
import_path      ::= STRING | IDENTIFIER ('.' IDENTIFIER)*
global_export    ::= visibility? global_decl
global_decl      ::= type_decl | use_decl | impl_decl | var_decl | func_decl
visibility       ::= 'public' | 'private'

type_decl        ::= 'type' IDENTIFIER '=' type
use_decl         ::= 'use' IDENTIFIER '=' type
var_decl         ::= 'let' IDENTIFIER (':' type)? ('=' expr)?
func_decl        ::= 'func' IDENTIFIER '(' params? ')' type? block_stmt?
impl_decl        ::= 'impl' type '{' func_decl* '}'

params           ::= param (',' param)*
param            ::= IDENTIFIER ':' type ('=' expr)?

statement        ::= return_stmt | decl_stmt | expr_stmt | for_stmt | block_stmt
return_stmt      ::= 'return' expr?
decl_stmt        ::= var_decl | type_decl | use_decl
expr_stmt        ::= expr
for_stmt         ::= 'for' for_header for_body
for_header       ::= expr | (decl_stmt? ';' expr? ';' expr?)
for_body         ::= statement | block_stmt
block_stmt       ::= '{' statement* '}'

expr             ::= expr '.' IDENTIFIER                    // member access
                  |  expr '[' expr ']'                      // index
                  |  expr '(' args? ')'                     // call
                  |  ('-' | 'not' | '&') expr               // unary
                  |  expr ('*' | '/') expr                  // multiplicative
                  |  expr ('+' | '-') expr                  // additive
                  |  expr ('<' | '>' | '<=' | '>=' | '==' | '!=') expr  // comparison
                  |  expr '=' expr                          // assignment
                  |  'if' expr statement ('else' statement)?  // if expression
                  |  '{' (IDENTIFIER ':' expr)* '}'         // object literal
                  |  IDENTIFIER                             // identifier
                  |  literal                                // literal

args             ::= expr (',' expr)*

type             ::= 'mut' type                             // mutable
                  |  type '.' IDENTIFIER                    // qualified
                  |  IDENTIFIER                             // simple
                  |  '*' 'nullable'? type                   // pointer
                  |  '[' type ']'                           // array
                  |  'func' '(' types? ')' type             // function
                  |  '{' field_types '}'                    // object
                  |  'enum' '{' enum_variants '}'           // enum

types            ::= type (',' type)*
field_types      ::= field_type (',' field_type)*
field_type       ::= visibility? IDENTIFIER ':' type
enum_variants    ::= enum_variant (',' enum_variant)*
enum_variant     ::= IDENTIFIER (':' type)?

literal          ::= BOOL | CHAR | NUMBER | STRING
```

## Appendix A: Comparison with Other Languages

RX shares features with several modern languages:

- **Rust**: Similar syntax for types, impl blocks, and traits
- **Go**: Simple package system, function syntax
- **C/C++**: Explicit pointer types, manual memory management
- **TypeScript**: Object literal syntax, type annotations

## Appendix B: Compiler Architecture

The RX compiler follows a traditional multi-phase architecture:

1. **Lexical Analysis**: Tokenization (LangLexer.g4)
2. **Parsing**: Parse tree construction (LangParser.g4)
3. **Semantic Analysis**: Type checking and name resolution
4. **IR Generation**: LLVM IR generation
5. **Optimization**: opt (LLVM optimizer)
6. **Code Generation**: llc (LLVM compiler)
7. **Linking**: Final executable generation

Compiler invocation: `rxc: rx-fe -> opt -> llc -> link`
