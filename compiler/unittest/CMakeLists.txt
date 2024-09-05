add_executable(
    unittest 
    test.cpp 
    TypesTest.cpp
)
target_link_libraries(unittest gtest gtest_main ${llvm_libs} ast)

add_custom_target(check-unit COMMAND $<TARGET_FILE:unittest> DEPENDS unittest)
