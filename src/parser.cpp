#include "parser.hpp"
#include "tokenizer.hpp"
#include <cctype>
#include <sstream>

namespace db {

static Type parse_type(const Token &t) {
  if (t.type != TokType::IDENT)
    throw ParseError("Expected type name");
  if (t.text == "int")
    return Type::INT;
  if (t.text == "str")
    return Type::STR;
  throw ParseError("Unknown type: " + t.text +
                   " (types must be 'int' or 'str')");
}

static Value parse_literal(Token t) {
  if (t.type == TokType::NUMBER) {
    long long v = 0;
    try {
      size_t pos = 0;
      v = std::stoll(t.text, &pos, 10);
      if (pos != t.text.size())
        throw ParseError("Invalid integer literal: " + t.text);
    } catch (...) {
      throw ParseError("Invalid integer literal: " + t.text);
    }
    return Value::make_int(v);
  } else if (t.type == TokType::STRING) {
    return Value::make_str(t.text);
  }
  throw ParseError("Expected literal (number or \"string\")");
}

static Condition::Op parse_op(Token t) {
  switch (t.type) {
  case TokType::EQUAL:
    return Condition::Op::EQ;
  case TokType::NOTEQUAL:
    return Condition::Op::NEQ;
  case TokType::LT:
    return Condition::Op::LT;
  case TokType::GT:
    return Condition::Op::GT;
  case TokType::LE:
    return Condition::Op::LE;
  case TokType::GE:
    return Condition::Op::GE;
  default:
    break;
  }
  throw ParseError("Expected comparison operator (=, !=, <, >, <=, >=)");
}

static std::optional<Condition> parse_where(Tokenizer &tz) {
  Token t = tz.peek();
  if (t.type == TokType::IDENT && t.text == "WHERE") {
    tz.next(); // WHERE
    Token col = tz.next();
    if (col.type != TokType::IDENT)
      throw ParseError("Expected column name after WHERE");
    Token op = tz.next();
    auto cop = parse_op(op);
    Token lit = tz.next();
    Value v = parse_literal(lit);
    return Condition{col.text, cop, v};
  }
  return std::nullopt;
}

static void expect_ident(Tokenizer &tz, const char *kw) {
  Token t = tz.next();
  if (t.type != TokType::IDENT || t.text != kw) {
    throw ParseError(std::string("Expected '") + kw + "'");
  }
}

static std::string expect_ident_any(Tokenizer &tz) {
  Token t = tz.next();
  if (t.type != TokType::IDENT)
    throw ParseError("Expected identifier");
  return t.text;
}

static void expect(Token t, TokType tt, const char *what) {
  if (t.type != tt)
    throw ParseError(std::string("Expected ") + what);
}

Statement parse_statement(const std::string &stmt) {
  Tokenizer tz(stmt);
  Token t = tz.next();
  if (t.type != TokType::IDENT)
    throw ParseError("Expected statement keyword");
  if (t.text == "CREATE") {
    expect_ident(tz, "TABLE");
    std::string tbl = expect_ident_any(tz);
    expect(tz.next(), TokType::LPAREN, "'('");
    std::vector<Column> cols;
    bool first = true;
    while (true) {
      Token nt = tz.peek();
      if (nt.type == TokType::RPAREN) {
        tz.next();
        break;
      }
      if (!first) {
        expect(tz.next(), TokType::COMMA, "','");
      }
      first = false;
      std::string colname = expect_ident_any(tz);
      Type ty = parse_type(tz.next());
      cols.push_back({colname, ty});
    }
    if (!tz.eof())
      throw ParseError("Unexpected tokens after CREATE TABLE");
    return StmtCreate{tbl, cols};
  } else if (t.text == "INSERT") {
    expect_ident(tz, "INTO");
    std::string tbl = expect_ident_any(tz);
    expect(tz.next(), TokType::LPAREN, "'('");
    std::vector<std::string> cols;
    bool first = true;
    while (true) {
      Token nt = tz.peek();
      if (nt.type == TokType::RPAREN) {
        tz.next();
        break;
      }
      if (!first) {
        expect(tz.next(), TokType::COMMA, "','");
      }
      first = false;
      cols.push_back(expect_ident_any(tz));
    }
    expect_ident(tz, "VALUES");
    std::vector<std::vector<Value>> values;
    bool first_tuple = true;
    while (true) {
      if (!first_tuple) {
        Token maybe_comma = tz.peek();
        if (maybe_comma.type == TokType::COMMA)
          tz.next();
        else
          break;
      }
      first_tuple = false;
      expect(tz.next(), TokType::LPAREN, "'('");
      std::vector<Value> tup;
      bool firstv = true;
      while (true) {
        Token nt = tz.peek();
        if (nt.type == TokType::RPAREN) {
          tz.next();
          break;
        }
        if (!firstv) {
          expect(tz.next(), TokType::COMMA, "','");
        }
        firstv = false;
        tup.push_back(parse_literal(tz.next()));
      }
      values.push_back(std::move(tup));
      Token next = tz.peek();
      if (next.type != TokType::COMMA)
        break;
    }
    if (!tz.eof())
      throw ParseError("Unexpected tokens after INSERT");
    return StmtInsert{tbl, cols, values};
  } else if (t.text == "DELETE") {
    expect_ident(tz, "FROM");
    std::string tbl = expect_ident_any(tz);
    auto where = parse_where(tz);
    if (!tz.eof())
      throw ParseError("Unexpected tokens after DELETE");
    return StmtDelete{tbl, where};
  } else if (t.text == "UPDATE") {
    std::string tbl = expect_ident_any(tz);
    expect_ident(tz, "SET");
    std::vector<std::pair<std::string, Value>> sets;
    bool first = true;
    while (true) {
      if (!first) {
        Token c = tz.peek();
        if (c.type == TokType::COMMA)
          tz.next();
        else
          break;
      }
      first = false;
      std::string col = expect_ident_any(tz);
      expect(tz.next(), TokType::EQUAL, "'='");
      Value v = parse_literal(tz.next());
      sets.emplace_back(col, v);
      Token nxt = tz.peek();
      if (nxt.type != TokType::COMMA)
        break;
    }
    auto where = parse_where(tz);
    if (!tz.eof())
      throw ParseError("Unexpected tokens after UPDATE");
    return StmtUpdate{tbl, sets, where};
  } else if (t.text == "SELECT") {
    std::vector<std::string> cols;
    bool star = false;
    Token a = tz.next();
    if (a.type == TokType::STAR) {
      star = true;
    } else if (a.type == TokType::IDENT) {
      cols.push_back(a.text);
      while (true) {
        Token c = tz.peek();
        if (c.type == TokType::COMMA) {
          tz.next();
          cols.push_back(expect_ident_any(tz));
        } else
          break;
      }
    } else {
      throw ParseError("Expected '*' or column list after SELECT");
    }
    expect_ident(tz, "FROM");
    std::string tbl = expect_ident_any(tz);
    auto where = parse_where(tz);
    if (!tz.eof())
      throw ParseError("Unexpected tokens after SELECT");
    return StmtSelect{tbl, cols, star, where};
  } else {
    throw ParseError("Unknown statement type: " + t.text +
                     " (keywords must be uppercase)");
  }
}

std::vector<std::string> split_statements(const std::string &input) {
  std::vector<std::string> out;
  std::string cur;
  bool in_string = false;
  for (size_t i = 0; i < input.size(); ++i) {
    char c = input[i];
    if (c == '"') {
      in_string = !in_string;
      cur.push_back(c);
    } else if (c == ';' && !in_string) {
      // end of stmt
      // trim whitespace
      size_t start = 0;
      while (start < cur.size() &&
             std::isspace(static_cast<unsigned char>(cur[start])))
        ++start;
      size_t end = cur.size();
      while (end > start &&
             std::isspace(static_cast<unsigned char>(cur[end - 1])))
        --end;
      if (end > start)
        out.emplace_back(cur.substr(start, end - start));
      cur.clear();
    } else {
      cur.push_back(c);
    }
  }
  // ignore trailing partial stmt without semicolon
  return out;
}

} // namespace db
