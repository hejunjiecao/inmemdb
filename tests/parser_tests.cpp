#include "parser.hpp"
#include <catch2/catch.hpp>

using namespace db;

TEST_CASE("Statement splitting", "[parser]") {
  SECTION("Basic splitting") {
    auto stmts =
        split_statements("CREATE TABLE test (id int); SELECT * FROM test;");
    REQUIRE(stmts.size() == 2);
    REQUIRE(stmts[0] == "CREATE TABLE test (id int)");
    REQUIRE(stmts[1] == "SELECT * FROM test");
  }

  SECTION("Semicolons in strings") {
    auto stmts = split_statements(
        "INSERT INTO test VALUES (\"hello;world\"); SELECT * FROM test;");
    REQUIRE(stmts.size() == 2);
    REQUIRE(stmts[0] == "INSERT INTO test VALUES (\"hello;world\")");
  }
}

TEST_CASE("SQL statement parsing", "[parser]") {
  SECTION("CREATE TABLE") {
    auto stmt = parse_statement("CREATE TABLE people (name str, age int)");
    REQUIRE(std::holds_alternative<StmtCreate>(stmt));

    auto create_stmt = std::get<StmtCreate>(stmt);
    REQUIRE(create_stmt.name == "people");
    REQUIRE(create_stmt.columns.size() == 2);
    REQUIRE(create_stmt.columns[0].name == "name");
    REQUIRE(create_stmt.columns[0].type == Type::STR);
  }

  SECTION("INSERT") {
    auto stmt = parse_statement(
        "INSERT INTO people (name, age) VALUES (\"alice\", 30)");
    REQUIRE(std::holds_alternative<StmtInsert>(stmt));

    auto insert_stmt = std::get<StmtInsert>(stmt);
    REQUIRE(insert_stmt.table == "people");
    REQUIRE(insert_stmt.values.size() == 1);
  }

  SECTION("SELECT") {
    auto stmt = parse_statement("SELECT * FROM people");
    REQUIRE(std::holds_alternative<StmtSelect>(stmt));

    auto select_stmt = std::get<StmtSelect>(stmt);
    REQUIRE(select_stmt.star == true);
    REQUIRE(select_stmt.table == "people");
  }

  SECTION("UPDATE") {
    auto stmt =
        parse_statement("UPDATE people SET age = 35 WHERE name = \"alice\"");
    REQUIRE(std::holds_alternative<StmtUpdate>(stmt));

    auto update_stmt = std::get<StmtUpdate>(stmt);
    REQUIRE(update_stmt.table == "people");
    REQUIRE(update_stmt.sets.size() == 1);
    REQUIRE(update_stmt.where.has_value());
  }

  SECTION("DELETE") {
    auto stmt = parse_statement("DELETE FROM people WHERE age < 18");
    REQUIRE(std::holds_alternative<StmtDelete>(stmt));

    auto delete_stmt = std::get<StmtDelete>(stmt);
    REQUIRE(delete_stmt.table == "people");
    REQUIRE(delete_stmt.where.has_value());
  }
}

TEST_CASE("WHERE conditions", "[parser]") {
  SECTION("Comparison operators") {
    auto stmt1 = parse_statement("SELECT * FROM people WHERE age = 25");
    auto select1 = std::get<StmtSelect>(stmt1);
    REQUIRE(select1.where->op == Condition::Op::EQ);

    auto stmt2 = parse_statement("SELECT * FROM people WHERE age > 25");
    auto select2 = std::get<StmtSelect>(stmt2);
    REQUIRE(select2.where->op == Condition::Op::GT);

    auto stmt3 = parse_statement("SELECT * FROM people WHERE age <= 25");
    auto select3 = std::get<StmtSelect>(stmt3);
    REQUIRE(select3.where->op == Condition::Op::LE);
  }

  SECTION("String literals") {
    auto stmt = parse_statement("SELECT * FROM people WHERE name = \"alice\"");
    auto select = std::get<StmtSelect>(stmt);
    REQUIRE(select.where->literal.type == Type::STR);
    REQUIRE(select.where->literal.s == "alice");
  }
}

TEST_CASE("Parser error handling", "[parser]") {
  SECTION("Invalid keywords") {
    REQUIRE_THROWS_AS(parse_statement("create TABLE test (id int)"),
                      ParseError);
    REQUIRE_THROWS_AS(parse_statement("select * FROM people"), ParseError);
  }

  SECTION("Malformed statements") {
    REQUIRE_THROWS_AS(parse_statement("CREATE TABLE test (id int"), ParseError);
    REQUIRE_THROWS_AS(parse_statement("SELECT * FROM"), ParseError);
  }
}
