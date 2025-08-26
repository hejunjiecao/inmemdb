#include "output.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace db {

static std::string csv_escape(const std::string &v) {
  bool needs = v.find_first_of(",\"\n") != std::string::npos;
  if (!needs)
    return v;
  std::string out = "\"";
  for (char c : v) {
    if (c == '"')
      out += "\"\"";
    else
      out += c;
  }
  out += "\"";
  return out;
}

std::string to_csv(const QueryResult &r) {
  std::ostringstream oss;
  // header
  for (size_t i = 0; i < r.headers.size(); ++i) {
    if (i)
      oss << ",";
    oss << csv_escape(r.headers[i]);
  }
  oss << "\n";
  // rows
  for (const auto &row : r.rows) {
    for (size_t i = 0; i < row.size(); ++i) {
      if (i)
        oss << ",";
      oss << csv_escape(row[i]);
    }
    oss << "\n";
  }
  return oss.str();
}

std::string to_ascii(const QueryResult &r) {
  std::vector<size_t> widths(r.headers.size(), 0);
  for (size_t i = 0; i < r.headers.size(); ++i)
    widths[i] = r.headers[i].size();
  for (const auto &row : r.rows) {
    for (size_t i = 0; i < row.size(); ++i)
      widths[i] = std::max(widths[i], row[i].size());
  }
  auto line = [&]() {
    std::ostringstream s;
    s << "+";
    for (auto w : widths) {
      s << std::string(w + 2, '-') << "+";
    }
    return s.str();
  };
  auto fmt_row = [&](const std::vector<std::string> &cells) {
    std::ostringstream s;
    s << "|";
    for (size_t i = 0; i < cells.size(); ++i) {
      s << " " << std::left << std::setw(static_cast<int>(widths[i]))
        << cells[i] << " |";
    }
    return s.str();
  };

  std::ostringstream oss;
  oss << line() << "\n";
  oss << fmt_row(r.headers) << "\n";
  oss << line() << "\n";
  for (const auto &row : r.rows) {
    oss << fmt_row(row) << "\n";
  }
  oss << line() << "\n";
  return oss.str();
}

} // namespace db
