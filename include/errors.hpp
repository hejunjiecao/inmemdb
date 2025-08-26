#pragma once
#include <stdexcept>
#include <string>

struct ParseError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct DBError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct TypeError : public DBError {
  using DBError::DBError;
};
