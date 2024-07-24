set(ANTLR4_JAR_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/antlr/antlr-4.13.1.jar)

find_package(antlr4-runtime REQUIRED)
find_package(antlr4-generator REQUIRED)

antlr4_generate(
    rx_lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LangLexer.g4
    LEXER
    FALSE
    FALSE
    "antlr4"
)

antlr4_generate(
    rx_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LangParser.g4
    PARSER
    FALSE
    TRUE
    "antlr4"
    "${ANTLR4_TOKEN_FILES_rx_lexer}"
    "${ANTLR4_TOKEN_DIRECTORY_rx_lexer}"
)

add_library(
    parser STATIC
    include/parser/parser.h
    lib/parser.cpp 
    lib/ParserError.cpp
    lib/ParserErrorListener.h
    lib/ParseTreeVisitor.h
    ${ANTLR4_SRC_FILES_rx_lexer} 
    ${ANTLR4_SRC_FILES_rx_parser}
)
add_dependencies(parser antlr4_shared)

target_link_libraries(parser PUBLIC ast)
target_link_libraries(parser PRIVATE antlr4_shared)

target_include_directories(parser PUBLIC include)
target_include_directories(parser PRIVATE 
    ${ANTLR4_INCLUDE_DIR} 
    ${ANTLR4_INCLUDE_DIR_rx_lexer} 
    ${ANTLR4_INCLUDE_DIR_rx_parser}
)

