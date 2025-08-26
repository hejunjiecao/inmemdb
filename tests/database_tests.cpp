#include "database.hpp"
#include <catch2/catch.hpp>

using namespace db;

TEST_CASE("Database table operations", "[database]") {
  SECTION("Create and access tables") {
    Database db;
    db.create_table("people", {{"name", Type::STR}, {"age", Type::INT}});

    auto &table = db.table("people");
    REQUIRE(table.get_name() == "people");
    REQUIRE(table.get_columns().size() == 2);
    REQUIRE(table.get_columns()[0].name == "name");
    REQUIRE(table.get_columns()[0].type == Type::STR);
  }

  SECTION("Duplicate table creation") {
    Database db;
    db.create_table("test", {{"id", Type::INT}});
    REQUIRE_THROWS_AS(db.create_table("test", {{"id", Type::INT}}), DBError);
  }

  SECTION("Access non-existent table") {
    Database db;
    REQUIRE_THROWS_AS(db.table("nonexistent"), DBError);
  }
}

TEST_CASE("Table row operations", "[database]") {
  Database db;
  db.create_table("people", {{"name", Type::STR}, {"age", Type::INT}});

  auto &table = db.table("people");

  SECTION("Insert and select rows") {
    table.insert_row({Value::make_str("alice"), Value::make_int(30)});

    auto result = table.select_where({}, true, std::nullopt);
    REQUIRE(result.rows.size() == 1);
    REQUIRE(result.rows[0][0] == "alice");
    REQUIRE(result.rows[0][1] == "30");
  }

  SECTION("Insert with default values") {
    // Clear the table first
    auto result1 = table.select_where({}, true, std::nullopt);
    size_t initial_count = result1.rows.size();

    table.insert_row({Value::make_str("bob"), std::nullopt});

    auto result2 = table.select_where({}, true, std::nullopt);
    REQUIRE(result2.rows.size() == initial_count + 1);
    REQUIRE(result2.rows[initial_count][1] == "0"); // default int value
  }

  SECTION("Type mismatch error") {
    REQUIRE_THROWS_AS(
        table.insert_row({Value::make_int(123), // wrong type for name column
                          Value::make_int(30)}),
        TypeError);
  }
}

TEST_CASE("Table selection operations", "[database]") {
  Database db;
  db.create_table(
      "people", {{"name", Type::STR}, {"age", Type::INT}, {"city", Type::STR}});

  auto &table = db.table("people");

  // Insert test data
  table.insert_row(
      {Value::make_str("alice"), Value::make_int(30), Value::make_str("NYC")});
  table.insert_row(
      {Value::make_str("bob"), Value::make_int(25), Value::make_str("LA")});
  table.insert_row({Value::make_str("carol"), Value::make_int(35),
                    Value::make_str("Chicago")});

  SECTION("Select all columns") {
    auto result = table.select_where({}, true, std::nullopt);
    REQUIRE(result.headers.size() == 3);
    REQUIRE(result.rows.size() == 3);
  }

  SECTION("Select specific columns") {
    auto result = table.select_where({"name", "age"}, false, std::nullopt);
    REQUIRE(result.headers.size() == 2);
    REQUIRE(result.rows.size() == 3);
  }

  SECTION("WHERE conditions") {
    Condition cond{"age", Condition::Op::GT, Value::make_int(30)};
    auto result = table.select_where({}, true, cond);
    REQUIRE(result.rows.size() == 1);
    REQUIRE(result.rows[0][0] == "carol");
  }
}

TEST_CASE("Table update and delete operations", "[database]") {
  Database db;
  db.create_table("people", {{"name", Type::STR}, {"age", Type::INT}});

  auto &table = db.table("people");

  // Insert test data
  table.insert_row({Value::make_str("alice"), Value::make_int(30)});
  table.insert_row({Value::make_str("bob"), Value::make_int(25)});

  SECTION("Update operations") {
    Condition cond{"name", Condition::Op::EQ, Value::make_str("alice")};
    size_t updated = table.update_where({{"age", Value::make_int(31)}}, cond);
    REQUIRE(updated == 1);

    auto result = table.select_where({}, true, cond);
    REQUIRE(result.rows[0][1] == "31");
  }

  SECTION("Delete operations") {
    Condition cond{"age", Condition::Op::LT, Value::make_int(30)};
    size_t deleted = table.delete_where(cond);
    REQUIRE(deleted == 1);

    auto result = table.select_where({}, true, std::nullopt);
    REQUIRE(result.rows.size() == 1);
  }
}

TEST_CASE("Value operations", "[database]") {
  SECTION("Value creation and comparison") {
    auto int_val = Value::make_int(42);
    auto str_val = Value::make_str("hello");

    REQUIRE(int_val.type == Type::INT);
    REQUIRE(int_val.i == 42);
    REQUIRE(str_val.type == Type::STR);
    REQUIRE(str_val.s == "hello");

    auto int_val2 = Value::make_int(10);
    REQUIRE(int_val.compare(int_val2) == 1);  // 42 > 10
    REQUIRE(int_val2.compare(int_val) == -1); // 10 < 42
  }

  SECTION("Type mismatch in comparison") {
    auto int_val = Value::make_int(42);
    auto str_val = Value::make_str("hello");
    REQUIRE_THROWS_AS(int_val.compare(str_val), TypeError);
  }
}
