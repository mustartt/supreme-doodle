set(ANTLR4_JAR_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/antlr/antlr-4.13.1.jar)

find_package(antlr4-runtime REQUIRED)
find_package(antlr4-generator REQUIRED)

antlr4_generate(
    antlrcpptest_lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LangLexer.g4
    LEXER
    FALSE
    FALSE
    "antlr4"
)

antlr4_generate(
    antlrcpptest_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LangParser.g4
    PARSER
    FALSE
    TRUE
    "antlr4"
    "${ANTLR4_TOKEN_FILES_antlrcpptest_lexer}"
    "${ANTLR4_TOKEN_DIRECTORY_antlrcpptest_lexer}"
)

include_directories(
    ${PROJECT_BINARY_DIR} 
    ${ANTLR4_INCLUDE_DIR} 
    ${ANTLR4_INCLUDE_DIR_antlrcpptest_lexer} 
    ${ANTLR4_INCLUDE_DIR_antlrcpptest_parser}
    include
)

add_library(parser 
    lib/parser.cpp 
    include/parser.h
    ${ANTLR4_SRC_FILES_antlrcpptest_lexer} 
    ${ANTLR4_SRC_FILES_antlrcpptest_parser}
)
add_dependencies(parser antlr4_shared)
target_link_libraries(parser PRIVATE antlr4_shared)
target_include_directories(parser PUBLIC include)


