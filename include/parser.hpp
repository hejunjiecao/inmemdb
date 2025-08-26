#pragma once
#include "database.hpp"
#include <string>
#include <vector>

namespace db {

// split input by semicolons outside of quotes
std::vector<std::string> split_statements(const std::string &input);

// parse a single statement (without trailing semicolon)
Statement parse_statement(const std::string &stmt);

} // namespace db
