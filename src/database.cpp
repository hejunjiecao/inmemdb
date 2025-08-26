#include "database.hpp"
#include <algorithm>
#include <iomanip>

namespace db {

Table::Table(std::string n, std::vector<Column> cols)
    : name(std::move(n)), columns(std::move(cols)) {
  for (size_t idx = 0; idx < columns.size(); ++idx) {
    name2idx.emplace(columns[idx].name, idx);
  }
}

size_t Table::col_index(const std::string &col) const {
  auto it = name2idx.find(col);
  if (it == name2idx.end())
    throw DBError("Unknown column: " + col);
  return it->second;
}

void Table::insert_row(const std::vector<std::optional<Value>> &row_values) {
  if (row_values.size() != columns.size())
    throw DBError("Internal error: wrong row size");
  Row r;
  r.cells.reserve(columns.size());
  for (size_t i = 0; i < columns.size(); ++i) {
    if (row_values[i].has_value()) {
      const Value &v = *row_values[i];
      if (v.type != columns[i].type)
        throw TypeError("Type mismatch on insert into column " +
                        columns[i].name);
      r.cells.push_back(v);
    } else {
      r.cells.push_back(Value::default_of(columns[i].type));
    }
  }
  rows.push_back(std::move(r));
}

bool Table::row_matches(const Row &r,
                        const std::optional<Condition> &cond) const {
  if (!cond)
    return true;
  return cond->matches(*this, r);
}

std::vector<size_t>
Table::build_projection(const std::vector<std::string> &out_cols,
                        bool star) const {
  std::vector<size_t> proj;
  if (star) {
    proj.resize(columns.size());
    for (size_t i = 0; i < columns.size(); ++i)
      proj[i] = i;
  } else {
    proj.reserve(out_cols.size());
    for (const auto &c : out_cols)
      proj.push_back(col_index(c));
  }
  return proj;
}

QueryResult Table::select_where(const std::vector<std::string> &out_cols,
                                bool star,
                                const std::optional<Condition> &cond) const {
  auto proj = build_projection(out_cols, star);
  QueryResult qr;
  qr.headers.reserve(proj.size());
  for (size_t idx : proj)
    qr.headers.push_back(columns[idx].name);
  for (const auto &row : rows) {
    if (!row_matches(row, cond))
      continue;
    std::vector<std::string> out;
    out.reserve(proj.size());
    for (size_t idx : proj)
      out.push_back(row.cells[idx].to_string());
    qr.rows.push_back(std::move(out));
  }
  return qr;
}

size_t Table::delete_where(const std::optional<Condition> &cond) {
  size_t before = rows.size();
  rows.erase(std::remove_if(rows.begin(), rows.end(),
                            [&](const Row &r) { return row_matches(r, cond); }),
             rows.end());
  return before - rows.size();
}

size_t
Table::update_where(const std::vector<std::pair<std::string, Value>> &sets,
                    const std::optional<Condition> &cond) {
  std::vector<size_t> idxs;
  idxs.reserve(sets.size());
  for (const auto &p : sets) {
    idxs.push_back(col_index(p.first));
  }
  size_t count = 0;
  for (auto &r : rows) {
    if (!row_matches(r, cond))
      continue;
    for (size_t k = 0; k < sets.size(); ++k) {
      const auto &v = sets[k].second;
      size_t idx = idxs[k];
      if (v.type != columns[idx].type)
        throw TypeError("Type mismatch in UPDATE for column " +
                        columns[idx].name);
      r.cells[idx] = v;
    }
    ++count;
  }
  return count;
}

void Database::create_table(const std::string &n,
                            const std::vector<Column> &cols) {
  if (tables.count(n))
    throw DBError("Table already exists: " + n);
  tables.emplace(n, Table{n, cols});
}

Table &Database::table(const std::string &n) {
  auto it = tables.find(n);
  if (it == tables.end())
    throw DBError("Unknown table: " + n);
  return it->second;
}

const Table &Database::table(const std::string &n) const {
  auto it = tables.find(n);
  if (it == tables.end())
    throw DBError("Unknown table: " + n);
  return it->second;
}

bool Condition::matches(const Table &t, const Row &r) const {
  size_t idx = t.col_index(column);
  const Value &v = r.cells[idx];
  int cmp = v.compare(literal);
  switch (op) {
  case Op::EQ:
    return cmp == 0;
  case Op::NEQ:
    return cmp != 0;
  case Op::LT:
    return cmp < 0;
  case Op::GT:
    return cmp > 0;
  case Op::LE:
    return cmp <= 0;
  case Op::GE:
    return cmp >= 0;
  }
  return false;
}

std::optional<QueryResult> execute(Database &db, const Statement &stmt) {
  if (std::holds_alternative<StmtCreate>(stmt)) {
    const auto &s = std::get<StmtCreate>(stmt);
    db.create_table(s.name, s.columns);
    return std::nullopt;
  } else if (std::holds_alternative<StmtInsert>(stmt)) {
    const auto &s = std::get<StmtInsert>(stmt);
    auto &t = db.table(s.table);
    // build mapping from table columns to provided values (or default)
    std::vector<size_t> idxs;
    idxs.reserve(s.columns.size());
    for (const auto &c : s.columns)
      idxs.push_back(t.col_index(c));
    for (const auto &tup : s.values) {
      if (tup.size() != s.columns.size())
        throw DBError("INSERT values tuple length mismatch");
      std::vector<std::optional<Value>> row_vals(t.get_columns().size());
      // set provided columns
      for (size_t k = 0; k < tup.size(); ++k) {
        size_t idx = idxs[k];
        row_vals[idx] = tup[k];
      }
      // remainings default in insert_row
      t.insert_row(row_vals);
    }
    return std::nullopt;
  } else if (std::holds_alternative<StmtDelete>(stmt)) {
    const auto &s = std::get<StmtDelete>(stmt);
    auto &t = db.table(s.table);
    t.delete_where(s.where);
    return std::nullopt;
  } else if (std::holds_alternative<StmtUpdate>(stmt)) {
    const auto &s = std::get<StmtUpdate>(stmt);
    auto &t = db.table(s.table);
    t.update_where(s.sets, s.where);
    return std::nullopt;
  } else {
    const auto &s = std::get<StmtSelect>(stmt);
    const auto &t = db.table(s.table);
    return t.select_where(s.columns, s.star, s.where);
  }
}

} // namespace db
