#define CATCH_CONFIG_MAIN
#include "database.hpp"
#include "output.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"
#include <catch2/catch.hpp>

using namespace db;

// Test utilities
Database create_test_db() {
  Database db;
  db.create_table("people", {{"name", Type::STR}, {"age", Type::INT}});
  return db;
}

Database create_complex_db() {
  Database db;
  db.create_table(
      "users", {{"id", Type::INT}, {"name", Type::STR}, {"email", Type::STR}});
  db.create_table(
      "orders",
      {{"id", Type::INT}, {"user_id", Type::INT}, {"amount", Type::INT}});
  return db;
}

// Include all test files
// Note: Catch2 will automatically discover and run all TEST_CASE macros
// from the included headers, so we don't need to explicitly include them here.
