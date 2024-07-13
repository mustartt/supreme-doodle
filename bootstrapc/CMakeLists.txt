cmake_minimum_required(VERSION 3.20)

project(MyProject)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fno-rtti)

set(ANTLR4_JAR_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third-party/antlr/antlr-4.13.1.jar)

find_package(antlr4-runtime REQUIRED)
find_package(antlr4-generator REQUIRED)

antlr4_generate(
    antlrcpptest_lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LangLexer.g4
    LEXER
    FALSE
    FALSE
    "antlrcpptest"
)

antlr4_generate(
    antlrcpptest_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/grammar/LangParser.g4
    PARSER
    FALSE
    TRUE
    "antlrcpptest"
    "${ANTLR4_TOKEN_FILES_antlrcpptest_lexer}"
    "${ANTLR4_TOKEN_DIRECTORY_antlrcpptest_lexer}"
)

include_directories(
    ${PROJECT_BINARY_DIR} 
    ${ANTLR4_INCLUDE_DIR} 
    ${ANTLR4_INCLUDE_DIR_antlrcpptest_lexer} 
    ${ANTLR4_INCLUDE_DIR_antlrcpptest_parser}
)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader option)

add_executable(parser-cli main.cpp ${ANTLR4_SRC_FILES_antlrcpptest_lexer} ${ANTLR4_SRC_FILES_antlrcpptest_parser})
add_dependencies(parser-cli antlr4_shared)

target_link_libraries(parser-cli PRIVATE antlr4_shared ${llvm_libs})

