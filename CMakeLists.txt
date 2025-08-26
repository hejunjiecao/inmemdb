cmake_minimum_required(VERSION 3.16)
project(inmemdb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Werror)

# Add Catch2 testing framework (header-only)
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.9
)
FetchContent_MakeAvailable(Catch2)

add_library(inmemdb_core
    src/database.cpp
    src/parser.cpp
    src/tokenizer.cpp
    src/output.cpp
)

target_include_directories(inmemdb_core PUBLIC include)

add_executable(inmemdb src/main.cpp)
target_link_libraries(inmemdb PRIVATE inmemdb_core)

# Add test executable with Catch2
add_executable(inmemdb_tests 
    tests/main.cpp
    tests/tokenizer_tests.cpp
    tests/parser_tests.cpp
    tests/database_tests.cpp
    tests/output_tests.cpp
    tests/integration_tests.cpp
)
target_link_libraries(inmemdb_tests PRIVATE inmemdb_core Catch2::Catch2)

# Enable testing
enable_testing()
add_test(NAME inmemdb_tests COMMAND inmemdb_tests)