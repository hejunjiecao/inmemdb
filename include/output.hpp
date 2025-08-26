#pragma once
#include <string>
#include <vector>

namespace db {

struct QueryResult {
  std::vector<std::string> headers;
  std::vector<std::vector<std::string>> rows;
};

enum class OutputMode { ASCII, CSV };

std::string to_csv(const QueryResult &r);
std::string to_ascii(const QueryResult &r);

} // namespace db
