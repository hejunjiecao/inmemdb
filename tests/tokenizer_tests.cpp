#include "tokenizer.hpp"
#include <catch2/catch.hpp>

using namespace db;

TEST_CASE("Tokenizer basic functionality", "[tokenizer]") {
  SECTION("Empty string and whitespace") {
    Tokenizer tz("");
    REQUIRE(tz.eof());

    Tokenizer tz2("   \t\n  ");
    REQUIRE(tz2.eof());
  }

  SECTION("Identifiers and numbers") {
    Tokenizer tz("hello world 123 -456");

    Token t1 = tz.next();
    REQUIRE(t1.type == TokType::IDENT);
    REQUIRE(t1.text == "hello");

    Token t2 = tz.next();
    REQUIRE(t2.type == TokType::IDENT);
    REQUIRE(t2.text == "world");

    Token t3 = tz.next();
    REQUIRE(t3.type == TokType::NUMBER);
    REQUIRE(t3.text == "123");

    Token t4 = tz.next();
    REQUIRE(t4.type == TokType::NUMBER);
    REQUIRE(t4.text == "-456");
  }

  SECTION("Strings") {
    Tokenizer tz("\"hello world\"");
    Token t1 = tz.next();
    REQUIRE(t1.type == TokType::STRING);
    REQUIRE(t1.text == "hello world");
  }

  SECTION("Symbols") {
    Tokenizer tz("( ) , ; =");
    REQUIRE(tz.next().type == TokType::LPAREN);
    REQUIRE(tz.next().type == TokType::RPAREN);
    REQUIRE(tz.next().type == TokType::COMMA);
    REQUIRE(tz.next().type == TokType::SEMICOLON);
    REQUIRE(tz.next().type == TokType::EQUAL);
  }

  SECTION("Comparison operators") {
    Tokenizer tz("= != < > <= >=");
    REQUIRE(tz.next().type == TokType::EQUAL);
    REQUIRE(tz.next().type == TokType::NOTEQUAL);
    REQUIRE(tz.next().type == TokType::LT);
    REQUIRE(tz.next().type == TokType::GT);
    REQUIRE(tz.next().type == TokType::LE);
    REQUIRE(tz.next().type == TokType::GE);
  }
}
