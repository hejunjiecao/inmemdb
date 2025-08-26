#include "output.hpp"
#include <catch2/catch.hpp>

using namespace db;

TEST_CASE("CSV output format", "[output]") {
  SECTION("Basic CSV formatting") {
    QueryResult result;
    result.headers = {"name", "age"};
    result.rows = {{"alice", "30"}, {"bob", "25"}};

    std::string csv = to_csv(result);
    REQUIRE(csv.find("name,age") != std::string::npos);
    REQUIRE(csv.find("alice,30") != std::string::npos);
    REQUIRE(csv.find("bob,25") != std::string::npos);
  }

  SECTION("CSV escaping") {
    QueryResult result;
    result.headers = {"name", "description"};
    result.rows = {{"alice", "lives in NYC, NY"}};

    std::string csv = to_csv(result);
    REQUIRE(csv.find("\"lives in NYC, NY\"") != std::string::npos);
  }

  SECTION("Empty result") {
    QueryResult result;
    std::string csv = to_csv(result);
    REQUIRE(csv == "\n");
  }
}

TEST_CASE("ASCII output format", "[output]") {
  SECTION("Basic ASCII formatting") {
    QueryResult result;
    result.headers = {"name", "age"};
    result.rows = {{"alice", "30"}, {"bob", "25"}};

    std::string ascii = to_ascii(result);
    REQUIRE(ascii.find("| name  | age |") != std::string::npos);
    REQUIRE(ascii.find("| alice | 30  |") != std::string::npos);
    REQUIRE(ascii.find("| bob   | 25  |") != std::string::npos);
  }

  SECTION("Column width calculation") {
    QueryResult result;
    result.headers = {"id", "name", "description"};
    result.rows = {{"1", "alice", "A very long description"},
                   {"2", "bob", "Short"}};

    std::string ascii = to_ascii(result);
    // Check that the output contains the expected format
    REQUIRE(ascii.find("| id | name  | description") != std::string::npos);
    REQUIRE(ascii.find("| 1  | alice | A very long description") !=
            std::string::npos);
  }

  SECTION("Empty result") {
    QueryResult result;
    std::string ascii = to_ascii(result);
    // ASCII formatter should still output headers and separators even for empty
    // results
    REQUIRE(ascii.find("+") != std::string::npos);
    REQUIRE(ascii.find("|") != std::string::npos);
  }
}

TEST_CASE("QueryResult structure", "[output]") {
  SECTION("Basic structure") {
    QueryResult result;
    result.headers = {"id", "name"};
    result.rows = {{"1", "alice"}, {"2", "bob"}};

    REQUIRE(result.headers.size() == 2);
    REQUIRE(result.rows.size() == 2);
    REQUIRE(result.rows[0][0] == "1");
    REQUIRE(result.rows[0][1] == "alice");
  }

  SECTION("Empty result") {
    QueryResult result;
    REQUIRE(result.headers.empty());
    REQUIRE(result.rows.empty());
  }
}
