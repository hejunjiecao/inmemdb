#include "tokenizer.hpp"
#include <cctype>

namespace db {

Tokenizer::Tokenizer(const std::string &s) : input(s), i(0) {}

void Tokenizer::skip_ws() {
  while (i < input.size() && std::isspace(static_cast<unsigned char>(input[i])))
    ++i;
}

Token Tokenizer::scan_string() {
  // assume input[i] == '"'
  ++i; // skip opening quote
  std::string out;
  while (i < input.size()) {
    char c = input[i++];
    if (c == '"')
      break;
    out.push_back(c);
  }
  return Token{TokType::STRING, out};
}

Token Tokenizer::scan_ident_or_number() {
  size_t start = i;
  if (input[i] == '-' || std::isdigit(static_cast<unsigned char>(input[i]))) {
    // number
    bool is_num = true;
    if (input[i] == '-')
      ++i;
    if (i >= input.size() ||
        !std::isdigit(static_cast<unsigned char>(input[i]))) {
      // '-' not followed by digit -> treat as ident
      is_num = false;
    } else {
      while (i < input.size() &&
             std::isdigit(static_cast<unsigned char>(input[i])))
        ++i;
      if (i < input.size() &&
          (std::isalpha(static_cast<unsigned char>(input[i])) ||
           input[i] == '_')) {
        // something like 123abc -> ident
        is_num = false;
      }
    }
    if (is_num)
      return Token{TokType::NUMBER, input.substr(start, i - start)};
    // fallthrough to ident
    i = start;
  }
  // ident: letters, digits, underscore
  while (i < input.size()) {
    char c = input[i];
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.') {
      ++i;
    } else
      break;
  }
  return Token{TokType::IDENT, input.substr(start, i - start)};
}

Token Tokenizer::peek() const {
  Tokenizer tmp = *this;
  return tmp.next();
}

Token Tokenizer::next() {
  skip_ws();
  if (i >= input.size())
    return Token{TokType::END, ""};
  char c = input[i];
  if (c == '"')
    return scan_string();
  if (std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '-' ||
      std::isdigit(static_cast<unsigned char>(c))) {
    return scan_ident_or_number();
  }
  ++i;
  switch (c) {
  case '*':
    return Token{TokType::STAR, "*"};
  case ',':
    return Token{TokType::COMMA, ","};
  case '(':
    return Token{TokType::LPAREN, "("};
  case ')':
    return Token{TokType::RPAREN, ")"};
  case ';':
    return Token{TokType::SEMICOLON, ";"};
  case '=':
    return Token{TokType::EQUAL, "="};
  case '!':
    if (i < input.size() && input[i] == '=') {
      ++i;
      return Token{TokType::NOTEQUAL, "!="};
    }
    break;
  case '<':
    if (i < input.size() && input[i] == '=') {
      ++i;
      return Token{TokType::LE, "<="};
    }
    return Token{TokType::LT, "<"};
  case '>':
    if (i < input.size() && input[i] == '=') {
      ++i;
      return Token{TokType::GE, ">="};
    }
    return Token{TokType::GT, ">"};
  default:
    break;
  }
  // unknown symbol
  return Token{TokType::SYMBOL, std::string(1, c)};
}

bool Tokenizer::eof() const {
  Token t = peek();
  return t.type == TokType::END;
}

} // namespace db
