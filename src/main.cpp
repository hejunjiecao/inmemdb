#include "database.hpp"
#include "output.hpp"
#include "parser.hpp"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

using namespace db;

int main(int argc, char **argv) {
  OutputMode mode = OutputMode::ASCII;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--csv")
      mode = OutputMode::CSV;
    else if (arg == "--ascii")
      mode = OutputMode::ASCII;
    else {
      std::cerr << "Unknown argument: " << arg << "\n";
      return 2;
    }
  }

  if (isatty(fileno(stdin))) {
    std::cerr << "Enter SQL statements (end with Ctrl+D):" << std::endl;
  }

  std::ostringstream buf;
  buf << std::cin.rdbuf();
  std::string input = buf.str();

  Database db;
  auto stmts = split_statements(input);

  for (size_t idx = 0; idx < stmts.size(); ++idx) {
    try {
      Statement s = parse_statement(stmts[idx]);
      auto res = execute(db, s);
      if (res.has_value()) {
        if (mode == OutputMode::CSV)
          std::cout << to_csv(*res);
        else
          std::cout << to_ascii(*res);
      }
    } catch (const ParseError &e) {
      std::cerr << "Parse error in statement " << (idx + 1) << ": " << e.what()
                << "\n";
    } catch (const DBError &e) {
      std::cerr << "Execution error in statement " << (idx + 1) << ": "
                << e.what() << "\n";
    } catch (const std::exception &e) {
      std::cerr << "Unexpected error in statement " << (idx + 1) << ": "
                << e.what() << "\n";
    }
  }
  return 0;
}