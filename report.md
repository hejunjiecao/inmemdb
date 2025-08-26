## Library Core Design

The In-Memory Database is built around a small library core that supports SQL parsing, execution, and data storage.

The design follows a **modular architecture** with four main parts: tokenizer, parser, database engine, and output formatter.

- **Tokenizer**: Reads the SQL input character by character and converts it into tokens (keywords, numbers, strings). This separation makes parsing easier.

- **Parser**: Uses recursive descent to build strongly typed statement objects (CREATE, INSERT, SELECT, UPDATE, DELETE). Each statement has its own structure, which improves readability and error handling.

- **Database Engine**: Stores data in tables with schemas. The design focuses on simplicity and correctness, rather than high performance.

- **Output Formatter**: Can print results as CSV or ASCII tables. The design makes it easy to add new formats in the future.

## Design Choices

We chose this layered design to follow the **Single Responsibility Principle**. Each module is independent and easy to test.

Using a static library simplifies integration and avoids runtime dependencies.

Error handling is done with C++ exceptions, which provide clear feedback when invalid SQL is executed.


## C++ Features

- **C++17**: Structured bindings, `std::optional` for optional values.

- **RAII**: Standard containers (`std::vector`, `std::string`) handle memory safely.

- **Strong typing**: Enums and structs for statement objects reduce runtime errors.

- **Exception safety**: RAII ensures consistent state even when errors occur.  