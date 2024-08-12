cmake_minimum_required(VERSION 3.13.4)
project(HeapMapPass)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_library(HeapMapPass SHARED
    HeapMapPass.cpp
)

llvm_map_components_to_libnames(LLVM_LIBS Support Core IRReader Passes)
target_link_libraries(HeapMapPass ${LLVM_LIBS})

set_target_properties(HeapMapPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
    PREFIX ""
)

