cmake_minimum_required(VERSION 3.20)

project(MyProject)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -fno-omit-frame-pointer -fsanitize=address)
add_link_options(-Wall -fno-omit-frame-pointer -fsanitize=address)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core option)

include_directories(include)
add_subdirectory(lib)

# Project Targets
add_executable(parse-tree parse-tree.cpp)
target_compile_options(parse-tree PRIVATE -fno-rtti)
target_link_libraries(parse-tree PRIVATE ${llvm_libs})
target_link_libraries(parse-tree PRIVATE parser Basic)

add_executable(rx-frontend rx-frontend.cpp)
target_compile_options(rx-frontend PRIVATE -fno-rtti)
target_link_libraries(rx-frontend PRIVATE ${llvm_libs} Basic parser ast sema Frontend)

# Testing Infra
add_subdirectory(third-party/googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(unittest unittest/test.cpp unittest/TypesTest.cpp)
target_link_libraries(unittest gtest gtest_main ${llvm_libs})
add_test(test unittest)

add_custom_target(check-all
    COMMAND ${CMAKE_COMMAND} 
        -E env PARSE_TREE_BIN=$<TARGET_FILE:parse-tree> FRONT_END_BIN=$<TARGET_FILE:rx-frontend>
        lit ${CMAKE_CURRENT_SOURCE_DIR}/test -vs
    DEPENDS parse-tree)

add_custom_target(check-unit COMMAND $<TARGET_FILE:unittest> DEPENDS unittest)



