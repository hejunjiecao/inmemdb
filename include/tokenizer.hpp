#pragma once
#include <optional>
#include <string>
#include <vector>

namespace db {

enum class TokType {
  IDENT,
  NUMBER,
  STRING,
  SYMBOL,
  STAR,
  COMMA,
  LPAREN,
  RPAREN,
  SEMICOLON,
  EQUAL,
  NOTEQUAL,
  LT,
  GT,
  LE,
  GE,
  END
};

struct Token {
  TokType type;
  std::string text;
};

class Tokenizer {
public:
  explicit Tokenizer(const std::string &s);
  Token peek() const;
  Token next();
  bool eof() const;

private:
  const std::string &input;
  size_t i;
  void skip_ws();
  Token scan_string();
  Token scan_ident_or_number();
};

} // namespace db
