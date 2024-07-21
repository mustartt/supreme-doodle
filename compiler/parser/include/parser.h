
#ifndef TEST_LANG_PARSER_PARSER_H
#define TEST_LANG_PARSER_PARSER_H

#include <istream>
#include <ostream>

void parse(std::istream&, std::ostream&, const std::string&) noexcept; 

void test();

#endif // TEST_LANG_PARSER_PARSER_H
