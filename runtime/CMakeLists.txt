cmake_minimum_required(VERSION 3.20)

project(RxRuntime)

set(CMAKE_CXX_STANDARD 20)

# include(cmake/FindLibunwind.cmake)

find_package(Boost REQUIRED COMPONENTS fiber context system)

include_directories(${Boost_INCLUDE_DIRS})
include_directories("${CMAKE_SOURCE_DIR}/third-party/llvm-statepoint-utils/dist")

add_library(llvm_statepoint_tablegen STATIC IMPORTED)
set_target_properties(llvm_statepoint_tablegen PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/third-party/llvm-statepoint-utils/dist/llvm-statepoint-tablegen.a"
)

include_directories("/usr/local/include")
add_library(libunwind STATIC IMPORTED)
set_target_properties(libunwind PROPERTIES
    IMPORTED_LOCATION "/usr/local/lib/libunwind.a"
)

add_executable(runtime runtime.cpp add.o main.o)
target_link_libraries(runtime PRIVATE llvm_statepoint_tablegen)
target_link_libraries(runtime PRIVATE libunwind)




