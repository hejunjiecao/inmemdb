#pragma once
#include "errors.hpp"
#include "output.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace db {

enum class Type { INT, STR };

struct Column {
  std::string name;
  Type type;
};

struct Value {
  Type type;
  long long i{};
  std::string s;

  static Value make_int(long long v) {
    Value x;
    x.type = Type::INT;
    x.i = v;
    return x;
  }
  static Value make_str(std::string v) {
    Value x;
    x.type = Type::STR;
    x.s = std::move(v);
    return x;
  }
  static Value default_of(Type t) {
    return t == Type::INT ? make_int(0) : make_str("");
  }

  std::string to_string() const {
    if (type == Type::INT)
      return std::to_string(i);
    return s;
  }

  // compare: return -1,0,1
  int compare(const Value &other) const {
    if (type != other.type)
      throw TypeError("Type mismatch in comparison");
    if (type == Type::INT) {
      if (i < other.i)
        return -1;
      if (i > other.i)
        return 1;
      return 0;
    } else {
      if (s < other.s)
        return -1;
      if (s > other.s)
        return 1;
      return 0;
    }
  }
};

struct Row {
  std::vector<Value> cells;
};

class Table {
public:
  Table() = default;
  Table(std::string name, std::vector<Column> cols);

  const std::string &get_name() const { return name; }
  const std::vector<Column> &get_columns() const { return columns; }

  size_t col_index(const std::string &col) const;
  const Column &col_at(size_t idx) const { return columns.at(idx); }

  void insert_row(const std::vector<std::optional<Value>> &row_values);
  size_t delete_where(const std::optional<struct Condition> &cond);
  size_t update_where(const std::vector<std::pair<std::string, Value>> &sets,
                      const std::optional<struct Condition> &cond);
  QueryResult select_where(const std::vector<std::string> &out_cols, bool star,
                           const std::optional<struct Condition> &cond) const;

private:
  std::string name;
  std::vector<Column> columns;
  std::unordered_map<std::string, size_t> name2idx;
  std::vector<Row> rows;

  bool row_matches(const Row &r,
                   const std::optional<struct Condition> &cond) const;
  std::vector<size_t> build_projection(const std::vector<std::string> &out_cols,
                                       bool star) const;
};

class Database {
public:
  void create_table(const std::string &name, const std::vector<Column> &cols);
  Table &table(const std::string &name);
  const Table &table(const std::string &name) const;

private:
  std::unordered_map<std::string, Table> tables;
};

// WHERE condition: simple binary comparison
struct Condition {
  enum class Op { EQ, NEQ, LT, GT, LE, GE };
  std::string column;
  Op op;
  Value literal;
  bool matches(const Table &t, const Row &r) const;
};

// --- Statements (parsed) ---
struct StmtCreate {
  std::string name;
  std::vector<Column> columns;
};
struct StmtInsert {
  std::string table;
  std::vector<std::string> columns;
  std::vector<std::vector<Value>> values;
};
struct StmtDelete {
  std::string table;
  std::optional<Condition> where;
};
struct StmtUpdate {
  std::string table;
  std::vector<std::pair<std::string, Value>> sets;
  std::optional<Condition> where;
};
struct StmtSelect {
  std::string table;
  std::vector<std::string> columns;
  bool star{false};
  std::optional<Condition> where;
};

using Statement =
    std::variant<StmtCreate, StmtInsert, StmtDelete, StmtUpdate, StmtSelect>;

// --- Execution helpers ---
std::optional<QueryResult> execute(Database &db, const Statement &stmt);

} // namespace db
