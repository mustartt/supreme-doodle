cmake_minimum_required(VERSION 3.20)

project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fno-omit-frame-pointer")

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core option)

include_directories(include)
add_subdirectory(lib)

add_executable(parse-tree parse-tree.cpp)
target_compile_options(parse-tree PRIVATE -fno-rtti)
target_link_libraries(parse-tree PRIVATE ${llvm_libs})
target_link_libraries(parse-tree PRIVATE parser)

add_executable(rx-frontend frontend.cpp)
target_compile_options(rx-frontend PRIVATE -fno-rtti)
target_link_libraries(rx-frontend PRIVATE ${llvm_libs} parser ast sema)

add_custom_target(check-all
    COMMAND ${CMAKE_COMMAND} 
        -E env PARSE_TREE_BIN=$<TARGET_FILE:parse-tree> 
        lit ${CMAKE_CURRENT_SOURCE_DIR}/test -vs
    DEPENDS parse-tree)

