#include "database.hpp"
#include "parser.hpp"
#include <catch2/catch.hpp>

using namespace db;

TEST_CASE("Complete SQL workflow", "[integration]") {
  SECTION("CREATE -> INSERT -> SELECT workflow") {
    Database db;

    // Parse and execute CREATE TABLE
    auto create_stmt =
        parse_statement("CREATE TABLE people (name str, age int)");
    auto create_result = execute(db, create_stmt);
    REQUIRE(!create_result.has_value());

    // Parse and execute INSERT
    auto insert_stmt = parse_statement(
        "INSERT INTO people (name, age) VALUES (\"alice\", 30)");
    auto insert_result = execute(db, insert_stmt);
    REQUIRE(!insert_result.has_value());

    // Parse and execute SELECT
    auto select_stmt = parse_statement("SELECT * FROM people");
    auto select_result = execute(db, select_stmt);
    REQUIRE(select_result.has_value());

    auto &result = *select_result;
    REQUIRE(result.headers.size() == 2);
    REQUIRE(result.rows.size() == 1);
    REQUIRE(result.rows[0][0] == "alice");
    REQUIRE(result.rows[0][1] == "30");
  }

  SECTION("Multiple operations") {
    Database db;

    // Create table
    auto create_stmt = parse_statement("CREATE TABLE users (id int, name str)");
    execute(db, create_stmt);

    // Insert multiple rows
    auto insert1 =
        parse_statement("INSERT INTO users (id, name) VALUES (1, \"alice\")");
    auto insert2 =
        parse_statement("INSERT INTO users (id, name) VALUES (2, \"bob\")");
    execute(db, insert1);
    execute(db, insert2);

    // Select all
    auto select_stmt = parse_statement("SELECT * FROM users");
    auto select_result = execute(db, select_stmt);
    REQUIRE(select_result.has_value());

    auto &result = *select_result;
    REQUIRE(result.rows.size() == 2);
    REQUIRE(result.rows[0][1] == "alice");
    REQUIRE(result.rows[1][1] == "bob");
  }

  SECTION("UPDATE and DELETE workflow") {
    Database db;

    // Create and populate table
    auto create_stmt =
        parse_statement("CREATE TABLE products (id int, name str, price int)");
    execute(db, create_stmt);

    auto insert_stmt = parse_statement(
        "INSERT INTO products (id, name, price) VALUES (1, \"laptop\", 1000)");
    execute(db, insert_stmt);

    // Update price
    auto update_stmt = parse_statement(
        "UPDATE products SET price = 900 WHERE name = \"laptop\"");
    execute(db, update_stmt);

    // Verify update
    auto select_stmt =
        parse_statement("SELECT * FROM products WHERE name = \"laptop\"");
    auto select_result = execute(db, select_stmt);
    REQUIRE(select_result.has_value());

    auto &result = *select_result;
    REQUIRE(result.rows.size() == 1);
    REQUIRE(result.rows[0][2] == "900");

    // Delete product
    auto delete_stmt =
        parse_statement("DELETE FROM products WHERE price < 1000");
    execute(db, delete_stmt);

    // Verify deletion
    auto final_select = parse_statement("SELECT * FROM products");
    auto final_result = execute(db, final_select);
    REQUIRE(final_result.has_value());

    auto &final = *final_result;
    REQUIRE(final.rows.empty());
  }
}

TEST_CASE("Error handling in SQL execution", "[integration]") {
  SECTION("Invalid table reference") {
    Database db;
    auto select_stmt = parse_statement("SELECT * FROM nonexistent_table");
    REQUIRE_THROWS_AS(execute(db, select_stmt), DBError);
  }

  SECTION("Type mismatch in INSERT") {
    Database db;
    auto create_stmt = parse_statement("CREATE TABLE test (id int)");
    execute(db, create_stmt);

    auto insert_stmt =
        parse_statement("INSERT INTO test (id) VALUES (\"not_a_number\")");
    REQUIRE_THROWS_AS(execute(db, insert_stmt), TypeError);
  }

  SECTION("Invalid column reference") {
    Database db;
    auto create_stmt = parse_statement("CREATE TABLE test (id int, name str)");
    execute(db, create_stmt);

    auto select_stmt = parse_statement("SELECT invalid_column FROM test");
    REQUIRE_THROWS_AS(execute(db, select_stmt), DBError);
  }
}

TEST_CASE("Multi-statement execution", "[integration]") {
  SECTION("Parse and execute from string input") {
    std::string sql_input =
        "CREATE TABLE people (name str, age int);"
        "INSERT INTO people (name, age) VALUES (\"alice\", 30), (\"bob\", 25);"
        "SELECT * FROM people WHERE age > 25;";

    auto statements = split_statements(sql_input);
    REQUIRE(statements.size() == 3);

    Database db;

    // Execute all statements
    for (const auto &stmt_str : statements) {
      auto stmt = parse_statement(stmt_str);
      execute(db, stmt);
    }

    // Verify final state
    auto final_select = parse_statement("SELECT * FROM people");
    auto final_result = execute(db, final_select);
    REQUIRE(final_result.has_value());

    auto &result = *final_result;
    REQUIRE(result.rows.size() == 2);
  }
}
